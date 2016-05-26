TEMPLATE = app

QT += qml quick widgets core gui
CONFIG += c++11 no_keywords

INCLUDEPATH += $$PWD/Dionysus/include/

LIBS += -lCGAL -lgmp -lmpfr -lboost_thread -lboost_system -lboost_filesystem -lboost_serialization

SOURCES += \
    src/triangulation.cpp \
    src/main.cpp

HEADERS += \
    src/triangulation.h

# Default rules for deployment.
include(deployment.pri)

QMAKE_CXXFLAGS += -Wno-unused -frounding-math
