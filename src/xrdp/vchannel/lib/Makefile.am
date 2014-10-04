AM_CFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" \
  -Wall

INCLUDES = \
  -I$(top_srcdir)/common

lib_LTLIBRARIES = \
  libxrdp_vchannel.la \
  libxrdp_chansrv.la

libxrdp_vchannel_la_SOURCES = \
  vchannel.h vchannel.c 

libxrdp_vchannel_la_LIBADD = \
  -lpthread \
  -ldl \
  $(top_builddir)/common/libxrdp_common.la 

libxrdp_chansrv_la_SOURCES = \
  chansrv.c chansrv.h

libxrdp_chansrv_la_LIBADD = \
  $(top_builddir)/common/libxrdp_common.la


chansrvsysconfdir=$(sysconfdir)/xrdp

chansrvsysconf_DATA = \
  vchannel.ini

EXTRA_DIST = \
  $(chansrvsysconf_DATA)
