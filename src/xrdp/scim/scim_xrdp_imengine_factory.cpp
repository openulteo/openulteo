/**
 * Copyright (C) 2012 Ulteo SAS
 * http://www.ulteo.com
 * Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2012
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

/* SCIM Related Definitions */
#define scim_module_init                    xrdp_LTX_scim_module_init
#define scim_module_exit                    xrdp_LTX_scim_module_exit
#define scim_imengine_module_init           xrdp_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory xrdp_LTX_scim_imengine_module_create_factory

#ifndef SCIM_XRDP_ICON_FILE
#define SCIM_XRDP_ICON_FILE           (SCIM_ICONDIR"/scim-xrdp.png")
#endif

extern "C" {
	void scim_module_init(void) { }
	void scim_module_exit(void) { }

	uint32 scim_imengine_module_init(const ConfigPointer &config) {
		return 1;
	}

	IMEngineFactoryPointer scim_imengine_module_create_factory(uint32 engine) {
		XrdpFactory *factory = 0;

		try {
			factory = new XrdpFactory(String("Other"), String("1467cae6-4f06-ec08-0304-07f6c6fbe9c9"));
		}
		catch (...) {
			delete factory;
			factory = 0;
		}

		return factory;
	}
}

XrdpFactory::XrdpFactory(const String &lang, const String &uuid)
: m_uuid (uuid) {
	set_languages(lang);
}

XrdpFactory::~XrdpFactory() { }

WideString XrdpFactory::get_name() const {
	return utf8_mbstowcs(String ("Xrdp"));
}

WideString XrdpFactory::get_authors() const {
	return WideString();
}

WideString XrdpFactory::get_credits() const {
	return WideString();
}

WideString XrdpFactory::get_help() const {
	return WideString();
}

String XrdpFactory::get_uuid() const {
	return m_uuid;
}

String XrdpFactory::get_icon_file() const {
	return String(SCIM_XRDP_ICON_FILE);
}

IMEngineInstancePointer XrdpFactory::create_instance(const String &encoding, int id) {
	return new XrdpInstance(this, encoding, id);
}

