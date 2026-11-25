QT += core testlib
QT -= gui

CONFIG += console c++11
CONFIG -= app_bundle

TARGET = proeve_skrynja_2

INCLUDEPATH += ../kilder

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
    ../kilder/logfile.cpp \
    ../kilder/acpframereader.cpp \
    ../kilder/acpmessage.cpp \
    ../kilder/agentprocess.cpp \
    ../kilder/acpprotokol.cpp \
    ../kilder/acpsession.cpp \
    ../kilder/acpclient.cpp \
    ../kilder/conversationmodel.cpp \
    ../kilder/sessionstore.cpp \
    ../kilder/toolcallmodel.cpp \
    ../kilder/permissionmodel.cpp \
    ../kilder/diffmodel.cpp \
    ../kilder/configurationmodel.cpp \
    ../kilder/konfiguration.cpp \
    ../kilder/selftest.cpp

HEADERS += \
    testregistrering.h \
    ../kilder/logfile.h \
    ../kilder/acpframereader.h \
    ../kilder/acpmessage.h \
    ../kilder/agentprocess.h \
    ../kilder/acpprotokol.h \
    ../kilder/acpsession.h \
    ../kilder/acpclient.h \
    ../kilder/conversationmodel.h \
    ../kilder/sessionstore.h \
    ../kilder/toolcallmodel.h \
    ../kilder/permissionmodel.h \
    ../kilder/diffmodel.h \
    ../kilder/configurationmodel.h \
    ../kilder/konfiguration.h \
    ../kilder/selftest.h
