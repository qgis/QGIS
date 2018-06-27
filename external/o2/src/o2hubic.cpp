#include "o2hubic.h"
#include "o2globals.h"
#include "o2replyserver.h"
#include <QHostAddress>

static const char *HubicScope = "usage.r,account.r,getAllLinks.r,credentials.r,activate.w,links.drw";
static const char *HubicEndpoint = "https://api.hubic.com/oauth/auth/";
static const char *HubicTokenUrl = "https://api.hubic.com/oauth/token/";
static const char *HubicRefreshUrl = "https://api.hubic.com/oauth/token/";

O2Hubic::O2Hubic(QObject *parent): O2(parent) {
    setRequestUrl(HubicEndpoint);
    setTokenUrl(HubicTokenUrl);
    setRefreshTokenUrl(HubicRefreshUrl);
    setScope(HubicScope);
    setLocalhostPolicy("http://localhost:%1/");
}
