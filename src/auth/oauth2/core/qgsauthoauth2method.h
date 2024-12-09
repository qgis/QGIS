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
#include <QReadWriteLock>
#include <QThread>

#include "qgsauthmethod.h"
#include "qgsauthmethodmetadata.h"

class QgsO2;
class QgsAuthOAuth2Config;

/**
 * A long-running thread on which all QgsO2 objects run.
 *
 * We have to take care that we don't directly create QgsO2 objects on the thread
 * where the authentication request is occurring, as QgsO2 objects are long running
 * but the thread requesting authentication may be short-lived.
 *
 * Accordingly, all QgsO2 objects run on an instance of QgsOAuth2Factory
 * (see QgsOAuth2Factory::instance()). This ensures that they will run
 * on a thread with an application-long lifetime.
 *
 * \ingroup auth_plugins
 * \since QGIS 3.42
 */
class QgsOAuth2Factory : public QThread
{
    Q_OBJECT

  public:
    /**
     * Creates a new QgsO2 object, ensuring that it is correctly created on the
     * QgsOAuth2Factory thread instance.
     *
     * The \a oauth2config object will be re-parented to the new QgsO2 object.
     */
    static QgsO2 *createO2( const QString &authcfg, QgsAuthOAuth2Config *oauth2config );

    /**
     * Request linking a \a o2 object. This ensures that the link is done
     * in a thread-safe manner.
     */
    static void requestLink( QgsO2 *o2 );

  private:
    static QgsOAuth2Factory *instance();

    QgsOAuth2Factory( QObject *parent = nullptr );

    QgsO2 *createO2Private( const QString &authcfg, QgsAuthOAuth2Config *oauth2config );

    static QgsOAuth2Factory *sInstance;
};


/**
 * The QgsAuthOAuth2Method class handles all network connection operation for the OAuth2 authentication plugin
 * \ingroup auth_plugins
 * \since QGIS 3.4
 */
class QgsAuthOAuth2Method : public QgsAuthMethod
{
    Q_OBJECT

  public:
    static const QString AUTH_METHOD_KEY;
    static const QString AUTH_METHOD_DESCRIPTION;
    static const QString AUTH_METHOD_DISPLAY_DESCRIPTION;

    explicit QgsAuthOAuth2Method();
    ~QgsAuthOAuth2Method() override;

    //! OAuth2 method key
    QString key() const override;

    //! OAuth2 method description
    QString description() const override;

    QString displayDescription() const override;

    //! Update network \a request with given \a authcfg and optional \a dataprovider
    bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider = QString() ) override;

    //! Update network \a reply with given \a authcfg and optional \a dataprovider
    bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg, const QString &dataprovider ) override;

    //! Update data source \a connectionItems with given \a authcfg and optional \a dataprovider
    bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg, const QString &dataprovider = QString() ) override;

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

    //! Triggered on reply finished
    void onReplyFinished();

    //! Triggered on network error
    void onNetworkError( QNetworkReply::NetworkError err );

    //! Triggered on refresh finished
    void onRefreshFinished( QNetworkReply::NetworkError err );

    //! Triggered when auth code needs to be manually entered by the user
    void onAuthCode();

#ifdef HAVE_GUI
    QWidget *editWidget( QWidget *parent ) const override;
#endif

  signals:

    //! Emitted when authcode was manually set by the user
    void setAuthCode( const QString code );

  private:
    QgsO2 *getOAuth2Bundle( const QString &authcfg, bool fullconfig = true );

    void putOAuth2Bundle( const QString &authcfg, QgsO2 *bundle );

    void removeOAuth2Bundle( const QString &authcfg );

    QReadWriteLock mO2CacheLock;
    QMap<QString, QgsO2 *> mOAuth2ConfigCache;

    QgsO2 *authO2( const QString &authcfg );

    // TODO: This mutex does not appear to be protecting anything which is thread-unsafe?
    QRecursiveMutex mNetworkRequestMutex;
};


class QgsAuthOAuth2MethodMetadata : public QgsAuthMethodMetadata
{
  public:
    QgsAuthOAuth2MethodMetadata()
      : QgsAuthMethodMetadata( QgsAuthOAuth2Method::AUTH_METHOD_KEY, QgsAuthOAuth2Method::AUTH_METHOD_DESCRIPTION )
    {}
    QgsAuthOAuth2Method *createAuthMethod() const override { return new QgsAuthOAuth2Method; }
    //QStringList supportedDataProviders() const override;
};

#endif // QGSAUTHOAUTH2METHOD_H
