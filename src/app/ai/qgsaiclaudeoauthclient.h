/***************************************************************************
    qgsaiclaudeoauthclient.h
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAICLAUDEOAUTHCLIENT_H
#define QGSAICLAUDEOAUTHCLIENT_H

#include "qgis_app.h"

#include <QString>
#include <QUrl>

class APP_EXPORT QgsAiClaudeOAuthClient
{
  public:
    struct AuthorizationRequest
    {
        QUrl authorizationUrl;
        QString codeVerifier;
        QString state;
        QString redirectUri;
    };

    struct TokenSet
    {
        QString accessToken;
        QString refreshToken;
    };

    static AuthorizationRequest buildAuthorizationRequest();
    static AuthorizationRequest buildAuthorizationRequest( const QString &redirectUri );
    static QString authorizationCodeFromInput( const QString &input );
    static QString authorizationStateFromInput( const QString &input );
    static bool exchangeAuthorizationCode( const QString &authorizationCode, const QString &codeVerifier, const QString &redirectUri, const QString &expectedState, QString *errorMessage = nullptr );
    static bool refreshAccessToken( TokenSet &tokens, QString *errorMessage = nullptr );
    static bool hasRefreshToken();
    static bool clearRefreshToken( QString *errorMessage = nullptr );
    static QString refreshTokenSettingKey();

    //! Redirects token POSTs to a loopback server (unit tests only).
    static void setTokenUrlForTesting( const QString &url );
    static void clearTokenUrlForTesting();
};

#endif // QGSAICLAUDEOAUTHCLIENT_H
