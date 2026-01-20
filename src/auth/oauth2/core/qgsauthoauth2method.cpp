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
#include "qgsauthoauth2config.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgso2.h"
#include "qgsproject.h"
#include "qgsprovidermetadata.h"
#include "qgsreadwritelocker.h"

#include "moc_qgsauthoauth2method.cpp"

#ifdef HAVE_GUI
#include "qgsauthoauth2edit.h"
#endif

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QPointer>
#include <QString>
#include <QMutexLocker>
#include <QUrlQuery>
#ifdef HAVE_GUI
#include <QInputDialog>
#endif

const QString QgsAuthOAuth2Method::AUTH_METHOD_KEY = u"OAuth2"_s;
const QString QgsAuthOAuth2Method::AUTH_METHOD_DESCRIPTION = u"OAuth2 authentication"_s;
const QString QgsAuthOAuth2Method::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "OAuth2 authentication" );


//
// QgsOAuth2Factory
//

QgsOAuth2Factory *QgsOAuth2Factory::sInstance = nullptr;

QgsOAuth2Factory::QgsOAuth2Factory( QObject *parent )
  : QThread( parent )
{
  // YES, this IS correct in this context!
  moveToThread( this );
  start();
}

QgsOAuth2Factory *QgsOAuth2Factory::instance()
{
  static QMutex sMutex;
  const QMutexLocker locker( &sMutex );
  if ( !sInstance )
  {
    sInstance = new QgsOAuth2Factory();
  }
  return sInstance;
}

QgsO2 *QgsOAuth2Factory::createO2( const QString &authcfg, QgsAuthOAuth2Config *oauth2config )
{
  return instance()->createO2Private( authcfg, oauth2config );
}

void QgsOAuth2Factory::requestLink( QgsO2 *o2 )
{
#ifndef __clang_analyzer__
  if ( QThread::currentThread() == o2->thread() )
    o2->link();
  else
    QMetaObject::invokeMethod( o2, &QgsO2::link, Qt::BlockingQueuedConnection );
#else
  ( void ) o2;
#endif
}

QgsO2 *QgsOAuth2Factory::createO2Private( const QString &authcfg, QgsAuthOAuth2Config *oauth2config )
{
  QgsO2 *o2 = nullptr;
  auto createO2InThread = [&o2, authcfg, oauth2config, this] {
    Q_ASSERT( QThread::currentThread() == this );
    oauth2config->moveToThread( this );
    o2 = new QgsO2( authcfg, oauth2config, nullptr, QgsNetworkAccessManager::instance() );
  };

  Q_ASSERT( isRunning() );

  // Make sure that O2 objects are created on the factory thread only!
  if ( QThread::currentThread() == this )
    createO2InThread();
  else
  {
    oauth2config->moveToThread( nullptr );
#ifndef __clang_analyzer__
    QMetaObject::invokeMethod( this, std::move( createO2InThread ), Qt::BlockingQueuedConnection );
#endif
  }
  Q_ASSERT( o2->thread() == this );

  return o2;
}

//
// QgsAuthOAuth2Method
//

QgsAuthOAuth2Method::QgsAuthOAuth2Method()
{
  setVersion( 1 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::NetworkReply );
  setDataProviders( QStringList() << u"ows"_s << u"wfs"_s // convert to lowercase
                                  << u"wcs"_s << u"wms"_s );

  const QStringList cachedirpaths = QStringList()
                                    << QgsAuthOAuth2Config::tokenCacheDirectory()
                                    << QgsAuthOAuth2Config::tokenCacheDirectory( true );

  for ( const QString &cachedirpath : cachedirpaths )
  {
    const QDir cachedir( cachedirpath );
    if ( !cachedir.mkpath( cachedirpath ) )
    {
      QgsDebugError( u"FAILED to create cache dir: %1"_s.arg( cachedirpath ) );
    }
  }

  // Fires every 15 minutes
  connect( &mCacheHousekeepingTimer, &QTimer::timeout, this, &QgsAuthOAuth2Method::cleanupCache );
#ifdef QGISDEBUG
  // A hidden setting can be used to adjust the interval for testing purposes
  const int interval = QgsSettings().value( u"oauth2/cacheHousekeepingInterval"_s, 15 * 60 * 1000, QgsSettings::Section::Auth ).toInt();
#else
  constexpr int interval = 15 * 60 * 1000;
#endif
  connect( QgsProject::instance(), &QgsProject::layersRemoved, this, [this]( const QStringList & ) {
    mCacheHousekeepingTimer.stop();
    cleanupCache();
    mCacheHousekeepingTimer.start();
  } );
  mCacheHousekeepingTimer.setInterval( static_cast<std::chrono::milliseconds>( interval ) );
  mCacheHousekeepingTimer.start();
}

