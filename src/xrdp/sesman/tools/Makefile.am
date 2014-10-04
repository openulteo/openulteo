# Copyright (C) 2010-2011 Ulteo SAS
# http://www.ulteo.com
# Author David LECHEVALIER <david@ulteo.com> 2010
# Author Samuel BOVEE <samuel@ulteo.com> 2011
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
  -DXRDP_PID_PATH=\"${localstatedir}/run\"

INCLUDES = \
  -I$(top_srcdir)/common \
  -I$(top_srcdir)/sesman/libscp \
  -I$(top_srcdir)/sesman

bin_PROGRAMS = \
  xrdp-sesrun \
  xrdp-sestest \
  xrdp-sesadmin

xrdp_sesrun_SOURCES = \
  sesrun.c \
  tcp.c tcp.h \
  ../config.c

xrdp_sestest_SOURCES = \
  sestest.c

xrdp_sesadmin_SOURCES = \
  sesadmin.c

xrdp_sesrun_LDADD = \
  $(top_builddir)/common/libxrdp_common.la

xrdp_sestest_LDADD = \
  $(top_builddir)/common/libxrdp_common.la \
  $(top_builddir)/sesman/libscp/libscp.la

xrdp_sesadmin_LDADD = \
  $(top_builddir)/common/libxrdp_common.la \
  $(top_builddir)/sesman/libscp/libscp.la
