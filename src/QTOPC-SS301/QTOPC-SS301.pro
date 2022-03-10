#-------------------------------------------------
#
# Project created by QtCreator 2016-12-25T19:47:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "QTOPC-SS301"
TEMPLATE = app

LIBS += -L"$$_PRO_FILE_PWD_/3rd/lightopc/" -llightopc
LIBS += -lole32 -loleaut32 -luuid
LIBS += -lws2_32
//LIBS += qaxcontainer.lib

SOURCES  += src/main.cpp\
	 forms/mainwindow.cpp \
	src/tlog.cpp \
	src/tdriver.cpp \
	src/tdevice.cpp \
	src/tpoll.cpp \
	3rd/serialport/serialport.cpp

HEADERS  += forms/mainwindow.h \
	src/tlog.h \
	src/tdriver.h \
	src/tdevice.h \
	src/serv_main.h \
	src/opc_main.h \
	src/main.h \
	src/tpoll.h \
	3rd/serialport/serialport.h

FORMS    += forms/mainwindow.ui

CONFIG   -= console
CONFIG   += qaxcontainer		

OTHER_FILES +=

RESOURCES += resource.qrc
