#include "o2spotify.h"

static const char *SpotifyEndpoint = "https://accounts.spotify.com/authorize";
static const char *SpotifyTokenUrl = "https://accounts.spotify.com/api/token";

O2Spotify::O2Spotify(QObject *parent): O2(parent) {
    setRequestUrl(SpotifyEndpoint);
    setTokenUrl(SpotifyTokenUrl);
    setRefreshTokenUrl(SpotifyTokenUrl);
}

const QString O2Spotify::Scope::PLAYLIST_READ_PRIVATE = "playlist-read-private";
const QString O2Spotify::Scope::PLAYLIST_READ_COLLABORATIVE = "playlist-read-collaborative";
const QString O2Spotify::Scope::PLAYLIST_MODIFY_PUBLIC = "playlist-modify-public";
const QString O2Spotify::Scope::PLAYLIST_MODIFY_PRIVATE = "playlist-modify-private";
const QString O2Spotify::Scope::STREAMING = "streaming";
const QString O2Spotify::Scope::USER_FOLLOW_MODIFY = "user-follow-modify";
const QString O2Spotify::Scope::USER_FOLLOW_READ = "user-follow-read";
const QString O2Spotify::Scope::USER_LIBRARY_READ = "user-library-read";
const QString O2Spotify::Scope::USER_LIBRARY_MODIFY = "user-library-modify";
const QString O2Spotify::Scope::USER_READ_PRIVATE = "user-read-private";
const QString O2Spotify::Scope::USER_READ_BIRTHDATE = "user-read-birthdate";
const QString O2Spotify::Scope::USER_READ_EMAIL = "user-read-email";
const QString O2Spotify::Scope::USER_TOP_READ = "user-top-read";

QStringList O2Spotify::Scope::allScopesList() {
    QStringList result;
    return result
            << PLAYLIST_READ_PRIVATE
            << PLAYLIST_READ_COLLABORATIVE
            << PLAYLIST_MODIFY_PUBLIC
            << PLAYLIST_MODIFY_PRIVATE
            << STREAMING
            << USER_FOLLOW_MODIFY
            << USER_FOLLOW_READ
            << USER_LIBRARY_READ
            << USER_LIBRARY_MODIFY
            << USER_READ_PRIVATE
            << USER_READ_BIRTHDATE
            << USER_READ_EMAIL
            << USER_TOP_READ;
}
