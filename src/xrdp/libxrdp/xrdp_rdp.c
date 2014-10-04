/**
 * Copyright (C) 2011-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2011, 2012, 2013
 * Author Vincent Roullier <vincent.roullier@ulteo.com> 2012, 2013
 * Author Thomas MOUTON <thomas@ulteo.com> 2012
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2012
 * Author James B. MacLean <macleajb@ednet.ns.ca> 2012
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

   rdp layer

*/

#include "libxrdp.h"
#include "mppc_enc.h"

/* some compilers need unsigned char to avoid warnings */
static tui8 g_unknown1[172] =
{ 0xff, 0x02, 0xb6, 0x00, 0x28, 0x00, 0x00, 0x00,
  0x27, 0x00, 0x27, 0x00, 0x03, 0x00, 0x04, 0x00,
  0x00, 0x00, 0x26, 0x00, 0x01, 0x00, 0x1e, 0x00,
  0x02, 0x00, 0x1f, 0x00, 0x03, 0x00, 0x1d, 0x00,
  0x04, 0x00, 0x27, 0x00, 0x05, 0x00, 0x0b, 0x00,
  0x06, 0x00, 0x28, 0x00, 0x08, 0x00, 0x21, 0x00,
  0x09, 0x00, 0x20, 0x00, 0x0a, 0x00, 0x22, 0x00,
  0x0b, 0x00, 0x25, 0x00, 0x0c, 0x00, 0x24, 0x00,
  0x0d, 0x00, 0x23, 0x00, 0x0e, 0x00, 0x19, 0x00,
  0x0f, 0x00, 0x16, 0x00, 0x10, 0x00, 0x15, 0x00,
  0x11, 0x00, 0x1c, 0x00, 0x12, 0x00, 0x1b, 0x00,
  0x13, 0x00, 0x1a, 0x00, 0x14, 0x00, 0x17, 0x00,
  0x15, 0x00, 0x18, 0x00, 0x16, 0x00, 0x0e, 0x00,
  0x18, 0x00, 0x0c, 0x00, 0x19, 0x00, 0x0d, 0x00,
  0x1a, 0x00, 0x12, 0x00, 0x1b, 0x00, 0x14, 0x00,
  0x1f, 0x00, 0x13, 0x00, 0x20, 0x00, 0x00, 0x00,
  0x21, 0x00, 0x0a, 0x00, 0x22, 0x00, 0x06, 0x00,
  0x23, 0x00, 0x07, 0x00, 0x24, 0x00, 0x08, 0x00,
  0x25, 0x00, 0x09, 0x00, 0x26, 0x00, 0x04, 0x00,
  0x27, 0x00, 0x03, 0x00, 0x28, 0x00, 0x02, 0x00,
  0x29, 0x00, 0x01, 0x00, 0x2a, 0x00, 0x05, 0x00,
  0x2b, 0x00, 0x2a, 0x00 };

/* some compilers need unsigned char to avoid warnings */
/*
static tui8 g_unknown2[8] =
{ 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00 };
*/

