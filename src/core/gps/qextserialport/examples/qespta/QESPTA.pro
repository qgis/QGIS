######################################################################
# QextSerialPort Test Application (QESPTA)
######################################################################


PROJECT = QESPTA
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += ../..
QMAKE_LIBDIR += ../../build


OBJECTS_DIR    = .obj
MOC_DIR        = .moc
UI_DIR         = .uic
CONFIG      += qt thread warn_on debug


HEADERS += MainWindow.h \
		MessageWindow.h \
		QespTest.h

SOURCES += main.cpp \
		MainWindow.cpp \
		MessageWindow.cpp \
		QespTest.cpp

CONFIG(debug, debug|release):LIBS  += -lqextserialportd
else:LIBS  += -lqextserialport

unix:DEFINES   = _TTY_POSIX_
win32:DEFINES  = _TTY_WIN_
