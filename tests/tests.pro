QT += core testlib
QT -= gui

CONFIG += console c++11
CONFIG -= app_bundle

TARGET = proeve_skrynja_2

INCLUDEPATH += ../src

SOURCES += \
    proeve_byggesti_og_logfil.cpp \
    proeve_acp_rygrad.cpp \
    proeve_agentproces_og_initialisering.cpp \
    proeve_prompt_og_svar.cpp \
    proeve_genoptagelse_af_samtale.cpp \
    proeve_vaerktoejskald_visning.cpp \
    proeve_tilladelsesdialog.cpp \
    proeve_stop_og_afvis.cpp \
    proeve_diff_og_uddata.cpp \
    proeve_konfiguration_af_agent.cpp \
    proeve_selvtest_med_verdict.cpp \
    proeve_proevesuite_i_tests.cpp \
    proeve_oprydning_af_trae.cpp \
    proeve_arkitektur_opdeling_protokol.cpp \
    proeve_arkitektur_opdeling_session.cpp \
    proeve_arkitektur_opdeling_ui_modeller.cpp \
    ../src/logfile.cpp \
    ../src/acpframereader.cpp \
    ../src/acpmessage.cpp \
    ../src/agentprocess.cpp \
    ../src/acpprotokol.cpp \
    ../src/acpsession.cpp \
    ../src/acpclient.cpp \
    ../src/conversationmodel.cpp \
    ../src/sessionstore.cpp \
    ../src/toolcallmodel.cpp \
    ../src/permissionmodel.cpp \
    ../src/diffmodel.cpp \
    ../src/configurationmodel.cpp \
    ../src/konfiguration.cpp \
    ../src/selftest.cpp

HEADERS += \
    testregistrering.h \
    ../src/logfile.h \
    ../src/acpframereader.h \
    ../src/acpmessage.h \
    ../src/agentprocess.h \
    ../src/acpprotokol.h \
    ../src/acpsession.h \
    ../src/acpclient.h \
    ../src/conversationmodel.h \
    ../src/sessionstore.h \
    ../src/toolcallmodel.h \
    ../src/permissionmodel.h \
    ../src/diffmodel.h \
    ../src/configurationmodel.h \
    ../src/konfiguration.h \
    ../src/selftest.h
