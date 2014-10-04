# Copyright (C) 2010-2013 Ulteo SAS
# http://www.ulteo.com
# Author Samuel BOVEE <samuel@ulteo.com> 2010-2011
# Author David PHAM-VAN <d.pham-van@ulteo.com> 2012-2013
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

Name: uxda-server
Version: @VERSION@
Release: @RELEASE@

Summary: RDP server for Linux
License: GPL2
Group: Applications/System
Vendor: Ulteo SAS
URL: http://www.ulteo.com
Packager: David PHAM-VAN <d.pham-van@ulteo.com>

Source: %{name}-%{version}.tar.gz
%if %{defined rhel}
ExclusiveArch: i386 x86_64
BuildRequires: libtool, gcc, libxml2-devel, xorg-x11-server-devel, openssl-devel, pam-devel, pulseaudio-libs-devel, cups-devel, fuse-devel, libXfixes-devel, libtool-ltdl-devel
Requires: python, ulteo-ovd-vnc-server, cups-libs, libcom_err, libgcrypt, gnutls, krb5-libs, pam, openssl, xorg-x11-server-Xorg, xorg-x11-server-utils, libxml2, zlib, lsb
%else
ExclusiveArch: i586 x86_64
BuildRequires: libtool, gcc, libxml2-devel, xorg-x11-libX11-devel, xorg-x11-libXfixes-devel, openssl-devel, pam-devel, pulseaudio-devel, cups-devel, fuse-devel, scim-devel
Requires: python, ulteo-ovd-vnc-server, cups-libs, libcom_err2, libgcrypt11, libgnutls26, krb5, pam, libopenssl1_0_0, xorg-x11-libX11, libxml2, zlib, xkeyboard-config
# xkeyboard-config [Bug 730027] New: xorg-x11-Xvnc: should depend on xkeyboard-config
%endif
Conflicts: xrdp

%description
UXDA Server is a RDP server for Linux. It provides remote display of a desktop and
many other features such as:
 * seamless display
 * printer and local device mapping

%changelog
* Wed Dec 19 2012 David PHAM-VAN <d.pham-van@ulteo.com> 99.99
- Initial release

%prep
%setup -q

%build
ARCH=$(getconf LONG_BIT)
if [ "$ARCH" = "32" ]; then
    LIBDIR=/usr/lib
elif [ "$ARCH" = "64" ]; then
    LIBDIR=/usr/lib64
fi
%if %{defined rhel}
./configure --mandir=/usr/share/man --libdir=$LIBDIR --disable-scim
%else
./configure --mandir=/usr/share/man --libdir=$LIBDIR
%endif
make -j 4
%{__python} setup.py build

%install
ARCH=$(getconf LONG_BIT)
if [ "$ARCH" = "32" ]; then
    LIBDIR=/usr/lib
elif [ "$ARCH" = "64" ]; then
    LIBDIR=/usr/lib64
