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

#ifndef __SCIM_XRDP_IMENGINE_H__
#define __SCIM_XRDP_IMENGINE_H__

#include <scim.h>

using namespace scim;

class XrdpInstance : public IMEngineInstanceBase {
	friend class XrdpFactory;

	private:
		XrdpFactory *m_factory;
	    WideString  m_preedit_string;

	public:
		XrdpInstance(XrdpFactory *factory, const String &encoding, int id = -1);
		virtual ~XrdpInstance();

		virtual bool process_key_event(const KeyEvent& key);
		virtual void move_preedit_caret(unsigned int pos);
		virtual void select_candidate(unsigned int item);
		virtual void update_lookup_table_page_size(unsigned int page_size);
		virtual void lookup_table_page_up(void);
		virtual void lookup_table_page_down(void);
		virtual void reset(void);
		virtual void focus_in(void);
		virtual void focus_out(void);
		virtual void trigger_property(const String &property);
};
#endif /* __SCIM_XRDP_IMENGINE_H__ */

