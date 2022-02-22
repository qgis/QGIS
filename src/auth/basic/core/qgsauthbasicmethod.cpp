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

#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"

#ifdef HAVE_GUI
#include "qgsauthbasicedit.h"
#endif

#include <QNetworkProxy>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QUuid>

const QString QgsAuthBasicMethod::AUTH_METHOD_KEY = QStringLiteral( "Basic" );
const QString QgsAuthBasicMethod::AUTH_METHOD_DESCRIPTION = QStringLiteral( "Basic authentication" );
const QString QgsAuthBasicMethod::AUTH_METHOD_DISPLAY_DESCRIPTION = tr( "Basic authentication" );

QMap<QString, QgsAuthMethodConfig> QgsAuthBasicMethod::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthBasicMethod::QgsAuthBasicMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceUri );
  setDataProviders( QStringList()
                    << QStringLiteral( "postgres" )
                    << QStringLiteral( "oracle" )
                    << QStringLiteral( "ows" )
                    << QStringLiteral( "wfs" )  // convert to lowercase
                    << QStringLiteral( "wcs" )
                    << QStringLiteral( "wms" )
                    << QStringLiteral( "ogr" )
                    << QStringLiteral( "gdal" )
                    << QStringLiteral( "proxy" ) );

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


bool QgsAuthBasicMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Update request config FAILED for authcfg: %1: config invalid" ).arg( authcfg ) );
    return false;
  }

  const QString username = mconfig.config( QStringLiteral( "username" ) );
  const QString password = mconfig.config( QStringLiteral( "password" ) );

  if ( !username.isEmpty() )
  {
    request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( username, password ).toUtf8().toBase64() );
  }
  return true;
}

