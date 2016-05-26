TEMPLATE = app

QT += qml quick widgets core gui
CONFIG += c++11 no_keywords

INCLUDEPATH += $$PWD/Dionysus/include/

LIBS += -lCGAL -lgmp -lmpfr -lboost_thread -lboost_system -lboost_filesystem -lboost_serialization

SOURCES += \
    src/main.cpp \
    src/persistence.cpp

HEADERS += \
    src/persistence.h

# Default rules for deployment.
include(deployment.pri)

QMAKE_CXXFLAGS += -Wno-unused -frounding-math
