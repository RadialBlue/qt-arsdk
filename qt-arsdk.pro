TEMPLATE = lib

QT += qml quick
CONFIG += qt plugin c++11

TARGET = qt-arsdk
TARGET = $$qtLibraryTarget($$TARGET)

uri = arsdk

#DEFINES += WANT_DEBUG
#DEFINES += WANT_TRACE

HEADERS += \
    src/common.h \
    src/config.h \
    src/arcommandcodec.h \
    src/arcommanddictionary.h \
    src/arcommandlistener.h \
    src/arconnector.h \
    src/arcontroller.h \
    src/ardevice.h \
    src/arsdk_plugin.h

SOURCES += \
    src/arcommandcodec.cpp \
    src/arcommanddictionary.cpp \
    src/arcommandlistener.cpp \
    src/arconnector.cpp \
    src/arcontroller.cpp \
    src/ardevice.cpp \
    src/arsdk_plugin.cpp

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}

DISTFILES = qmldir
