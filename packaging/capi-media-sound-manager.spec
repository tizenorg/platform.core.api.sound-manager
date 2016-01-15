Name:       capi-media-sound-manager
Summary:    Sound Manager library
Version:    0.3.41
Release:    0
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: 	capi-media-sound-manager.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(mm-session)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(libpulse)
BuildRequires:  pkgconfig(gio-2.0)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
A Sound Manager library in Tizen C API

%package devel
Summary:  Sound Manager library (Development)
Group:    Multimedia/Development
Requires: %{name} = %{version}-%{release}

%description devel
A Sound Manager library in Tizen C API (DEV)

%devel_desc

%prep
%setup -q
cp %{SOURCE1001} .

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}
%__make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/usr/bin
cp LICENSE %{buildroot}/usr/share/license/%{name}
cp test/sound_manager_test %{buildroot}/usr/bin

%make_install

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%{_libdir}/libcapi-media-sound-manager.so.*
%{_datadir}/license/%{name}
/usr/bin/sound_manager_test

%files devel
%manifest %{name}.manifest
%{_includedir}/media/sound_manager.h
%{_includedir}/media/sound_manager_internal.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-sound-manager.so
