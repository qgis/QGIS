PROJECT                 = qextserialport
TEMPLATE                = lib

CONFIG                 += qt warn_on thread debug_and_release
CONFIG                  += dll
#CONFIG                 += staticlib

# event driven device enumeration on windows requires the gui module
!win32:QT               -= gui

OBJECTS_DIR             = build/obj
MOC_DIR                 = build/moc
DEPENDDIR               = .
INCLUDEDIR              = .
HEADERS                 = qextserialport.h \
                          qextserialenumerator.h
SOURCES                 = qextserialport.cpp \
                          qextserialenumerator.cpp

unix:SOURCES           += posix_qextserialport.cpp
macx: LIBS             += -framework IOKit

win32:SOURCES          += win_qextserialport.cpp
win32:DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
win32:LIBS             += -lsetupapi


DESTDIR                 = build
#DESTDIR                = examples/enumerator/debug
#DESTDIR                = examples/qespta/debug
#DESTDIR                = examples/event/debug

CONFIG(debug, debug|release) {
    TARGET = qextserialportd
} else {
    TARGET = qextserialport
}

VERSION            = 1.2.0
