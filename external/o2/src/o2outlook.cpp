#include "o2outlook.h"
#include "o0globals.h"

O2Outlook::O2Outlook(QObject *parent): O2Skydrive(parent) {
    setRequestUrl("https://login.microsoftonline.com/common/oauth2/v2.0/authorize");
    setTokenUrl("https://login.microsoftonline.com/common/oauth2/v2.0/token");
    setRefreshTokenUrl("https://login.microsoftonline.com/common/oauth2/v2.0/token");
    redirectUri_ = QString("https://login.live.com/oauth20_desktop.srf");
}

QUrl O2Outlook::redirectUrl()
{
    return redirectUri_;
}
