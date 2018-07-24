/***************************************************************************
    begin                : July 13, 2016
    copyright            : (C) 2016 by Monsanto Company, USA
    author               : Larry Shaffer, Boundless Spatial
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSAUTHOAUTH2METHOD_H
#define QGSAUTHOAUTH2METHOD_H

#include <QObject>
#include <QDialog>
#include <QEventLoop>
#include <QTimer>
#include <QMutex>

#include "qgsauthmethod.h"


class QgsO2;

/**
 * The QgsAuthOAuth2Method class handles all network connection operation for the OAuth2 authentication plugin
 * \ingroup auth_plugins
 * \since QGIS 3.4
 */
class QgsAuthOAuth2Method : public QgsAuthMethod
{
    Q_OBJECT

  public:
    explicit QgsAuthOAuth2Method();
    ~QgsAuthOAuth2Method() override;

    //! OAuth2 method key
    QString key() const override;

    //! OAuth2 method description
    QString description() const override;

    //! Human readable description
    QString displayDescription() const override;

    //! Update network \a request with given \a authcfg and optional \a dataprovider
    bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
                               const QString &dataprovider = QString() ) override;

    //! Update network \a reply with given \a authcfg and optional \a dataprovider
    bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
                             const QString &dataprovider ) override;

    //! Update data source \a connectionItems with given \a authcfg and optional \a dataprovider
    bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
                                   const QString &dataprovider = QString() ) override;

    //! Clear cached configuration for given \a authcfg
    void clearCachedConfig( const QString &authcfg ) override;

    //! Update OAuth2 method configuration with \a config
    void updateMethodConfig( QgsAuthMethodConfig &mconfig ) override;

  public slots:

    //! Triggered when linked condition has changed
    void onLinkedChanged();

    //! Triggered when linking operation failed
    void onLinkingFailed();

    //! Triggered when linking operation succeeded
    void onLinkingSucceeded();

    //! Triggered when the browser needs to be opened at \a url
    void onOpenBrowser( const QUrl &url );

    //! Triggered on browser close
    void onCloseBrowser();

    //! Triggered on reply finished
    void onReplyFinished();

    //! Triggered on network error
    void onNetworkError( QNetworkReply::NetworkError err );

    //! Triggered on refresh finished
    void onRefreshFinished( QNetworkReply::NetworkError err );

    //! Triggered when auth code needs to be manually entered by the user
    void onAuthCode();

  signals:

    //! Emitted when authcode was manually set by the user
    void setAuthCode( const QString code );

  private:
    QString mTempStorePath;

    QgsO2 *getOAuth2Bundle( const QString &authcfg, bool fullconfig = true );

    void putOAuth2Bundle( const QString &authcfg, QgsO2 *bundle );

    void removeOAuth2Bundle( const QString &authcfg );

    static QMap<QString, QgsO2 *> sOAuth2ConfigCache;

    QgsO2 *authO2( const QString &authcfg );

    QMutex mNetworkRequestMutex;
};

#endif // QGSAUTHOAUTH2METHOD_H
