/***************************************************************************
    qgsauthbasicmethod.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthbasicmethod.h"

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"

#include "moc_qgsauthbasicmethod.cpp"

#ifdef HAVE_GUI
#include "qgsauthbasicedit.h"
#endif

#include <QNetworkProxy>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QUuid>

const QString QgsAuthBasicMethod::AUTH_METHOD_KEY = u"Basic"_s;
const QString QgsAuthBasicMethod::AUTH_METHOD_DESCRIPTION = u"Basic authentication"_s;
const QString QgsAuthBasicMethod::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "Basic authentication" );

QMap<QString, QgsAuthMethodConfig> QgsAuthBasicMethod::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthBasicMethod::QgsAuthBasicMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceUri );
  setDataProviders( QStringList() << u"postgres"_s << u"oracle"_s << u"ows"_s << u"wfs"_s // convert to lowercase
                                  << u"wcs"_s << u"wms"_s << u"ogr"_s << u"gdal"_s << u"proxy"_s );
}

QString QgsAuthBasicMethod::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthBasicMethod::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthBasicMethod::displayDescription() const
{
  return AUTH_METHOD_DISPLAY_DESCRIPTION;
}


bool QgsAuthBasicMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugError( u"Update request config FAILED for authcfg: %1: config invalid"_s.arg( authcfg ) );
    return false;
  }

  const QString username = mconfig.config( u"username"_s );
  const QString password = mconfig.config( u"password"_s );

  if ( !username.isEmpty() )
  {
    request.setRawHeader( "Authorization", "Basic " + u"%1:%2"_s.arg( username, password ).toUtf8().toBase64() );
  }
  return true;
}

bool QgsAuthBasicMethod::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mMutex );
  const QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugError( u"Update URI items FAILED for authcfg: %1: basic config invalid"_s.arg( authcfg ) );
    return false;
  }

  QString username = mconfig.config( u"username"_s );
  QString password = mconfig.config( u"password"_s );

  if ( username.isEmpty() )
  {
    QgsDebugError( u"Update URI items FAILED for authcfg: %1: username empty"_s.arg( authcfg ) );
    return false;
  }

  QString sslMode = u"prefer"_s;
  const thread_local QRegularExpression sslModeRegExp( "^sslmode=.*" );
  const int sslModeIdx = connectionItems.indexOf( sslModeRegExp );
  if ( sslModeIdx != -1 )
  {
    sslMode = connectionItems.at( sslModeIdx ).split( '=' ).at( 1 );
  }

  // SSL Extra CAs
  QString caparam;
  QList<QSslCertificate> cas;
  if ( sslMode.startsWith( "verify-"_L1 ) )
  {
    cas = QgsApplication::authManager()->trustedCaCerts();
    // save CAs to temp file
    const QString tempFileBase = u"tmp_basic_%1.pem"_s;
    const QString caFilePath = QgsAuthCertUtils::pemTextToTempFile(
      tempFileBase.arg( QUuid::createUuid().toString() ),
      QgsAuthCertUtils::certsToPemText( cas )
    );
    if ( !caFilePath.isEmpty() )
    {
      caparam = "sslrootcert='" + caFilePath + "'";
    }
  }

  // Branch for OGR
  if ( dataprovider == "ogr"_L1 || dataprovider == "gdal"_L1 )
  {
    if ( !password.isEmpty() )
    {
      const QString fullUri( connectionItems.first() );
      QString uri( fullUri );
      // Handle sub-layers
      if ( fullUri.contains( '|' ) )
      {
        uri = uri.left( uri.indexOf( '|' ) );
      }
      // At least username must be set... password can be empty
      if ( !username.isEmpty() )
      {
        // Inject credentials
        if ( uri.startsWith( "PG:"_L1 ) )
        {
          bool chopped = false;
          if ( uri.endsWith( '"' ) )
          {
            uri.chop( 1 );
            chopped = true;
          }
          uri += u" user='%1'"_s.arg( username );
          uri += u" password='%1'"_s.arg( password );
          // add extra CAs
          if ( !caparam.isEmpty() )
          {
            uri += ' ' + caparam;
          }
          if ( chopped )
            uri += '"';
        }
        else if ( uri.startsWith( "SDE:"_L1 ) )
        {
          const thread_local QRegularExpression sRx = QRegularExpression( ",$" );
          uri = uri.replace( sRx, u",%1,%2"_s.arg( username, password ) );
        }
        else if ( uri.startsWith( "IDB"_L1 ) )
        {
          bool chopped = false;
          if ( uri.endsWith( '"' ) )
          {
            uri.chop( 1 );
            chopped = true;
          }
          uri += u" user=%1"_s.arg( username );
          uri += u" pass=%1"_s.arg( password );
          if ( chopped )
            uri += '"';
        }
        else if ( uri.startsWith( "@driver=ingres"_L1 ) )
        {
          uri += u",userid=%1"_s.arg( username );
          uri += u",password=%1"_s.arg( password );
        }
        else if ( uri.startsWith( "MySQL:"_L1 ) )
        {
          // If username or password contains comma or double quote we need to quote the string
          if ( username.contains( ',' ) || username.contains( '"' ) )
          {
            username.replace( '"', R"(\")"_L1 );
            username.prepend( '"' );
            username.append( '"' );
          }

          if ( password.contains( ',' ) || password.contains( '"' ) )
          {
            password.replace( '"', R"(\")"_L1 );
            password.prepend( '"' );
            password.append( '"' );
          }
          uri += u",user=%1"_s.arg( username );
          uri += u",password=%1"_s.arg( password );
        }
        else if ( uri.startsWith( "MSSQL:"_L1 ) )
        {
          uri += u";uid=%1"_s.arg( username );
          uri = uri.replace( ";trusted_connection=yes"_L1, QString() );
          uri += u";pwd=%1"_s.arg( password );
        }
        else if ( uri.startsWith( "OCI:"_L1 ) )
        {
          // OCI:userid/password@database_instance:table,table
          uri = uri.replace( "OCI:/"_L1, u"OCI:%1/%2"_s.arg( username, password ) );
        }
        else if ( uri.startsWith( "ODBC:"_L1 ) )
        {
          const thread_local QRegularExpression sOdbcRx = QRegularExpression( "^ODBC:@?" );
          uri = uri.replace( sOdbcRx, "ODBC:" + username + '/' + password + '@' );
        }
        else if ( uri.startsWith( "couchdb"_L1 )
                  || uri.startsWith( "DODS"_L1 )
                  || uri.startsWith( "http://" )
                  || uri.startsWith( "/vsicurl/http://" )
                  || uri.startsWith( "https://" )
                  || uri.startsWith( "/vsicurl/https://" )
                  || uri.startsWith( "ftp://" )
                  || uri.startsWith( "/vsicurl/ftp://" ) )
        {
          uri = uri.replace( "://"_L1, u"://%1:%2@"_s.arg( username, password ) );
        }
      }
      // Handle sub-layers
      if ( fullUri.contains( '|' ) )
      {
        uri += '|' + fullUri.right( fullUri.length() - fullUri.lastIndexOf( '|' ) - 1 );
      }
      connectionItems.replace( 0, uri );
    }
    else
    {
      QgsDebugError( u"Update URI items FAILED for authcfg: %1: password empty"_s.arg( authcfg ) );
    }
  }
  else // Not-ogr
  {
    const QString userparam = "user='" + escapeUserPass( username ) + '\'';
    const thread_local QRegularExpression userRegExp( "^user='.*" );
    const int userindx = connectionItems.indexOf( userRegExp );
    if ( userindx != -1 )
    {
      connectionItems.replace( userindx, userparam );
    }
    else
    {
      connectionItems.append( userparam );
    }

    const QString passparam = "password='" + escapeUserPass( password ) + '\'';
    const thread_local QRegularExpression passRegExp( "^password='.*" );
    const int passindx = connectionItems.indexOf( passRegExp );
    if ( passindx != -1 )
    {
      connectionItems.replace( passindx, passparam );
    }
    else
    {
      connectionItems.append( passparam );
    }
    // add extra CAs
    if ( !caparam.isEmpty() )
    {
      const thread_local QRegularExpression sslcaRegExp( "^sslrootcert='.*" );
      const int sslcaindx = connectionItems.indexOf( sslcaRegExp );
      if ( sslcaindx != -1 )
      {
        connectionItems.replace( sslcaindx, caparam );
      }
      else
      {
        connectionItems.append( caparam );
      }
    }
  }


  return true;
}

bool QgsAuthBasicMethod::updateNetworkProxy( QNetworkProxy &proxy, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mMutex );

  const QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugError( u"Update proxy config FAILED for authcfg: %1: config invalid"_s.arg( authcfg ) );
    return false;
  }

  const QString username = mconfig.config( u"username"_s );
  const QString password = mconfig.config( u"password"_s );

  if ( !username.isEmpty() )
  {
    proxy.setUser( username );
    proxy.setPassword( password );
  }
  return true;
}

void QgsAuthBasicMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  if ( mconfig.hasConfig( u"oldconfigstyle"_s ) )
  {
    QgsDebugMsgLevel( u"Updating old style auth method config"_s, 2 );

    const QStringList conflist = mconfig.config( u"oldconfigstyle"_s ).split( u"|||"_s );
    mconfig.setConfig( u"realm"_s, conflist.at( 0 ) );
    mconfig.setConfig( u"username"_s, conflist.at( 1 ) );
    mconfig.setConfig( u"password"_s, conflist.at( 2 ) );
    mconfig.removeConfig( u"oldconfigstyle"_s );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

#ifdef HAVE_GUI
QWidget *QgsAuthBasicMethod::editWidget( QWidget *parent ) const
{
  return new QgsAuthBasicEdit( parent );
}
#endif

void QgsAuthBasicMethod::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

QgsAuthMethodConfig QgsAuthBasicMethod::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsAuthMethodConfig mconfig;

  // check if it is cached
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    mconfig = sAuthConfigCache.value( authcfg );
    QgsDebugMsgLevel( u"Retrieved config for authcfg: %1"_s.arg( authcfg ), 2 );
    return mconfig;
  }

  // else build basic bundle
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugError( u"Retrieve config FAILED for authcfg: %1"_s.arg( authcfg ) );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  putMethodConfig( authcfg, mconfig );

  return mconfig;
}

void QgsAuthBasicMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsgLevel( u"Putting basic config for authcfg: %1"_s.arg( authcfg ), 2 );
  sAuthConfigCache.insert( authcfg, mconfig );
}

void QgsAuthBasicMethod::removeMethodConfig( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QgsDebugMsgLevel( u"Removed basic config for authcfg: %1"_s.arg( authcfg ), 2 );
  }
}

QString QgsAuthBasicMethod::escapeUserPass( const QString &val, QChar delim ) const
{
  QString escaped = val;

  escaped.replace( '\\', "\\\\"_L1 );
  escaped.replace( delim, u"\\%1"_s.arg( delim ) );

  return escaped;
}

//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsAuthMethodMetadata *authMethodMetadataFactory()
{
  return new QgsAuthBasicMethodMetadata();
}
#endif
