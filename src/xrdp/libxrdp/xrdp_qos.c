/**
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2013
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#include "libxrdp.h"


struct xrdp_qos* DEFAULT_CC
xrdp_qos_create(struct xrdp_session* session)
{
  struct xrdp_qos* result = g_malloc(sizeof(struct xrdp_qos), true);

  if (!session)
  {
    printf("Failed to initialize qos object, [session: %x]\n", (unsigned int)session);
    return NULL;
  }

  if (result == NULL)
  {
    printf("Failed to initialize qos object, unable to allocate object\n");
    return NULL;
  }

  result->session = session;
  result->spooled_packet = list_create();
  result->spooled_packet->auto_free = true;

  return result;
}

bw_limit* DEFAULT_CC
xrdp_qos_get_channel(bw_limit_list* channel_bw_limit, char* chan_name)
{
  bw_limit* temp = NULL;
  struct list* chan_list = NULL;
  int i;

  if (channel_bw_limit == NULL)
  {
    return NULL;
  }

  chan_list = channel_bw_limit->chan_list;

  for(i = 0 ; i < chan_list->count ; i++)
  {
    temp = (bw_limit*)list_get_item(chan_list, i);
    if (g_strcmp(temp->channel_name, chan_name) == 0)
    {
      return temp;
    }
  }

  return NULL;
}

void DEFAULT_CC
xrdp_qos_add_limitation(bw_limit_list* channel_bw_limit, char* chan_name, char* value)
{
  bw_limit* chan = xrdp_qos_get_channel(channel_bw_limit, chan_name);
  if (chan)
  {
    printf("channel limitation already set for %s=>%s, updating it with %s\n", chan_name, chan->bw_limit, value);
    if (chan->bw_limit)
    {
      g_free(chan->bw_limit);
    }
    chan->bw_limit = g_strdup(value);
    return;
  }

  printf("channel limitation %s=>%s\n", chan_name, value);
  chan = g_malloc(sizeof(bw_limit), true);
  chan->channel_name = g_strdup(chan_name);
  chan->bw_limit = g_strdup(value);
  chan->already_sended = 0;

  list_add_item(channel_bw_limit->chan_list, (tbus)chan);
}

int DEFAULT_CC
xrdp_qos_get_limitation(bw_limit* c, int bw)
{
  int limit;
  int index;

  if (c == NULL)
  {
    return -1;
  }

  limit = g_atoi(c->bw_limit);
  if (g_str_end_with(c->bw_limit, "%") == 0)
  {
    if (limit < 1 && limit > 100)
    {
      printf("invalid bandwidth limitation %i\n", limit);
      return -1;
    }

    limit = (bw * limit / 100);
  }

  return limit;
}

void DEFAULT_CC
xrdp_qos_reset_channel_counter(bw_limit_list* channels_limitation)
{
  bw_limit* temp = NULL;
  struct list* chan_list = NULL;
  int i;

  if (channels_limitation == NULL)
  {
    return;
  }

  chan_list = channels_limitation->chan_list;
  channels_limitation->lastReset = g_time3();

  for(i = 0 ; i < chan_list->count ; i++)
  {
    temp = (bw_limit*)list_get_item(chan_list, i);
    temp->already_sended = 0;
  }
}


bool DEFAULT_CC
xrdp_qos_can_send_to_channel(bw_limit_list* channels_limitation, char* chan_name, unsigned int bandwidth, int data_len)
{
  unsigned int current_time = g_time3();
  bw_limit* c;
  if (channels_limitation == NULL)
  {
    return true;
  }

  if (data_len == 0)
  {
    return true;
  }

  c = xrdp_qos_get_channel(channels_limitation, chan_name);
  if (c == NULL)
  {
    return true;
  }

  int available = xrdp_qos_get_limitation(c, bandwidth);

  // Check reset limitation
  if (current_time - channels_limitation->lastReset > 1000)
  {
    xrdp_qos_reset_channel_counter(channels_limitation);
  }

  if (data_len == 0)
  {
    return true;
  }

  if ((c->already_sended + data_len) > available)
  {
    return false;
  }
  c->already_sended += data_len;
  return true;
}


bw_limit_list* DEFAULT_CC
xrdp_qos_create_bw_limit()
{
  bw_limit_list* res = g_malloc(sizeof(bw_limit_list), true);

  res->lastReset = g_time3();
  res->chan_list = list_create();
  res->chan_list->auto_free = false;

  return res;
}

void DEFAULT_CC
xrdp_qos_delete(struct xrdp_qos* qos)
{
  list_delete(qos->spooled_packet);
  g_free(qos);
}

static void APP_CC
xrdp_qos_stream_copy(struct spacket* packet, struct stream* src)
{
  int size = src->end - src->data;
  make_stream(packet->data);
  init_stream(packet->data, src->size);
  struct stream* dst = packet->data;

  g_memcpy(dst->data, src->data, (src->end-src->data));
  dst->size = size;

  dst->channel_hdr = dst->data + (src->channel_hdr - src->data);
  dst->end = dst->data + (src->end - src->data);
  dst->iso_hdr = dst->data + (src->iso_hdr - src->data);
  dst->mcs_hdr = dst->data + (src->mcs_hdr - src->data);
  dst->sec_hdr = dst->data + (src->sec_hdr - src->data);
  dst->next_packet = dst->data + (src->next_packet - src->data);
  dst->p = dst->data + (src->p - src->data);
  dst->rdp_hdr = dst->data + (src->rdp_hdr - src->data);
}


int APP_CC
xrdp_qos_spool(struct xrdp_qos* self, struct stream* s, int data_pdu_type, int packet_type)
{
  DEBUG(("    in xrdp_qos_spool, gota send %d bytes", (s->end - s->data)));
  struct spacket* p = g_malloc(sizeof(struct spacket), true);

  xrdp_qos_stream_copy(p, s);
  p->update_type = data_pdu_type;
  p->packet_type = packet_type;

  list_add_item(self->spooled_packet, (tbus)p);

  DEBUG(("    out xrdp_qos_spool, sent %d bytes ok", (s->end - s->data)));

  return 0;
}

