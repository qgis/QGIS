######################################################################
# Enumerator
######################################################################


PROJECT = event
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += ../..
QMAKE_LIBDIR += ../../build


OBJECTS_DIR    = .obj #avoid using obj here, on some systems 'make' will try to switch into obj from the current directory and you will get "Cannot find file" error
MOC_DIR        = .moc
UI_DIR         = .uic
CONFIG      += qt thread warn_on console


SOURCES += main.cpp \
			PortListener.cpp

HEADERS += PortListener.h


CONFIG(debug, debug|release):LIBS  += -lqextserialportd
else:LIBS  += -lqextserialport

unix:DEFINES   = _TTY_POSIX_
win32:DEFINES  = _TTY_WIN_
