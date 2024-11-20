#include <cstddef>

#include "o1smugmug.h"

static QString basicAuthorizationUrl() {
    return "https://secure.smugmug.com/services/oauth/1.0a/authorize";
}

static const char *accessToString(O1SmugMug::Access access) {
    const char * const strings[] = {
        "Public",
        "Full"
    };
    const std::size_t stringsSize = sizeof(strings) / sizeof(strings[0]);
    Q_ASSERT(access >= 0 && static_cast<std::size_t>(access) < stringsSize
                && "Unsupported SmugMug authorization access!");
    Q_UNUSED(stringsSize)

    return strings[access];
}

static const char *permissionsToString(O1SmugMug::Permissions permissions) {
    const char * const strings[] = {
        "Read",
        "Add",
        "Modify"
    };
    const std::size_t stringsSize = sizeof(strings) / sizeof(strings[0]);
    Q_ASSERT(permissions >= 0 && static_cast<std::size_t>(permissions) < stringsSize
                && "Unsupported SmugMug authorization permissions!");
    Q_UNUSED(stringsSize)

    return strings[permissions];
}


void O1SmugMug::initAuthorizationUrl(Access access, Permissions permissions) {
    setAuthorizeUrl(QUrl(::basicAuthorizationUrl()
                         + "?Access=" + ::accessToString(access)
                         + "&Permissions=" + ::permissionsToString(permissions)));
}

#if QT_VERSION >= 0x050000

void O1SmugMug::AuthorizationUrlBuilder::setAccess(Access value) {
    query_.addQueryItem("Access", ::accessToString(value));
}

void O1SmugMug::AuthorizationUrlBuilder::setPermissions(Permissions value) {
    query_.addQueryItem("Permissions", ::permissionsToString(value));
}

void O1SmugMug::AuthorizationUrlBuilder::setAllowThirdPartyLogin(bool value) {
    query_.addQueryItem("allowThirdPartyLogin", value ? "1" : "0");
}

void O1SmugMug::AuthorizationUrlBuilder::setShowSignUpButton(bool value) {
    query_.addQueryItem("showSignUpButton", value ? "true" : "false");
}

void O1SmugMug::AuthorizationUrlBuilder::setPrepopulatedUsername(const QString &value) {
    query_.addQueryItem("username", value);
}

void O1SmugMug::AuthorizationUrlBuilder::setViewportScale(double value) {
    Q_ASSERT(value >= 0 && value <= 1 && "Invalid SmugMug authorization viewportScale!");
    query_.addQueryItem("viewportScale", QString::number(value, 'f'));
}

QUrl O1SmugMug::AuthorizationUrlBuilder::url() const {
    QUrl result(::basicAuthorizationUrl());
    result.setQuery(query_);
    return result;
}

void O1SmugMug::initAuthorizationUrl(const AuthorizationUrlBuilder &builder) {
    setAuthorizeUrl(builder.url());
}

#endif // QT_VERSION >= 0x050000

O1SmugMug::O1SmugMug(QObject *parent, QNetworkAccessManager *manager, O0AbstractStore *store)
    : O1(parent, manager, store) {
   setRequestTokenUrl(QUrl("https://secure.smugmug.com/services/oauth/1.0a/getRequestToken"));
   setAccessTokenUrl(QUrl("https://secure.smugmug.com/services/oauth/1.0a/getAccessToken"));
}
