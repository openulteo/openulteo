if SCIM_ENABLED

AM_CFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -Wall -O2 -D_REENTRANT

INCLUDES = \
  -I$(top_srcdir)/common \
  -I$(top_srcdir)/vchannel/lib

bin_PROGRAMS = \
  ukbrdr

ukbrdr_SOURCES = \
  main.c \
  proto.h \
  ukbrdr.h
  

ukbrdr_LDADD = \
  -lpthread \
  $(top_builddir)/vchannel/lib/libxrdp_vchannel.la \
  $(top_builddir)/common/libxrdp_common.la 

ukbrdrsysconfdir=$(sysconfdir)/xrdp

ukbrdrsysconf_DATA = \
  ukbrdr.conf 

EXTRA_DIST = \
  $(ukbrdrsysconf_DATA)

endif # SCIM_ENABLED