fi
rm -fr %{buildroot}
make install DESTDIR=%{buildroot}
%{__python} setup.py install --prefix=%{_prefix} --root=%{buildroot} --record=INSTALLED_FILES
mkdir -p %{buildroot}/var/log/xrdp %{buildroot}/var/spool/xrdp
%if %{defined rhel}
install -D instfiles/init/redhat/uxda-server %{buildroot}/etc/init.d/uxda-server
install -D instfiles/pam.d/xrdp-sesman.rhel  %{buildroot}/etc/pam.d/xrdp-sesman
mv %{buildroot}/etc/asound.conf %{buildroot}/etc/xrdp/
%else
install -D instfiles/init/suse/uxda-server %{buildroot}/etc/init.d/uxda-server
install -D instfiles/pam.d/xrdp-sesman.suse  %{buildroot}/etc/pam.d/xrdp-sesman
mkdir -p %{buildroot}/etc/ld.so.conf.d
echo "$LIBDIR/%{name}" > %{buildroot}/%{_sysconfdir}/ld.so.conf.d/%{name}.conf
%endif

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%config /etc/xrdp/xrdp-log.conf
%config /etc/xrdp/startwm.sh
%config /etc/xrdp/*.ini
%config /etc/xrdp/Xserver/*
%config /etc/pam.d/*
%config /etc/init.d/*
%if %{undefined rhel}
# Hack because opensuse is unable to use the Xrdp rpath attribute correctly on OpenSUSE...
#  http://en.opensuse.org/openSUSE:Packaging_checks#Beware_of_Rpath
%config %{_sysconfdir}/ld.so.conf.d/%{name}.conf
%endif
/usr/lib*/%{name}/libmc.so*
/usr/lib*/%{name}/librdp.so*
/usr/lib*/%{name}/libscp.so*
/usr/lib*/%{name}/libvnc.so*
/usr/lib*/%{name}/libxrdp.so*
/usr/lib*/%{name}/libxup.so*
/usr/lib*/%{name}/lib_uc_proxy.so*
/usr/lib*/%{name}/lib_uc_advance.so*
/usr/lib*/*.so*
/usr/sbin/xrdp*
/usr/share/xrdp/*
/usr/bin/xrdp-disconnect
/usr/bin/xrdp-genkeymap
/usr/bin/xrdp-keygen
/usr/bin/xrdp-logoff
/usr/bin/xrdp-sesadmin
/usr/bin/xrdp-sesrun
/usr/bin/xrdp-sestest
%doc /usr/share/man/man1/xrdp-*.1.gz
%doc /usr/share/man/man5/*
%doc /usr/share/man/man8/*
%dir /var/log/xrdp
%dir /var/spool/xrdp


%post
getent group tsusers >/dev/null || groupadd tsusers
chgrp tsusers /var/spool/xrdp

ldconfig
chkconfig --add uxda-server > /dev/null
service uxda-server start


%preun
service uxda-server stop
chkconfig --del uxda-server > /dev/null


%postun
rm -rf /var/log/xrdp /var/spool/xrdp
if [ "$1" = "0" ]; then
    getent group tsusers >/dev/null && groupdel tsusers
fi

ldconfig


###########################################
%package seamrdp
###########################################

Summary: Seamless UXDA Shell
Group: Applications/System
%if %{defined rhel}
Requires: uxda-server, xorg-x11-server-Xorg, xfwm4
%else
Requires: uxda-server, xorg-x11-libX11
%endif


%description seamrdp
Seamlessrdpshell is a rdp addon offering the possibility to have an
application without a desktop


%files seamrdp
%defattr(-,root,root)
%config /etc/xrdp/seamrdp.conf
/usr/bin/seamlessrdpshell
/usr/bin/startapp
/usr/bin/XHook
%doc /usr/share/man/man1/seamlessrdpshell.1.gz
%doc /usr/share/man/man1/startapp.1.gz
%doc /usr/share/man/man1/XHook.1.gz


###########################################
%package rdpdr
###########################################

Summary: UXDA disks redirection
Group: Applications/System
Requires: uxda-server, fuse, libxml2


%description rdpdr
UXDA channel that handle disks redirection.


%files rdpdr
%defattr(-,root,root)
%config /etc/xrdp/rdpdr.conf
/usr/sbin/vchannel_rdpdr
%doc /usr/share/man/man1/rdpdr_disk.1.gz
%doc /usr/share/man/man1/vchannel_rdpdr.1.gz


%post rdpdr
grep -q -E "^ *[^#] *user_allow_other *" /etc/fuse.conf 2>/dev/null
if [ $? -ne 0 ]; then
	echo "user_allow_other" >> /etc/fuse.conf
fi


###########################################
%package clipboard
###########################################

Summary: UXDA clipboard
Group: Applications/System
%if %{defined rhel}
Requires: uxda-server, xorg-x11-server-Xorg
%else
Requires: uxda-server, xorg-x11-libX11
%endif


%description clipboard
UXDA channel providing copy/past text functionnality.


%files clipboard
%defattr(-,root,root)
%config /etc/xrdp/cliprdr.conf
/usr/sbin/vchannel_cliprdr


###########################################
%package sound
###########################################

Summary: UXDA plugin for PulseAudio
Group: Applications/System
%if %{defined rhel}
Requires: uxda-server, pulseaudio, alsa-utils, alsa-plugins-pulseaudio
%else
Requires: uxda-server, pulseaudio, alsa-utils, libasound2
%endif


%description sound
This package contains the UXDA plugin for PulseAudio, a sound server for POSIX
and WIN32 systems

%files sound
%defattr(-,root,root)
%if %{defined rhel}
%config /etc/xrdp/asound.conf
%else
%config /etc/asound.conf
%endif
%config /etc/xrdp/rdpsnd.*
/usr/sbin/vchannel_rdpsnd

%post sound
%if %{defined rhel}
[ -f /etc/asound.conf ] && cp /etc/asound.conf /etc/asound.conf.BACK
cp /etc/xrdp/asound.conf /etc/asound.conf
%endif

###########################################
%package printer
###########################################

Summary: cups file converter to ps format
Group: Applications/System
Requires: uxda-server-rdpdr, python, ghostscript, cups


%description printer
UXDA-Printer convert a ps file from cups in ps


%files printer
%defattr(-,root,root)
%config /etc/cups/xrdp_printer.conf
/usr/lib*/cups/backend/xrdpprinter
/usr/share/cups/model/PostscriptColor.ppd.gz
%doc /usr/share/man/man1/rdpdr_printer.1.gz
%defattr(-,lp,root)
%dir /var/spool/xrdp_printer/SPOOL


###########################################
%package python
###########################################

Summary: Python API for UXDA
Group: Applications/System
Requires: uxda-server, python


%description python
UXDA-Python is a Python wrapper for UXDA


%files -f INSTALLED_FILES python
%defattr(-,root,root)


###########################################
%package devel
###########################################

Summary: Developpement files for UXDA
Group: Development
Requires: uxda-server

# TODO: headers missing


%description devel
Developpement files for UXDA


%files devel
%defattr(-,root,root)
/usr/lib*/*.a
/usr/lib*/*.la
/usr/lib*/*.so
/usr/lib*/%{name}/*.a
/usr/lib*/%{name}/*.la
/usr/lib*/%{name}/*.so


%if %{undefined rhel}
###########################################
%package scim
###########################################

Summary: UXDA Unicode input method
Group: Applications/System
Requires: scim


%description scim
UXDA-Scim provides unicode input support for UXDA using Scim


%files scim
%defattr(-,root,root)
/usr/share/scim/*
/usr/lib*/scim-1.0/*
%config /etc/xrdp/scim/*
%config /etc/xrdp/ukbrdr.conf
/usr/bin/xrdp-scim-panel
/usr/bin/ukbrdr
%endif
