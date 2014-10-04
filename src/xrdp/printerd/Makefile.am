AM_CFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" 

INCLUDES = \
  -I$(top_srcdir)/common \
  `xml2-config --cflags` \
  `cups-config --cflags`
  
AM_LDFLAGS = \
  `xml2-config --libs` \
  `cups-config --libs`


sbin_PROGRAMS = \
  xrdp-printerd

xrdp_printerd_SOURCES = \
  main.c printerd.h \
  xml_printer_communication.c xml_printer_communication.h \
  printer.c printer.h

xrdp_printerd_LDADD = \
  $(top_builddir)/common/libxrdp_common.la 
  


xrdp_printerdsysconfdir=$(sysconfdir)/xrdp

xrdp_printerdsysconf_DATA = \
  printerd.ini


EXTRA_DIST = \
  $(xrdp_printerdsysconf_DATA)
