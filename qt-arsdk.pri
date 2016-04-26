QT += qml quick
CONFIG += qt c++11

android {
    QT += androidextras xml
}

DEFINES += STATIC_PLUGIN

#DEFINES += WANT_DEBUG
#DEFINES += WANT_TRACE

HEADERS += \
    $$PWD/src/common.h \
    $$PWD/src/config.h \
    $$PWD/src/arcommandcodec.h \
    $$PWD/src/arcommanddictionary.h \
    $$PWD/src/arcommandlistener.h \
    $$PWD/src/arcontroller.h \
    $$PWD/src/ardevice.h \
    $$PWD/src/arsdk_plugin.h \
    $$PWD/src/arconnector.h

SOURCES += \
    $$PWD/src/arcommandcodec.cpp \
    $$PWD/src/arcommanddictionary.cpp \
    $$PWD/src/arcommandlistener.cpp \
    $$PWD/src/arcontroller.cpp \
    $$PWD/src/ardevice.cpp \
    $$PWD/src/arsdk_plugin.cpp \
    $$PWD/src/arconnector.cpp

RESOURCES += \
    $$PWD/arsdk.qrc