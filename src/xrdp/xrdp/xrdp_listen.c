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
   Copyright (C) Jay Sorg 2004-2010

   listen for incoming connection

*/

#include "xrdp.h"

/* 'g_process' is protected by the semaphore 'g_process_sem'.  One thread sets
   g_process and waits for the other to process it */
static tbus g_process_sem = 0;
static struct xrdp_process* g_process = 0;
static int use_multi_process = 0;

/*****************************************************************************/
void DEFAULT_CC
xrdp_stop_process(int sig)
{
	g_process->cont = 0;
}

/*****************************************************************************/
struct xrdp_listen* APP_CC
xrdp_listen_create(void)
{
  struct xrdp_listen* self;
  int pid;
  char text[256];

  pid = g_getpid();
  self = (struct xrdp_listen*)g_malloc(sizeof(struct xrdp_listen), 1);
  g_snprintf(text, 255, "xrdp_%8.8x_listen_pro_done_event", pid);
  self->pro_done_event = g_create_wait_obj(text);
  self->process_list = list_create();
  if (g_process_sem == 0)
  {
    g_process_sem = tc_sem_create(0);
  }
  self->listen_trans = trans_create(TRANS_MODE_TCP, 16, 16);
  if (self->listen_trans == 0)
  {
    g_writeln("xrdp_listen_main_loop: trans_create failed");
  }
  return self;
}

/*****************************************************************************/
void APP_CC
xrdp_listen_delete(struct xrdp_listen* self)
{
  if (self->listen_trans != 0)
  {
    trans_delete(self->listen_trans);
  }
  if (g_process_sem != 0)
  {
    tc_sem_delete(g_process_sem);
    g_process_sem = 0;
  }
  g_delete_wait_obj(self->pro_done_event);
  list_delete(self->process_list);
  g_free(self);
}

