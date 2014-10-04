/**
 * Copyright (C) 2011-2012 Ulteo SAS
 * http://www.ulteo.com
 * Author Vincent Roullier <vincent.roullier@ulteo.com> 2012
 * Author Thomas MOUTON <thomas@ulteo.com> 2012
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
/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Implements Microsoft Point to Point Compression (MPPC) protocol
 *
 * Copyright 2012 Laxmikant Rashinkar <LK.Rashinkar@gmail.com>
 * Copyright 2012 Jay Sorg <jay.sorg@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MPPC_ENC_H
#define __MPPC_ENC_H

#include "arch.h"

/* Compression Types */
#define PACKET_COMPRESSED		0x20
#define PACKET_AT_FRONT			0x40
#define PACKET_FLUSHED			0x80
#define PACKET_COMPR_TYPE_8K		0x00
#define PACKET_COMPR_TYPE_64K		0x01
#define PACKET_COMPR_TYPE_RDP6		0x02
#define PACKET_COMPR_TYPE_RDP61		0x03
#define CompressionTypeMask		0x0F

#define RDP6_HISTORY_BUF_SIZE		65536
#define RDP6_OFFSET_CACHE_SIZE		8

struct rdp_mppc_enc
{
	int   protocol_type;    /* PROTO_RDP_40, PROTO_RDP_50 etc */
	char* historyBuffer;    /* contains uncompressed data */
	char* outputBuffer;     /* contains compressed data */
	char* outputBufferPlus;
	int   historyOffset;    /* next free slot in historyBuffer */
	int   buf_len;          /* length of historyBuffer, protocol dependant */
	int   bytes_in_opb;     /* compressed bytes available in outputBuffer */
	int   flags;            /* PACKET_COMPRESSED, PACKET_AT_FRONT, PACKET_FLUSHED etc */
	int   flagsHold;
	int   first_pkt;        /* this is the first pkt passing through enc */
	tui16* hash_table;
};

bool compress_rdp(struct rdp_mppc_enc* enc, tui8* srcData, int len);
bool compress_rdp_4(struct rdp_mppc_enc* enc, tui8* srcData, int len);
bool compress_rdp_5(struct rdp_mppc_enc* enc, tui8* srcData, int len);
struct rdp_mppc_enc* mppc_enc_new(int protocol_type);
void mppc_enc_free(struct rdp_mppc_enc* enc);

#endif
