#ifndef O2SPOTIFY_H
#define O2SPOTIFY_H

#include "o2.h"

/// Spotify's dialect of OAuth 2.0
class O0_EXPORT O2Spotify: public O2 {
    Q_OBJECT

public:
    explicit O2Spotify(QObject *parent = 0);

    struct Scope {
        static const QString PLAYLIST_READ_PRIVATE;
        static const QString PLAYLIST_READ_COLLABORATIVE;
        static const QString PLAYLIST_MODIFY_PUBLIC;
        static const QString PLAYLIST_MODIFY_PRIVATE;
        static const QString STREAMING;
        static const QString USER_FOLLOW_MODIFY;
        static const QString USER_FOLLOW_READ;
        static const QString USER_LIBRARY_READ;
        static const QString USER_LIBRARY_MODIFY;
        static const QString USER_READ_PRIVATE;
        static const QString USER_READ_BIRTHDATE;
        static const QString USER_READ_EMAIL;
        static const QString USER_TOP_READ;

        static QStringList allScopesList();
    };
};

#endif // O2SPOTIFY_H
