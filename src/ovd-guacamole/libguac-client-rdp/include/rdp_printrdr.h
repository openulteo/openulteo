/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 * Jocelyn Delalande <j.delalande@ulteo.com>.
 * Portions created by the Initial Developer are Copyright (C) 2012
 * Ulteo SAS <http://www.ulteo.com>.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __GUAC_RDP_RDP_PRINTRDR_H
#define __GUAC_RDP_RDP_PRINTRDR_H

#include <freerdp/freerdp.h>


#define MAX_PDF_PRINTJOB_NAME_LEN 200

/** 
 * From the client Point of view, the HTTP URL where it can get a printjob
 *
 * The job is served via the guacamole GuacamolePrinterServlet.""
 */
#define GUAC_PDF_PRINTER_JOB_URL "/printer/job/%s/get\n"

/**
 * Guacamole message to give to the client the URL of the PDF job freshly created.
 */
void guac_rdp_process_printing_notification(guac_client* client, int fd);

/**
 * Reads from the FreeRDP-fed fifo and sends a guacamole message to notify browser.
 */
void guac_rdp_process_printing_notification(guac_client* client, int fd);

#endif // __GUAC_RDP_RDP_PRINTRDR_H
