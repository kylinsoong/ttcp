Name:           ttcp
Version:        1.13
Release:        3
Summary:        Test TCP.

License:        Apache 2.0
Source0:        https://cloudadc.github.io/%{name}/releases/%{name}-%{version}.tar.gz
URL:            https://cloudadc.github.io/%{name}

BuildRequires:  gcc
BuildRequires:  make

%description
Test TCP in C.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
%make_install

%files
%{_bindir}/chat
%{_bindir}/echoS
%{_bindir}/ttcp
%{_bindir}/bancs
/etc/ttcp.conf
/etc/bancs.data

%changelog
* Sun Sep 25 2022 Kylin Soong <kylinsoong.1214@gmail.com> - 1.13-1
- add core banking bancs card demostration project
* Mon Apr 24 2022 Kylin Soong <kylinsoong.1214@gmail.com> - 1.12-1
- use the sub project ttcp as release name
- add ttcp.conf for holding global configs
* Fri Apr 02 2022 Kylin Soong <kylinsoong.1214@gmail.com> - 1.1-1
- Merge chat server and client to one file
- Merge echo server and client to one file
- Initial add ttcp
- Ipv6 support in chat and echo
* Tue Dec 21 2021 Kylin Soong <kylinsoong.1214@gmail.com> - 1.0-1
- First  package
