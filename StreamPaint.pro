#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T16:55:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StreamPaint
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ladybugwidget.cpp

HEADERS  += mainwindow.h \
    ladybugwidget.h

FORMS    += mainwindow.ui


INCLUDEPATH +="C:/Program Files (x86)/Point Grey Research/Ladybug/include"

INCLUDEPATH +="C:/Program Files (x86)/Point Grey Research/Ladybug/bin"
LADYBUG = "C:/Program Files (x86)/Point Grey Research/Ladybug/lib"
LIBS+= -L$$LADYBUG/ -lLadybugGUI
LIBS+= -L$$LADYBUG/ -lladybug

FREEGLUT = "C:/freeglut-2.8.0/freeglut-2.8.0"
INCLUDEPATH += $$FREEGLUT/include
LIBS += -L$$FREEGLUT/src/.libs -lglut
