# Copyright (C) 2010-2011 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2010-2011
# Author David LECHEVALIER <david@ulteo.com> 2010-2011
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

SUBDIRS = \
  init \
  pam.d \
  Xserver

kminidir=$(sysconfdir)/xrdp
kmini_DATA = \
  unicode_exception.ini \
  km-0401.ini \
  km-0406.ini \
  km-0407.ini \
  km-0409.ini \
  km-040a.ini \
  km-040b.ini \
  km-040c.ini \
  km-0410.ini \
  km-0411.ini \
  km-0414.ini \
  km-0415.ini \
  km-0416.ini \
  km-0419.ini \
  km-041a.ini \
  km-041d.ini \
  km-041f.ini \
  km-0424.ini \
  km-0426.ini \
  km-0427.ini \
  km-042f.ini \
  km-0809.ini \
  km-080c.ini \
  km-0816.ini


switchresolutiondir=$(sbindir)
switchresolution_DATA = \
  xrdp-switch-resolution

install-data-hook:
	chmod 755 $(DESTDIR)$(sbindir)/xrdp-switch-resolution

EXTRA_DIST = \
  $(switchresolution_DATA) \
  $(kmini_DATA)

