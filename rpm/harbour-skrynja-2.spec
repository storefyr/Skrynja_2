Name: harbour-skrynja-2
Summary: Kontrolcenter for realiseringsøkosystemet
Version: 1.0.0
Release: 1
License: GPL-3.0
URL: https://github.com/storefyr/Skrynja_2
Source0: %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)

%description
Skrynja_2 er en ACP-klient til Sailfish OS: den starter agenten
(vibe-acp) som underordnet proces over stdio (ACP, protocolVersion 1)
og viser værktøjskald, filændringer, tilladelser og tilstande, mens de
sker. Tilladelser besvares med et enkelt tryk.

%prep
%setup -q

%build
%qmake5 harbour-skrynja-2.pro
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}

%files
%defattr(-,root,root,-)
/usr/bin/harbour-skrynja-2
/usr/share/applications/harbour-skrynja-2.desktop
/usr/share/harbour-skrynja-2/qml/
%{_datadir}/icons/hicolor/*/apps/%{name}.png
