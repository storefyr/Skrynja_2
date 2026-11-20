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
    tst_prompt_og_svar.cpp \
    tst_genoptagelse_af_samtale.cpp \
    ../src/logfil.cpp \
    ../src/acpframereader.cpp \
    ../src/acpbesked.cpp \
    ../src/agentproces.cpp \
    ../src/acpclient.cpp \
    ../src/samtalemodel.cpp \
    ../src/sessionlager.cpp

HEADERS += \
    testregistrering.h \
    ../src/logfil.h \
    ../src/acpframereader.h \
    ../src/acpbesked.h \
    ../src/agentproces.h \
    ../src/acpclient.h \
    ../src/samtalemodel.h \
    ../src/sessionlager.h