/*****************************************************************************/
static int APP_CC
xrdp_rdp_read_config(struct xrdp_client_info* client_info)
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

  client_info->connectivity_check= 0;
  client_info->connectivity_check_interval= DEFAULT_XRDP_CONNECTIVITY_CHECK_INTERVAL;

  for (index = 0; index < items->count; index++)
  {
    item = (char*)list_get_item(items, index);
    value = (char*)list_get_item(values, index);
    if (g_strcasecmp(item, "bitmap_cache") == 0)
    {
      if ((g_strcasecmp(value, "yes") == 0) ||
          (g_strcasecmp(value, "true") == 0) ||
          (g_strcasecmp(value, "1") == 0))
      {
        client_info->use_bitmap_cache = 1;
      }
    }
    else if (g_strcasecmp(item, "bitmap_compression") == 0)
    {
      if (g_strcasecmp(value, "yes") == 0 ||
          g_strcasecmp(value, "true") == 0 ||
          g_strcasecmp(value, "1") == 0)
      {
        client_info->use_bitmap_comp = 1;
      }
    }
    else if (g_strcasecmp(item, "use_compression") == 0)
    {
      if (g_strcasecmp(value, "yes") == 0 ||
          g_strcasecmp(value, "true") == 0 ||
          g_strcasecmp(value, "1") == 0)
      {
        client_info->use_compression = 1;
      }
    }
    else if (g_strcasecmp(item, "crypt_level") == 0)
    {
      if (g_strcasecmp(value, "low") == 0)
      {
        client_info->crypt_level = 1;
      }
      else if (g_strcasecmp(value, "medium") == 0)
      {
        client_info->crypt_level = 2;
      }
      else if (g_strcasecmp(value, "high") == 0)
      {
        client_info->crypt_level = 3;
      }
    }
    else if (g_strcasecmp(item, "channel_code") == 0)
    {
      if (g_strcasecmp(value, "1") == 0)
      {
        client_info->channel_code = 1;
      }
    }
    else if (g_strcasecmp(item, "jpeg_quality") == 0)
    {
      client_info->jpeg_quality = g_atoi(value);
      if (client_info->jpeg_quality == 0)
      {
        printf("Invalid value for jpeg quality: %s\n", value);
      }
      if (client_info->jpeg_quality < 0 || client_info->jpeg_quality > 100)
      {
        printf("Invalid value for jpeg quality: %s. Quality must be set between in [1-100]\n", value);
      }
    }
    else if (g_strcasecmp(item, "use_jpeg") == 0)
    {
      if (g_strcasecmp(value, "1") == 0)
      {
        client_info->can_use_jpeg = 1;
      }
      if (client_info->jpeg_quality == 0 )
      {
        client_info->jpeg_quality = 80;
      }
    }
    else if (g_strcasecmp(item, "use_unicode") == 0)
    {
      if (g_strcasecmp(value, "1") == 0)
      {
        client_info->use_unicode = 1;
      }
    }
    else if (g_strcasecmp(item, "use_scim") == 0)
    {
      client_info->use_scim = log_text2bool(value);
    }
    else if (g_strcasecmp(item, "connectivity_check") == 0)
    {
      if (g_strcasecmp(value, "1") == 0)
      {
        printf("Connectivity_check activated\n");
        client_info->connectivity_check = 1;
      }
    }
    else if (g_strcasecmp(item, "connectivity_check_interval") == 0)
    {
      client_info->connectivity_check_interval = g_atoi(value);
      if (client_info->connectivity_check_interval <= 0)
      {
        printf("Invalid value for 'connectivity_check_interval': %s\n", value);
        client_info->connectivity_check_interval = DEFAULT_XRDP_CONNECTIVITY_CHECK_INTERVAL;
      }

      printf("Connectivity_check_interval: %i\n", client_info->connectivity_check_interval);
    }
    else if (g_strcasecmp(item, "use_frame_marker") == 0)
    {
      if (g_strcasecmp(value, "1") == 0)
      {
        client_info->can_use_frame_marker = 1;
        printf("Server can use frame marker\n");
      }
    }
    else if (g_strcasecmp(item, "image_policy") == 0)
    {
      if (g_strcasecmp(value, "full") == 0)
      {
        client_info->image_policy = IMAGE_COMP_POLICY_FULL;
        client_info->image_policy_ptr = libxrdp_orders_send_image_full;
        printf("Server send all image with the same compression method\n");
      }
      else if (g_strcasecmp(value, "adaptative") == 0)
      {
        client_info->image_policy = IMAGE_COMP_POLICY_ADAPTATIVE;
        client_info->image_policy_ptr = libxrdp_orders_send_image_adaptative;
        printf("Server adapt image compression to minimize buffer\n");
      }
    }
    else if (g_strcasecmp(item, "use_fastpath") == 0)
    {
      client_info->support_fastpath = log_text2bool(value);
      printf("Server support fastPath\n");
    }
    else if (g_strcasecmp(item, "use_network_detection") == 0)
    {
      client_info->support_network_detection = log_text2bool(value);
      printf("Server support network detection\n");
    }
    else if (g_strcasecmp(item, "network_detection_interval") == 0)
    {
      client_info->network_detection_interval = g_atoi(value);
      printf("Network detection interval: %u\n", client_info->network_detection_interval);
    }
    else if (g_strcasecmp(item, "use_static_frame_rate") == 0)
    {
      client_info->use_static_frame_rate = log_text2bool(value);
      printf("Use static frame rate: %i\n", client_info->use_static_frame_rate);
    }
    else if (g_strcasecmp(item, "frame_rate") == 0)
    {
      client_info->frame_rate = g_atoi(value);
      printf("frame rate: %i\n", client_info->frame_rate);
    }
    else if (g_strcasecmp(item, "user_channel_plugin") == 0)
    {
      g_strncpy(client_info->user_channel_plugin, value, sizeof(client_info->user_channel_plugin));
      printf("user_channel_plugin: %s\n", client_info->user_channel_plugin);
    }
    else if (g_strcasecmp(item, "use_qos") == 0)
    {
      client_info->use_qos = log_text2bool(value);
      printf("use QOS: %i\n", client_info->use_qos);
    }
    else if (g_strcasecmp(item, "static_bandwidth") == 0)
    {
      client_info->static_bandwidth = g_atol(value);
      printf("static_bandwidth: %li\n", client_info->static_bandwidth);
    }
    else if (g_strcasecmp(item, "static_rtt") == 0)
    {
      client_info->static_rtt = g_atoi(value);
      printf("static round trip: %i\n", client_info->static_rtt);
    }
    else if (g_strcasecmp(item, "use_video_detection") == 0)
    {
      client_info->use_video_detection = log_text2bool(value);
      printf("Use video detection : %i \n", client_info->use_video_detection);
    }
    else if (g_strcasecmp(item, "video_detection_fps") == 0)
    {
      client_info->video_detection_fps = g_atoi(value);
      printf("Video detection fps : %i \n", client_info->video_detection_fps);
    }
    else if (g_strcasecmp(item, "video_detection_maxfps") == 0)
    {
      client_info->video_detection_maxfps = g_atoi(value);
      printf("Video detection max fps : %i \n", client_info->video_detection_maxfps);
    }
    else if (g_strcasecmp(item, "video_detection_updatetime") == 0)
    {
      client_info->video_detection_updatetime = g_atoi(value);
      printf("Video detection update_time : %i \n", client_info->video_detection_updatetime);
    }
    else if (g_strcasecmp(item, "video_display_borders") == 0)
    {
        client_info->video_display_borders = log_text2bool(value);
        printf("Video display borders : %i \n", client_info->video_display_borders);
    }
    else if (g_strcasecmp(item, "video_display_fps") == 0)
    {
        client_info->video_display_fps = g_atoi(value);
        printf("Video display fps : %i \n", client_info->video_display_fps);
    }
    else if (g_strcasecmp(item, "video_display_box_time_delay") == 0)
    {
      client_info->video_display_box_time_delay = g_atoi(value);
      printf("Video display box time delay : %i \n", client_info->video_display_box_time_delay);
    }
    else if (g_strcasecmp(item, "use_subtiling") == 0)
    {
      client_info->use_subtiling = log_text2bool(value);
      printf("Use subtiling : %i \n", client_info->use_subtiling);
    }
    else if (g_strcasecmp(item, "use_progressive_display") == 0)
    {
        client_info->use_progressive_display = log_text2bool(value);
        printf("Use progressive display : %i \n", client_info->use_progressive_display);
    }
    else if (g_strcasecmp(item, "progressive_display_nb_level") == 0)
    {
        client_info->progressive_display_nb_level = g_atoi(value);
        printf("Nb level in progressive display : %i \n", client_info->progressive_display_nb_level);
    }
    else if (g_strcasecmp(item, "progressive_display_scale") == 0)
    {
        client_info->progressive_display_scale = g_atoi(value);
        printf("progressive display scale factor : %i \n", client_info->progressive_display_scale);
    }
    else if (g_strcasecmp(item, "progressive_display_maxfps") == 0)
    {
      client_info->progressive_display_maxfps = g_atoi(value);
      printf("progressive display max fps : %i \n", client_info->progressive_display_maxfps);
    }
    else if (g_strcasecmp(item, "progressive_display_minfps") == 0)
    {
      client_info->progressive_display_minfps = g_atoi(value);
      printf("progressive display in fps : %i \n", client_info->progressive_display_minfps);
    }
    else if (g_strcasecmp(item, "channel_priority") == 0)
    {
      g_strtrim(value, 4);
      client_info->channel_priority = g_str_split_to_list(value, ',');
      printf("channel priority:\n");
      list_dump_items(client_info->channel_priority);
    }
    else if (g_strcasecmp(item, "order_packet_size") == 0)
    {
      client_info->order_packet_size = g_atoi(value);
      if (client_info->order_packet_size)
      {
        printf("order packet size: %i \n", client_info->order_packet_size);
      }
    }
    else if (g_strcasecmp(item, "tcp_frame_size") == 0)
    {
      client_info->tcp_frame_size = g_atoi(value);
      if (client_info->tcp_frame_size)
      {
        printf("tcp frame size: %i \n", client_info->tcp_frame_size);
      }
    }

    // Check if item is a bw limitation
    else if(g_str_end_with(item, "_bw") == 0)
    {
      item[g_strlen(item) - 3] = '\0';
      xrdp_qos_add_limitation(client_info->channels_bw_limit, item, value);
    }

  }
  list_delete(items);
  list_delete(values);

  return 0;
}

/*****************************************************************************/
struct xrdp_rdp* APP_CC
xrdp_rdp_create(struct xrdp_session* session, struct trans* trans)
{
  struct xrdp_rdp* self;

  DEBUG(("in xrdp_rdp_create"));
  self = (struct xrdp_rdp*)g_malloc(sizeof(struct xrdp_rdp), 1);
  self->session = session;
  self->share_id = 66538;
  /* read ini settings */
  self->client_info.image_policy = IMAGE_COMP_POLICY_FULL;
  self->client_info.image_policy_ptr = libxrdp_orders_send_image_full;
  self->client_info.support_fastpath = false;
  self->client_info.use_qos = false;
  self->client_info.support_network_detection = false;
  self->client_info.connection_type = CONNECTION_TYPE_UNKNOWN;
  self->client_info.network_detection_interval = 10000;
  self->client_info.frame_rate = 40;
  self->client_info.use_static_frame_rate = true;
  self->client_info.user_channel_plugin[0] = '\0';
  self->client_info.channel_priority = g_str_split_to_list(DEFAULT_CHANNEL_PRIORITY, ',');
  self->client_info.static_bandwidth = 0;
  self->client_info.static_rtt = 0;
  self->client_info.use_video_detection = false;
  self->client_info.video_detection_fps = 15;
  self->client_info.video_detection_maxfps = 30;
  self->client_info.video_detection_updatetime = 400;
  self->client_info.video_display_borders = false;
  self->client_info.video_display_fps = 0;
  self->client_info.video_display_box_time_delay = 2000;
  self->client_info.use_subtiling = false;
  self->client_info.use_progressive_display = false;
  self->client_info.progressive_display_nb_level = 3;
  self->client_info.progressive_display_scale = 4;
  self->client_info.progressive_display_maxfps = 24;
  self->client_info.progressive_display_minfps = 5;
  self->client_info.use_scim = true;
  self->client_info.use_subtiling = false;
  self->client_info.channels_bw_limit = xrdp_qos_create_bw_limit();
  self->client_info.order_packet_size = 0;
  xrdp_rdp_read_config(&self->client_info);
  /* create sec layer */
  self->sec_layer = xrdp_sec_create(self, trans, self->client_info.crypt_level,
                                    self->client_info.channel_code);
  /* default 8 bit v1 color bitmap cache entries and size */
  self->client_info.cache1_entries = 600;
  self->client_info.cache1_size = 256;
  self->client_info.cache2_entries = 300;
  self->client_info.cache2_size = 1024;
  self->client_info.cache3_entries = 262;
  self->client_info.cache3_size = 4096;
  self->compressor = 0;
  if (self->client_info.channel_priority == NULL)
  {
    self->client_info.channel_priority = list_create();
    self->client_info.channel_priority->auto_free = true;
  }
  DEBUG(("out xrdp_rdp_create"));
  return self;
}

