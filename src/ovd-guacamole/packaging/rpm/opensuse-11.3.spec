# Copyright (C) 2013-2014 Ulteo SAS
# http://www.ulteo.com
# Author Vincent ROULLIER <v.roullier@ulteo.com> 2013
# Author David PHAM-VAN <d.pham-van@ulteo.com> 2014
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

%if %{defined rhel}
%define httpd httpd
%define apachectl apachectl
%else
%define httpd apache2
%define apachectl apache2ctl
%endif


Name: ulteo-ovd-guacamole
Version: @VERSION@
Release: @RELEASE@

Summary: Ulteo Open Virtual Desktop - html5 client
License: GPL2
Group: Applications/System
Vendor: Ulteo SAS
URL: http://www.ulteo.com
Packager: Vincent Roullier <v.roullier@ulteo.com>

Source: %{name}-%{version}.tar.gz
%if %{defined rhel}
ExclusiveArch: i386 x86_64
%else
ExclusiveArch: i586 x86_64
%endif
Buildrequires: intltool
Requires: tomcat6

%description
This is a html5 based client for Ulteo OVD.


###########################################
# %package
###########################################

Summary: Ulteo Open Virtual Desktop - html5 client
Group: Applications/System
Requires: tomcat6

%description
This is a html5 based client for Ulteo OVD.

%prep
%setup -q

%build 
%if %{defined rhel}
make
%else
if [ $(getconf LONG_BIT) -eq 64 ]; then
  make LIBDIR=lib64
else
  make
fi
%endif

%install
make install DESTDIR=%{buildroot}
rm -rf %{buildroot}/usr/include
rm -rf %{buildroot}/usr/share/freerdp
rm -f %{buildroot}/usr/lib*/*.a
rm -f %{buildroot}/usr/lib*/*.la
rm -rf %{buildroot}/usr/lib*/pkgconfig
%if %{defined rhel}
mv %{buildroot}/etc/init.d/guacd.redhat %{buildroot}/etc/init.d/guacd
rm -rf %{buildroot}/srv
%else
mv %{buildroot}/etc/init.d/guacd.suse %{buildroot}/etc/init.d/guacd
mkdir -p %{buildroot}/srv/tomcat6/webapps
mv %{buildroot}/var/lib/tomcat6/webapps/*.war %{buildroot}/srv/tomcat6/webapps/
%endif
rm -f %{buildroot}/etc/init.d/guacd.*
rm -rf %{buildroot}/usr/share/man

%clean
rm -rf ${buildroot}

%post
A2CONFDIR=/etc/%{httpd}/conf.d
CONFDIR=/etc/ulteo/webclient

%if %{defined rhel}

%else
a2enmod proxy
a2enmod proxy_http
%endif

if [ ! -e $A2CONFDIR/webclient-html5.conf ]; then
    ln -sf $CONFDIR/apache2-html5.conf $A2CONFDIR/webclient-html5.conf
    if %{apachectl} configtest 2>/dev/null; then
        service %{httpd} restart || true
    else
        echo << EOF
"Your apache configuration is broken!
Correct it and restart apache."
EOF
    fi
fi

if [ -f /etc/ulteo/webclient/config.inc.php ]; then
	sed -i "s@// define('RDP_PROVIDER_HTML5_INSTALLED', true);@define('RDP_PROVIDER_HTML5_INSTALLED', true);@g" /etc/ulteo/webclient/config.inc.php
fi

ldconfig

%preun
if [ -f /etc/ulteo/webclient/config.inc.php ]; then
	sed -i "s@define('RDP_PROVIDER_HTML5_INSTALLED', true);@// define('RDP_PROVIDER_HTML5_INSTALLED', true);@g" /etc/ulteo/webclient/config.inc.php
fi

%files
%defattr(-,root,root)
%config /etc/default/*
%config /etc/init.d/*
%config /etc/ulteo/webclient/apache2-html5.conf
/usr/lib*/*
/usr/sbin/guacd
%if %{defined rhel}
/var/lib/tomcat6/webapps/guacamole.war
%else
/srv/tomcat6/webapps/guacamole.war
%endif
/usr/share/tomcat6/lib/guacamole.properties
%dir /usr/share/tomcat6

