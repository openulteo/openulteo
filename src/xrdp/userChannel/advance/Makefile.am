AM_CFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" \
  -DXRDP_UNICODE_EXCEPTION_FILENAME=\"unicode_exception.ini\" \
  -Wall

INCLUDES = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/common \
  -I$(top_srcdir)/userChannel/ip \
  -I$(top_srcdir)/userChannel/abstract \
  -I$(top_srcdir)/libxrdp

pkglib_LTLIBRARIES = \
  lib_uc_advance.la

lib_uc_advance_la_SOURCES = \
  video_detection.c \
  video_detection.h \
  progressive_display.c \
  progressive_display.h \
  quality_params.c \
  quality_params.h \
  advanced.c

lib_uc_advance_la_LIBADD = \
  $(top_builddir)/userChannel/abstract/lib_uc_abstract.a \
  $(top_builddir)/userChannel/ip/libip.a \
  $(top_builddir)/common/libxrdp_common.la

