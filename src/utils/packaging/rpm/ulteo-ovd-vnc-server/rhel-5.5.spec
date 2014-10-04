Name: ulteo-ovd-vnc-server
Version: @VERSION@
Release: @RELEASE@

Summary: Virtual network computing server software
License: GPL2
Group: Applications/x11
Vendor: Ulteo SAS
Packager: David PHAM-VAN <d.pham-van@ulteo.com>
URL: http://www.ulteo.com
Conflicts: tigervnc-server

Source: %{name}-%{version}.tar.gz
Buildroot: %{buildroot}

%description
VNC stands for Virtual Network Computing. It is, in essence, a remote
display system which allows you to view a computing `desktop' environment
not only on the machine where it is running, but from anywhere on the
Internet and from a wide variety of machine architectures.

This package provides a standalone vncserver to which X clients can connect.
The server generates a display that can be viewed with a vncviewer.

Note: This server does not need a display. You need a vncviewer to see
something. This viewer may also be on a computer running other operating
systems.

%prep

%setup

%install
install -D bin/xovdvnc %{buildroot}%{_bindir}/xovdvnc
mkdir -p %{buildroot}%{_libdir}/ovdvnc_dri
install lib/dri/* %{buildroot}%{_libdir}/ovdvnc_dri/
ln -s /usr/bin/xovdvnc %buildroot/usr/bin/Xvnc

%clean
%{__rm} -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/*
%{_libdir}/*
