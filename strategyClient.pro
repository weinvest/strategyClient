#-------------------------------------------------
#
# Project created by QtCreator 2014-10-03T10:31:06
#
#-------------------------------------------------

TARGET = strategyClient
TEMPLATE = lib

CONFIG += c++11
CONFIG -= qt

INCLUDEPATH += ../webquant
DEFINES += STRATEGYCLIENT_LIBRARY

SOURCES += StrategyClient.cpp \
    proto/common_constants.cpp \
    proto/common_types.cpp \
    proto/get_constants.cpp \
    proto/get_types.cpp

HEADERS += StrategyClient.h\
        strategyclient_global.h \
    proto/common_constants.h \
    proto/common_types.h \
    proto/get_constants.h \
    proto/get_types.h

LIBS += -lnetwork -lwcrypto -lcommon -lthrift
unix {
    target.path = /lib
    INSTALLS += target
}
