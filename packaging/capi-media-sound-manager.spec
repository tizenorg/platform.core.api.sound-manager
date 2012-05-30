Name:       capi-media-sound-manager
Summary:    Sound Manager library
Version:    0.1.0
Release:    8
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: packaging/capi-media-sound-manager.manifest 
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-sound)
BuildRequires:  pkgconfig(mm-session)
BuildRequires:  pkgconfig(capi-base-common)
Requires(post): /sbin/ldconfig  
Requires(postun): /sbin/ldconfig

%description
A Sound Manager library in Tizen Native API

%package devel
Summary:  Sound Manager library (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel
A Sound Manager library in Tizen Native API (DEV)

%prep
%setup -q


%build
cp %{SOURCE1001} .
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest capi-media-sound-manager.manifest
%{_libdir}/libcapi-media-sound-manager.so.*

%files devel
%manifest capi-media-sound-manager.manifest
%{_includedir}/media/sound_manager.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-sound-manager.so


