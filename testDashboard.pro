#-------------------------------------------------
#
# Project created by QtCreator 2019-10-31T20:44:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testDashboard
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
    dashboard.cpp

HEADERS += \
        mainwindow.h \
    dashboard.h \
    customdashboard.h


FORMS += \
        mainwindow.ui \
    dashboard.ui


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lcustomdashboardplugin
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lcustomdashboardplugind
else:unix: LIBS += -L$$PWD/./ -lcustomdashboardplugin

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/./libcustomdashboardplugin.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/./libcustomdashboardplugind.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/./customdashboardplugin.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/./customdashboardplugind.lib
else:unix: PRE_TARGETDEPS += $$PWD/./libcustomdashboardplugin.a

DISTFILES +=