bool QgsAuthBasicMethod::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )
  const QMutexLocker locker( &mMutex );
  const QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Update URI items FAILED for authcfg: %1: basic config invalid" ).arg( authcfg ) );
    return false;
  }

  const QString username = mconfig.config( QStringLiteral( "username" ) );
  const QString password = mconfig.config( QStringLiteral( "password" ) );

  if ( username.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Update URI items FAILED for authcfg: %1: username empty" ).arg( authcfg ) );
    return false;
  }

  QString sslMode = QStringLiteral( "prefer" );
  const thread_local QRegularExpression sslModeRegExp( "^sslmode=.*" );
  const int sslModeIdx = connectionItems.indexOf( sslModeRegExp );
  if ( sslModeIdx != -1 )
  {
    sslMode = connectionItems.at( sslModeIdx ).split( '=' ).at( 1 );
  }

  // SSL Extra CAs
  QString caparam;
  QList<QSslCertificate> cas;
  if ( sslMode.startsWith( QLatin1String( "verify-" ) ) )
  {
    cas = QgsApplication::authManager()->trustedCaCerts();
    // save CAs to temp file
    const QString tempFileBase = QStringLiteral( "tmp_basic_%1.pem" );
    const QString caFilePath = QgsAuthCertUtils::pemTextToTempFile(
                                 tempFileBase.arg( QUuid::createUuid().toString() ),
                                 QgsAuthCertUtils::certsToPemText( cas ) );
    if ( ! caFilePath.isEmpty() )
    {
      caparam = "sslrootcert='" + caFilePath + "'";
    }
  }

  // Branch for OGR
  if ( dataprovider == QLatin1String( "ogr" ) || dataprovider == QLatin1String( "gdal" ) )
  {
    if ( ! password.isEmpty() )
    {
      const QString fullUri( connectionItems.first() );
      QString uri( fullUri );
      // Handle sub-layers
      if ( fullUri.contains( '|' ) )
      {
        uri = uri.left( uri.indexOf( '|' ) );
      }
      // At least username must be set... password can be empty
      if ( ! username.isEmpty() )
      {
        // Inject credentials
        if ( uri.startsWith( QLatin1String( "PG:" ) ) )
        {
          bool chopped = false;
          if ( uri.endsWith( '"' ) )
          {
            uri.chop( 1 );
            chopped = true;
          }
          uri += QStringLiteral( " user='%1'" ).arg( username );
          uri += QStringLiteral( " password='%1'" ).arg( password );
          // add extra CAs
          if ( ! caparam.isEmpty() )
          {
            uri += ' ' + caparam;
          }
          if ( chopped )
            uri += '"';
        }
        else if ( uri.startsWith( QLatin1String( "SDE:" ) ) )
        {
          uri = uri.replace( QRegularExpression( ",$" ), QStringLiteral( ",%1,%2" ).arg( username, password ) );
        }
        else if ( uri.startsWith( QLatin1String( "IDB" ) ) )
        {
          bool chopped = false;
          if ( uri.endsWith( '"' ) )
          {
            uri.chop( 1 );
            chopped = true;
          }
          uri += QStringLiteral( " user=%1" ).arg( username );
          uri += QStringLiteral( " pass=%1" ).arg( password );
          if ( chopped )
            uri += '"';
        }
        else if ( uri.startsWith( QLatin1String( "@driver=ingres" ) ) )
        {
          uri += QStringLiteral( ",userid=%1" ).arg( username );
          uri += QStringLiteral( ",password=%1" ).arg( password );
        }
        else if ( uri.startsWith( QLatin1String( "MySQL:" ) ) )
        {
          uri += QStringLiteral( ",user=%1" ).arg( username );
          uri += QStringLiteral( ",password=%1" ).arg( password );
        }
        else if ( uri.startsWith( QLatin1String( "MSSQL:" ) ) )
        {
          uri += QStringLiteral( ";uid=%1" ).arg( username );
          uri = uri.replace( QLatin1String( ";trusted_connection=yes" ), QString() );
          uri += QStringLiteral( ";pwd=%1" ).arg( password );
        }
        else if ( uri.startsWith( QLatin1String( "OCI:" ) ) )
        {
          // OCI:userid/password@database_instance:table,table
          uri = uri.replace( QLatin1String( "OCI:/" ),  QStringLiteral( "OCI:%1/%2" ).arg( username, password ) );
        }
        else if ( uri.startsWith( QLatin1String( "ODBC:" ) ) )
        {
          uri = uri.replace( QRegularExpression( "^ODBC:@?" ), "ODBC:" + username + '/' + password + '@' );
        }
        else if ( uri.startsWith( QLatin1String( "couchdb" ) )
                  || uri.startsWith( QLatin1String( "DODS" ) )
                  || uri.startsWith( "http://" )
                  || uri.startsWith( "/vsicurl/http://" )
                  || uri.startsWith( "https://" )
                  || uri.startsWith( "/vsicurl/https://" )
                  || uri.startsWith( "ftp://" )
                  || uri.startsWith( "/vsicurl/ftp://" )
                )
        {
          uri = uri.replace( QLatin1String( "://" ), QStringLiteral( "://%1:%2@" ).arg( username, password ) );
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
      QgsDebugMsg( QStringLiteral( "Update URI items FAILED for authcfg: %1: password empty" ).arg( authcfg ) );
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
    if ( ! caparam.isEmpty() )
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
    QgsDebugMsg( QStringLiteral( "Update proxy config FAILED for authcfg: %1: config invalid" ).arg( authcfg ) );
    return false;
  }

  const QString username = mconfig.config( QStringLiteral( "username" ) );
  const QString password = mconfig.config( QStringLiteral( "password" ) );

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
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsg( QStringLiteral( "Updating old style auth method config" ) );

    const QStringList conflist = mconfig.config( QStringLiteral( "oldconfigstyle" ) ).split( QStringLiteral( "|||" ) );
    mconfig.setConfig( QStringLiteral( "realm" ), conflist.at( 0 ) );
    mconfig.setConfig( QStringLiteral( "username" ), conflist.at( 1 ) );
    mconfig.setConfig( QStringLiteral( "password" ), conflist.at( 2 ) );
    mconfig.removeConfig( QStringLiteral( "oldconfigstyle" ) );
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
    QgsDebugMsg( QStringLiteral( "Retrieved config for authcfg: %1" ).arg( authcfg ) );
    return mconfig;
  }

  // else build basic bundle
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugMsg( QStringLiteral( "Retrieve config FAILED for authcfg: %1" ).arg( authcfg ) );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  putMethodConfig( authcfg, mconfig );

  return mconfig;
}

void QgsAuthBasicMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig )
{
  const QMutexLocker locker( &mMutex );
  QgsDebugMsg( QStringLiteral( "Putting basic config for authcfg: %1" ).arg( authcfg ) );
  sAuthConfigCache.insert( authcfg, mconfig );
}

void QgsAuthBasicMethod::removeMethodConfig( const QString &authcfg )
{
  const QMutexLocker locker( &mMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QgsDebugMsg( QStringLiteral( "Removed basic config for authcfg: %1" ).arg( authcfg ) );
  }
}

QString QgsAuthBasicMethod::escapeUserPass( const QString &val, QChar delim ) const
{
  QString escaped = val;

  escaped.replace( '\\', QLatin1String( "\\\\" ) );
  escaped.replace( delim, QStringLiteral( "\\%1" ).arg( delim ) );

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



