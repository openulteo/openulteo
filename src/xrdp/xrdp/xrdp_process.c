/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   xrdp: A Remote Desktop Protocol server.
   Copyright (C) Jay Sorg 2004-2009
   Copyright (C) 2011-2014 Ulteo SAS
   http://www.ulteo.com
   Author David LECHEVALIER <david@ulteo.com> 2011, 2012, 2013, 2014
   Author James B. MacLean <macleajb@ednet.ns.ca> 2012

   main rdp process

*/

#include "xrdp.h"
#include "libxrdp.h"
#include "abstract/xrdp_module.h"


static int g_session_id = 0;

/*****************************************************************************/
/* always called from xrdp_listen thread */
struct xrdp_process* APP_CC
xrdp_process_create(struct xrdp_listen* owner, tbus done_event)
{
  struct xrdp_process* self;
  char event_name[256];
  int pid;

  self = (struct xrdp_process*)g_malloc(sizeof(struct xrdp_process), 1);
  self->lis_layer = owner;
  self->done_event = done_event;
  g_session_id++;
  self->session_id = g_session_id;
  pid = g_getpid();
  g_snprintf(event_name, 255, "xrdp_%8.8x_process_self_term_event_%8.8x",
             pid, self->session_id);
  self->self_term_event = g_create_wait_obj(event_name);
  return self;
}

/*****************************************************************************/
void APP_CC
xrdp_process_delete(struct xrdp_process* self)
{
  if (self == 0)
  {
    return;
  }
  g_delete_wait_obj(self->self_term_event);
  libxrdp_exit(self->session);
  xrdp_module_unload(self);
  trans_delete(self->server_trans);
  g_free(self);
}

/*****************************************************************************/
static int APP_CC
xrdp_process_loop(struct xrdp_process* self)
{
  int rv;

  rv = 0;
  if (self->session != 0)
  {
    rv = libxrdp_process_data(self->session);
  }
  if ((self->mod->wm == 0) && (self->session->up_and_running) && (rv == 0))
  {
    DEBUG(("calling xrdp_wm_init and creating wm"));
    self->mod->connect(self->mod, self->session_id, self->session);
    /* at this point the wm(window manager) is create and wm::login_mode is
       zero and login_mode_event is set so xrdp_wm_init should be called by
       xrdp_wm_check_wait_objs */
  }
  return rv;
}

/*****************************************************************************/
/* returns boolean */
/* this is so libxrdp.so can known when to quit looping */
static int DEFAULT_CC
xrdp_is_term(void)
{
  return g_is_term();
}

/*****************************************************************************/
static int APP_CC
xrdp_process_mod_end(struct xrdp_process* self)
{
  if (self->mod != 0)
  {
    self->mod->end(self->mod);
  }
  return 0;
}

/*****************************************************************************/
static int DEFAULT_CC
xrdp_process_data_in(struct trans* self)
{
  struct xrdp_process* pro;

  DEBUG(("xrdp_process_data_in"));
  pro = (struct xrdp_process*)(self->callback_data);
  if (xrdp_process_loop(pro) != 0)
  {
    return 1;
  }
  return 0;
}

/*****************************************************************************/
static bool DEFAULT_CC
xrdp_process_check_connectivity(struct xrdp_session* session)
{
  int connectivity_check_interval = session->client_info->connectivity_check_interval * 1000;
  int current_time = 0;


  if (!session || !session->client_info->connectivity_check)
  {
    return true;
  }

  current_time =  g_time2();
  if (current_time - session->trans->last_time >= connectivity_check_interval)
  {
    libxrdp_send_keepalive(session);
    session->trans->last_time = current_time;
    if (g_tcp_socket_ok(session->trans->sck) == 0)
    {
      printf("Connection end due to keepalive\n");
      return false;
    }
  }

  return true;
}


/*****************************************************************************/
static void DEFAULT_CC
xrdp_process_flush_main_channel(struct xrdp_rdp* rdp, struct list* data_to_send)
{
  struct spacket* s;
  int i = 0;

  for(i = 0 ; i < data_to_send->count ; i++)
  {
    s = (struct spacket*)list_get_item(data_to_send, i);
    if (s->packet_type == 1)
    {
      xrdp_rdp_send_data(rdp, s->data, s->update_type);
    }
    if (s->packet_type == 2)
    {
      xrdp_rdp_send_fast_path_update(rdp, s->data, s->update_type);
    }

    free_stream(s->data);
  }
  list_clear(data_to_send);

}