/*****************************************************************************/
void APP_CC
xrdp_rdp_delete(struct xrdp_rdp* self)
{
  if (self == 0)
  {
    return;
  }
  xrdp_sec_delete(self->sec_layer);
  mppc_enc_free(self->compressor);
  g_free(self);
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_init(struct xrdp_rdp* self, struct stream* s)
{
  if (xrdp_sec_init(self->sec_layer, s) != 0)
  {
    return 1;
  }
  s_push_layer(s, rdp_hdr, 6);
  return 0;
}

/*****************************************************************************/
bool APP_CC
xrdp_rdp_init_compressor(struct xrdp_rdp* self, int mppc_version)
{
  if (self == 0)
  {
    return false;
  }
  
  if (mppc_version < 0)
  {
    return false;
  }
  
  if (mppc_version > PACKET_COMPR_TYPE_64K)
  {
    mppc_version = PACKET_COMPR_TYPE_64K;
  }
  
  self->compressor = mppc_enc_new(mppc_version);
  return true;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_init_data(struct xrdp_rdp* self, struct stream* s)
{
  if (xrdp_sec_init(self->sec_layer, s) != 0)
  {
    return 1;
  }
  s_push_layer(s, rdp_hdr, 18);
  return 0;
}

/*****************************************************************************/
/* returns erros */
int APP_CC
xrdp_rdp_recv(struct xrdp_rdp* self, struct stream* s, int* code)
{
  int error;
  int len;
  int pdu_code;
  int chan;
  struct xrdp_emt* emt = self->sec_layer->chan_layer->emt_channel;

  DEBUG(("in xrdp_rdp_recv"));
  if (s->next_packet == 0 || s->next_packet >= s->end)
  {
    chan = 0;
    error = xrdp_sec_recv(self->sec_layer, s, &chan);
    if (error == -1) /* special code for send demand active */
    {
      s->next_packet = 0;
      *code = -1;
      DEBUG(("out xrdp_rdp_recv"));
      return 0;
    }
    if (error != 0)
    {
      DEBUG(("out xrdp_rdp_recv error"));
      return 1;
    }
    if ((chan != MCS_GLOBAL_CHANNEL) && (chan > 0))
    {
      if (chan > MCS_GLOBAL_CHANNEL)
      {

        if ((emt != NULL) && (emt->chanid == chan))
        {
          xrdp_emt_process(self, s);
        }
        else
        {
          xrdp_channel_process(self->sec_layer->chan_layer, s, chan);
        }
      }
      s->next_packet = 0;
      *code = 0;
      DEBUG(("out xrdp_rdp_recv"));
      return 0;
    }
    s->next_packet = s->p;
  }
  else
  {
    s->p = s->next_packet;
  }
  in_uint16_le(s, len);
  if (len == 0x8000)
  {
    s->next_packet += 8;
    *code = 0;
    DEBUG(("out xrdp_rdp_recv"));
    return 0;
  }
  in_uint16_le(s, pdu_code);
  *code = pdu_code & 0xf;
  in_uint8s(s, 2); /* mcs user id */
  s->next_packet += len;
  DEBUG(("out xrdp_rdp_recv"));
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_send(struct xrdp_rdp* self, struct stream* s, int pdu_type)
{
  int len;

  DEBUG(("in xrdp_rdp_send"));
  s_pop_layer(s, rdp_hdr);
  len = s->end - s->p;
  out_uint16_le(s, len);
  out_uint16_le(s, 0x10 | pdu_type);
  out_uint16_le(s, self->mcs_channel);
  if (xrdp_sec_send(self->sec_layer, s, MCS_GLOBAL_CHANNEL) != 0)
  {
    DEBUG(("out xrdp_rdp_send error"));
    return 1;
  }
  DEBUG(("out xrdp_rdp_send"));
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_spool_data(struct xrdp_rdp* self, struct stream* s, int data_pdu_type)
{
  return xrdp_qos_spool(self->session->qos, s, data_pdu_type, 1);
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_spool_fast_path_update(struct xrdp_rdp* self, struct stream* s, int update_code)
{
  return xrdp_qos_spool(self->session->qos, s, update_code, 2);
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_send_data(struct xrdp_rdp* self, struct stream* s,
                   int data_pdu_type)
{
  int uncompressed_length = 0;
  int compression_type = 0;
  int compressed_length = 0;

  DEBUG(("in xrdp_rdp_send_data"));
  s_pop_layer(s, rdp_hdr);
  uncompressed_length = s->end - s->p;

  //temporary invalidate compression
  self->client_info.bitmap_cache_persist_enable = 0;
  if (data_pdu_type != RDP_DATA_PDU_CONTROL &&
  		data_pdu_type != RDP_DATA_PDU_SYNCHRONISE &&
  		data_pdu_type != RDP_DATA_PDU_TYPE2_FONTMAP &&
  		self->client_info.use_compression == 1 &&
  		self->client_info.rdp_compression == 1 &&
  		self->compressor != 0)
  {
  	int old_size = s->size;
  	s->size = uncompressed_length - 18;
  	s->p+=18;

	if (compress_rdp(self->compressor, s->p, s->size))
	{
		if (self->compressor->flags & PACKET_COMPRESSED)
		{
			DEBUG(("compressed"));
			compression_type = self->compressor->flags;
			compressed_length = self->compressor->bytes_in_opb;
			g_memcpy(s->p, self->compressor->outputBuffer, compressed_length);
		}
		else
		{
			compression_type = 0;
			compressed_length = 0;
			DEBUG(("Packet is not compressed"));
		}
	}
	else
	{
		compression_type = 0;
		compressed_length = 0;
		DEBUG(("Failed to compress packet"));
	}
	
  	s->size = old_size;
  	s->p -= 18;
  }

  if (compressed_length > 0)
    s->end = s->p + compressed_length + 18;

  out_uint16_le(s, compressed_length > 0 ? compressed_length + 18 : uncompressed_length);
  out_uint16_le(s, 0x10 | RDP_PDU_DATA);
  out_uint16_le(s, self->mcs_channel);
  out_uint32_le(s, self->share_id);
  out_uint8(s, 0);
  out_uint8(s, 1);                            /* stream priority: channel send optimization */
  out_uint16_le(s, uncompressed_length);      /* uncompressed length */
  out_uint8(s, data_pdu_type);
  out_uint8(s, compression_type);             /* compression type */
  out_uint16_le(s, compressed_length > 0 ? compressed_length + 18 : 0);        /* compressed length */

  if (xrdp_sec_send(self->sec_layer, s, MCS_GLOBAL_CHANNEL) != 0)
  {
    DEBUG(("out xrdp_rdp_send_data error"));
    return 1;
  }
  DEBUG(("out xrdp_rdp_send_data"));
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_send_fast_path_update(struct xrdp_rdp* self, struct stream* s, int update_code)
{
  int uncompressed_length = s->end - (s->data + FASTPATH_HEADER_LENGTH + FASTPATH_UPDATE_HEADER_LENGTH);
  int compression_type = 0;
  int compressed_length = 0;
  int update_header = update_code | FASTPATH_FRAGMENT_SINGLE << 4;

  DEBUG(("out xrdp_rdp_send_fast_path_update"));
  s->p = s->data + FASTPATH_HEADER_LENGTH;

  if (self->client_info.use_compression == 1 && self->client_info.rdp_compression == 1 && self->compressor != 0)
  {
    uncompressed_length = s->end - (s->data + FASTPATH_HEADER_LENGTH + FASTPATH_UPDATE_COMPRESSED_HEADER_LENGTH);
    if (compress_rdp(self->compressor, s->p + FASTPATH_UPDATE_COMPRESSED_HEADER_LENGTH, uncompressed_length))
    {
      if (self->compressor->flags & PACKET_COMPRESSED)
      {
        compression_type = self->compressor->flags;
        compressed_length = self->compressor->bytes_in_opb;
        g_memcpy(s->p + FASTPATH_UPDATE_COMPRESSED_HEADER_LENGTH, self->compressor->outputBuffer, compressed_length);
        s->end = s->p + FASTPATH_UPDATE_COMPRESSED_HEADER_LENGTH + compressed_length;
        s->size = s->end - s->data;
      }
      else
      {
        DEBUG(("Packet is not compressed"));
        compression_type = 0;
        compressed_length = 0;
        memmove(s->p + FASTPATH_UPDATE_HEADER_LENGTH, s->p + FASTPATH_UPDATE_COMPRESSED_HEADER_LENGTH, uncompressed_length);
        s->end --;
        s->size --;
      }
    }
    else
    {
      compression_type = 0;
      compressed_length = 0;
      printf("Failed to compress packet\n");
    }
  }

  if (compressed_length > 0)
  {
    update_header |= FASTPATH_OUTPUT_COMPRESSION_USED << 6;
    out_uint8(s, update_header);
    out_uint8(s, compression_type);
  }
  else
  {
    out_uint8(s, update_header);
  }

  out_uint16_le(s, compressed_length > 0 ? compressed_length : uncompressed_length);
  if (xrdp_fast_path_send(self->sec_layer, s) != 0)
  {
    DEBUG(("out xrdp_rdp_send_fast_path_update error"));
    return 1;
  }
  DEBUG(("out xrdp_rdp_send_fast_path_update"));
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_send_data_update_sync(struct xrdp_rdp* self)
{
  struct stream* s;

  make_stream(s);
  init_stream(s, 8192);
  DEBUG(("in xrdp_rdp_send_data_update_sync"));
  if (xrdp_rdp_init_data(self, s) != 0)
  {
    DEBUG(("out xrdp_rdp_send_data_update_sync error"));
    free_stream(s);
    return 1;
  }
  out_uint16_le(s, RDP_UPDATE_SYNCHRONIZE);
  out_uint8s(s, 2);
  s_mark_end(s);
  if (xrdp_rdp_send_data(self, s, RDP_DATA_PDU_UPDATE) != 0)
  {
    DEBUG(("out xrdp_rdp_send_data_update_sync error"));
    free_stream(s);
    return 1;
  }
  DEBUG(("out xrdp_rdp_send_data_update_sync"));
  free_stream(s);
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_parse_client_mcs_data(struct xrdp_rdp* self)
{
  struct stream* p;
  int postBeta2ColorDepth;
  int highColorDepth;

  p = &(self->sec_layer->client_mcs_data);
  p->p = p->data;
  in_uint8s(p, 27);

  // Client Core Data parsing
  in_uint8s(p, 4);                                  // version
  in_uint16_le(p, self->client_info.width);         // desktopWidth
  in_uint16_le(p, self->client_info.height);        // desktopHeight
  in_uint8s(p, 2);                                  // colorDepth
  in_uint8s(p, 2);                                  // SASSequence
  in_uint8s(p, 4);                                  // keyboardLayout
  in_uint8s(p, 4);                                  // clientBuild
  in_uint8s(p, 32);                                 // clientName
  in_uint8s(p, 4);                                  // keyboardType
  in_uint8s(p, 4);                                  // keyboardSubType
  in_uint8s(p, 4);                                  // keyboardFunctionKey
  in_uint8s(p, 64);                                 // imeFileName
  in_uint16_le(p, postBeta2ColorDepth);             // postBeta2ColorDepth
  in_uint8s(p, 2);                                  // clientProductId
  in_uint8s(p, 4);                                  // serialNumber
  in_uint16_le(p, highColorDepth);                  // highColorDepth
  in_uint8s(p, 2);                                  // supportedColorDepths
  in_uint8s(p, 2);                                  // earlyCapabilityFlags
  in_uint8s(p, 64);                                 // clientDigProductId
  in_uint8(p, self->client_info.connection_type);   // connectionType
  in_uint8s(p, 1);                                  // pad1octet
  in_uint8s(p, 4);                                  // serverSelectedProtocol
  in_uint8s(p, 4);                                  // desktopPhysicalWidth
  in_uint8s(p, 4);                                  // desktopPhysicalHeight
  in_uint8s(p, 2);                                  // reserved

  self->client_info.bpp = 8;
  switch (postBeta2ColorDepth)
  {
    case 0xca01:
      if (highColorDepth > 8)
      {
        self->client_info.bpp = highColorDepth;
      }
      break;
    case 0xca02:
      self->client_info.bpp = 15;
      break;
    case 0xca03:
      self->client_info.bpp = 16;
      break;
    case 0xca04:
      self->client_info.bpp = 24;
      break;
  }
  if (self->client_info.bpp > 24)
  {
    self->client_info.bpp = 24;
  }
  p->p = p->data;
  DEBUG(("client width %d, client height %d bpp %d",
         self->client_info.width, self->client_info.height,
         self->client_info.bpp));
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_incoming(struct xrdp_rdp* self)
{
  DEBUG(("in xrdp_rdp_incoming"));
  if (xrdp_sec_incoming(self->sec_layer) != 0)
  {
    return 1;
  }
  self->mcs_channel = self->sec_layer->mcs_layer->userid +
                      MCS_USERCHANNEL_BASE;
  xrdp_rdp_parse_client_mcs_data(self);

  switch (self->client_info.connection_type)
  {
  case CONNECTION_TYPE_MODEM:
	  self->session->bandwidth = 56000/8/1024;      // 56Kb/s
	  self->session->average_RTT = 100;
	  break;
  case CONNECTION_TYPE_BROADBAND_LOW:
	  self->session->bandwidth = 1000000/8/1024;    // 1Mb/s
	  self->session->average_RTT = 100;
	  break;

  case CONNECTION_TYPE_SATELLITE:
	  self->session->bandwidth = 8000000/8/1024;    // 8Mb/s
	  self->session->average_RTT = 800;
	  break;

  case CONNECTION_TYPE_BROADBAND_HIGH:
	  self->session->bandwidth = 5000000/8/1024;    // 5Mb/s
	  self->session->average_RTT = 200;
	  break;

  case CONNECTION_TYPE_WAN:
	  self->session->bandwidth = 5000000/8/1024;    // 5Mb/s
	  self->session->average_RTT = 200;
	  break;

  case CONNECTION_TYPE_UNKNOWN:
  case CONNECTION_TYPE_LAN:
	  self->session->bandwidth = 100000000/8/1024;  // 100Mb/s
	  self->session->average_RTT = 1;
	  break;

  case CONNECTION_TYPE_AUTODETECT:
  default:
	  break;
  }

  if (self->client_info.static_bandwidth != 0 && !self->client_info.support_network_detection)
  {
	  self->session->bandwidth = self->client_info.static_bandwidth;
  }

  DEBUG(("out xrdp_rdp_incoming mcs channel %d", self->mcs_channel));
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_send_demand_active(struct xrdp_rdp* self)
{
  struct stream* s;
  int caps_count;
  int caps_size;
  char* caps_count_ptr;
  char* caps_size_ptr;
  char* caps_ptr;
  int input_flags = INPUT_FLAG_SCANCODES;
  int extra_flags = 0;
  int flags = NEGOTIATEORDERSUPPORT | COLORINDEXSUPPORT;

  if (self->client_info.support_fastpath)
    extra_flags = FASTPATH_OUTPUT_SUPPORTED;

  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init(self, s) != 0)
  {
    free_stream(s);
    return 1;
  }

  caps_count = 0;
  out_uint32_le(s, self->share_id);
  out_uint16_le(s, 4); /* 4 chars for RDP\0 */
  /* 2 bytes size after num caps, set later */
  caps_size_ptr = s->p;
  out_uint8s(s, 2);
  out_uint8a(s, "RDP", 4);
  /* 4 byte num caps, set later */
  caps_count_ptr = s->p;
  out_uint8s(s, 4);
  caps_ptr = s->p;

  /* Output share capability set */
  caps_count++;
  out_uint16_le(s, RDP_CAPSET_SHARE);
  out_uint16_le(s, RDP_CAPLEN_SHARE);
  out_uint16_le(s, self->mcs_channel);
  out_uint16_be(s, 0xb5e2); /* 0x73e1 */

  /* Output general capability set */
  caps_count++;
  out_uint16_le(s, RDP_CAPSET_GENERAL); /* 1 */
  out_uint16_le(s, RDP_CAPLEN_GENERAL); /* 24(0x18) */
  out_uint16_le(s, 1); /* OS major type */
  out_uint16_le(s, 3); /* OS minor type */
  out_uint16_le(s, 0x200); /* Protocol version */
  out_uint16_le(s, 0); /* pad */
  out_uint16_le(s, 0); /* Compression types */
  out_uint16_le(s, extra_flags); /* extra flags */
  out_uint16_le(s, 0); /* Update capability */
  out_uint16_le(s, 0); /* Remote unshare capability */
  out_uint16_le(s, 0); /* Compression level */
  out_uint16_le(s, 0); /* Pad */

  /* Output bitmap capability set */
  caps_count++;
  out_uint16_le(s, RDP_CAPSET_BITMAP); /* 2 */
  out_uint16_le(s, RDP_CAPLEN_BITMAP); /* 28(0x1c) */
  out_uint16_le(s, self->client_info.bpp); /* Preferred BPP */
  out_uint16_le(s, 1); /* Receive 1 BPP */
  out_uint16_le(s, 1); /* Receive 4 BPP */
  out_uint16_le(s, 1); /* Receive 8 BPP */
  out_uint16_le(s, self->client_info.width); /* width */
  out_uint16_le(s, self->client_info.height); /* height */
  out_uint16_le(s, 0); /* Pad */
  out_uint16_le(s, 1); /* Allow resize */
  out_uint16_le(s, 1); /* bitmap compression */
  out_uint16_le(s, 0); /* unknown */
  out_uint16_le(s, 0); /* unknown */
  out_uint16_le(s, 0); /* pad */

  /* Output font capability set */
  caps_count++;
  out_uint16_le(s, RDP_CAPSET_FONT); /* 14 */
  out_uint16_le(s, RDP_CAPLEN_FONT); /* 4 */

  /* Output order capability set */
  caps_count++;
  out_uint16_le(s, RDP_CAPSET_ORDER); /* 3 */
  out_uint16_le(s, RDP_CAPLEN_ORDER); /* 88(0x58) */
  out_uint8s(s, 16);
  out_uint32_be(s, 0x40420f00);
  out_uint16_le(s, 1); /* Cache X granularity */
  out_uint16_le(s, 20); /* Cache Y granularity */
  out_uint16_le(s, 0); /* Pad */
  out_uint16_le(s, 1); /* Max order level */
  out_uint16_le(s, 0x2f); /* Number of fonts */

  if (self->client_info.can_use_frame_marker)
  {
	  flags |= ORDERFLAGS_EXTRA_FLAGS;
  }
  out_uint16_le(s, flags); /* Capability flags */
  /* caps */
  out_uint8(s, 1); /* dest blt */
  out_uint8(s, 1); /* pat blt */
  out_uint8(s, 1); /* screen blt */
  out_uint8(s, 1); /* mem blt */
  out_uint8(s, 0); /* tri blt */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* nine grid */
  out_uint8(s, 1); /* line to */
  out_uint8(s, 0); /* multi nine grid */
  out_uint8(s, 1); /* rect */
  out_uint8(s, 0); /* desk save */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* multi dest blt */
  out_uint8(s, 0); /* multi pat blt */
  out_uint8(s, 0); /* multi screen blt */
  out_uint8(s, 0); /* multi rect */
  out_uint8(s, 0); /* fast index */
  out_uint8(s, 0); /* polygon */
  out_uint8(s, 0); /* polygon */
  out_uint8(s, 0); /* polyline */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* fast glyph */
  out_uint8(s, 0); /* ellipse */
  out_uint8(s, 0); /* ellipse */
  out_uint8(s, 0); /* ? */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* unused */
  out_uint8(s, 0); /* unused */
  out_uint16_le(s, 0x6a1);

  if (flags & ORDERFLAGS_EXTRA_FLAGS)
  {
    out_uint16_le(s, ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT);
  }
  else
  {
    out_uint8s(s, 2); /* ? */
  }
  out_uint32_le(s, 0x0f4240); /* desk save */
  out_uint32_le(s, 0x0f4240); /* desk save */
  out_uint32_le(s, 1); /* ? */
  out_uint32_le(s, 0); /* ? */

  /* Output color cache capability set */
  caps_count++;
  out_uint16_le(s, RDP_CAPSET_COLCACHE);
  out_uint16_le(s, RDP_CAPLEN_COLCACHE);
  out_uint16_le(s, 6); /* cache size */
  out_uint16_le(s, 0); /* pad */

  /* Output pointer capability set */
  caps_count++;
  out_uint16_le(s, RDP_CAPSET_POINTER);
  out_uint16_le(s, RDP_CAPLEN_POINTER);
  out_uint16_le(s, 1); /* Colour pointer */
  out_uint16_le(s, 0x19); /* Cache size */
  out_uint16_le(s, 0x19); /* Cache size */

  /* Output input capability set */
  caps_count++;
  out_uint16_le(s, RDP_CAPSET_INPUT); /* 13(0xd) */
  out_uint16_le(s, RDP_CAPLEN_INPUT); /* 88(0x58) */
  if (self->client_info.use_unicode)
	  input_flags |= INPUT_FLAG_UNICODE;

  out_uint8(s, input_flags);
  out_uint8s(s, 83);

  out_uint8s(s, 4); /* pad */

  s_mark_end(s);

  caps_size = (int)(s->end - caps_ptr);
  caps_size_ptr[0] = caps_size;
  caps_size_ptr[1] = caps_size >> 8;

  caps_count_ptr[0] = caps_count;
  caps_count_ptr[1] = caps_count >> 8;
  caps_count_ptr[2] = caps_count >> 16;
  caps_count_ptr[3] = caps_count >> 24;

  if (xrdp_rdp_send(self, s, RDP_PDU_DEMAND_ACTIVE) != 0)
  {
    free_stream(s);
    return 1;
  }

  free_stream(s);
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_process_capset_general(struct xrdp_rdp* self, struct stream* s,
                            int len)
{
  int extraflags;

  in_uint8s(s, 2);                      // osMajorType
  in_uint8s(s, 2);                      // osMinorType
  in_uint8s(s, 2);                      // protocolVersion
  in_uint8s(s, 2);                      // pad2octetsA
  in_uint8s(s, 2);                      // generalCompressionTypes
  in_uint16_le(s, extraflags);          // extraFlags
  in_uint8s(s, 2);                      // updateCapabilityFlag
  in_uint8s(s, 2);                      // remoteUnshareFlag
  in_uint8s(s, 2);                      // generalCompressionLevel
  in_uint8s(s, 1);                      // refreshRectSupport
  in_uint8s(s, 1);                      // refreshRectSupport

  /* use_compact_packets is pretty much 'use rdp5' */
  self->client_info.use_compact_packets = (extraflags != 0);
  /* op2 is a boolean to use compact bitmap headers in bitmap cache */
  /* set it to same as 'use rdp5' boolean */
  self->client_info.op2 = self->client_info.use_compact_packets;

  if (!(extraflags && FASTPATH_OUTPUT_SUPPORTED))
  {
    self->client_info.support_fastpath = false;
  }
  if (self->client_info.support_fastpath)
  {
    printf("Fastpath activated\n");
  }

  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_process_capset_order(struct xrdp_rdp* self, struct stream* s,
                          int len)
{
  int i;
  char order_caps[32];
  unsigned short order_flags = 0;
  unsigned short order_flags_ext = 0;
  int parse_order_support_ex_flags = 0;

  DEBUG(("order capabilities"));
  in_uint8s(s, 20); /* Terminal desc, pad */
  in_uint8s(s, 2); /* Cache X granularity */
  in_uint8s(s, 2); /* Cache Y granularity */
  in_uint8s(s, 2); /* Pad */
  in_uint8s(s, 2); /* Max order level */
  in_uint8s(s, 2); /* Number of fonts */
  in_uint16_le(s, order_flags); /* Capability flags */

  if (order_flags & ORDERFLAGS_EXTRA_FLAGS)
  {
    parse_order_support_ex_flags = 1;
  }

  in_uint8a(s, order_caps, 32); /* Orders supported */
  DEBUG(("dest blt-0 %d", order_caps[0]));
  DEBUG(("pat blt-1 %d", order_caps[1]));
  DEBUG(("screen blt-2 %d", order_caps[2]));
  DEBUG(("memblt-3-13 %d %d", order_caps[3], order_caps[13]));
  DEBUG(("triblt-4-14 %d %d", order_caps[4], order_caps[14]));
  DEBUG(("line-8 %d", order_caps[8]));
  DEBUG(("line-9 %d", order_caps[9]));
  DEBUG(("rect-10 %d", order_caps[10]));
  DEBUG(("desksave-11 %d", order_caps[11]));
  DEBUG(("polygon-20 %d", order_caps[20]));
  DEBUG(("polygon2-21 %d", order_caps[21]));
  DEBUG(("polyline-22 %d", order_caps[22]));
  DEBUG(("ellipse-25 %d", order_caps[25]));
  DEBUG(("ellipse2-26 %d", order_caps[26]));
  DEBUG(("text2-27 %d", order_caps[27]));
  DEBUG(("order_caps dump"));
#if defined(XRDP_DEBUG)
  g_hexdump(order_caps, 32);
#endif
  in_uint8s(s, 2); /* Text capability flags */

  if (parse_order_support_ex_flags)
  {
    in_uint16_le(s, order_flags_ext);

    self->client_info.use_frame_marker = 0;
    if ((order_flags_ext & ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT) && self->client_info.can_use_frame_marker)
    {
      self->client_info.use_frame_marker = 1;
      printf("client want to use frame marker\n");
    }
  }
  else
  {
    in_uint8s(s, 2); /* Pad */
  }

  in_uint8s(s, 4); /* Pad */
  in_uint32_le(s, i); /* desktop cache size, usually 0x38400 */
  self->client_info.desktop_cache = i;
  DEBUG(("desktop cache size %d", i));
  in_uint8s(s, 4); /* Unknown */
  in_uint8s(s, 4); /* Unknown */
  return 0;
}

/*****************************************************************************/
/* get the bitmap cache size */
static int APP_CC
xrdp_process_capset_bmpcache(struct xrdp_rdp* self, struct stream* s,
                             int len)
{
  in_uint8s(s, 24);
  in_uint16_le(s, self->client_info.cache1_entries);
  in_uint16_le(s, self->client_info.cache1_size);
  in_uint16_le(s, self->client_info.cache2_entries);
  in_uint16_le(s, self->client_info.cache2_size);
  in_uint16_le(s, self->client_info.cache3_entries);
  in_uint16_le(s, self->client_info.cache3_size);
  DEBUG(("cache1 entries %d size %d", self->client_info.cache1_entries,
         self->client_info.cache1_size));
  DEBUG(("cache2 entries %d size %d", self->client_info.cache2_entries,
         self->client_info.cache2_size));
  DEBUG(("cache3 entries %d size %d", self->client_info.cache3_entries,
         self->client_info.cache3_size));
  return 0;
}

/*****************************************************************************/
/* get the bitmap cache size */
static int APP_CC
xrdp_process_capset_jpegcache(struct xrdp_rdp* self, struct stream* s, int len)
{
  int active = 0;
  in_uint16_le(s, active);
  if (self->client_info.can_use_jpeg == 1)
  {
    self->client_info.use_jpeg = active;
  }
  return 0;
}

/*****************************************************************************/
/* get the bitmap cache size */
static int APP_CC
xrdp_process_capset_bmpcache2(struct xrdp_rdp* self, struct stream* s,
                              int len)
{
  int Bpp;
  int i;

  self->client_info.bitmap_cache_version = 2;
  Bpp = (self->client_info.bpp + 7) / 8;
  if (Bpp == 3)
  {
    Bpp = 4;
  }
  in_uint16_le(s, i);
  self->client_info.bitmap_cache_persist_enable = i;
  in_uint8s(s, 2); /* number of caches in set, 3 */
  in_uint32_le(s, i);
  i = MIN(i, 2000);
  self->client_info.cache1_entries = i;
  self->client_info.cache1_size = 256 * Bpp;
  in_uint32_le(s, i);
  i = MIN(i, 2000);
  self->client_info.cache2_entries = i;
  self->client_info.cache2_size = 1024 * Bpp;
  in_uint32_le(s, i);
  i = i & 0x7fffffff;
  i = MIN(i, 2000);
  self->client_info.cache3_entries = i;
  self->client_info.cache3_size = 4096 * Bpp;
  DEBUG(("cache1 entries %d size %d", self->client_info.cache1_entries,
         self->client_info.cache1_size));
  DEBUG(("cache2 entries %d size %d", self->client_info.cache2_entries,
         self->client_info.cache2_size));
  DEBUG(("cache3 entries %d size %d", self->client_info.cache3_entries,
         self->client_info.cache3_size));
  return 0;
}

/*****************************************************************************/
/* get the number of client cursor cache */
static int APP_CC
xrdp_process_capset_pointercache(struct xrdp_rdp* self, struct stream* s,
                                 int len)
{
  int i;

  in_uint8s(s, 2); /* color pointer */
  in_uint16_le(s, i);
  i = MIN(i, 32);
  self->client_info.pointer_cache_entries = i;
  return 0;
}

/*****************************************************************************/
/* get the type of client brush cache */
static int APP_CC
xrdp_process_capset_brushcache(struct xrdp_rdp* self, struct stream* s,
                               int len)
{
  int code;

  in_uint32_le(s, code);
  self->client_info.brush_cache_code = code;
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_process_confirm_active(struct xrdp_rdp* self, struct stream* s)
{
  int cap_len;
  int source_len;
  int num_caps;
  int index;
  int type;
  int len;
  char* p;

  DEBUG(("in xrdp_rdp_process_confirm_active"));
  in_uint8s(s, 4); /* rdp_shareid */
  in_uint8s(s, 2); /* userid */
  in_uint16_le(s, source_len); /* sizeof RDP_SOURCE */
  in_uint16_le(s, cap_len);
  in_uint8s(s, source_len);
  in_uint16_le(s, num_caps);
  in_uint8s(s, 2); /* pad */
  for (index = 0; index < num_caps; index++)
  {
    p = s->p;
    in_uint16_le(s, type);
    in_uint16_le(s, len);
    switch (type)
    {
      case RDP_CAPSET_GENERAL: /* 1 */
        DEBUG(("RDP_CAPSET_GENERAL"));
        xrdp_process_capset_general(self, s, len);
        break;
      case RDP_CAPSET_BITMAP: /* 2 */
        DEBUG(("RDP_CAPSET_BITMAP"));
        break;
      case RDP_CAPSET_ORDER: /* 3 */
        DEBUG(("RDP_CAPSET_ORDER"));
        xrdp_process_capset_order(self, s, len);
        break;
      case RDP_CAPSET_BMPCACHE: /* 4 */
        DEBUG(("RDP_CAPSET_BMPCACHE"));
        xrdp_process_capset_bmpcache(self, s, len);
        break;
      case RDP_CAPSET_CONTROL: /* 5 */
        DEBUG(("RDP_CAPSET_CONTROL"));
        break;
      case RDP_CAPSET_ACTIVATE: /* 7 */
        DEBUG(("RDP_CAPSET_ACTIVATE"));
        break;
      case RDP_CAPSET_POINTER: /* 8 */
        DEBUG(("RDP_CAPSET_POINTER"));
        xrdp_process_capset_pointercache(self, s, len);
        break;
      case RDP_CAPSET_SHARE: /* 9 */
        DEBUG(("RDP_CAPSET_SHARE"));
        break;
      case RDP_CAPSET_COLCACHE: /* 10 */
        DEBUG(("RDP_CAPSET_COLCACHE"));
        break;
      case 12: /* 12 */
        DEBUG(("--12"));
        break;
      case 13: /* 13 */
        DEBUG(("--13"));
        break;
      case 14: /* 14 */
        DEBUG(("--14"));
        break;
      case RDP_CAPSET_BRUSHCACHE: /* 15 */
        xrdp_process_capset_brushcache(self, s, len);
        break;
      case 16: /* 16 */
        DEBUG(("--16"));
        break;
      case 17: /* 17 */
        DEBUG(("--16"));
        break;
      case RDP_CAPSET_BMPCACHE2: /* 19 */
        DEBUG(("RDP_CAPSET_BMPCACHE2"));
        xrdp_process_capset_bmpcache2(self, s, len);
        break;
      case 20: /* 20 */
        DEBUG(("--20"));
        break;
      case 21: /* 21 */
        DEBUG(("--21"));
        break;
      case 22: /* 22 */
        DEBUG(("--22"));
        break;
      case 26: /* 26 */
        DEBUG(("--26"));
      case RDP_CAPSET_JPEGCACHE: /* 99 */
        DEBUG(("RDP_CAPSET_JPEGCACHE"));
        xrdp_process_capset_jpegcache(self, s, len);
        break;
      default:
        g_writeln("unknown in xrdp_rdp_process_confirm_active %d", type);
        break;
    }
    s->p = p + len;
  }
  DEBUG(("out xrdp_rdp_process_confirm_active"));
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_process_data_pointer(struct xrdp_rdp* self, struct stream* s)
{
  return 0;
}

/*****************************************************************************/
/* RDP_DATA_PDU_INPUT */
static int APP_CC
xrdp_rdp_process_data_input(struct xrdp_rdp* self, struct stream* s)
{
  int num_events;
  int index;
  int msg_type;
  int device_flags;
  int param1;
  int param2;
  int time;

  in_uint16_le(s, num_events);
  in_uint8s(s, 2); /* pad */
  DEBUG(("in xrdp_rdp_process_data_input %d events", num_events));
  for (index = 0; index < num_events; index++)
  {
    in_uint32_le(s, time);
    in_uint16_le(s, msg_type);
    in_uint16_le(s, device_flags);
    in_sint16_le(s, param1);
    in_sint16_le(s, param2);
    DEBUG(("xrdp_rdp_process_data_input event %4.4x flags %4.4x param1 %d \
param2 %d time %d", msg_type, device_flags, param1, param2, time));
    if (self->session->callback != 0)
    {
      /* msg_type can be
         RDP_INPUT_SYNCHRONIZE - 0
         RDP_INPUT_SCANCODE - 4
         RDP_INPUT_MOUSE - 0x8001 */
      /* call to xrdp_wm.c : callback */
      self->session->callback(self->session->id, msg_type, param1, param2,
                              device_flags, time);
    }
  }
  DEBUG(("out xrdp_rdp_process_data_input"));
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_send_synchronise(struct xrdp_rdp* self)
{
  struct stream* s;

  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init_data(self, s) != 0)
  {
    free_stream(s);
    return 1;
  }
  out_uint16_le(s, 1);
  out_uint16_le(s, 1002);
  s_mark_end(s);
  if (xrdp_rdp_send_data(self, s, RDP_DATA_PDU_SYNCHRONISE) != 0)
  {
    free_stream(s);
    return 1;
  }
  free_stream(s);
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_send_control(struct xrdp_rdp* self, int action)
{
  struct stream* s;

  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init_data(self, s) != 0)
  {
    free_stream(s);
    return 1;
  }
  out_uint16_le(s, action);
  out_uint16_le(s, 0); /* userid */
  out_uint32_le(s, 1002); /* control id */
  s_mark_end(s);
  if (xrdp_rdp_send_data(self, s, RDP_DATA_PDU_CONTROL) != 0)
  {
    free_stream(s);
    return 1;
  }
  free_stream(s);
  return 0;
}

int APP_CC
xrdp_rdp_send_logon(struct xrdp_rdp* self)
{
  struct stream* s;
  char data[512];
  int len;

  DEBUG((" in xrdp_rdp_send_logon"));
  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init_data(self, s) != 0)
  {
    DEBUG(("xrdp_rdp_send_logon failed to init stream"));
    free_stream(s);
    return 1;
  }

  out_uint32_le(s, 0); /* Simple Login */


  len = ((g_strlen(self->client_info.domain)+1)*2);
  DEBUG(("  set xrdp_rdp_send_logon domain %s uni len is %d", self->client_info.domain, len));
  out_uint32_le(s, len - 1);
  uni_rdp_out_str(s, self->client_info.domain, len);
  out_uint8s(s, 52 - len );

  len = (g_strlen(self->client_info.username)+1)*2;
  DEBUG(("  set xrdp_rdp_send_logon user %s uni len is %d", self->client_info.username, len));
  out_uint32_le(s, len - 1);
  uni_rdp_out_str(s, self->client_info.username, len);
  out_uint8s(s, 512 - len );

  DEBUG(("  set xrdp_rdp_send_logon session 0x%x", self->session->id));
  out_uint32_le(s, self->session->id);

  s_mark_end(s);
  if (xrdp_rdp_send_data(self, s, RDP_DATA_PDU_LOGON) != 0)
  {
    DEBUG(("xrdp_rdp_send_logon failed to send RDP_DATA_PDU_LOGON"));
    free_stream(s);
    return 1;
  }
  free_stream(s);
  DEBUG(("  out xrdp_rdp_send_logon"));
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_process_data_control(struct xrdp_rdp* self, struct stream* s)
{
  int action;

  DEBUG(("xrdp_rdp_process_data_control"));
  in_uint16_le(s, action);
  in_uint8s(s, 2); /* user id */
  in_uint8s(s, 4); /* control id */
  if (action == RDP_CTL_REQUEST_CONTROL)
  {
    DEBUG(("xrdp_rdp_process_data_control got RDP_CTL_REQUEST_CONTROL"));
    DEBUG(("xrdp_rdp_process_data_control calling xrdp_rdp_send_synchronise"));
    xrdp_rdp_send_synchronise(self);
    DEBUG(("xrdp_rdp_process_data_control sending RDP_CTL_COOPERATE"));
    xrdp_rdp_send_control(self, RDP_CTL_COOPERATE);
    DEBUG(("xrdp_rdp_process_data_control sending RDP_CTL_GRANT_CONTROL"));
    xrdp_rdp_send_control(self, RDP_CTL_GRANT_CONTROL);
  }
  else
  {
    DEBUG(("xrdp_rdp_process_data_control unknown action"));
  }
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_process_data_sync(struct xrdp_rdp* self)
{
  DEBUG(("xrdp_rdp_process_data_sync"));
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_process_screen_update(struct xrdp_rdp* self, struct stream* s)
{
  int op;
  int left;
  int top;
  int right;
  int bottom;
  int cx;
  int cy;

  in_uint32_le(s, op);
  in_uint16_le(s, left);
  in_uint16_le(s, top);
  in_uint16_le(s, right);
  in_uint16_le(s, bottom);
  cx = (right - left) + 1;
  cy = (bottom - top) + 1;
  if (self->session->callback != 0)
  {
    self->session->callback(self->session->id, 0x4444, left, top, cx, cy);
  }
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_send_unknown1(struct xrdp_rdp* self)
{
  struct stream* s;

  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init_data(self, s) != 0)
  {
    free_stream(s);
    return 1;
  }
  out_uint8a(s, g_unknown1, 172);
  s_mark_end(s);
  if (xrdp_rdp_send_data(self, s, RDP_DATA_PDU_TYPE2_FONTMAP) != 0)
  {
    free_stream(s);
    return 1;
  }
  free_stream(s);
  return 0;
}

/*****************************************************************************/
static int APP_CC
xrdp_rdp_process_data_font(struct xrdp_rdp* self, struct stream* s)
{
  int seq;

  DEBUG(("in xrdp_rdp_process_data_font"));
  in_uint8s(s, 2); /* num of fonts */
  in_uint8s(s, 2); /* unknown */
  in_uint16_le(s, seq);
  /* 419 client sends Seq 1, then 2 */
  /* 2600 clients sends only Seq 3 */
  if (seq == 2 || seq == 3) /* after second font message, we are up and */
  {                                           /* running */
    DEBUG(("sending unknown1"));
    xrdp_rdp_send_unknown1(self);
    self->session->up_and_running = 1;
    DEBUG(("up_and_running set"));
    xrdp_rdp_send_data_update_sync(self);
  }
  DEBUG(("out xrdp_rdp_process_data_font"));
  return 0;
}

/*****************************************************************************/
/* sent 37 pdu */
static int APP_CC
xrdp_rdp_send_disconnect_query_response(struct xrdp_rdp* self)
{
  struct stream* s;

  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init_data(self, s) != 0)
  {
    free_stream(s);
    return 1;
  }
  s_mark_end(s);
  if (xrdp_rdp_spool_data(self, s, 37) != 0)
  {
    free_stream(s);
    return 1;
  }
  free_stream(s);
  return 0;
}

#if 0 /* not used */
/*****************************************************************************/
/* sent RDP_DATA_PDU_DISCONNECT 47 pdu */
static int APP_CC
xrdp_rdp_send_disconnect_reason(struct xrdp_rdp* self, int reason)
{
  struct stream* s;

  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init_data(self, s) != 0)
  {
    free_stream(s);
    return 1;
  }
  out_uint32_le(s, reason);
  s_mark_end(s);
  if (xrdp_rdp_spool_data(self, s, RDP_DATA_PDU_DISCONNECT) != 0)
  {
    free_stream(s);
    return 1;
  }
  free_stream(s);
  return 0;
}
#endif

/*****************************************************************************/
/* RDP_PDU_DATA */
int APP_CC
xrdp_rdp_process_data(struct xrdp_rdp* self, struct stream* s)
{
  int len;
  int data_type;
  int ctype;
  int clen;

  in_uint8s(s, 6);
  in_uint16_le(s, len);
  in_uint8(s, data_type);
  in_uint8(s, ctype);
  in_uint16_le(s, clen);
  DEBUG(("xrdp_rdp_process_data code %d", data_type));
  switch (data_type)
  {
    case RDP_DATA_PDU_POINTER: /* 27(0x1b) */
      xrdp_rdp_process_data_pointer(self, s);
      break;
    case RDP_DATA_PDU_INPUT: /* 28(0x1c) */
      xrdp_rdp_process_data_input(self, s);
      break;
    case RDP_DATA_PDU_CONTROL: /* 20(0x14) */
      xrdp_rdp_process_data_control(self, s);
      break;
    case RDP_DATA_PDU_SYNCHRONISE: /* 31(0x1f) */
      xrdp_rdp_process_data_sync(self);
      break;
    case 33: /* 33(0x21) ?? Invalidate an area I think */
      xrdp_rdp_process_screen_update(self, s);
      break;
    case 35: /* 35(0x23) */
      /* 35 ?? this comes when minimuzing a full screen mstsc.exe 2600 */
      /* I think this is saying the client no longer wants screen */
      /* updates and it will issue a 33 above to catch up */
      /* so minimized apps don't take bandwidth */
      break;
    case 36: /* 36(0x24) ?? disconnect query? */
      /* when this message comes, send a 37 back so the client */
      /* is sure the connection is alive and it can ask if user */
      /* really wants to disconnect */
      xrdp_rdp_send_disconnect_query_response(self); /* send a 37 back */
      break;
    case RDP_DATA_PDU_FONT2: /* 39(0x27) */
      xrdp_rdp_process_data_font(self, s);
      break;
    default:
      g_writeln("unknown in xrdp_rdp_process_data %d", data_type);
      break;
  }
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_disconnect(struct xrdp_rdp* self)
{
  int rv;

  DEBUG(("in xrdp_rdp_disconnect"));
  rv = xrdp_sec_disconnect(self->sec_layer);
  DEBUG(("out xrdp_rdp_disconnect"));
  return rv;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_send_deactive(struct xrdp_rdp* self)
{
  struct stream* s;

  DEBUG(("in xrdp_rdp_send_deactive"));
  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init(self, s) != 0)
  {
    free_stream(s);
    DEBUG(("out xrdp_rdp_send_deactive error"));
    return 1;
  }
  s_mark_end(s);
  if (xrdp_rdp_send(self, s, RDP_PDU_DEACTIVATE) != 0)
  {
    free_stream(s);
    DEBUG(("out xrdp_rdp_send_deactive error"));
    return 1;
  }
  free_stream(s);
  DEBUG(("out xrdp_rdp_send_deactive"));
  return 0;
}

/*****************************************************************************/
int APP_CC
xrdp_rdp_send_keepalive(struct xrdp_rdp* self)
{
  struct stream* s;

  DEBUG(("in xrdp_rdp_send_keepalive"));
  make_stream(s);
  init_stream(s, 8192);
  if (xrdp_rdp_init(self, s) != 0)
  {
    free_stream(s);
    DEBUG(("out xrdp_rdp_send_keepalive error"));
    return 1;
  }
  s_mark_end(s);

  s_pop_layer(s, rdp_hdr);
  out_uint16_le(s, 0x8000);
  out_uint16_le(s, 0x10);
  out_uint16_le(s, self->mcs_channel);
  if (xrdp_sec_send(self->sec_layer, s, MCS_GLOBAL_CHANNEL) != 0)
  {
    free_stream(s);
    DEBUG(("out xrdp_rdp_send_keepalive error"));
    return 1;
  }

  free_stream(s);
  DEBUG(("out xrdp_rdp_send_keepalive"));
  return 0;
}
