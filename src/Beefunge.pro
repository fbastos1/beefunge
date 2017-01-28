#-------------------------------------------------
#
# Project created by QtCreator 2016-12-05T23:13:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Beefunge
TEMPLATE = app


SOURCES += main.cpp\
        beefunge.cpp \
    honeycombsyntaxhighlighter.cpp \
    beeterpreter.cpp

HEADERS  += beefunge.h \
    honeycombsyntaxhighlighter.h \
    beeterpreter.h

FORMS    += beefunge.ui
