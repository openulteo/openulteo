/**
 * Copyright (C) 2008 Ulteo SAS
 * http://www.ulteo.com
 * Author David Lechevalier <david@ulteo.com> 2010
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

#ifndef PRINTER_H_
#define PRINTER_H_

#include "log.h"
#include "arch.h"
#include "os_calls.h"
#include "parse.h"

#define PPD_FILE            "/usr/share/cups/model/PostscriptColor.ppd.gz"
#define DEVICE_URI          "xrdpprinter:"
#define SPOOL_DIR           "/var/spool/xrdp_printer/"
#define LP_SPOOL_DIR        "/var/spool/xrdp_printer/SPOOL"

int DEFAULT_CC
printer_delete_job(char* jobs);
int APP_CC
printer_add(const char* username, const char* printer_name);
int APP_CC
printer_add(const char* username, const char* printer_name);
int APP_CC
printer_init_printer_socket( char* printer_name);
int APP_CC
printer_get_printer_socket();
int APP_CC
printer_purge_all();
int APP_CC
printer_purge();

#endif /* PRINTER_H_ */
