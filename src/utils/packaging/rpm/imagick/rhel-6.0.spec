# Copyright (C) 2008-2013 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2011
# Author Remi Collet <rpms@famillecollet.com> 2008
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

%define php_ver %((echo %{default_apiver}; php -i 2>/dev/null | sed -n 's/^PHP Version => //p') | tail -1)
%define php_bin %(basename `php-config --php-binary`)
%if %{defined rhel}
%define php_etc php.d
%else
%define php_etc php5/conf.d
%endif
%{!?php_extdir: %{expand: %%global php_extdir %(php-config --extension-dir)}}

%define pecl_name imagick
%define pecl_xmldir /usr/share/doc/packages/
# maybe not the good folder ! or remove it ?

Summary:       Extension to create and modify images using ImageMagick
Name:          php-imagick
Version:       @VERSION@
Release:       @RELEASE@
License:       PHP
Group:         Development/Languages
Vendor:        Ulteo SAS <http://www.ulteo.com>
Packager:      David PHAM-VAN <d.pham-van@ulteo.com>

Source:        %{name}-%{version}.tar.gz
BuildRoot:     %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: %{php_bin}-devel >= 5.1.3, %{php_bin}-pear, ImageMagick-devel >= 6.2.4

%if %{?php_zend_api}0
Requires:      php(zend-abi) = %{php_zend_api}
Requires:      php(api) = %{php_core_api}
%else
Requires:      php = %{php_ver}
%endif
Provides:      php-pecl(%{pecl_name}) = %{version}, php5-imagick


%description
Imagick is a native php extension to create and modify images
using the ImageMagick API.


%prep
%setup -q -c
cd %{pecl_name}-%{version}


%build
cd %{pecl_name}-%{version}
%{_bindir}/phpize
%configure --with-imagick=%{prefix}
%{__make} %{?_smp_mflags}


%install
pushd %{pecl_name}-%{version}
%{__rm} -rf %{buildroot}
%{__make} install INSTALL_ROOT=%{buildroot}

# Drop in the bit of configuration
%{__mkdir_p} %{buildroot}%{_sysconfdir}/%{php_etc}
%{__cat} > %{buildroot}%{_sysconfdir}/%{php_etc}/%{pecl_name}.ini << 'EOF'
; Enable %{pecl_name} extension module
extension = %{pecl_name}.so

; Option not documented
imagick.locale_fix=0
EOF

popd
# Install XML package description
%{__mkdir_p} %{buildroot}/%{pecl_xmldir}
install -pm 644 package.xml %{buildroot}/%{pecl_xmldir}/%{name}.xml


%if 0%{?pecl_install:1}
%post
%{pecl_install} %{pecl_xmldir}/%{name}.xml >/dev/null || :
%endif


%if 0%{?pecl_uninstall:1}
%postun
if [ $1 -eq 0 ] ; then
    %{pecl_uninstall} %{pecl_name} >/dev/null || :
fi
%endif


%clean
%{__rm} -rf %{buildroot}


%files
%defattr(-, root, root, 0755)
%doc %{pecl_name}-%{version}/CREDITS %{pecl_name}-%{version}/TODO
%doc %{pecl_name}-%{version}/examples
%config(noreplace) %{_sysconfdir}/%{php_etc}/%{pecl_name}.ini
%{php_extdir}/%{pecl_name}.so
%{pecl_xmldir}/%{name}.xml
/usr/include/%{php_bin}/ext/imagick/*.h


%changelog
* Thu Nov 25 2010 Samuel Bovee <samuel@ulteo.com> 3.0.1-1
* Tue Sep 02 2010 Samuel Bovee <samuel@ulteo.com> 3.0.0-1
* Mon Aug 09 2010 Samuel Bovée <samuel@ulteo.com> 2.3.0-1
* Sat Dec 13 2008 Remi Collet <rpms@famillecollet.com> 2.2.1-1.fc#.remi.1
- rebuild with php 5.3.0-dev
- add imagick-2.2.1-php53.patch

* Sat Dec 13 2008 Remi Collet <rpms@famillecollet.com> 2.2.1-1
- update to 2.2.1

* Sat Jul 19 2008 Remi Collet <rpms@famillecollet.com> 2.2.0-1.fc9.remi.1
- rebuild with php 5.3.0-dev

* Sat Jul 19 2008 Remi Collet <rpms@famillecollet.com> 2.2.0-1
- update to 2.2.0

* Thu Apr 24 2008 Remi Collet <rpms@famillecollet.com> 2.1.1-1
- Initial package

