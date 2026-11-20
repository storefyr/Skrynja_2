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
    tst_vaerktoejskald_visning.cpp \
    tst_tilladelsesdialog.cpp \
    tst_stop_og_afvis.cpp \
    tst_diff_og_uddata.cpp \
    tst_konfiguration_af_agent.cpp \
    ../src/logfil.cpp \
    ../src/acpframereader.cpp \
    ../src/acpbesked.cpp \
    ../src/agentproces.cpp \
    ../src/acpclient.cpp \
    ../src/samtalemodel.cpp \
    ../src/sessionlager.cpp \
    ../src/vaerktoejskaldsmodel.cpp \
    ../src/tilladelsesmodel.cpp \
    ../src/diffmodel.cpp \
    ../src/konfigurationsmodel.cpp

HEADERS += \
    testregistrering.h \
    ../src/logfil.h \
    ../src/acpframereader.h \
    ../src/acpbesked.h \
    ../src/agentproces.h \
    ../src/acpclient.h \
    ../src/samtalemodel.h \
    ../src/sessionlager.h \
    ../src/vaerktoejskaldsmodel.h \
    ../src/tilladelsesmodel.h \
    ../src/diffmodel.h \
    ../src/konfigurationsmodel.h
