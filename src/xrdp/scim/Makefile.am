# Copyright (C) 2012 Ulteo SAS
# http://www.ulteo.com
# Author Alexandre CONFIANT-LATOUR <a.confiant@ulteo.com> 2012
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2
# of the License
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

if SCIM_ENABLED

AM_CPPFLAGS = \
	-DSCIM_XRDP_LOCALEDIR=\"$(datadir)/locale\" \
	-DSCIM_ICONDIR=\"$(datarootdir)/scim/icons\" \
	@SCIM_CFLAGS@

xrdp_la_CFLAGS = @SCIM_CFLAGS@
xrdp_la_CXXFLAGS = @SCIM_CFLAGS@
xrdp_la_LDFLAGS = @SCIM_LDFLAGS@ \
	-avoid-version \
	-rpath $(moduledir) \
	-module

xrdp_la_SOURCES = \
	scim_xrdp_imengine_factory.h \
	scim_xrdp_imengine_factory.cpp \
	scim_xrdp_imengine.h \
	scim_xrdp_imengine.cpp

xrdp_scim_panel_CFLAGS = @SCIMGTKUTILS_CFLAGS@
xrdp_scim_panel_CXXFLAGS = @SCIMGTKUTILS_CFLAGS@
xrdp_scim_panel_LDFLAGS = @SCIMGTKUTILS_LDFLAGS@

xrdp_scim_panel_SOURCES = \
  proto.h \
  scim_panel.cpp

xrdp_scim_panel_LDADD = \
  $(top_builddir)/common/libxrdp_common.la

moduledir = $(libdir)/scim-1.0/1.4.0/IMEngine
module_LTLIBRARIES = \
  xrdp.la

moduleconfdir=$(sysconfdir)/xrdp/scim
moduleconf_DATA = \
  config \
  global

moduledatadir=$(datarootdir)/scim/icons
moduledata_DATA = \
  scim-xrdp.png

bin_PROGRAMS = \
  xrdp-scim-panel

EXTRA_DIST = \
  $(module_LTLIBRARIES) \
  $(moduleconf_DATA) \
  $(moduledata_DATA)

endif # SCIM_ENABLED
