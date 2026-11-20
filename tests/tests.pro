QT += core testlib
QT -= gui

CONFIG += console c++11
CONFIG -= app_bundle

TARGET = tst_skrynja_2

INCLUDEPATH += ../src

SOURCES += \
    tst_byggesti_og_logfil.cpp \
    tst_acp_rygrad.cpp \
    tst_agentproces_og_initialisering.cpp \
    ../src/logfil.cpp \
    ../src/acpframereader.cpp \
    ../src/acpbesked.cpp \
    ../src/agentproces.cpp \
    ../src/acpclient.cpp

HEADERS += \
    testregistrering.h \
    ../src/logfil.h \
    ../src/acpframereader.h \
    ../src/acpbesked.h \
    ../src/agentproces.h \
    ../src/acpclient.h
