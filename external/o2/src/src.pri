QT *= network

# script module is deprecated since Qt 5.5 (http://wiki.qt.io/New-Features-in-Qt-5.5)
!qtHaveModule(qml): QT *= script
qtHaveModule(qml): QT *= qml

INCLUDEPATH += $$PWD
SOURCES += \
    $$PWD/o1.cpp \
    $$PWD/o1requestor.cpp \
    $$PWD/o1timedreply.cpp \
    $$PWD/o2.cpp \
    $$PWD/o2facebook.cpp \
    $$PWD/o2gft.cpp \
    $$PWD/o2reply.cpp \
    $$PWD/o2replyserver.cpp \
    $$PWD/o2requestor.cpp \
    $$PWD/o2skydrive.cpp \
    $$PWD/oxtwitter.cpp \
    $$PWD/o2simplecrypt.cpp \
    $$PWD/o0baseauth.cpp \
    $$PWD/o0settingsstore.cpp \
    $$PWD/o2spotify.cpp

HEADERS += \
    $$PWD/o1.h \
    $$PWD/o1dropbox.h \
    $$PWD/o1flickr.h \
    $$PWD/o1requestor.h \
    $$PWD/o1twitter.h \
    $$PWD/o1timedreply.h \
    $$PWD/o2.h \
    $$PWD/o2facebook.h \
    $$PWD/o2gft.h \
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
    $$PWD/o2spotify.h
