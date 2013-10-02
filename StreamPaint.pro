#-------------------------------------------------
#
# Project created by QtCreator 2013-08-13T16:55:17
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StreamPaint
TEMPLATE = app

include(../../QMapControl/QMapControl.pri)

DEPENDPATH += . ../../QMapControl/src
INCLUDEPATH += . ../../QMapControl/src
INCLUDEPATH += . ../../QMapControl

SOURCES += main.cpp\
        mainwindow.cpp \
    ladybugwidget.cpp \
    DObject.cpp

HEADERS  += mainwindow.h \
    ladybugwidget.h \
    DObject.hpp

FORMS    += mainwindow.ui


###### Ladybug SDK  ##################
INCLUDEPATH +="C:/Program Files (x86)/Point Grey Research/Ladybug/include"
INCLUDEPATH +="C:/Program Files (x86)/Point Grey Research/Ladybug/bin"
LADYBUG = "C:/Program Files (x86)/Point Grey Research/Ladybug/lib"
LIBS+= -L$$LADYBUG/ -lLadybugGUI
LIBS+= -L$$LADYBUG/ -lladybug

###### OpenCV  ###########################
INCLUDEPATH +="C:/opencv/install/include"
INCLUDEPATH +="C:/opencv/install/bin"
LIBS+=-L"C:/opencv/install/lib" \
       -lopencv_core240 \
       -lopencv_features2d240 \
       -lopencv_highgui240 \
       -lopencv_imgproc240 \
       -lopencv_objdetect240

RESOURCES += \
    LBWidget.qrc
