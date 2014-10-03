TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += .. ../../webquant
SOURCES += main.cpp

LIBS += -lstrategyClient -lnetwork -lwcrypto -lcommon -lboost_thread -lboost_system -lthrift -lboost_filesystem -lboost_system

