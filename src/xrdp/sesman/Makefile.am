# Copyright (C) 2010-2013 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2010
# Author David LECHEVALIER <david@ulteo.com> 2010, 2013
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
  `xml2-config --libs`

INCLUDES = \
  -I$(top_srcdir)/common \
  -I$(top_srcdir)/sesman \
  -I$(top_srcdir)/sesman/libscp \
  `xml2-config --cflags`

if PREMIUM_ENABLED
INCLUDES += -I$(top_srcdir)/premium
AM_CFLAGS += -DCHECK_PREMIUM_EDITION @PE_CFLAGS@
AM_LDFLAGS = @PE_LDFLAGS@  `curl-config --libs`
endif

if SESMAN_NOPAM
AUTH_C = verify_user.c
AUTH_LIB = -lcrypt
else
if SESMAN_PAMUSERPASS
AUTH_C = verify_user_pam_userpass.c
AUTH_LIB = -lpam -lpam_userpass
else
if SESMAN_KERBEROS
AUTH_C = verify_user_kerberos.c
AUTH_LIB = -lkrb5
else
AUTH_C = verify_user_pam.c
AUTH_LIB = -lpam
endif
endif
endif

sbin_PROGRAMS = \
  xrdp-sesman

xrdp_sesman_SOURCES = \
  scp.c scp.h \
  scp_v0.c scp_v0.h \
  scp_v1.c scp_v1.h \
  scp_v1_mng.c scp_v1_mng.h \
  sesman.c sesman.h \
  session.c session.h \
  sig.c sig.h \
  thread.c thread.h \
  lock.c lock.h \
  access.c access.h \
  config.c config.h \
  env.c env.h \
  auth.h

xrdp_sesman_LDADD = \
  $(top_builddir)/common/libxrdp_common.la \
  $(top_builddir)/sesman/libscp/libscp.la \
  $(AUTH_LIB)

if PREMIUM_ENABLED
  xrdp_sesman_LDADD += $(top_builddir)/premium/lib_licensing.la
endif

sesmansysconfdir=$(sysconfdir)/xrdp

sesmansysconf_DATA = \
  sesman.ini \
  startwm.sh

SUBDIRS = \
  libscp \
  tools \
  sessvc

install-data-hook:
	chmod 755 $(DESTDIR)$(sysconfdir)/xrdp/startwm.sh

EXTRA_DIST = \
  $(sesmansysconf_DATA)
