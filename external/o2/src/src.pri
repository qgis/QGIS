QT *= network

lessThan(QT_MAJOR_VERSION, 5): QT *= script

INCLUDEPATH += $$PWD
SOURCES += \
    $$PWD/o0jsonresponse.cpp \
    $$PWD/o1.cpp \
    $$PWD/o1requestor.cpp \
    $$PWD/o1smugmug.cpp \
    $$PWD/o1timedreply.cpp \
    $$PWD/o2.cpp \
    $$PWD/o2facebook.cpp \
    $$PWD/o2gft.cpp \
    $$PWD/o2outlook.cpp \
    $$PWD/o2pollserver.cpp \
    $$PWD/o2reply.cpp \
    $$PWD/o2replyserver.cpp \
    $$PWD/o2requestor.cpp \
    $$PWD/o2skydrive.cpp \
    $$PWD/oxtwitter.cpp \
    $$PWD/o2simplecrypt.cpp \
    $$PWD/o0baseauth.cpp \
    $$PWD/o0settingsstore.cpp \
    $$PWD/o2spotify.cpp \
    $$PWD/o2google.cpp \
    $$PWD/o2googledevice.cpp \
    $$PWD/o2uber.cpp \
    $$PWD/o2msgraph.cpp

HEADERS += \
    $$PWD/o0export.h \
    $$PWD/o0jsonresponse.h \
    $$PWD/o1.h \
    $$PWD/o1dropbox.h \
    $$PWD/o1flickr.h \
    $$PWD/o1requestor.h \
    $$PWD/o1smugmug.h \
    $$PWD/o1twitter.h \
    $$PWD/o1timedreply.h \
    $$PWD/o1upwork.h \
    $$PWD/o2.h \
    $$PWD/o2facebook.h \
    $$PWD/o2gft.h \
    $$PWD/o2outlook.h \
    $$PWD/o2pollserver.h \
    $$PWD/o2reply.h \
    $$PWD/o2replyserver.h \
    $$PWD/o2requestor.h \
    $$PWD/o2skydrive.h \
    $$PWD/oxtwitter.h \
    $$PWD/o1freshbooks.h \
    $$PWD/o0baseauth.h \
    $$PWD/o0globals.h \
    $$PWD/o0simplecrypt.h \
    $$PWD/o0requestparameter.h \
    $$PWD/o0abstractstore.h \
    $$PWD/o0settingsstore.h \
    $$PWD/o2spotify.h \
    $$PWD/o2google.h \
    $$PWD/o2googledevice.h \
    $$PWD/o2uber.h \
    $$PWD/o2msgraph.h

headers.files += \
    $$PWD/o0jsonresponse.h \
    $$PWD/o1.h \
    $$PWD/o1dropbox.h \
    $$PWD/o1flickr.h \
    $$PWD/o1requestor.h \
    $$PWD/o1smugmug.h \
    $$PWD/o1twitter.h \
    $$PWD/o1timedreply.h \
    $$PWD/o1upwork.h \
    $$PWD/o2.h \
    $$PWD/o2facebook.h \
    $$PWD/o2gft.h \
    $$PWD/o2pollserver.h \
    $$PWD/o2reply.h \
    $$PWD/o2replyserver.h \
    $$PWD/o2requestor.h \
    $$PWD/o2skydrive.h \
    $$PWD/oxtwitter.h \
    $$PWD/o1freshbooks.h \
    $$PWD/o0baseauth.h \
    $$PWD/o0globals.h \
    $$PWD/o0simplecrypt.h \
    $$PWD/o0requestparameter.h \
    $$PWD/o0abstractstore.h \
    $$PWD/o0settingsstore.h \
    $$PWD/o2spotify.h \
    $$PWD/o2google.h \
    $$PWD/o2googledevice.h \
    $$PWD/o2uber.h \
    $$PWD/o2msgraph.h \
    $$PWD/o2outlook.h
