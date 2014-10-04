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

#ifndef PRINTERD_H_
#define PRINTERD_H_

#include "string.h"
#include "xrdp_constants.h"
#include "xml_printer_communication.h"
#include "thread_calls.h"
#include "thread_pool.h"
#include "file.h"
#include "log.h"


/* config constant */
#define PRINTERD_CFG_GLOBAL                        "Globals"
#define PRINTERD_CFG_GLOBAL_THREAD_COUNT           "PrinterThreadCount"
#define PRINTERD_CFG_NAME                          "Name"
#define PRINTERD_CFG_LOGGING                       "Logging"
#define PRINTERD_CFG_LOG_DIR                       "LogDir"
#define PRINTERD_CFG_LOG_LEVEL                     "LogLevel"
#define PRINTERD_CFG_LOG_ENABLE_SYSLOG             "EnableSyslog"
#define PRINTERD_CFG_LOG_SYSLOG_LEVEL              "SyslogLevel"



#endif /* PRINTERD_H_ */
