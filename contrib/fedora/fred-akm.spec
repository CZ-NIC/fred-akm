Name:		fred-akm
Version:    %{our_version}
Release:    %{?our_release}%{!?our_release:1}%{?dist}
Summary:	FRED - automated keyset management client
Group:		Applications/Utils
License:	GPL
URL:		http://fred.nic.cz
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  git, gcc-c++, spdlog-devel >= 0.13, fred-idl >= 2.27, omniORB-devel, boost-devel, sqlite-devel
%if 0%{?centos}
BuildRequires:  centos-release-scl, devtoolset-7, devtoolset-7-build, llvm-toolset-7-cmake, llvm-toolset-7-build
Requires:       sqlite
%else
BuildRequires:  cmake
Requires:       sqlite-libs
%endif
Requires:       glibc, libstdc++, boost-regex, boost-program-options, omniORB, cdnskey-scanner

%description
FRED (Free Registry for Enum and Domain) is free registry system for 
managing domain registrations. This package contains cmdline script for
Automated Keyset Management feature

%prep
%setup -q

%build
%if 0%{?centos}
%{?scl:scl enable devtoolset-7 llvm-toolset-7 - << \EOF}
%endif
%cmake -DVERSION=%{version} -DIDL_DIR=%{path_to_idl} .
%make_build
%if 0%{?centos}
%{?scl:EOF}
%endif

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/fred/
mkdir -p $RPM_BUILD_ROOT/%{_sharedstatedir}/fred-akm
install contrib/fedora/fred-akm.conf $RPM_BUILD_ROOT/%{_sysconfdir}/fred/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/fred-akm
%config %{_sysconfdir}/fred/fred-akm.conf
%dir %{_sharedstatedir}/fred-akm/ 
