#-------------------------------------------------
#
# Project created by QtCreator 2016-04-04T10:14:26
#
#-------------------------------------------------

QT += core gui
QT += widgets

TARGET = GntDecoder
TEMPLATE = app


SOURCES += main.cpp \
    QxAboutDialog.cpp \
    QxDecodeOptionDlg.cpp \
    QxMainWindow.cpp

HEADERS  += \
    QxAboutDialog.h \
    QxDecodeOptionDlg.h \
    QxMainWindow.h

RESOURCES += \
    gntdecoder.qrc \

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include \
                /usr/local/bin \
                /usr/local/lib \
                /usr/lib \
                /usr/bin


unix:!macx: LIBS += -lopencv_core

unix:!macx: LIBS += -lopencv_highgui

unix:!macx: LIBS += -lopencv_imgproc
