#-------------------------------------------------
#
# Project created by QtCreator 2017-07-24T11:38:28
#
#-------------------------------------------------

QT       += core gui serialbus serialport sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ProximityCardsUI
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    serialporthandler.cpp \
    serialbusactions.cpp \
    proximitycardsreaderbusactions.cpp \
    serialportmasterthread.cpp \
    sqlhandler.cpp

HEADERS += \
        mainwindow.h \
    serialporthandler.h \
    serialbusactions.h \
    proximitycardsreaderbusactions.h \
    serialportmasterthread.h \
    sqlhandler.h

FORMS += \
        mainwindow.ui
#Please, correct INCLUDEPATH and LIBS pathes for mysql
unix:!macx {
    "INCLUDEPATH+=/usr/local/include"
    "LIBS+=-L/usr/local/lib -lmysqlclient_r"
}
win32 {
    "INCLUDEPATH+=C:/MySQL/include"
    "LIBS+=C:/MYSQL/lib/opt/libmysql.lib"
}

RESOURCES += \
    main.qrc
