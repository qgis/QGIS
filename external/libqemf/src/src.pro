# qmake project file for building libqemf

include( ../config.pri )

TARGET       = qemf
TEMPLATE     = lib

MOC_DIR      = ../tmp
OBJECTS_DIR  = ../tmp
DESTDIR      = ../

contains(CONFIG, QEmfDll){
    CONFIG  += dll
    DEFINES += QEMF_DLL QEMF_DLL_BUILD
} else {
    CONFIG  += staticlib
}

INCLUDEPATH += .

HEADERS = Bitmap.h\
	BitmapHeader.h\
	EmfEnums.h\
	EmfHeader.h\
	EmfLogger.h\
	EmfObjects.h\
	EmfOutput.h\
	EmfParser.h\
	EmfRecords.h\
	QEmfRenderer.h

SOURCES = Bitmap.cpp\
	BitmapHeader.cpp\
	EmfHeader.cpp\
	EmfLogger.cpp\
	EmfObjects.cpp\
	EmfOutput.cpp\
	EmfParser.cpp\
	EmfRecords.cpp\
	QEmfRenderer.cpp
