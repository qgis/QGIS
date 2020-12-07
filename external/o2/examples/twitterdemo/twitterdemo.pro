QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../../src/src.pri)

TARGET = twitterdemo
TEMPLATE = app

SOURCES += main.cpp \
    tweeter.cpp

HEADERS += \
    tweeter.h
