# lib3dc rpm spec file
#
%define prefix	/usr
Summary: library to control spacepilot pro
Name: libsdc
Version: 0.0.1
Release: 1
License: GPL
Group: Applications/System
Source: https://github.com/wlbaker/spacepilotpro-lcd.git
URL: https://github.com/wlbaker/spacepilotpro-lcd
Distribution: Linux
Vendor: NONE
Packager: William Baker <wlbaker@github.com>
Buildroot: /var/tmp/lib3c-%{PACKAGE_VERSION}-root
Requires: libusb
BuildRequires: libusb-devel
Provides: lib3dc

%description
lib3dc controls the LCD ofthe SpacePilot Pro

%package devel
Summary: lib3dc controls the LCD of the 3D Connexion SpacePilot Pro
Group: System Environment/Libraries
Requires: libusb
Requires: lib3dc
Provides: lib3dc-devel

%description devel
lib3dc controls the LCD display of the SpacePilot Pro

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT
%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files 
%defattr(-, root, root)

%doc AUTHORS COPYING NEWS README
%{prefix}/lib/lib*.la
%{prefix}/lib/lib3dc.so*

%files devel
%defattr(-, root, root)

%doc AUTHORS COPYING NEWS README
%{prefix}/lib/lib*.a
%{prefix}/include/*