/*****************************************************************************/
static int DEFAULT_CC
xrdp_process_check_main_channel(struct xrdp_process* process)
{
  struct list* data_to_send;
  struct xrdp_session* session = process->session;
  struct xrdp_rdp* rdp;
  int received = 0;
  int spent_time;

  if (!session)
  {
    return 0;
  }

  rdp = session->rdp;
  data_to_send = process->mod->get_data(process->mod);
  if (data_to_send == NULL)
  {
    return 0;
  }

  if (data_to_send->count > 0)
  {
    struct spacket* s;
    int i = 0;

    for(i = 0 ; i < data_to_send->count ; i++)
    {
      s = (struct spacket*)list_get_item(data_to_send, i);
      received += s->data->size;
    }

    if (received > 5000)
    {
      libxrdp_emt_start_check(session);
      spent_time = g_time3();
    }

    xrdp_process_flush_main_channel(rdp, data_to_send);

    if (received > 5000)
    {
      spent_time = g_time3() - spent_time;
      libxrdp_emt_stop_check(session, spent_time);
      if (session->network_stat_updated)
      {
        process->mod->set_network_stat(process->mod, session->bandwidth, session->average_RTT);
        session->network_stat_updated = false;
      }
    }
  }

  return received;
}

/*****************************************************************************/
static int DEFAULT_CC
xrdp_process_check_channel(struct xrdp_process* self, bool use_qos, int bandwidth)
{
  struct stream* channel_data;
  char* chan_name;
  int index = 0;
  int chan_id;
  int chan_flags;
  int size;
  int available_data;
  int total_send = 0;
  bool can_send = true;
  struct list* channel_priority;
  bw_limit_list* channels_limitation;

  if (! self->vc || ! self->session)
  {
    return 0;
  }

  channel_priority = self->session->client_info->channel_priority;
  channels_limitation = self->session->client_info->channels_bw_limit;
  for(index = 0 ; index< channel_priority->count ; index++)
  {
    if (total_send > bandwidth)
    {
      return total_send;
    }

    chan_name = (char*)list_get_item(channel_priority, index);
    if (g_strcmp(chan_name, "main") == 0)
    {
      total_send += xrdp_process_check_main_channel(self);
      continue;
    }
    else
    {
      chan_id = libxrdp_get_channel_id(self->session, chan_name);
      if (chan_id == -1)
      {
        continue;
      }

      available_data = self->vc->has_data(self->vc, chan_id);
      if (use_qos && available_data > 0)
      {
        can_send = libxrdp_can_send_to_channel(channels_limitation, chan_name, bandwidth, available_data);
      }

      if(available_data > 0 && can_send)
      {
        make_stream(channel_data);
        init_stream(channel_data, available_data);

        self->vc->get_data(self->vc, chan_id, channel_data);

        xrdp_vchannel_send_data(self->vc, chan_id, channel_data->data, available_data);
        total_send += available_data;
        free_stream(channel_data);
      }
    }
  }

  return total_send;
}


/*****************************************************************************/
THREAD_RV THREAD_CC
xrdp_qos_loop(void* in_val)
{
  struct xrdp_process* process = (struct xrdp_process*)in_val;
  struct xrdp_qos* qos = (struct xrdp_qos*)process->qos;
  struct xrdp_session* session = process->session;
  struct xrdp_rdp* rdp;
  int timeout = 1000;
  int static_frame_rate = 0;
  int robjs_count;
  int wobjs_count;
  unsigned int spent_time;
  tbus robjs[32];
  tbus wobjs[32];
  tbus term_event;
  bool use_qos = false;
  long total_tosend;


  if (qos == NULL)
  {
    printf("Failed to initialize qos thread\n");
    return (void*)1;
  }

  if (process->vc == NULL)
  {
    printf("Failed to initialize vchannel\n");
    return (void*)1;
  }

  printf("=====> QOS Start\n");

  use_qos = session->client_info->use_qos;
  session->trans = process->server_trans;

  while(process->cont)
  {
    /* build the wait obj list */
    timeout = 1000;
    total_tosend = 0;
    robjs_count = 0;
    wobjs_count = 0;

    if (! xrdp_process_check_connectivity(session))
    {
      break;
    }

    if (g_is_wait_obj_set(process->self_term_event))
    {
      break;
    }

    process->mod->get_data_descriptor(process->mod, robjs, &robjs_count, wobjs, &wobjs_count, &timeout);
    process->vc->get_data_descriptor(process->vc, robjs, &robjs_count, wobjs, &wobjs_count, &timeout);

    // Waiting for new data
    if (g_obj_wait(robjs, robjs_count, wobjs, wobjs_count, timeout) != 0)
    {
      /* error, should not get here */
      g_sleep(100);
    }

    // Processing data
    spent_time = g_time3();
    total_tosend += xrdp_process_check_channel(process, use_qos, (session->bandwidth - total_tosend));
    spent_time = g_time3() - spent_time;

    if(use_qos && (session->bandwidth > 0))
    {
      int next_request_time = (total_tosend/session->bandwidth) - spent_time;
      if (next_request_time <= 0)
      {
        next_request_time = 10;
      }
      if (next_request_time > 1000)
      {
        next_request_time = 1000;
      }
      session->next_request_time = next_request_time;
      g_sleep(next_request_time);
    }
  }

  return 0;
}

