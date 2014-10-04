/*
 * Copyright (C) 2013 Ulteo SAS
 * http://www.ulteo.com
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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

#include "urdp_event.h"

//#include <freerdp/locale/keyboard.h>

AND_EXIT_CODE urdp_event_init(urdp_context* context) {
	//context->context.instance->settings->kbd_layout = freerdp_keyboard_init(context->context.instance->settings->kbd_layout);
	return SUCCESS;
}

AND_EXIT_CODE urdp_click_down(urdp_context* context, uint16 x, uint16 y, uint8 button) {
	uint16 flags;
	uint16 wheel = false;
	uint16 extended = false;

	switch (button) {
	case 1:
		flags = PTR_FLAGS_BUTTON1 | PTR_FLAGS_DOWN;
		break;
	case 2:
		flags = PTR_FLAGS_BUTTON3 | PTR_FLAGS_DOWN;
		break;
	case 3:
		flags = PTR_FLAGS_BUTTON2 | PTR_FLAGS_DOWN;
		break;
	case 4:
		wheel = true;
		flags = PTR_FLAGS_WHEEL | 0x0078;
		break;
	case 5:
		wheel = true;
		flags = PTR_FLAGS_WHEEL | PTR_FLAGS_WHEEL_NEGATIVE | 0x0088;
		break;
	case 6: // wheel left or back
	case 8: // back
	case 97: // Xming
		extended = true;
		flags = PTR_XFLAGS_DOWN | PTR_XFLAGS_BUTTON1;
		break;
	case 7: // wheel right or forward
	case 9: // forward
	case 112: // Xming
		extended = true;
		flags = PTR_XFLAGS_DOWN | PTR_XFLAGS_BUTTON2;
		break;
	default:
		flags = 0;
		break;
	}
	//log_debug("Panel : click down : x:%d, y:%d, b:%d, wheel:%d, extended:%d\n", x, y, button, wheel, extended);
	pthread_mutex_lock(&context->event_mutex);
	if (wheel) {
		context->context.instance->input->MouseEvent(context->context.instance->input, flags, 0, 0);
	} else {
		if (extended)
			context->context.instance->input->ExtendedMouseEvent(context->context.instance->input, flags, x, y);
		else
			context->context.instance->input->MouseEvent(context->context.instance->input, flags, x, y);
	}
	pthread_mutex_unlock(&context->event_mutex);
	return 0;
}

AND_EXIT_CODE urdp_click_up(urdp_context* context, uint16 x, uint16 y, uint8 button) {
	uint16 flags;
	boolean extended = false;

	//log_debug("Panel : click up : x = %d - y = %d, b = %d", x, y, button);

	switch (button) {
	case 1:
		flags = PTR_FLAGS_BUTTON1;
		break;
	case 2:
		flags = PTR_FLAGS_BUTTON3;
		break;
	case 3:
		flags = PTR_FLAGS_BUTTON2;
		break;
	case 6: // wheel left or back
	case 8: // back

	case 97: // Xming
		extended = true;
		flags = PTR_XFLAGS_BUTTON1;
		break;
	case 7: // wheel right or forward
	case 9: // forward
	case 112: // Xming
		extended = true;
		flags = PTR_XFLAGS_BUTTON2;
		break;
	default:
		flags = 0;
		break;
	}
	pthread_mutex_lock(&context->event_mutex);
	if (flags != 0) {
		if (extended)
			context->context.instance->input->ExtendedMouseEvent(context->context.instance->input, flags, x, y);
		else
			context->context.instance->input->MouseEvent(context->context.instance->input, flags, x, y);
	}
	pthread_mutex_unlock(&context->event_mutex);
	return 0;
}

AND_EXIT_CODE urdp_click_move(urdp_context* context, uint16 x, uint16 y) {
	//log_debug("Panel : click down : x = %d - y = %d", x, y);
	pthread_mutex_lock(&context->event_mutex);
	context->context.instance->input->MouseEvent(context->context.instance->input, PTR_FLAGS_MOVE, x, y);
	pthread_mutex_unlock(&context->event_mutex);
	return 0;
}

AND_EXIT_CODE urdp_send_unicode(urdp_context* context, uint16 unicode) {
	log_debug("urdp_send_unicode : 0x%4.4x", unicode);
	pthread_mutex_lock(&context->event_mutex);
	context->context.instance->input->UnicodeKeyboardEvent(context->context.instance->input, KBD_FLAGS_DOWN, unicode);
	pthread_mutex_unlock(&context->event_mutex);
	return 0;
}

AND_EXIT_CODE urdp_send_scancode(urdp_context* context, uint16 flags, uint16 scancode) {
	log_debug("urdp_send_scancode: flags=0x%4.4x scancode=0x%4.4x", flags, scancode);
	pthread_mutex_lock(&context->event_mutex);
	context->context.instance->input->KeyboardEvent(context->context.instance->input, flags, scancode);
	pthread_mutex_unlock(&context->event_mutex);
	return 0;
}