QgsAuthOAuth2Method::~QgsAuthOAuth2Method()
{
  const QDir tempdir( QgsAuthOAuth2Config::tokenCacheDirectory( true ) );
  const QStringList dirlist = tempdir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  for ( const QString &f : dirlist )
  {
    const QString tempfile( tempdir.path() + '/' + f );
    if ( !QFile::remove( tempfile ) )
    {
      QgsDebugError( u"FAILED to delete temp token cache file: %1"_s.arg( tempfile ) );
    }
  }
  if ( !tempdir.rmdir( tempdir.path() ) )
  {
    QgsDebugError( u"FAILED to delete temp token cache directory: %1"_s.arg( tempdir.path() ) );
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
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}

bool QgsAuthOAuth2Method::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  const QMutexLocker locker( &mNetworkRequestMutex );

  QString msg;

  QgsO2 *o2 = getOAuth2Bundle( authcfg );
  if ( !o2 )
  {
    msg = u"Update request FAILED for authcfg %1: null object for requestor"_s.arg( authcfg );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return false;
  }

  if ( o2->linked() )
  {
    // Check if the cache file has been deleted outside core method routines
    const QString tokencache = QgsAuthOAuth2Config::tokenCachePath( authcfg, !o2->oauth2config()->persistToken() );
    if ( !QFile::exists( tokencache ) )
    {
      msg = u"Token cache removed for authcfg %1: unlinking authenticator"_s.arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
      o2->unlink();
    }
  }

  if ( o2->linked() )
  {
    // First, check if it is expired
    bool expired = false;
    if ( o2->expires() > 0 ) // u""_s.toInt() result for tokens with no expiration
    {
      const int cursecs = static_cast<int>( QDateTime::currentMSecsSinceEpoch() / 1000 );
      const int lExpirationDelay = o2->expirationDelay();
      // try refresh with expired or two minutes to go (or a fraction of the initial expiration delay if it is short)
      const int refreshThreshold = lExpirationDelay > 0 ? std::min( 120, std::max( 2, lExpirationDelay / 10 ) ) : 120;
      expired = ( ( o2->expires() - cursecs ) < refreshThreshold );
    }

    if ( expired )
    {
      if ( o2->refreshToken().isEmpty() || o2->refreshTokenUrl().isEmpty() )
      {
        msg = u"Token expired, but no refresh token or URL defined for authcfg %1"_s.arg( authcfg );
        QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
        // clear any previous token session properties
        o2->unlink();
      }
      else
      {
        msg = u"Token expired, attempting refresh for authcfg %1"_s.arg( authcfg );
        QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

        // Try to get a refresh token first
        o2->refreshSynchronous();

        // refresh result should set o2 to (un)linked
        if ( o2->linked() )
        {
          o2->computeExpirationDelay();
        }
      }
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
    connect( o2, &QgsO2::getAuthCode, this, &QgsAuthOAuth2Method::onAuthCode, Qt::UniqueConnection );
    connect( this, &QgsAuthOAuth2Method::setAuthCode, o2, &QgsO2::onSetAuthCode, Qt::UniqueConnection );
    connect( o2, &QgsO2::refreshFinished, this, &QgsAuthOAuth2Method::onRefreshFinished, Qt::UniqueConnection );


    const int prevtimeout = QgsNetworkAccessManager::settingsNetworkTimeout->value();
    const int reqtimeout = o2->oauth2config()->requestTimeout() * 1000;
    QgsNetworkAccessManager::settingsNetworkTimeout->setValue( reqtimeout );

    // go into local event loop and wait for a fired linking-related slot
    QEventLoop loop( nullptr );
    connect( o2, &QgsO2::linkingFailed, &loop, &QEventLoop::quit );
    connect( o2, &QgsO2::linkingSucceeded, &loop, &QEventLoop::quit );
    connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::authBrowserAborted, &loop, &QEventLoop::quit );

    // add single shot timer to quit linking after an allotted timeout
    // this should keep the local event loop from blocking forever
    QTimer timer( nullptr );
    timer.setInterval( reqtimeout * 5 );
    timer.setSingleShot( true );
    connect( &timer, &QTimer::timeout, o2, &QgsO2::linkingFailed );
    timer.start();

    // asynchronously attempt the linking
    QgsOAuth2Factory::requestLink( o2 );

    // block request update until asynchronous linking loop is quit
    loop.exec();
    if ( timer.isActive() )
    {
      timer.stop();
    }

    // don't re-apply a setting that wasn't already set
    if ( !prevtimeout )
    {
      QgsNetworkAccessManager::settingsNetworkTimeout->remove();
    }
    else
    {
      QgsNetworkAccessManager::settingsNetworkTimeout->setValue( prevtimeout );
    }

    if ( !o2->linked() )
    {
      msg = u"Update request FAILED for authcfg %1: requestor could not link app"_s.arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
      return false;
    }

    o2->computeExpirationDelay();
  }

  if ( o2->token().isEmpty() )
  {
    msg = u"Update request FAILED for authcfg %1: access token is empty"_s.arg( authcfg );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return false;
  }

  // update the request
  const QgsAuthOAuth2Config::AccessMethod accessmethod = o2->oauth2config()->accessMethod();

  QUrl url = request.url();
  QUrlQuery query( url );

  switch ( accessmethod )
  {
    case QgsAuthOAuth2Config::AccessMethod::Header:
    {
      const QString header = o2->oauth2config()->customHeader().isEmpty() ? QString( O2_HTTP_AUTHORIZATION_HEADER ) : o2->oauth2config()->customHeader();
      request.setRawHeader( header.toLatin1(), u"Bearer %1"_s.arg( o2->token() ).toLatin1() );

      const QVariantMap extraTokens = o2->oauth2config()->extraTokens();
      if ( !extraTokens.isEmpty() )
      {
        const QVariantMap receivedExtraTokens = o2->extraTokens();
        const QStringList extraTokenNames = extraTokens.keys();
        for ( const QString &extraTokenName : extraTokenNames )
        {
          if ( receivedExtraTokens.contains( extraTokenName ) )
          {
            request.setRawHeader( extraTokens[extraTokenName].toString().replace( '_', '-' ).toLatin1(), receivedExtraTokens[extraTokenName].toString().toLatin1() );
          }
        }
      }

#ifdef QGISDEBUG
      msg = u"Updated request HEADER with access token for authcfg: %1"_s.arg( authcfg );
      QgsDebugMsgLevel( msg, 2 );
#endif
      break;
    }
    case QgsAuthOAuth2Config::AccessMethod::Form:
      // FIXME: what to do here if the parent request is not POST?
      //        probably have to skip this until auth system support is moved into QgsNetworkAccessManager
      msg = u"Update request FAILED for authcfg %1: form POST token update is unsupported"_s.arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
      break;
    case QgsAuthOAuth2Config::AccessMethod::Query:
      if ( !query.hasQueryItem( O2_OAUTH2_ACCESS_TOKEN ) )
      {
        query.addQueryItem( O2_OAUTH2_ACCESS_TOKEN, o2->token() );
        url.setQuery( query );
        request.setUrl( url );
#ifdef QGISDEBUG
        msg = u"Updated request QUERY with access token for authcfg: %1"_s.arg( authcfg );
#endif
      }
      else
      {
#ifdef QGISDEBUG
        msg = u"Updated request QUERY with access token SKIPPED (existing token) for authcfg: %1"_s.arg( authcfg );
#endif
      }
      QgsDebugMsgLevel( msg, 2 );
      break;
  }

  return true;
}

