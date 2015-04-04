#-------------------------------------------------
#
# Project created by QtCreator 2014-08-31T19:45:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Jason
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp \
    jasonparser.cpp \
    jasongui.cpp \
    executer.cpp \
    jsonparser.cpp \
    desktoptools.cpp \
    modules/variablehandler.cpp \
    jason-tools/jasoncore.cpp \
    jsonstaticfuncs.cpp \
    jason-tools/systemcontainer.cpp \
    jason-tools/environmentcontainer.cpp \
    jason-tools/subsystemcontainer.cpp \
    jason-tools/runtimequeue.cpp \
    jason-tools/activeoptionscontainer.cpp \
    modules/executionunit.cpp \
    kaidan.cpp

HEADERS  += jasonparser.h \
    jasongui.h \
    executer.h \
    jsonparser.h \
    desktoptools.h \
    modules/variablehandler.h \
    jason-tools/jasoncore.h \
    jsonstaticfuncs.h \
    jason-tools/systemcontainer.h \
    jason-tools/environmentcontainer.h \
    jason-tools/subsystemcontainer.h \
    jason-tools/runtimequeue.h \
    jason-tools/activeoptionscontainer.h \
    modules/executionunit.h \
    kaidan.h \
    modules/uiglue.h

TRANSLATIONS += \
    jason-en.ts

OTHER_FILES += \
    jason-en.ts \
    MANUAL.md \
    README.md

FORMS += \
    jasongui.ui \
    kaidan.ui
