xserverdir=$(sysconfdir)/xrdp/Xserver

xserver_DATA = \
  rgb.txt \
  SecurityPolicy

xserverbindir=$(bindir)/

xserverbin_DATA = \
  xrdp-logoff \
  xrdp-disconnect \
  seamlessrdpshell \
  startapp


install-data-hook:
	chmod 755 $(DESTDIR)$(bindir)/seamlessrdpshell
	chmod 755 $(DESTDIR)$(bindir)/xrdp-logoff
	chmod 755 $(DESTDIR)$(bindir)/xrdp-disconnect
	chmod 755 $(DESTDIR)$(bindir)/startapp

EXTRA_DIST = \
  $(xserverbin_DATA) \
  $(xserver_DATA)
