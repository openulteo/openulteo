/**
 * FreeRDP: A Remote Desktop Protocol client.
 * Print Virtual Channel - Ulteo Open Virtual Desktop PDF printer
 *
 * Copyright 2012 Ulteo SAS http://www.ulteo.com
 *    Author: Jocelyn DELALANDE <j.delalande@ulteo.com>
 *
 * Inspired by printer_cups.h - Copyright 2010-2011 Vic Lee
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

#ifndef __PRINTER_ULTEO_PDF_H
#define __PRINTER_ULTEO_PDF_H

#include <freerdp/freerdp.h>
#include "printer_main.h"

#define FREERDP_ULTEO_SPOOL_PATH "/var/spool/ulteo/pdf-printer"
#define MAX_PATH_SIZE 300
rdpPrinterDriver* printer_ulteo_pdf_get_driver(void);

/* Dummy rdp_guac_client_data structure */
/* Must be kept in sync with the libguac-client-rdp rdp_guac_client_data */
typedef struct rdp_guac_client_data {
    freerdp* rdp_inst;
    rdpSettings* settings;
    int printjob_notif_fifo;
} rdp_guac_client_data;

#endif

