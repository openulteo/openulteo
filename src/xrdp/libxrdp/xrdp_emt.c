/**
 * Copyright (C) 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2012
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
#include <limits.h>

bool APP_CC
xrdp_emt_send_packet(struct xrdp_rdp* self, struct xrdp_emt* emt, struct stream* s)
{
  if (emt->chanid == 0)
  {
    return false;
  }

  if (xrdp_sec_send_cflags(self->sec_layer, s, emt->chanid, (SEC_ENCRYPT | SEC_AUTODETECT_REQ)) != 0)
  {
    printf("failed to send message\n");
    return false;
  }

  return true;
}

bool APP_CC
xrdp_emt_send_request(struct xrdp_rdp* self, struct xrdp_emt* emt, int type)
{
  struct stream* s;

  if (emt == NULL)
  {
    printf("emt is null\n");
    return false;
  }

  make_stream(s);
  init_stream(s, 40);
  xrdp_sec_init(self->sec_layer, s);

  out_uint8(s, SEC_AUTODETECT_REQ_LENGTH);               // headerLength
  out_uint8(s, TYPE_ID_AUTODETECT_REQUEST);              // headerTypeId
  out_uint16_le(s, emt->seq_number++);                   // sequenceNumber
  out_uint16_le(s, type);                                // responseType
  s_mark_end(s);

  xrdp_emt_send_packet(self, emt, s);
  free_stream(s);

  return true;
}

bool APP_CC
xrdp_emt_send_result(struct xrdp_rdp* self, struct xrdp_emt* emt)
{
  struct stream* s;

  if (emt == NULL)
  {
    printf("emt is null\n");
    return false;
  }

  make_stream(s);
  init_stream(s, 40);
  xrdp_sec_init(self->sec_layer, s);

  out_uint8(s, SEC_AUTODETECT_REQ_LENGTH);               // headerLength
  out_uint8(s, TYPE_ID_AUTODETECT_REQUEST);              // headerTypeId
  out_uint16_le(s, emt->seq_number++);                   // sequenceNumber
  out_uint16_le(s, field_all);                           // responseType

  out_uint32_le(s, self->session->base_RTT);
  out_uint32_le(s, self->session->bandwidth);
  out_uint32_le(s, self->session->average_RTT);

  s_mark_end(s);

  xrdp_emt_send_packet(self, emt, s);
  free_stream(s);

  return true;
}

void APP_CC
xrdp_emt_process_results(struct xrdp_rdp* self, struct xrdp_emt* emt, struct stream* s)
{
  int time_delta;
  int byte_count;
  int current_time = g_time3();
  int current_rtt = current_time - emt->stop_time;

  in_uint32_le(s, time_delta);
  in_uint32_le(s, byte_count);

  emt->total_delta += time_delta;
  emt->total_byte_count += byte_count;

  if (emt->total_byte_count > LONG_MAX)
  {
    emt->total_byte_count = emt->total_byte_count / emt->total_delta;
    emt->total_delta = 1;
  }

  if (byte_count > 1000)
  {
    if (emt->total_delta == 0)
    {
      self->session->bandwidth = byte_count / 0.9;
    }
    else
    {
      self->session->bandwidth = emt->total_byte_count/emt->total_delta; // Ko/s
    }
  }

  if (emt->time_processing < 2)
  {
    if (self->session->base_RTT > current_rtt)
    {
      self->session->base_RTT = current_rtt;
    }

    if (self->session->average_RTT == 0)
    {
      self->session->average_RTT = current_rtt;
    }
    else
    {
      self->session->average_RTT = (self->session->average_RTT + current_rtt) / 2;
    }
  }

  printf("bandwidth: %i Ko/s\n", self->session->bandwidth);
  printf("base RTT: %i ms\n", self->session->base_RTT);
  printf("average RTT: %i ms\n", self->session->average_RTT);

  self->session->network_stat_updated = true;
}

bool APP_CC
xrdp_emt_process(struct xrdp_rdp* self, struct stream* s)
{
  int sequenceNumber;
  int requestType;
  unsigned int current_time = g_time3();
  struct xrdp_emt* emt = self->sec_layer->chan_layer->emt_channel;

  in_uint8s(s, 1);                                 // headerLength
  in_uint8s(s, 1);                                 // headerTypeId
  in_uint16_le(s, sequenceNumber);                 // sequenceNumber
  in_uint16_le(s, requestType);                    // requestType

  switch (requestType)
  {
  case RDP_RTT_RESPONSE_TYPE:
    emt->state = ready;
    break;

  case RDP_BW_RESULTS:
    xrdp_emt_process_results(self, emt, s);
    break;

  default:
    printf("Unknow request type: 0x%x", requestType);
    return false;
  }

  return true;
}

bool APP_CC
xrdp_emt_send_init(struct xrdp_rdp* self)
{
  struct xrdp_emt* emt = self->sec_layer->chan_layer->emt_channel;
  if (emt == NULL || !emt->activated)
  {
    return false;
  }

  return xrdp_emt_send_request(self, emt, RDP_RTT_REQUEST_TYPE);
}

bool APP_CC
xrdp_emt_bw_check_start(struct xrdp_rdp* self)
{
  struct xrdp_emt* emt = self->sec_layer->chan_layer->emt_channel;
  int current_time = g_time3();
  if (emt == NULL || !emt->activated)
  {
    return false;
  }

  if (emt->state == wait_next_mesure)
  {
    if (current_time > emt->next_check)
    {
      emt->state = ready;
    }
    else
    {
      return true;
    }
  }

  if (emt->state != ready)
  {
    return true;
  }

  emt->state = check_bw;
  emt->time_processing = 0;

  return xrdp_emt_send_request(self, emt, RDP_BW_SESSION_START);
}

bool APP_CC
xrdp_emt_bw_check_stop(struct xrdp_rdp* self, int time_processing)
{
  struct xrdp_emt* emt = self->sec_layer->chan_layer->emt_channel;
  int current_time = g_time3();
  bool res;
  if (emt == NULL || !emt->activated)
  {
    return false;
  }

  if (emt->state != check_bw)
  {
    emt->time_processing+=time_processing;
    return true;
  }

  emt->time_processing = 0;

  emt->next_check = current_time + self->client_info.network_detection_interval;
  emt->state = wait_next_mesure;
  res = xrdp_emt_send_request(self, emt, RDP_BW_SESSION_STOP);
  emt->stop_time = g_time3();

  if (emt->need_result && self->session->bandwidth > 0)
  {
    xrdp_emt_send_result(self, emt);
    emt->need_result = false;
  }

  return res;
}
