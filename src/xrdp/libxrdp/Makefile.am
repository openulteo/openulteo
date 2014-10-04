
AM_CFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" \
  -ljpeg

INCLUDES = \
  -I$(top_srcdir)/common

pkglib_LTLIBRARIES = \
  libxrdp.la

libxrdp_la_SOURCES = \
  libxrdp.c libxrdp.h libxrdpinc.h\
  mppc_enc.h mppc_enc.c\
  xrdp_image.c \
  xrdp_channel.c \
  xrdp_iso.c \
  xrdp_mcs.c \
  xrdp_orders.c \
  xrdp_rdp.c \
  xrdp_sec.c \
  xrdp_tcp.c \
  xrdp_emt.c \
  xrdp_qos.c \
  xrdp_bitmap_compress.c

libxrdp_la_LIBADD = \
  $(top_builddir)/common/libxrdp_common.la
