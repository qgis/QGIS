/***************************************************************************
    qgsauthplanetarycomputermethod.h
    ------------------------
    begin                : August 2025
    copyright            : (C) 2025 by Stefanos Natsis
    author               : Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHPLANETARYCOMPUTERMETHOD_H
#define QGSAUTHPLANETARYCOMPUTERMETHOD_H

#include "qgsauthconfig.h"
#include "qgsauthmethod.h"
#include "qgsauthmethodmetadata.h"

#include <QMutex>
#include <QObject>

class QgsAuthPlanetaryComputerMethod : public QgsAuthMethod
{
    Q_OBJECT

  public:
    static const QString AUTH_METHOD_KEY;
    static const QString AUTH_METHOD_DESCRIPTION;
    static const QString AUTH_METHOD_DISPLAY_DESCRIPTION;

    explicit QgsAuthPlanetaryComputerMethod();

    // QgsAuthMethod interface
    [[nodiscard]] QString key() const override;

    [[nodiscard]] QString description() const override;

    [[nodiscard]] QString displayDescription() const override;

    bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider = QString() ) override;

    bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg, const QString &dataprovider = QString() ) override;

    void clearCachedConfig( const QString &authcfg ) override;
    void updateMethodConfig( QgsAuthMethodConfig &config ) override;

#ifdef HAVE_GUI
    QWidget *editWidget( QWidget *parent ) const override;
#endif

  private:
    struct SasToken
    {
        [[nodiscard]] bool isValid() const { return !token.isEmpty() && !( expiry < QDateTime::currentDateTimeUtc().addSecs( 300 ) ); }
        QDateTime expiry;
        QString token;
    };

    void updateUri( QString &uri, const QgsAuthMethodConfig &config, const QString &authcfg );

    QString sasTokenForUrl( const QUrl &url, const QString &signUrl, const QString &authcfg, bool isPro );

    void storeSasToken( const QString &authcfg, const QString &account, const QString &container, const SasToken &token );
    SasToken retrieveSasToken( const QString &authcfg, const QString &account, const QString &container );

    QgsAuthMethodConfig getMethodConfig( const QString &authcfg, bool fullconfig = true );

    void putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &config );

    void removeMethodConfig( const QString &authcfg );

    QgsAuthMethod *mOauth2 = nullptr;

    static const QString OPEN_SAS_SIGN_URL;
    static const QString PRO_SAS_SIGN_URL;
    static const QString BLOB_STORAGE_DOMAIN;

    static QMap<QString, SasToken> sSasTokensCache;
    static QMap<QString, QgsAuthMethodConfig> sAuthConfigCache;
};


class QgsAuthPlanetaryComputerMethodMetadata : public QgsAuthMethodMetadata
{
  public:
    QgsAuthPlanetaryComputerMethodMetadata()
      : QgsAuthMethodMetadata( QgsAuthPlanetaryComputerMethod::AUTH_METHOD_KEY, QgsAuthPlanetaryComputerMethod::AUTH_METHOD_DESCRIPTION )
    {}
    [[nodiscard]] QgsAuthPlanetaryComputerMethod *createAuthMethod() const override { return new QgsAuthPlanetaryComputerMethod; }
};

#endif // QGSAUTHPLANETARYCOMPUTERMETHOD_H
