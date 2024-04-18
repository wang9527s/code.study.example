QT += core \
    widgets \
    gui concurrent

CONFIG += c++11

TARGET = ResourceWatcher
CONFIG += console
CONFIG -= app_bundle

DESTDIR = $$PWD\bin

TEMPLATE = app

SOURCES += main.cpp \
    widget.cpp


HEADERS += \
    widget.h \
    tool.h

DISTFILES += \
    process_resource.py

# 拷贝失败
# QMAKE_POST_LINK += $$system(copy /y "$$PWD/process_resource.py" "$$DEST_DIR")

