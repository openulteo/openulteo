/*
 * Copyright (C) 2013-2014 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2014
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
 */

#ifndef __URDP_EVENT_H
#define __URDP_EVENT_H

#include "urdp.h"

AND_EXIT_CODE urdp_event_init(urdp_context* context);
AND_EXIT_CODE urdp_click_down(urdp_context* context, uint16 x, uint16 y, uint8 button);
AND_EXIT_CODE urdp_click_up(urdp_context* context, uint16 x, uint16 y, uint8 button);
AND_EXIT_CODE urdp_click_move(urdp_context* context, uint16 x, uint16 y);
AND_EXIT_CODE urdp_send_unicode(urdp_context* context, uint16 unicode);
AND_EXIT_CODE urdp_send_scancode(urdp_context* context, uint16 flags, uint16 scancode);
void urdp_ukbrdr_send_ime_preedit_string(urdp_context* context, const char* data, const long size);
void urdp_ukbrdr_send_ime_preedit_string_stop(urdp_context* context);

#endif /* __URDP_EVENT_H */
