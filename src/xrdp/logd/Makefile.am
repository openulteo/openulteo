# Copyright (C) 2010 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2010
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

AM_CFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -Wall -O2 -D_REENTRANT \
  -DXRDP_PID_PATH=\"${localstatedir}/run\"

INCLUDES = \
  -I$(top_srcdir)/common

sbin_PROGRAMS = \
  xrdp-logd

xrdp_logd_SOURCES = \
  main.c

xrdp_logd_LDADD = \
  -lpthread \
  $(top_builddir)/common/libxrdp_common.la

xrdp_logd_confdir=$(sysconfdir)/xrdp
xrdp_logd_conf_DATA = \
  xrdp-log.conf

install-exec-hook:
	chmod 755 $(DESTDIR)$(sbindir)/xrdp-logd

EXTRA_DIST = \
  $(xrdp_logd_conf_DATA)

