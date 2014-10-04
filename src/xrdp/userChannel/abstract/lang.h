/**
 * Copyright (C) 2011 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2011
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

#ifndef LANG_H
#define LANG_H

#include "userChannel.h"

struct xrdp_key_info
{
  int sym;
  int chr;
};

struct xrdp_key_info2
{
  char* sym;
  int chr ;
};

struct xrdp_keymap
{
  struct xrdp_key_info keys_noshift[128];
  struct xrdp_key_info keys_shift[128];
  struct xrdp_key_info keys_altgr[128];
  struct xrdp_key_info keys_capslock[128];
  struct xrdp_key_info keys_shiftcapslock[128];
  struct xrdp_key_info *keys_unicode_exceptions;
  struct xrdp_key_info2 *keys_unicode_combinaisons;
  int keys_unicode_exceptions_count;
  int keys_unicode_combinaisons_count;
};


struct xrdp_key_info* APP_CC
get_key_info_from_scan_code(int device_flags, int scan_code, int* keys, int caps_lock, int num_lock, int scroll_lock, struct xrdp_keymap* keymap);
int APP_CC
get_keysym_from_scan_code(int device_flags, int scan_code, int* keys, int caps_lock, int num_lock, int scroll_lock, struct xrdp_keymap* keymap);
twchar APP_CC
get_char_from_scan_code(int device_flags, int scan_code, int* keys, int caps_lock, int num_lock, int scroll_lock, struct xrdp_keymap* keymap);
int* APP_CC
get_keysym_from_unicode(int unicode, struct xrdp_keymap* keymap);
void APP_CC
km_get_unicode_exception(int fd, struct xrdp_keymap* keymap);
struct xrdp_key_info2* APP_CC
km_get_unicode_combinaison(int fd, struct xrdp_keymap* keymap);
int APP_CC
get_unicode_exception(struct xrdp_keymap* keymap);
int APP_CC
get_keymaps(int keylayout, struct xrdp_keymap* keymap);

#endif // LANG_H
