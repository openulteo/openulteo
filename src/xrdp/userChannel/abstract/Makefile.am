AM_CFLAGS = \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" \
  -DXRDP_UNICODE_EXCEPTION_FILENAME=\"unicode_exception.ini\" \
  -Wall -fPIC

INCLUDES = \
  -I$(top_srcdir) \
  -I$(top_srcdir)/common \
  -I$(top_srcdir)/libxrdp \
  -I$(top_srcdir)/vchannel/lib \
  -I$(top_srcdir)/userChannel/ip

lib_LIBRARIES = \
  lib_uc_abstract.a

lib_uc_abstract_a_SOURCES = \
  abstract.h \
  update_order.h \
  xrdp_module.h \
  funcs.c funcs.h \
  graphics.c graphics.h \
  xrdp_bitmap.c xrdp_bitmap.h \
  xrdp_region.c xrdp_region.h \
  xrdp_painter.c xrdp_painter.h \
  xrdp_wm.c xrdp_wm.h \
  xrdp_mm.c xrdp_mm.h \
  xrdp_cache.c xrdp_cache.h \
  xrdp_font.c xrdp_font.h \
  xrdp_login_wnd.c xrdp_login_wnd.h \
  lang.c lang.h \
  xrdp_screen.c xrdp_screen.h \
  userChannel.c  userChannel.h


