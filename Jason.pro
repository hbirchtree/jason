#-------------------------------------------------
#
# Project created by QtCreator 2014-08-31T19:45:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Jason
TEMPLATE = app


SOURCES += main.cpp \
    jasongraphical.cpp \
    jasonparser.cpp

HEADERS  += jasonparser.h \
    jasongraphical.h

TRANSLATIONS += \
    jason-en.ts

OTHER_FILES += \
    jason-en.ts \
    MANUAL.md \
    README.md
