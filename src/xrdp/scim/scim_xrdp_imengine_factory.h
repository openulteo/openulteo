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

#ifndef __SCIM_XRDP_IMENGINE_FACTORY_H__
#define __SCIM_XRDP_IMENGINE_FACTORY_H__

#define Uses_SCIM_ICONV
#include <scim.h>

using namespace scim;

class XrdpFactory : public IMEngineFactoryBase {
	friend class XrdpInstance;

	private:
		String m_uuid;

	public:
		XrdpFactory(const String &lang, const String &uuid );
		virtual ~XrdpFactory();

		virtual WideString get_name () const;
		virtual WideString get_authors() const;
		virtual WideString get_credits() const;
		virtual WideString get_help() const;
		virtual String get_uuid() const;
		virtual String get_icon_file() const;

		virtual IMEngineInstancePointer create_instance(const String& encoding, int id = -1);
};

#endif /* __SCIM_XRDP_IMENGINE_FACTORY_H__ */

