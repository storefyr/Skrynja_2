Name: harbour-skrynja-2
Summary: Kontrolcenter for realiseringsøkosystemet
Version: 1.0.0
Release: 1
License: GPL-3.0
URL: https://github.com/storefyr/Skrynja_2
Source0: %{name}-%{version}.tar.gz
BuildRequires: qt5-qmake
BuildRequires: qtbase5-dev
BuildRequires: qtdeclarative5-dev

%description
Skrynja_2 er en ACP-klient til Sailfish OS: den starter agenten
(vibe-acp) som underordnet proces over stdio (ACP, protocolVersion 1)
og viser værktøjskald, filændringer, tilladelser og tilstande, mens de
sker. Tilladelser besvares med et enkelt tryk.

%build
qmake harbour-skrynja-2.pro
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}

%files
%defattr(-,root,root,-)
/usr/bin/harbour-skrynja-2
/usr/share/applications/harbour-skrynja-2.desktop
/usr/share/harbour-skrynja-2/qml/
