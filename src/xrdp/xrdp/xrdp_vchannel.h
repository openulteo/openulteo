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
#ifndef XRDP_VCHANNEL_H_
#define XRDP_VCHANNEL_H_


#include "chansrv.h"



vchannel* APP_CC
xrdp_vchannel_create();
void APP_CC
xrdp_vchannel_delete(vchannel* vc);
bool APP_CC
xrdp_vchannel_setup(vchannel* vc);

int APP_CC
xrdp_vchannel_process_channel_data(vchannel* vc, tbus param1, tbus param2, tbus param3, tbus param4);




#endif /* XRDP_VCHANNEL_H_ */
