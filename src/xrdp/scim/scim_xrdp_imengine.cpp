/**
 * Copyright (C) 2012-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2012, 2013
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

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_CONFIG_BASE

#include <scim.h>
#include "scim_xrdp_imengine_factory.h"
#include "scim_xrdp_imengine.h"
#include <syslog.h>
#include <stdarg.h>


#ifdef DEBUG
  #define log_message(args...) log_message_internal(args);
#else
  #define log_message(args...)
#endif


int log_message_internal(const char* msg, ...) {
	va_list ap;
	char buffer[2048] = {0};
	int len;

	va_start(ap, msg);
	len = vsnprintf(buffer, sizeof(buffer), msg, ap);
	va_end(ap);

	openlog("uxda-scim-engine", LOG_PID|LOG_CONS, LOG_USER);
	syslog(LOG_ERR, "%s", buffer);
	closelog();

	return 0;
}


XrdpInstance::XrdpInstance(XrdpFactory *factory, const String &encoding, int id)
:IMEngineInstanceBase(factory, encoding, id), m_factory(factory) { }

XrdpInstance::~XrdpInstance() { }

bool XrdpInstance::process_key_event(const KeyEvent& key) {
	return false; /* X handled */
}

void XrdpInstance::move_preedit_caret(unsigned int pos) {
	this->update_preedit_caret(pos);
}

void XrdpInstance::select_candidate(unsigned int item) {
	if (item == 0) {
		commit_string(m_preedit_string);
		reset();
		return;
	}

	WideString ret;
	ret.push_back(item);
	commit_string(ret);
}

void XrdpInstance::update_lookup_table_page_size(unsigned int page_size) { }
void XrdpInstance::lookup_table_page_up() { }
void XrdpInstance::lookup_table_page_down() { }


void XrdpInstance::reset() {
	m_preedit_string = WideString ();
	hide_preedit_string();
}


void XrdpInstance::focus_in() {
	if (m_preedit_string.length ()) {
		update_preedit_string (m_preedit_string);
		update_preedit_caret (m_preedit_string.length ());
		show_preedit_string ();
	}
}


void XrdpInstance::focus_out() {
	reset();
}


void XrdpInstance::trigger_property(const String &property) {
	log_message("trigger property %s", property.c_str());

	m_preedit_string = utf8_mbstowcs(property);

	show_preedit_string();
	update_preedit_string(m_preedit_string);
	update_preedit_caret (m_preedit_string.length());
}
