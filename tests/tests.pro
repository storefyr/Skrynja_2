QT += core testlib
QT -= gui

CONFIG += console c++11
CONFIG -= app_bundle

TARGET = tst_skrynja_2

INCLUDEPATH += ../src

SOURCES += \
    tst_byggesti_og_logfil.cpp \
    ../src/logfil.cpp

HEADERS += \
    testregistrering.h \
    ../src/logfil.h
