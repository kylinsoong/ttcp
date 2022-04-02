Name:           l4-client-server
Version:        1.1
Release:        1
Summary:        L4 client server service in C

License:        Apache 2.0
Source0:        https://cloudadc.github.io/%{name}/releases/%{name}-%{version}.tar.gz
URL:            https://cloudadc.github.io/%{name}

BuildRequires:  gcc
BuildRequires:  make

%description
L4 client server service in C.

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

%changelog
* Fri Apr 02 2022 Kylin Soong <kylinsoong.1214@gmail.com> - 1.1-1
- Merge chat server and client to one file
- Merge echo server and client to one file
- Initial add ttcp
- Ipv6 support in chat and echo
* Tue Dec 21 2021 Kylin Soong <kylinsoong.1214@gmail.com> - 1.0-1
- First  package
