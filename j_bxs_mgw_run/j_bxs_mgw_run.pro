#-------------------------------------------------
#
# Project created by QtCreator 2017-11-20T16:53:56
#
#-------------------------------------------------
#include( $${PWD}/examples.pri)

#include( $${PWD}/include_customCtrl/qwt.pri)


QT       += core gui
QT       += sql
QT       +=xml
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
STATECHARTS += transsrc.qm
TARGET = j_bxs_mgw_run
TEMPLATE = app
TRANSLATIONS += transsrc.ts
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
        maindialog.cpp \
    dommodel.cpp \
    form_customview.cpp \
    form_viewcontainer.cpp \
    pcmpara.cpp \
    include_customCtrl/circularbuffer.cpp \
    include_customCtrl/plot.cpp

HEADERS += \
        maindialog.h \
    dommodel.h \
    form_customview.h \
    form_viewcontainer.h \
    pcmpara.h \
    include_customCtrl/circularbuffer.h \
    include_customCtrl/plot.h \
    include_customCtrl/settings.h

FORMS += \
        maindialog.ui \
    form_customview.ui \
    form_viewcontainer.ui

RESOURCES += \
    resource.qrc

#DISTFILES +=

DEFINES += QT_DLL QWT_DLL
LIBS += -L"E:\tool\Qt5.9.1\5.9.1\msvc2013_64\lib" -lqwtd
LIBS += -L"E:\tool\Qt5.9.1\5.9.1\msvc2013_64\lib" -lqwt
INCLUDEPATH += E:\tool\Qt5.9.1\5.9.1\msvc2013_64\include\Qwt
#LIBS += -L"E:\tool\Qt5.9.1\5.9.1\msvc2013\lib" -lqwtd
#LIBS += -L"E:\tool\Qt5.9.1\5.9.1\msvc2013\lib" -lqwt
#INCLUDEPATH += E:\tool\Qt5.9.1\5.9.1\msvc2013\include\Qwt

INCLUDEPATH += $$PWD/include_customCtrl

SOURCES += \
  include_customCtrl/j_customcontrolled.cpp \

HEADERS += \
  include_customCtrl/j_customcontrolled.h \

#INCLUDEPATH += $$PWD\customControl\customledctrl\
#LIBS += $$PWD\customControl\customledctrl\customledctrlplugin.lib


#QWT_ROOT = e:\tool\Qt5.9.1\qtCtrl\qwt-6.1.2
##QWT_ROOT = e:/tool/Qt5.9.1/qtCtrl/build-qwt-qt_5_6_2_msvc2013_32_bit-Release
#include( $${QWT_ROOT}/qwtfunctions.pri )
#INCLUDEPATH += $${QWT_ROOT}/src
#DEPENDPATH  += $${QWT_ROOT}/src
##%QWT_CONFIG  += QwtDll %如果需要使用继承时需要这句 -- 下面依次类推
#contains(QWT_CONFIG, QwtFramework) {
#    LIBS      += -F$${QWT_ROOT}/lib
#}
#else {
#    LIBS      += -L$${QWT_ROOT}/lib
#}
#qwtAddLibrary(qwt)
#contains(QWT_CONFIG, QwtOpenGL ) {
#    QT += opengl
#}
#else {
#    DEFINES += QWT_NO_OPENGL
#}
#contains(QWT_CONFIG, QwtSvg) {
#    QT += svg
#}
#else {
#    DEFINES += QWT_NO_SVG
#}
#win32 {
#    contains(QWT_CONFIG, QwtDll) {
#        DEFINES    += QT_DLL QWT_DLL
#    }
#}