int APP_CC
xrdp_qos_start(struct xrdp_process* process)
{
  process->qos = process->session->qos;
  process->qos->thread_handle = tc_thread_create(xrdp_qos_loop, process);
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_process_main_loop(struct xrdp_process* self)
{
  int robjs_count;
  int wobjs_count;
  int cont;
  int timeout;
  int frame_rate;
  tbus robjs[32];
  tbus wobjs[32];
  tbus term_obj;
  int current_time = 0;
  int last_update_time = 0;

  DEBUG(("xrdp_process_main_loop"));
  self->status = 1;
  self->server_trans->trans_data_in = xrdp_process_data_in;
  self->server_trans->callback_data = self;
  self->session = libxrdp_init(self->server_trans);
  if (!xrdp_module_load(self, self->session->client_info->user_channel_plugin))
  {
    printf("Failed to load module %s\n", self->session->client_info->user_channel_plugin);
    return 0;
  }

  self->vc = xrdp_vchannel_create();
  if (self->vc == NULL)
  {
    return 0;
  }

  self->session->id = (tbus)self->mod;
  self->session->callback = self->mod->callback;
  self->session->chan_id = (tbus)self->vc;
  self->session->channel_callback = (int (*)(long, long, long, long, long))xrdp_vchannel_process_channel_data;
  self->mod->self_term_event = self->self_term_event;

  /* this callback function is in xrdp_wm.c */
  /* this function is just above */
  self->session->is_term = xrdp_is_term;
  self->server_trans->last_time = g_time2();

  if (libxrdp_process_incomming(self->session) == 0)
  {
    term_obj = g_get_term_event();
    self->cont = 1;

    xrdp_qos_start(self);

    while (self->cont)
    {
      /* build the wait obj list */
      timeout = 1000;
      robjs_count = 0;
      wobjs_count = 0;
      robjs[robjs_count++] = term_obj;
      robjs[robjs_count++] = self->self_term_event;

      // Channel connection
      if (self->mod->get_login_mode(self->mod) > 10 && self->vc->session == 0)
      {
        self->vc->session = (tbus)self->session;
      	self->vc->username = self->session->client_info->username;
      	xrdp_vchannel_setup(self->vc);

        if (self->session->client_info->use_static_frame_rate)
        {
          self->mod->set_static_framerate(self->mod, self->session->client_info->frame_rate);
        }
      }

      trans_get_wait_objs(self->server_trans, robjs, &robjs_count, &timeout);
      /* wait */
      if (g_obj_wait(robjs, robjs_count, wobjs, wobjs_count, timeout) != 0)
      {
        /* error, should not get here */
        g_sleep(100);
      }
      if (g_is_wait_obj_set(term_obj)) /* term */
      {
        break;
      }
      if (g_is_wait_obj_set(self->self_term_event))
      {
        break;
      }
      if (trans_check_wait_objs(self->server_trans) != 0)
      {
        break;
      }
    }
  }

  xrdp_vchannel_delete(self->vc);

  if( self->mod != 0)
  {
    self->mod->disconnect(self->mod);
  }

  self->cont = false;
  libxrdp_disconnect(self->session);
  xrdp_process_mod_end(self);
  libxrdp_exit(self->session);
  self->session = 0;
  self->status = -1;
  g_set_wait_obj(self->done_event);
  return 0;
}
