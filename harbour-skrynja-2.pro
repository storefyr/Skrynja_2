TARGET = harbour-skrynja-2

QT += core gui qml quick

CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/logfile.cpp \
    src/acpframereader.cpp \
    src/acpmessage.cpp \
    src/agentprocess.cpp \
    src/acpprotokol.cpp \
    src/acpsession.cpp \
    src/acpclient.cpp \
    src/conversationmodel.cpp \
    src/sessionstore.cpp \
    src/toolcallmodel.cpp \
    src/permissionmodel.cpp \
    src/diffmodel.cpp \
    src/configurationmodel.cpp \
    src/selftest.cpp

HEADERS += \
    src/logfile.h \
    src/acpframereader.h \
    src/acpmessage.h \
    src/agentprocess.h \
    src/acpprotokol.h \
    src/acpsession.h \
    src/acpclient.h \
    src/conversationmodel.h \
    src/sessionstore.h \
    src/toolcallmodel.h \
    src/permissionmodel.h \
    src/diffmodel.h \
    src/configurationmodel.h \
    src/selftest.h

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
