#include "o2msgraph.h"

static const char *MsgraphEndpoint = "https://login.microsoftonline.com/common/oauth2/v2.0/authorize";
static const char *MsgraphTokenUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/token";

O2Msgraph::O2Msgraph(QObject *parent): O2(parent) {
    setRequestUrl(MsgraphEndpoint);
    setTokenUrl(MsgraphTokenUrl);
    setRefreshTokenUrl(MsgraphTokenUrl);
}