/*****************************************************************************/
/* returns error */
static int APP_CC
xrdp_listen_add_pro(struct xrdp_listen* self, struct xrdp_process* process)
{
  list_add_item(self->process_list, (tbus)process);
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_listen_delete_done_pro(struct xrdp_listen* self)
{
  int i;
  struct xrdp_process* pro;

  for (i = self->process_list->count - 1; i >= 0; i--)
  {
    pro = (struct xrdp_process*)list_get_item(self->process_list, i);
    if (pro != 0)
    {
      if (pro->status < 0)
      {
        xrdp_process_delete(pro);
        list_remove_item(self->process_list, i);
      }
    }
  }
  return 0;
}

/*****************************************************************************/
/* i can't get stupid in_val to work, hum using global var for now */
THREAD_RV THREAD_CC
xrdp_process_run(void* in_val)
{
  struct xrdp_process* process;

  DEBUG(("process started"));
  process = g_process;
  g_process = 0;
  tc_sem_inc(g_process_sem);
  xrdp_process_main_loop(process);
  DEBUG(("process done"));
  return 0;
}

int
xrdp_fork_process(struct xrdp_process* process)
{
  DEBUG(("process started"));
  g_process = process;
  g_signal_user_interrupt(xrdp_stop_process); /* SIGINT */
  g_signal_kill(xrdp_stop_process); /* SIGKILL */
  g_signal_terminate(xrdp_stop_process); /* SIGTERM */

  xrdp_process_main_loop(process);
  DEBUG(("process done"));
  return 0;
}

/*****************************************************************************/
static int
xrdp_listen_get_port(char* port, int port_bytes)
{
  int fd;
  int error;
  int index;
  char* val;
  struct list* names;
  struct list* values;
  char cfg_file[256];

  /* default to port 3389 */
  g_strncpy(port, "3389", port_bytes - 1);
  /* see if port is in xrdp.ini file */
  g_snprintf(cfg_file, 255, "%s/xrdp.ini", XRDP_CFG_PATH);
  fd = g_file_open(cfg_file);
  if (fd > 0)
  {
    names = list_create();
    names->auto_free = 1;
    values = list_create();
    values->auto_free = 1;
    if (file_read_section(fd, "globals", names, values) == 0)
    {
      for (index = 0; index < names->count; index++)
      {
        val = (char*)list_get_item(names, index);
        if (val != 0)
        {
          if (g_strcasecmp(val, "port") == 0)
          {
            val = (char*)list_get_item(values, index);
            error = g_atoi(val);
            if ((error > 0) && (error < 65000))
            {
              g_strncpy(port, val, port_bytes - 1);
            }
            break;
          }
        }
      }
    }
    list_delete(names);
    list_delete(values);
    g_file_close(fd);
  }
  return 0;
}

/*****************************************************************************/
static int
xrdp_listen_update_process_mode()
{
  int index;
  struct list* items;
  struct list* values;
  char* item;
  char* value;
  char cfg_file[256];

  items = list_create();
  items->auto_free = 1;
  values = list_create();
  values->auto_free = 1;
  g_snprintf(cfg_file, 255, "%s/xrdp.ini", XRDP_CFG_PATH);
  file_by_name_read_section(cfg_file, "globals", items, values);
  for (index = 0; index < items->count; index++)
  {
    item = (char*)list_get_item(items, index);
    value = (char*)list_get_item(values, index);
    if (g_strcasecmp(item, "use_multi_process") == 0)
    {
      if ((g_strcasecmp(value, "yes") == 0) ||
          (g_strcasecmp(value, "true") == 0) ||
          (g_strcasecmp(value, "1") == 0))
      {
        use_multi_process = 1;
      }
    }
  }
  list_delete(items);
  list_delete(values);
  return 0;
}

/*****************************************************************************/
/* a new connection is coming in */
int DEFAULT_CC
xrdp_listen_conn_in(struct trans* self, struct trans* new_self)
{
  struct xrdp_process* process;
  struct xrdp_listen* lis;

  g_tcp_set_no_delay(new_self->sck);

  if (use_multi_process == 1) {
    int pid = 0;

    pid = g_fork();
    if (pid < 0) {
      DEBUG(("Error while forking"));
      g_tcp_close(new_self->sck);
      return -1;
    }
    if (pid == 0) {
      g_file_close(self->sck);
      lis = (struct xrdp_listen*)(self->callback_data);
      process = xrdp_process_create(lis, lis->pro_done_event);

      if (xrdp_listen_add_pro(lis, process) == 0)
      {
        process->server_trans = new_self;
        xrdp_fork_process(process);
      }
      else
      {
        xrdp_process_delete(process);
      }
      g_exit(0);
    }
    else {
      g_file_close(new_self->sck);
      return 0;
    }
  }

  lis = (struct xrdp_listen*)(self->callback_data);
  process = xrdp_process_create(lis, lis->pro_done_event);
  if (xrdp_listen_add_pro(lis, process) == 0)
  {
    /* start thread */
    process->server_trans = new_self;
    g_process = process;
    tc_thread_create(xrdp_process_run, 0);
    tc_sem_dec(g_process_sem); /* this will wait */
  }
  else
  {
    xrdp_process_delete(process);
  }
  return 0;
}

/*****************************************************************************/
/* wait for incoming connections */
int APP_CC
xrdp_listen_main_loop(struct xrdp_listen* self)
{
  int error;
  int robjs_count;
  int cont;
  int timeout;
  char port[8];
  tbus robjs[8];
  tbus term_obj;
  tbus sync_obj;
  tbus sck_obj;
  tbus done_obj;

  self->status = 1;
  xrdp_listen_update_process_mode();
  if (xrdp_listen_get_port(port, sizeof(port)) != 0)
  {
    g_writeln("xrdp_listen_main_loop: xrdp_listen_get_port failed");
    self->status = -1;
    return 1;
  }
  error = trans_listen(self->listen_trans, port);
  if (error == 0)
  {
    self->listen_trans->trans_conn_in = xrdp_listen_conn_in;
    self->listen_trans->callback_data = self;
    term_obj = g_get_term_event();
    sync_obj = g_get_sync_event();
    done_obj = self->pro_done_event;
    cont = 1;
    while (cont)
    {
      /* build the wait obj list */
      robjs_count = 0;
      robjs[robjs_count++] = term_obj;
      robjs[robjs_count++] = sync_obj;
      robjs[robjs_count++] = done_obj;
      timeout = -1;
      if (trans_get_wait_objs(self->listen_trans, robjs, &robjs_count,
                              &timeout) != 0)
      {
        break;
      }
      /* wait */
      if (g_obj_wait(robjs, robjs_count, 0, 0, timeout) != 0)
      {
        /* error, should not get here */
        g_sleep(100);
      }
      if (g_is_wait_obj_set(term_obj)) /* term */
      {
        break;
      }
      if (g_is_wait_obj_set(sync_obj)) /* sync */
      {
        g_reset_wait_obj(sync_obj);
        g_loop();
      }
      if (g_is_wait_obj_set(done_obj)) /* pro_done_event */
      {
        g_reset_wait_obj(done_obj);
        xrdp_listen_delete_done_pro(self);
      }
      if (trans_check_wait_objs(self->listen_trans) != 0)
      {
        break;  
      }
    }
    /* stop listening */
    trans_delete(self->listen_trans);
    self->listen_trans = 0;
    /* second loop to wait for all process threads to close */
    cont = 1;
    while (cont)
    {
      if (self->process_list->count == 0)
      {
        break;
      }
      /* build the wait obj list */
      robjs_count = 0;
      robjs[robjs_count++] = sync_obj;
      robjs[robjs_count++] = done_obj;
      /* wait */
      if (g_obj_wait(robjs, robjs_count, 0, 0, -1) != 0)
      {
        /* error, should not get here */
        g_sleep(100);
      }
      if (g_is_wait_obj_set(sync_obj)) /* sync */
      {
        g_reset_wait_obj(sync_obj);
        g_loop();
      }
      if (g_is_wait_obj_set(done_obj)) /* pro_done_event */
      {
        g_reset_wait_obj(done_obj);
        xrdp_listen_delete_done_pro(self);
      }
    }
  }
  else
  {
    DEBUG(("listen error in xrdp_listen_main_loop"));
  }
  self->status = -1;
  return 0;
}
