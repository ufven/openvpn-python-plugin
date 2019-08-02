Name:           openvpn-python-plugin
Version:        1.0.2
Release:        1%{?dist}
Summary:        OpenVPN Python plugin

License:        GPLv2+
URL:            https://github.com/ufven/%{name}
Source0:        https://github.com/ufven/%{name}/archive/v%{version}/%{name}-%{version}.tar.gz

Requires:      openvpn
Requires:      python%{python3_pkgversion} >= 3.5

BuildRequires: openvpn-devel
BuildRequires: python%{python3_pkgversion}-devel >= 3.5

%description
An OpenVPN plugin that allows for handling events in Python.

%package devel
Summary:  OpenVPN Python plugin (devel)
Requires: %{name} = %{version}-%{release}

%description devel
An OpenVPN plugin that allows for handling events in Python (devel.)

%prep
%setup -q

%build
%configure
%make_build

%install
%make_install

%files
%doc LICENSE
%doc README.rst
%{python3_sitelib}/openvpn_plugin.py
%{python3_sitelib}/__pycache__/*
%{_libdir}/openvpn/plugins/openvpn-python-plugin.so

%files devel
%{_libdir}/openvpn/plugins/openvpn-python-plugin.la

%changelog
* Fri Aug 02 2019 Daniel Uvehag <daniel.uvehag@gmail.com> - 1.0.2-1
- Fixes C89 incompatibilities

* Mon Jul 29 2019 Daniel Uvehag <daniel.uvehag@gmail.com> - 1.0.1-1
- Update to version 1.0.1

* Mon Jul 29 2019 Daniel Uvehag <daniel.uvehag@gmail.com> - 1.0.0-1
- Initial package
