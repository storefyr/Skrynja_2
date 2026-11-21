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
    tst_selvtest_med_verdict.cpp \
    tst_proevesuite_i_tests.cpp \
    tst_oprydning_af_trae.cpp \
    tst_arkitektur_opdeling_protokol.cpp \
    tst_arkitektur_opdeling_session.cpp \
    tst_arkitektur_opdeling_ui_modeller.cpp \
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
    ../src/selftest.h
