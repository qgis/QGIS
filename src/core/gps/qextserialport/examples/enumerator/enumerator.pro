######################################################################
# Enumerator
######################################################################


PROJECT = enumerator
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += ../..
QMAKE_LIBDIR += ../../build


OBJECTS_DIR    = obj
MOC_DIR        = moc
UI_DIR         = uic
CONFIG      += qt warn_on console


SOURCES += main.cpp


CONFIG(debug, debug|release):LIBS  += -lqextserialportd
else:LIBS  += -lqextserialport

unix:DEFINES   = _TTY_POSIX_
win32:DEFINES  = _TTY_WIN_
