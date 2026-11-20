TARGET = harbour-skrynja-2

QT += core gui qml quick

CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/logfil.cpp \
    src/acpframereader.cpp \
    src/acpbesked.cpp \
    src/agentproces.cpp \
    src/acpclient.cpp \
    src/samtalemodel.cpp \
    src/sessionlager.cpp \
    src/vaerktoejskaldsmodel.cpp \
    src/tilladelsesmodel.cpp \
    src/diffmodel.cpp

HEADERS += \
    src/logfil.h \
    src/acpframereader.h \
    src/acpbesked.h \
    src/agentproces.h \
    src/acpclient.h \
    src/samtalemodel.h \
    src/sessionlager.h \
    src/vaerktoejskaldsmodel.h \
    src/tilladelsesmodel.h \
    src/diffmodel.h

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
qmlpages.files = qml/pages/StartSide.qml qml/pages/TilladelsesDialog.qml qml/pages/DiffVisning.qml
qmlpages.path = /usr/share/harbour-skrynja-2/qml/pages
INSTALLS += qml qmlpages
