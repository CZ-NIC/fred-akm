Name:       %{project_name}
Version:    %{our_version}
Release:    %{?our_release}%{!?our_release:1}%{?dist}
Summary:	FRED - automated keyset management client
Group:		Applications/Utils
License:	GPLv3+
URL:		http://fred.nic.cz
Source0:	%{name}-%{version}.tar.gz
Source1:    idl-%{idl_branch}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  git, gcc-c++, omniORB-devel, boost-devel, sqlite-devel
%if 0%{?el7}
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
%setup -b 1

%build
%if 0%{?el7}
%{?scl:scl enable devtoolset-7 llvm-toolset-7 - << \EOF}
%global __cmake /opt/rh/llvm-toolset-7/root/usr/bin/cmake
%endif
%cmake -DCMAKE_INSTALL_PREFIX=/ -DUSE_USR_PREFIX=1 -DVERSION=%{version} -DIDL_DIR=%{_topdir}/BUILD/idl-%{idl_branch}/idl .
%if 0%{?el7}
%make_build
%else
%cmake_build
%endif
%if 0%{?el7}
%{?scl:EOF}
%endif

%install
rm -rf $RPM_BUILD_ROOT
%if 0%{?el7}
%make_install
%else
%cmake_install
%endif
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
