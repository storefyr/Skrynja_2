TARGET = harbour-skrynja-2

QT += core gui qml quick

CONFIG += c++11

SOURCES += \
    kilder/main.cpp \
    kilder/logfile.cpp \
    kilder/acpframereader.cpp \
    kilder/acpmessage.cpp \
    kilder/agentprocess.cpp \
    kilder/acpprotokol.cpp \
    kilder/acpsession.cpp \
    kilder/acpclient.cpp \
    kilder/conversationmodel.cpp \
    kilder/sessionstore.cpp \
    kilder/toolcallmodel.cpp \
    kilder/permissionmodel.cpp \
    kilder/diffmodel.cpp \
    kilder/configurationmodel.cpp \
    kilder/konfiguration.cpp \
    kilder/selftest.cpp

HEADERS += \
    kilder/logfile.h \
    kilder/acpframereader.h \
    kilder/acpmessage.h \
    kilder/agentprocess.h \
    kilder/acpprotokol.h \
    kilder/acpsession.h \
    kilder/acpclient.h \
    kilder/conversationmodel.h \
    kilder/sessionstore.h \
    kilder/toolcallmodel.h \
    kilder/permissionmodel.h \
    kilder/diffmodel.h \
    kilder/configurationmodel.h \
    kilder/konfiguration.h \
    kilder/selftest.h

OTHER_FILES += \
    harbour-skrynja-2.desktop \
    harbour-skrynja-2.yaml \
    rpm/harbour-skrynja-2.spec \
    realiseringsmanifest_for_funktioner_og_parrede_tests.json \
    projektbeskrivelse.adoc

target.path = /usr/bin
INSTALLS += target

desktop.files = harbour-skrynja-2.desktop
desktop.path = /usr/share/applications
INSTALLS += desktop

qml.files = qml/harbour-skrynja-2.qml
qml.path = /usr/share/harbour-skrynja-2/qml
qmlpages.files = qml/pages/StartSide.qml qml/pages/TilladelsesDialog.qml qml/pages/DiffVisning.qml qml/pages/KonfigurationsSide.qml
qmlpages.path = /usr/share/harbour-skrynja-2/qml/pages
INSTALLS += qml qmlpages

icon86.files = icons/86x86/harbour-skrynja-2.png
icon86.path = /usr/share/icons/hicolor/86x86/apps
icon108.files = icons/108x108/harbour-skrynja-2.png
icon108.path = /usr/share/icons/hicolor/108x108/apps
icon128.files = icons/128x128/harbour-skrynja-2.png
icon128.path = /usr/share/icons/hicolor/128x128/apps
icon172.files = icons/172x172/harbour-skrynja-2.png
icon172.path = /usr/share/icons/hicolor/172x172/apps
INSTALLS += icon86 icon108 icon128 icon172
