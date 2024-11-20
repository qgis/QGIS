#include "o2googledevice.h"

static const char *GoogleDeviceTokenUrl = "https://oauth2.googleapis.com/token";
static const char *GoogleDeviceRefreshUrl = "https://oauth2.googleapis.com/token";
static const char *GoogleDeviceEndpoint = "https://oauth2.googleapis.com/device/code";
// Google uses a different grant type value than specified in RFC 8628
static const char *GoogleDeviceGrantType = "http://oauth.net/grant_type/device/1.0";

O2GoogleDevice::O2GoogleDevice(QObject *parent) : O2(parent) {
    setGrantFlow(GrantFlowDevice);
    setGrantType(GoogleDeviceGrantType);
    setRequestUrl(GoogleDeviceEndpoint);
    setTokenUrl(GoogleDeviceTokenUrl);
    setRefreshTokenUrl(GoogleDeviceRefreshUrl);
}
