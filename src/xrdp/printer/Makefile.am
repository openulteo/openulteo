# Copyright (C) 2010 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2010
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

printerconfdir=$(sysconfdir)/cups
printerconf_DATA=xrdp_printer.conf

printerlibdir=$(libdir)/cups/backend
printerlib_DATA=xrdpprinter

printermodeldir=$(datadir)/cups/model/
printermodel_DATA=PostscriptColor.ppd.gz

install-data-hook:
	chmod +x $(DESTDIR)$(libdir)/cups/backend/xrdpprinter
	mkdir -p $(DESTDIR)$(localstatedir)/spool/xrdp_printer/SPOOL

EXTRA_DIST = \
  $(printerconf_DATA) \
  $(printerlib_DATA) \
  $(printermodel_DATA)
