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

#ifndef XML_PRINTER_COMMUNICATION_H_
#define XML_PRINTER_COMMUNICATION_H_

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>

#include <log.h>
#include <os_calls.h>
#include <parse.h>
#include <string.h>

#include "xrdp_constants.h"
#include "printer.h"

#define REQUEST_ACTION_XPATH            "/request/@action"
#define REQUEST_TYPE_XPATH              "/request/@type"
#define PRINTER_NAME_XPATH              "/request/@printer"
#define USER_NAME_XPATH                 "/request/@username"


int
xml_printerd_process_request(int client);


#endif /* XML_PRINTER_COMMUNICATION_H_ */
