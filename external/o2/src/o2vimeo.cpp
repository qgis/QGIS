#include "o2vimeo.h"

// Vimeo supported scopes: https://developer.vimeo.com/api/authentication#supported-scopes
static const char *VimeoScope = "public";
static const char *VimeoEndpoint = "https://api.vimeo.com/oauth/authorize";
static const char *VimeoTokenUrl = "https://api.vimeo.com/oauth/access_token";

O2Vimeo::O2Vimeo(QObject *parent): O2(parent) {
    setRequestUrl(VimeoEndpoint);
    setTokenUrl(VimeoTokenUrl);
    setRefreshTokenUrl(VimeoTokenUrl);
    setScope(VimeoScope);
}
