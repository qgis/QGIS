TARGET = Sialis
TEMPLATE = app
QT += qml quick webview
CONFIG += c++11
SOURCES += main.cpp \
    twitterapi.cpp \
    tweetmodel.cpp
RESOURCES += qml.qrc

# "Bird" icon by snap2objects: http://www.snap2objects.com
ICON = sialis.icns

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

# Core O2
include(../../src/src.pri)

HEADERS += \
    twitterapi.h \
    tweetmodel.h
