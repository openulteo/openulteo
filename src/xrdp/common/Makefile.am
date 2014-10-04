
AM_CFLAGS = \
  -Wall \
  -DXRDP_CFG_PATH=\"${sysconfdir}/xrdp\" \
  -DXRDP_SBIN_PATH=\"${sbindir}\" \
  -DXRDP_SHARE_PATH=\"${datadir}/xrdp\" \
  -DXRDP_PID_PATH=\"${localstatedir}/run\" \
  -D_GNU_SOURCE \
  -DHAVE_ICONV \
  -DHAVE_ICONV_H 
  


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
AUTH_C = verify_user_pam.c verify_user_pam.h
AUTH_LIB = -lpam
endif
endif
endif


lib_LTLIBRARIES = \
  libxrdp_common.la

libxrdp_common_la_SOURCES = \
  xrdp_constants.h \
  arch.h defines.h parse.h \
  d3des.c d3des.h \
  file.c file.h file_loc.h \
  list.c list.h \
  fifo.c fifo.h \
  log.c log.h \
  os_calls.c os_calls.h \
  ssl_calls.c ssl_calls.h \
  thread_calls.c thread_calls.h \
  thread_pool.c thread_pool.h \
  trans.c trans.h \
  uni_rdp.c uni_rdp.h \
  cp1252_rdp.c cp1252_rdp.h \
  $(AUTH_C) 

libxrdp_common_la_LIBADD = \
  -lcrypto \
  -lpthread \
  -ldl \
  $(AUTH_LIB)
