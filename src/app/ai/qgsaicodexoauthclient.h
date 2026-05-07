/***************************************************************************
    qgsaicodexoauthclient.h
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

#ifndef QGSAICODEXOAUTHCLIENT_H
#define QGSAICODEXOAUTHCLIENT_H

#include "qgis_app.h"

#include <QString>

class APP_EXPORT QgsAiCodexOAuthClient
{
  public:
    struct DeviceCode
    {
        QString verificationUrl;
        QString userCode;
        QString deviceAuthId;
        int intervalSeconds = 5;
    };

    struct TokenSet
    {
        QString accessToken;
        QString refreshToken;
        QString idToken;
        QString chatGptAccountId;
    };

    static bool requestDeviceCode( DeviceCode &deviceCode, QString *errorMessage = nullptr );
    static bool completeDeviceCodeLogin( const DeviceCode &deviceCode, QString *errorMessage = nullptr );
    static bool refreshAccessToken( TokenSet &tokens, QString *errorMessage = nullptr );
    static bool hasRefreshToken();
    static bool clearRefreshToken( QString *errorMessage = nullptr );
    static QString extractChatGptAccountId( const QString &idToken );
    static QString refreshTokenSettingKey();
};

#endif // QGSAICODEXOAUTHCLIENT_H
