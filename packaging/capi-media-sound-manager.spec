Name:       capi-media-sound-manager
Summary:    Sound Manager library
Version:    0.1.0
Release:    1.1
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
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
FULLVER=%{version}
MAJORVER=`echo ${FULLVER} | cut -d '.' -f 1`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=${FULLVER} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%{_libdir}/libcapi-media-sound-manager.so*

%files devel
%{_includedir}/media/sound_manager.h
%{_libdir}/pkgconfig/*.pc


