# Copyright (C) 2010-2013 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2010-2011
# Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
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

%define php_bin %(basename `php-config --php-binary`)
%if %{defined rhel}
%define datadir %{pecl_datadir}
%else
%define datadir /usr/share/%{php_bin}
%endif

Name: php-libchart
Version: @VERSION@
Release: @RELEASE@

Summary: Simple PHP chart drawing library
License: GPL2
Group: Applications/web
Vendor: Ulteo SAS
Packager: David PHAM-VAN <d.pham-van@ulteo.com>
URL: http://www.ulteo.com

Source: %{name}-%{version}.tar.gz
BuildArch: noarch
Buildroot: %{buildroot}
Requires: %{php_bin}, %{php_bin}-gd
Provides: php5-libchart

%description
Libchart is a free chart creation PHP library, that is easy to use.

%prep
%setup -q

%install
PHPDIR=%{buildroot}%{datadir}
LIBCHARTDIR=$PHPDIR/libchart
mkdir -p $PHPDIR
cp -r libchart $PHPDIR
cp -r demo $LIBCHARTDIR

rmdir $LIBCHARTDIR/demo/generated
rm -rf $LIBCHARTDIR/images
rm $LIBCHARTDIR/COPYING $LIBCHARTDIR/ChangeLog $LIBCHARTDIR/README

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc libchart/COPYING
%doc libchart/ChangeLog
%doc libchart/README
/usr

%changelog
* Wed Sep 20 2011 Samuel Bovée <samuel@ulteo.com> 1.3
- Initial release
