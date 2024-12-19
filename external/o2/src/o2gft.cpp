#include "o2gft.h"

static const char *GftScope = "https://www.googleapis.com/auth/fusiontables";
static const char *GftEndpoint = "https://accounts.google.com/o/oauth2/auth";
static const char *GftTokenUrl = "https://accounts.google.com/o/oauth2/token";
static const char *GftRefreshUrl = "https://accounts.google.com/o/oauth2/token";

O2Gft::O2Gft(QObject *parent): O2Google(parent) {
    setScope(GftScope);
}
