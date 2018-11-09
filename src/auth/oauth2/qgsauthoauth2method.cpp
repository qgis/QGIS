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

#include "qgsauthoauth2method.h"

#include "o0globals.h"
#include "o0requestparameter.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgso2.h"
#include "qgsauthoauth2config.h"
#include "qgsauthoauth2edit.h"
#include "qgsnetworkaccessmanager.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"

#include <QDateTime>
#include <QInputDialog>
#include <QDesktopServices>
#include <QDir>
#include <QEventLoop>
#include <QString>
#include <QMutexLocker>


static const QString AUTH_METHOD_KEY = QStringLiteral( "OAuth2" );
static const QString AUTH_METHOD_DESCRIPTION = QStringLiteral( "OAuth2 authentication" );

QMap<QString, QgsO2 * > QgsAuthOAuth2Method::sOAuth2ConfigCache =
  QMap<QString, QgsO2 * >();


QgsAuthOAuth2Method::QgsAuthOAuth2Method()
{
  setVersion( 1 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::NetworkReply );
  setDataProviders( QStringList()
                    << QStringLiteral( "ows" )
                    << QStringLiteral( "wfs" )  // convert to lowercase
                    << QStringLiteral( "wcs" )
                    << QStringLiteral( "wms" ) );

  QStringList cachedirpaths;
  cachedirpaths << QgsAuthOAuth2Config::tokenCacheDirectory()
                << QgsAuthOAuth2Config::tokenCacheDirectory( true );

  Q_FOREACH ( const QString &cachedirpath, cachedirpaths )
  {
    QDir cachedir( cachedirpath );
    if ( !cachedir.mkpath( cachedirpath ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to create cache dir: %1" ).arg( cachedirpath ) );
    }
  }
}

QgsAuthOAuth2Method::~QgsAuthOAuth2Method()
{
  QDir tempdir( QgsAuthOAuth2Config::tokenCacheDirectory( true ) );
  QStringList dirlist = tempdir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  Q_FOREACH ( const QString &f, dirlist )
  {
    QString tempfile( tempdir.path() + QStringLiteral( "/" ) + f );
    if ( !QFile::remove( tempfile ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to delete temp token cache file: %1" ).arg( tempfile ) );
    }
  }
  if ( !tempdir.rmdir( tempdir.path() ) )
  {
    QgsDebugMsg( QStringLiteral( "FAILED to delete temp token cache directory: %1" ).arg( tempdir.path() ) );
  }
}

QString QgsAuthOAuth2Method::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthOAuth2Method::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthOAuth2Method::displayDescription() const
{
  return tr( "OAuth2 authentication" );
}

bool QgsAuthOAuth2Method::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  QMutexLocker locker( &mNetworkRequestMutex );

  QString msg;

  QgsO2 *o2 = getOAuth2Bundle( authcfg );
  if ( !o2 )
  {
    msg = QStringLiteral( "Update request FAILED for authcfg %1: null object for requestor" ).arg( authcfg );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return false;
  }

  if ( o2->linked() )
  {
    // Check if the cache file has been deleted outside core method routines
    QString tokencache = QgsAuthOAuth2Config::tokenCachePath( authcfg, !o2->oauth2config()->persistToken() );
    if ( !QFile::exists( tokencache ) )
    {
      msg = QStringLiteral( "Token cache removed for authcfg %1: unlinking authenticator" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
      o2->unlink();
    }
  }

  if ( o2->linked() )
  {
    // First, check if it is expired
    bool expired = false;
    if ( o2->expires() > 0 )  // QStringLiteral("").toInt() result for tokens with no expiration
    {
      int cursecs = static_cast<int>( QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000 );
      expired = ( ( o2->expires() - cursecs ) < 120 ); // try refresh with expired or two minutes to go
    }

    if ( expired )
    {
      msg = QStringLiteral( "Token expired, attempting refresh for authcfg %1" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

      // Try to get a refresh token first
      // go into local event loop and wait for a fired refresh-related slot
      QEventLoop rloop( nullptr );
      connect( o2, &QgsO2::refreshFinished, &rloop, &QEventLoop::quit );

      // Asynchronously attempt the refresh
      // TODO: This already has a timed reply setup in O2 base class (and in QgsNetworkAccessManager!)
      //       May need to address this or app crashes will occur!
      o2->refresh();

      // block request update until asynchronous linking loop is quit
      rloop.exec();

      // refresh result should set o2 to (un)linked
    }
  }

  if ( !o2->linked() )
  {
    // link app
    // clear any previous token session properties
    o2->unlink();

    connect( o2, &QgsO2::linkedChanged, this, &QgsAuthOAuth2Method::onLinkedChanged, Qt::UniqueConnection );
    connect( o2, &QgsO2::linkingFailed, this, &QgsAuthOAuth2Method::onLinkingFailed, Qt::UniqueConnection );
    connect( o2, &QgsO2::linkingSucceeded, this, &QgsAuthOAuth2Method::onLinkingSucceeded, Qt::UniqueConnection );
    connect( o2, &QgsO2::openBrowser, this, &QgsAuthOAuth2Method::onOpenBrowser, Qt::UniqueConnection );
    connect( o2, &QgsO2::closeBrowser, this, &QgsAuthOAuth2Method::onCloseBrowser, Qt::UniqueConnection );
    connect( o2, &QgsO2::getAuthCode, this, &QgsAuthOAuth2Method::onAuthCode, Qt::UniqueConnection );
    connect( this, &QgsAuthOAuth2Method::setAuthCode, o2,  &QgsO2::onSetAuthCode, Qt::UniqueConnection );
    //qRegisterMetaType<QNetworkReply::NetworkError>( QStringLiteral( "QNetworkReply::NetworkError" )) // for Qt::QueuedConnection, if needed;
    connect( o2, &QgsO2::refreshFinished, this, &QgsAuthOAuth2Method::onRefreshFinished, Qt::UniqueConnection );


    QgsSettings settings;
    QString timeoutkey = QStringLiteral( "qgis/networkAndProxy/networkTimeout" );
    int prevtimeout = settings.value( timeoutkey, QStringLiteral( "-1" ) ).toInt();
    int reqtimeout = o2->oauth2config()->requestTimeout() * 1000;
    settings.setValue( timeoutkey, reqtimeout );

    // go into local event loop and wait for a fired linking-related slot
    QEventLoop loop( nullptr );
    connect( o2, &QgsO2::linkingFailed, &loop, &QEventLoop::quit );
    connect( o2, &QgsO2::linkingSucceeded, &loop, &QEventLoop::quit );

    // add singlshot timer to quit linking after an alloted timeout
    // this should keep the local event loop from blocking forever
    QTimer timer( nullptr );
    timer.setInterval( reqtimeout * 5 );
    timer.setSingleShot( true );
    connect( &timer, &QTimer::timeout, o2, &QgsO2::linkingFailed );
    timer.start();

    // asynchronously attempt the linking
    o2->link();

    // block request update until asynchronous linking loop is quit
    loop.exec();
    if ( timer.isActive() )
    {
      timer.stop();
    }

    // don't re-apply a setting that wasn't already set
    if ( prevtimeout == -1 )
    {
      settings.remove( timeoutkey );
    }
    else
    {
      settings.setValue( timeoutkey, prevtimeout );
    }

    if ( !o2->linked() )
    {
      msg = QStringLiteral( "Update request FAILED for authcfg %1: requestor could not link app" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
      return false;
    }
  }

  if ( o2->token().isEmpty() )
  {
    msg = QStringLiteral( "Update request FAILED for authcfg %1: access token is empty" ).arg( authcfg );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return false;
  }

  // update the request
  QgsAuthOAuth2Config::AccessMethod accessmethod = o2->oauth2config()->accessMethod();

  QUrl url = request.url();
  QUrlQuery query( url );

  switch ( accessmethod )
  {
    case QgsAuthOAuth2Config::Header:
      request.setRawHeader( O2_HTTP_AUTHORIZATION_HEADER, QStringLiteral( "Bearer %1" ).arg( o2->token() ).toAscii() );
      msg = QStringLiteral( "Updated request HEADER with access token for authcfg: %1" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
      break;
    case QgsAuthOAuth2Config::Form:
      // FIXME: what to do here if the parent request is not POST?
      //        probably have to skip this until auth system support is moved into QgsNetworkAccessManager
      msg = QStringLiteral( "Update request FAILED for authcfg %1: form POST token update is unsupported" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
      break;
    case QgsAuthOAuth2Config::Query:
      if ( !query.hasQueryItem( O2_OAUTH2_ACCESS_TOKEN ) )
      {
        query.addQueryItem( O2_OAUTH2_ACCESS_TOKEN, o2->token() );
        url.setQuery( query );
        request.setUrl( url );
        msg = QStringLiteral( "Updated request QUERY with access token for authcfg: %1" ).arg( authcfg );
      }
      else
      {
        msg = QStringLiteral( "Updated request QUERY with access token SKIPPED (existing token) for authcfg: %1" ).arg( authcfg );
      }
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
      break;
  }

  return true;
}

bool QgsAuthOAuth2Method::updateNetworkReply( QNetworkReply *reply, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  QMutexLocker locker( &mNetworkRequestMutex );

  // TODO: handle token refresh error on the reply, see O2Requestor::onRequestError()
  // Is this doable if the errors are also handled in qgsapp (and/or elsewhere)?
  // Can we block as long as needed if the reply gets deleted elsewhere,
  // or will a local loop's connection keep it alive after a call to deletelater()?

  if ( !reply )
  {
    QString msg = QStringLiteral( "Updated reply with token refresh connection FAILED"
                                  " for authcfg %1: null reply object" ).arg( authcfg );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return false;
  }
  reply->setProperty( "authcfg", authcfg );

  // converting this to new-style Qt5 connection causes odd linking error with static o2 library
  connect( reply, SIGNAL( error( QNetworkReply::NetworkError ) ),
           this, SLOT( onNetworkError( QNetworkReply::NetworkError ) ), Qt::QueuedConnection );
  //connect( reply, static_cast<void ( QNetworkReply::* )( QNetworkReply::NetworkError )>( &QNetworkReply::error ),
  //         this, &QgsAuthOAuth2Method::onNetworkError, Qt::QueuedConnection );

  QString msg = QStringLiteral( "Updated reply with token refresh connection for authcfg: %1" ).arg( authcfg );
  QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

  return true;
}

void QgsAuthOAuth2Method::onLinkedChanged()
{
  // Linking (login) state has changed.
  // Use o2->linked() to get the actual state
  QgsDebugMsg( QStringLiteral( "Link state changed" ) );
}

void QgsAuthOAuth2Method::onLinkingFailed()
{
  // Login has failed
  QgsMessageLog::logMessage( tr( "Authenticator linking (login) has failed" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
}

void QgsAuthOAuth2Method::onLinkingSucceeded()
{
  QgsO2 *o2 = qobject_cast<QgsO2 *>( sender() );
  if ( !o2 )
  {
    QgsMessageLog::logMessage( tr( "Linking succeeded, but authenticator access FAILED: null object" ),
                               AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return;
  }

  if ( !o2->linked() )
  {
    QgsMessageLog::logMessage( tr( "Linking apparently succeeded, but authenticator FAILED to verify it is linked" ),
                               AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return;
  }

  QgsMessageLog::logMessage( tr( "Linking succeeded" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

  //###################### DO NOT LEAVE ME UNCOMMENTED ######################
  //QgsDebugMsg( QStringLiteral( "Access token: %1" ).arg( o2->token() ) );
  //QgsDebugMsg( QStringLiteral( "Access token secret: %1" ).arg( o2->tokenSecret() ) );
  //###################### DO NOT LEAVE ME UNCOMMENTED ######################

  QVariantMap extraTokens = o2->extraTokens();
  if ( !extraTokens.isEmpty() )
  {
    QString msg = QStringLiteral( "Extra tokens in response:\n" );
    Q_FOREACH ( const QString &key, extraTokens.keys() )
    {
      // don't expose the values in a log (unless they are only 3 chars long, of course)
      msg += QStringLiteral( "    %1:%2…\n" ).arg( key, extraTokens.value( key ).toString().left( 3 ) );
    }
    QgsDebugMsg( msg );
  }
}

void QgsAuthOAuth2Method::onOpenBrowser( const QUrl &url )
{
  // Open a web browser or a web view with the given URL.
  // The user will interact with this browser window to
  // enter login name, password, and authorize your application
  // to access the Twitter account
  QgsMessageLog::logMessage( tr( "Open browser requested" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

  QDesktopServices::openUrl( url );
}

void QgsAuthOAuth2Method::onCloseBrowser()
{
  // Close the browser window opened in openBrowser()
  QgsMessageLog::logMessage( tr( "Close browser requested" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

  // Bring focus back to QGIS app
  if ( qobject_cast<QApplication *>( qApp ) )
  {
    Q_FOREACH ( QWidget *topwdgt, QgsApplication::topLevelWidgets() )
    {
      if ( topwdgt->objectName() == QStringLiteral( "MainWindow" ) )
      {
        topwdgt->raise();
        topwdgt->activateWindow();
        topwdgt->show();
        break;
      }
    }
  }
}

void QgsAuthOAuth2Method::onReplyFinished()
{
  QgsMessageLog::logMessage( tr( "Network reply finished" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  QgsMessageLog::logMessage( tr( "Results: %1" ).arg( QString( reply->readAll() ) ),
                             AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
}

void QgsAuthOAuth2Method::onNetworkError( QNetworkReply::NetworkError err )
{
  QMutexLocker locker( &mNetworkRequestMutex );
  QString msg;
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  if ( !reply )
  {
    msg = tr( "Network error but no reply object accessible" );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return;
  }
  if ( err != QNetworkReply::NoError )
  {
    msg = tr( "Network error: %1" ).arg( reply->errorString() );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
  }

  // TODO: update debug messages to output to QGIS

  int status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  msg = tr( "Network error, HTTP status: %1" ).arg(
          reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString() );
  QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

  if ( status == 401 )
  {
    msg = tr( "Attempting token refresh…" );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

    QString authcfg = reply->property( "authcfg" ).toString();
    if ( authcfg.isEmpty() )
    {
      msg = tr( "Token refresh FAILED: authcfg empty" );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
      return;
    }

    // get the cached authenticator
    QgsO2 *o2 = getOAuth2Bundle( authcfg );

    if ( o2 )
    {
      // Call O2::refresh. Note the O2 instance might live in a different thread from reply,
      // so don't block here. User will just have to re-attempt connection
      o2->refresh();

      msg = tr( "Background token refresh underway for authcfg: %1" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
    }
    else
    {
      msg = tr( "Background token refresh FAILED for authcfg %1: could not get authenticator object" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    }
  }
}

void QgsAuthOAuth2Method::onRefreshFinished( QNetworkReply::NetworkError err )
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  if ( err != QNetworkReply::NoError )
  {
    QgsMessageLog::logMessage( tr( "Token refresh error: %1" ).arg( reply->errorString() ),
                               AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
  }
}

void QgsAuthOAuth2Method::onAuthCode()
{
  bool ok = false;
  QString code = QInputDialog::getText( QApplication::activeWindow(), QStringLiteral( "Enter the authorization code" ), QStringLiteral( "Authoriation code" ), QLineEdit::Normal, QStringLiteral( "Required" ), &ok, Qt::Dialog, Qt::InputMethodHint::ImhNone );
  if ( ok && !code.isEmpty() )
  {
    emit setAuthCode( code );
  }
}

bool QgsAuthOAuth2Method::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( connectionItems )
  Q_UNUSED( authcfg )
  Q_UNUSED( dataprovider )

  return true;
}

void QgsAuthOAuth2Method::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsg( QStringLiteral( "Updating old style auth method config" ) );
  }

  // NOTE: add updates as method version() increases due to config storage changes
}

void QgsAuthOAuth2Method::clearCachedConfig( const QString &authcfg )
{
  removeOAuth2Bundle( authcfg );
}

QgsO2 *QgsAuthOAuth2Method::getOAuth2Bundle( const QString &authcfg, bool fullconfig )
{
  // TODO: update to QgsMessageLog output where appropriate

  // check if it is cached
  if ( sOAuth2ConfigCache.contains( authcfg ) )
  {
    QgsDebugMsg( QStringLiteral( "Retrieving OAuth bundle for authcfg: %1" ).arg( authcfg ) );
    return sOAuth2ConfigCache.value( authcfg );
  }

  QgsAuthOAuth2Config *config = new QgsAuthOAuth2Config( );
  QgsO2 *nullbundle =  nullptr;

  // else build oauth2 config
  QgsAuthMethodConfig mconfig;
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugMsg( QStringLiteral( "Retrieve config FAILED for authcfg: %1" ).arg( authcfg ) );
    config->deleteLater();
    return nullbundle;
  }

  QgsStringMap configmap = mconfig.configMap();

  // do loading of method config into oauth2 config

  if ( configmap.contains( QStringLiteral( "oauth2config" ) ) )
  {
    QByteArray configtxt = configmap.value( QStringLiteral( "oauth2config" ) ).toUtf8();
    if ( configtxt.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load OAuth2 config: empty config txt" ) );
      config->deleteLater();
      return nullbundle;
    }
    //###################### DO NOT LEAVE ME UNCOMMENTED #####################
    //QgsDebugMsg( QStringLiteral( "LOAD oauth2config configtxt: \n\n%1\n\n" ).arg( QString( configtxt ) ) );
    //###################### DO NOT LEAVE ME UNCOMMENTED #####################

    if ( !config->loadConfigTxt( configtxt, QgsAuthOAuth2Config::JSON ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load OAuth2 config into object" ) );
      config->deleteLater();
      return nullbundle;
    }
  }
  else if ( configmap.contains( QStringLiteral( "definedid" ) ) )
  {
    bool ok = false;
    QString definedid = configmap.value( QStringLiteral( "definedid" ) );
    if ( definedid.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load a defined ID for OAuth2 config" ) );
      config->deleteLater();
      return nullbundle;
    }

    QString extradir = configmap.value( QStringLiteral( "defineddirpath" ) );
    if ( extradir.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "No custom defined dir path to load OAuth2 config" ) );
    }

    QgsStringMap definedcache = QgsAuthOAuth2Config::mappedOAuth2ConfigsCache( this, extradir );

    if ( !definedcache.contains( definedid ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load OAuth2 config for defined ID: missing ID or file for %1" ).arg( definedid ) );
      config->deleteLater();
      return nullbundle;
    }

    QByteArray definedtxt = definedcache.value( definedid ).toUtf8();
    if ( definedtxt.isNull() || definedtxt.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load config text for defined ID: empty text for %1" ).arg( definedid ) );
      config->deleteLater();
      return nullbundle;
    }

    if ( !config->loadConfigTxt( definedtxt, QgsAuthOAuth2Config::JSON ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load config text for defined ID: %1" ).arg( definedid ) );
      config->deleteLater();
      return nullbundle;
    }

    QByteArray querypairstxt = configmap.value( QStringLiteral( "querypairs" ) ).toUtf8();
    if ( !querypairstxt.isNull() && !querypairstxt.isEmpty() )
    {
      QVariantMap querypairsmap =
        QgsAuthOAuth2Config::variantFromSerialized( querypairstxt, QgsAuthOAuth2Config::JSON, &ok );
      if ( !ok )
      {
        QgsDebugMsg( QStringLiteral( "No query pairs to load OAuth2 config: FAILED to parse" ) );
      }
      if ( querypairsmap.isEmpty() )
      {
        QgsDebugMsg( QStringLiteral( "No query pairs to load OAuth2 config: parsed pairs are empty" ) );
      }
      else
      {
        config->setQueryPairs( querypairsmap );
      }
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "No query pairs to load OAuth2 config: empty text" ) );
    }
  }

  // TODO: instantiate particular QgsO2 subclassed authenticators relative to config ???

  QgsDebugMsg( QStringLiteral( "Loading authenticator object with %1 flow properties of OAuth2 config: %2" )
               .arg( QgsAuthOAuth2Config::grantFlowString( config->grantFlow() ), authcfg ) );

  QgsO2 *o2 = new QgsO2( authcfg, config, nullptr, QgsNetworkAccessManager::instance() );

  // cache bundle
  putOAuth2Bundle( authcfg, o2 );

  return o2;
}

void QgsAuthOAuth2Method::putOAuth2Bundle( const QString &authcfg, QgsO2 *bundle )
{
  QgsDebugMsg( QStringLiteral( "Putting oauth2 bundle for authcfg: %1" ).arg( authcfg ) );
  sOAuth2ConfigCache.insert( authcfg, bundle );
}

void QgsAuthOAuth2Method::removeOAuth2Bundle( const QString &authcfg )
{
  if ( sOAuth2ConfigCache.contains( authcfg ) )
  {
    sOAuth2ConfigCache.value( authcfg )->deleteLater();
    sOAuth2ConfigCache.remove( authcfg );
    QgsDebugMsg( QStringLiteral( "Removed oauth2 bundle for authcfg: %1" ).arg( authcfg ) );
  }
}


//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////

/**
 * Required class factory to return a pointer to a newly created object
 */
QGISEXTERN QgsAuthOAuth2Method *classFactory()
{
  return new QgsAuthOAuth2Method();
}

/**
 * Required key function (used to map the plugin to a data store type)
 */
QGISEXTERN QString authMethodKey()
{
  return AUTH_METHOD_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return AUTH_METHOD_DESCRIPTION;
}

/**
 * Required isAuthMethod function. Used to determine if this shared library
 * is an authentication method plugin
 */
QGISEXTERN bool isAuthMethod()
{
  return true;
}

/**
 * Optional class factory to return a pointer to a newly created edit widget
 */
QGISEXTERN QgsAuthOAuth2Edit *editWidget( QWidget *parent )
{
  return new QgsAuthOAuth2Edit( parent );
}

/**
 * Required cleanup function
 */
QGISEXTERN void cleanupAuthMethod() // pass QgsAuthMethod *method, then delete method  ?
{
}
