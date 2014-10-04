/**
 * Copyright (C) 2008 Ulteo SAS
 * http://www.ulteo.com
 * Author David LECHEVALIER <david@ulteo.com> 2010
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

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>

#include "xrdp_constants.h"
#include "log.h"
#include "parse.h"
#include "os_calls.h"

#define XML_RESPONSE_PATH             "/response/text()"
#define XML_ERROR_PATH                "/error/text()"

int DEFAULT_CC
printer_init();
int DEFAULT_CC
printer_dinit();
int DEFAULT_CC
printer_add(const char* username, const char* printer);
int DEFAULT_CC
printer_del(const char* username, const char* printer);
int DEFAULT_CC
printer_purge(const char* username);
char* DEFAULT_CC
printer_convert_name(const char *name);


#endif /* PRINTER_H_ */
