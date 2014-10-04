# Copyright (C) 2010-2011 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2010
# Author David LECHEVALIER <david@ulteo.com> 2010-2011
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
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" \
  -DXRDP_UNICODE_EXCEPTION_FILENAME=\"unicode_exception.ini\" -g

INCLUDES = \
  -I$(top_srcdir)/common \
  -I$(top_srcdir)/libxrdp \
  -I$(top_srcdir)/userChannel \
  -I$(top_srcdir)/vchannel/lib

sbin_PROGRAMS = \
  xrdp

xrdp_SOURCES = \
  xrdp.c xrdp.h xrdp_types.h \
  xrdp_vchannel.c xrdp_vchannel.h \
  xrdp_listen.c \
  xrdp_module.c \
  xrdp_process.c

xrdp_LDADD = \
  $(top_builddir)/common/libxrdp_common.la \
  $(top_builddir)/libxrdp/libxrdp.la

xrdpsysconfdir=$(sysconfdir)/xrdp
xrdpsysconf_DATA = \
  xrdp.ini \
  rsakeys.ini

xrdppkgdatadir=$(datadir)/xrdp
xrdppkgdata_DATA = \
  ad24b.bmp \
  ad256.bmp \
  sans-10.fv1 \
  cursor0.cur \
  cursor1.cur

# must be tab below
install-data-hook:
	chmod 600 $(DESTDIR)$(sysconfdir)/xrdp/rsakeys.ini

EXTRA_DIST = \
  $(xrdppkgdata_DATA) \
  $(xrdpsysconf_DATA)
