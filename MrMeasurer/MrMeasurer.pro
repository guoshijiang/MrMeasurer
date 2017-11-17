#-------------------------------------------------
#
# Project created by QtCreator 2017-06-06T11:20:17
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hdtas_viewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += HDTAS_WINDOWS
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    tcp_worker.cpp \
    tcp_socket_thread.cpp \
    tcp_socket_object.cpp \
    udp_worker.cpp \
    udp_socket_object.cpp \
    udp_socket_thread.cpp \
    ../hdtasparser/hdtasparser/hdtasparser.cpp \
    ../hdtasparser/hdtasparser/protocol_data_parser.cpp \
    ../hdtasparser/hdtasparser/stdafx.cpp \
    ../hdtasparser/hdtasparser/utility.cpp \
    common.cpp

HEADERS  += mainwindow.h \
    tcp_worker.h \
    tcp_socket_thread.h \
    tcp_socket_object.h \
    udp_worker.h \
    udp_socket_object.h \
    udp_socket_thread.h \
    ../hdtasparser/hdtasparser/hdtasparser.h \
    ../hdtasparser/hdtasparser/header.h \
    ../hdtasparser/hdtasparser/protocol_data_parser.h \
    ../hdtasparser/hdtasparser/stdafx.h \
    ../hdtasparser/hdtasparser/struct.h \
    ../hdtasparser/hdtasparser/targetver.h \
    ../hdtasparser/hdtasparser/utility.h \
    common.h

FORMS    += mainwindow.ui

RESOURCES += \
    resource.qrc

RC_FILE = app.rc