bool QgsAuthOAuth2Method::updateNetworkReply( QNetworkReply *reply, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mNetworkRequestMutex );

  // TODO: handle token refresh error on the reply, see O2Requestor::onRequestError()
  // Is this doable if the errors are also handled in qgsapp (and/or elsewhere)?
  // Can we block as long as needed if the reply gets deleted elsewhere,
  // or will a local loop's connection keep it alive after a call to deletelater()?

  if ( !reply )
  {
    const QString msg = QStringLiteral( "Updated reply with token refresh connection FAILED"
                                        " for authcfg %1: null reply object" )
                          .arg( authcfg );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return false;
  }
  reply->setProperty( "authcfg", authcfg );

  connect( reply, &QNetworkReply::errorOccurred, this, &QgsAuthOAuth2Method::onNetworkError, Qt::QueuedConnection );

#ifdef QGISDEBUG
  const QString msg = u"Updated reply with token refresh connection for authcfg: %1"_s.arg( authcfg );
  QgsDebugMsgLevel( msg, 2 );
#endif

  return true;
}

void QgsAuthOAuth2Method::onLinkedChanged()
{
  // Linking (login) state has changed.
  // Use o2->linked() to get the actual state
  QgsDebugMsgLevel( u"Link state changed"_s, 2 );
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
    QgsMessageLog::logMessage( tr( "Linking succeeded, but authenticator access FAILED: null object" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return;
  }

  if ( !o2->linked() )
  {
    QgsMessageLog::logMessage( tr( "Linking apparently succeeded, but authenticator FAILED to verify it is linked" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return;
  }

  QgsMessageLog::logMessage( tr( "Linking succeeded" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Info );

  //###################### DO NOT LEAVE ME UNCOMMENTED ######################
  //QgsDebugMsgLevel( u"Access token: %1"_s.arg( o2->token() ), 2 );
  //QgsDebugMsgLevel( u"Access token secret: %1"_s.arg( o2->tokenSecret() ), 2 );
  //###################### DO NOT LEAVE ME UNCOMMENTED ######################

  const QVariantMap extraTokens = o2->extraTokens();
  if ( !extraTokens.isEmpty() )
  {
    QString msg = u"Extra tokens in response:\n"_s;
    const QStringList extraTokenKeys = extraTokens.keys();
    for ( const QString &key : extraTokenKeys )
    {
      // don't expose the values in a log (unless they are only 3 chars long, of course)
      msg += u"    %1:%2…\n"_s.arg( key, extraTokens.value( key ).toString().left( 3 ) );
    }
    QgsDebugMsgLevel( msg, 2 );
  }
}

void QgsAuthOAuth2Method::onReplyFinished()
{
  QgsMessageLog::logMessage( tr( "Network reply finished" ), AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  if ( !reply )
  {
    const QString msg = tr( "Network reply finished but no reply object accessible" );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    return;
  }
  QgsMessageLog::logMessage( tr( "Results: %1" ).arg( QString( reply->readAll() ) ), AUTH_METHOD_KEY, Qgis::MessageLevel::Info );
}

void QgsAuthOAuth2Method::onNetworkError( QNetworkReply::NetworkError err )
{
  const QMutexLocker locker( &mNetworkRequestMutex );
  QString msg;
  const QPointer<QNetworkReply> reply = qobject_cast<QNetworkReply *>( sender() );
  if ( reply.isNull() )
  {
#ifdef QGISDEBUG
    msg = tr( "Network error but no reply object accessible" );
    QgsDebugError( msg );
#endif
    return;
  }

  // Grab some reply properties before object is deleted elsewhere
  const QVariant replyStatus = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
  const QVariant replyAuthProp = reply->property( "authcfg" );
  const QString replyErrString = reply->errorString();

  if ( err != QNetworkReply::NoError && err != QNetworkReply::OperationCanceledError )
  {
    msg = tr( "Network error: %1" ).arg( replyErrString );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
  }

  if ( !replyStatus.isValid() )
  {
    if ( err != QNetworkReply::OperationCanceledError )
    {
      msg = tr( "Network error but no reply object attributes found" );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
    }
    return;
  }

  if ( replyStatus.toInt() == 401 )
  {
    msg = tr( "Attempting token refresh…" );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Info );


    if ( !replyAuthProp.isValid() )
    {
      msg = tr( "Token refresh FAILED: authcfg property invalid" );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
      return;
    }
    const QString authcfg = replyAuthProp.toString();
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

static QString networkErrorToString( QNetworkReply::NetworkError code )
{
  const int index = QNetworkReply::staticMetaObject.indexOfEnumerator( "NetworkError" );
  const QMetaEnum metaEnum = QNetworkReply::staticMetaObject.enumerator( index );
  return QString::fromLatin1( metaEnum.valueToKey( code ) );
}

void QgsAuthOAuth2Method::onRefreshFinished( QNetworkReply::NetworkError err )
{
  if ( err != QNetworkReply::NoError )
  {
    QgsMessageLog::logMessage( tr( "Token refresh error: %1" ).arg( networkErrorToString( err ) ), AUTH_METHOD_KEY, Qgis::MessageLevel::Warning );
  }
}

void QgsAuthOAuth2Method::onAuthCode()
{
#ifdef WITH_GUI
  bool ok = false;
  QString code = QInputDialog::getText( QApplication::activeWindow(), u"Authoriation Code"_s, u"Enter the authorization code"_s, QLineEdit::Normal, u"Required"_s, &ok, Qt::Dialog, Qt::InputMethodHint::ImhNone );
  if ( ok && !code.isEmpty() )
  {
    emit setAuthCode( code );
  }
#endif
}

bool QgsAuthOAuth2Method::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( connectionItems )
  Q_UNUSED( authcfg )
  Q_UNUSED( dataprovider )

  return true;
}

void QgsAuthOAuth2Method::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( u"oldconfigstyle"_s ) )
  {
    QgsDebugMsgLevel( u"Updating old style auth method config"_s, 2 );
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
  QgsReadWriteLocker locker( mO2CacheLock, QgsReadWriteLocker::Read );
  if ( QgsO2 *cachedBundle = mOAuth2ConfigCache.value( authcfg ) )
  {
    QgsDebugMsgLevel( u"Retrieving OAuth bundle for authcfg: %1"_s.arg( authcfg ), 2 );
    return cachedBundle;
  }
  locker.unlock();

  // else build oauth2 config
  QgsAuthMethodConfig mconfig;
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugError( u"Retrieve config FAILED for authcfg: %1"_s.arg( authcfg ) );
    return nullptr;
  }

  const QgsStringMap configmap = mconfig.configMap();

  // do loading of method config into oauth2 config

  auto config = std::make_unique<QgsAuthOAuth2Config>();
  if ( configmap.contains( u"oauth2config"_s ) )
  {
    const QByteArray configtxt = configmap.value( u"oauth2config"_s ).toUtf8();
    if ( configtxt.isEmpty() )
    {
      QgsDebugError( u"FAILED to load OAuth2 config: empty config txt"_s );
      return nullptr;
    }
    //###################### DO NOT LEAVE ME UNCOMMENTED #####################
    //QgsDebugMsgLevel( u"LOAD oauth2config configtxt: \n\n%1\n\n"_s.arg( QString( configtxt ) ), 2 );
    //###################### DO NOT LEAVE ME UNCOMMENTED #####################

    if ( !config->loadConfigTxt( configtxt, QgsAuthOAuth2Config::ConfigFormat::JSON ) )
    {
      QgsDebugError( u"FAILED to load OAuth2 config into object"_s );
      return nullptr;
    }
  }
  else if ( configmap.contains( u"definedid"_s ) )
  {
    bool ok = false;
    const QString definedid = configmap.value( u"definedid"_s );
    if ( definedid.isEmpty() )
    {
      QgsDebugError( u"FAILED to load a defined ID for OAuth2 config"_s );
      return nullptr;
    }

    const QString extradir = configmap.value( u"defineddirpath"_s );
    if ( extradir.isEmpty() )
    {
      QgsDebugError( u"No custom defined dir path to load OAuth2 config"_s );
    }

    const QgsStringMap definedcache = QgsAuthOAuth2Config::mappedOAuth2ConfigsCache( this, extradir );

    if ( !definedcache.contains( definedid ) )
    {
      QgsDebugError( u"FAILED to load OAuth2 config for defined ID: missing ID or file for %1"_s.arg( definedid ) );
      return nullptr;
    }

    const QByteArray definedtxt = definedcache.value( definedid ).toUtf8();
    if ( definedtxt.isNull() || definedtxt.isEmpty() )
    {
      QgsDebugError( u"FAILED to load config text for defined ID: empty text for %1"_s.arg( definedid ) );
      return nullptr;
    }

    if ( !config->loadConfigTxt( definedtxt, QgsAuthOAuth2Config::ConfigFormat::JSON ) )
    {
      QgsDebugError( u"FAILED to load config text for defined ID: %1"_s.arg( definedid ) );
      return nullptr;
    }

    const QByteArray querypairstxt = configmap.value( u"querypairs"_s ).toUtf8();
    if ( !querypairstxt.isNull() && !querypairstxt.isEmpty() )
    {
      const QVariantMap querypairsmap = QgsAuthOAuth2Config::variantFromSerialized( querypairstxt, QgsAuthOAuth2Config::ConfigFormat::JSON, &ok );
      if ( !ok )
      {
        QgsDebugError( u"No query pairs to load OAuth2 config: FAILED to parse"_s );
      }
      if ( querypairsmap.isEmpty() )
      {
        QgsDebugError( u"No query pairs to load OAuth2 config: parsed pairs are empty"_s );
      }
      else
      {
        config->setQueryPairs( querypairsmap );
      }
    }
    else
    {
      QgsDebugError( u"No query pairs to load OAuth2 config: empty text"_s );
    }
  }

  // TODO: instantiate particular QgsO2 subclassed authenticators relative to config ???

  QgsDebugMsgLevel( u"Loading authenticator object with %1 flow properties of OAuth2 config: %2"_s.arg( QgsAuthOAuth2Config::grantFlowString( config->grantFlow() ), authcfg ), 2 );

  QgsO2 *o2 = QgsOAuth2Factory::createO2( authcfg, config.release() );

  // cache bundle
  putOAuth2Bundle( authcfg, o2 );

  return o2;
}

void QgsAuthOAuth2Method::putOAuth2Bundle( const QString &authcfg, QgsO2 *bundle )
{
  QgsReadWriteLocker locker( mO2CacheLock, QgsReadWriteLocker::Write );
  QgsDebugMsgLevel( u"Putting oauth2 bundle for authcfg: %1"_s.arg( authcfg ), 2 );
  mOAuth2ConfigCache.insert( authcfg, bundle );
  // Restart the timer so that we have a full interval before the next cleanup
  mCacheHousekeepingTimer.start();
}

void QgsAuthOAuth2Method::removeOAuth2Bundle( const QString &authcfg )
{
  QgsReadWriteLocker locker( mO2CacheLock, QgsReadWriteLocker::Read );
  auto it = mOAuth2ConfigCache.find( authcfg );
  if ( it != mOAuth2ConfigCache.end() )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    it.value()->deleteLater();
    mOAuth2ConfigCache.erase( it );
    QgsDebugMsgLevel( u"Removed oauth2 bundle for authcfg: %1"_s.arg( authcfg ), 2 );
  }
}

void QgsAuthOAuth2Method::cleanupCache()
{
  if ( mOAuth2ConfigCache.isEmpty() )
  {
    return;
  }

  QSet<QString> authcfgInUse;
  const QMap<QString, QgsMapLayer *> allMapLayers = QgsProject::instance()->mapLayers();
  for ( auto it = allMapLayers.constBegin(); it != allMapLayers.constEnd(); ++it )
  {
    QgsMapLayer *mapLayer = it.value();
    const QVariantMap uriParts { mapLayer->providerMetadata()->decodeUri( mapLayer->source() ) };
    const QString authCfg { uriParts.value( u"authcfg"_s, QString() ).toString() };
    if ( !authCfg.isEmpty() )
    {
      authcfgInUse.insert( authCfg );
    }
  }

  QgsReadWriteLocker locker( mO2CacheLock, QgsReadWriteLocker::Read );
  const QStringList authKeys { mOAuth2ConfigCache.keys() };
  for ( const QString &cachedAuth : std::as_const( authKeys ) )
  {
    if ( !authcfgInUse.contains( cachedAuth ) )
    {
      auto it = mOAuth2ConfigCache.find( cachedAuth );
      if ( it != mOAuth2ConfigCache.end() )
      {
        // The timer may live in another thread: enqueue a call to stop it
        if ( QThread::currentThread() == it.value()->thread() )
        {
          it.value()->stopRefreshTimer();
        }
        else
        {
          // Suppress warning: Potential leak of memory pointed to by 'callable' [clang-analyzer-cplusplus.NewDeleteLeaks]
#ifndef __clang_analyzer__
          QMetaObject::invokeMethod( it.value(), &QgsO2::stopRefreshTimer, Qt::QueuedConnection );
#endif
        }
      }
    }
  }
}

#ifdef HAVE_GUI
QWidget *QgsAuthOAuth2Method::editWidget( QWidget *parent ) const
{
  return new QgsAuthOAuth2Edit( parent );
}
#endif

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthOAuth2MethodMetadata();
}
#endif
