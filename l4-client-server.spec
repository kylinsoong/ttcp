Name:           l4-client-server
Version:        1.0
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
%{_bindir}/tcp-chat-server
%{_bindir}/tcp-chat-client
%{_bindir}/tcp-echo-server
%{_bindir}/tcp-echo-client

%changelog
* Tue Dec 21 2021 Kylin Soong <kylinsoong.1214@gmail.com> - 1.0-1
- First  package
