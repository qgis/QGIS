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
#include "qgsauthbasicedit.h"

#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsapplication.h"

#include <QNetworkProxy>
#include <QMutexLocker>

static const QString AUTH_METHOD_KEY = QStringLiteral( "Basic" );
static const QString AUTH_METHOD_DESCRIPTION = QStringLiteral( "Basic authentication" );

QMap<QString, QgsAuthMethodConfig> QgsAuthBasicMethod::sAuthConfigCache = QMap<QString, QgsAuthMethodConfig>();


QgsAuthBasicMethod::QgsAuthBasicMethod()
{
  setVersion( 2 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::DataSourceUri );
  setDataProviders( QStringList()
                    << QStringLiteral( "postgres" )
                    << QStringLiteral( "db2" )
                    << QStringLiteral( "ows" )
                    << QStringLiteral( "wfs" )  // convert to lowercase
                    << QStringLiteral( "wcs" )
                    << QStringLiteral( "wms" )
                    << QStringLiteral( "ogr" )
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
  return tr( "Basic authentication" );
}

bool QgsAuthBasicMethod::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugMsg( QString( "Update request config FAILED for authcfg: %1: config invalid" ).arg( authcfg ) );
    return false;
  }

  QString username = mconfig.config( QStringLiteral( "username" ) );
  QString password = mconfig.config( QStringLiteral( "password" ) );

  if ( !username.isEmpty() )
  {
    request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( username, password ).toLatin1().toBase64() );
  }
  return true;
}

bool QgsAuthBasicMethod::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
  QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugMsg( QString( "Update URI items FAILED for authcfg: %1: basic config invalid" ).arg( authcfg ) );
    return false;
  }

  QString username = mconfig.config( QStringLiteral( "username" ) );
  QString password = mconfig.config( QStringLiteral( "password" ) );

  if ( username.isEmpty() )
  {
    QgsDebugMsg( QString( "Update URI items FAILED for authcfg: %1: username empty" ).arg( authcfg ) );
    return false;
  }

  // Branch for OGR
  if ( dataprovider == QStringLiteral( "ogr" ) )
  {
    if ( ! password.isEmpty() )
    {
      QString fullUri( connectionItems.first() );
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
        if ( uri.startsWith( QStringLiteral( "PG:" ) ) )
        {
          if ( !username.isEmpty() )
          {
            uri += QStringLiteral( " user='%1'" ).arg( username );

            if ( !password.isEmpty() )
              uri += QStringLiteral( " password='%1'" ).arg( password );
          }
        }
        else if ( uri.startsWith( QStringLiteral( "SDE:" ) ) )
        {
          uri = uri.replace( QRegExp( ",$" ), QStringLiteral( ",%1,%2" ).arg( username, password ) );
        }
        else if ( uri.startsWith( QStringLiteral( "IDB" ) ) )
        {
          uri += QStringLiteral( " user=%1" ).arg( username );
          if ( !password.isEmpty() )
            uri += QStringLiteral( " pass=%1" ).arg( password );
        }
        else if ( uri.startsWith( QStringLiteral( "@driver=ingres" ) ) )
        {
          uri += QStringLiteral( ",userid=%1" ).arg( username );
          if ( !password.isEmpty() )
            uri += QStringLiteral( ",password=%1" ).arg( password );
        }
        else if ( uri.startsWith( QStringLiteral( "MySQL:" ) ) )
        {
          uri += QStringLiteral( ",userid=%1" ).arg( username );
          if ( !password.isEmpty() )
            uri += QStringLiteral( ",password=%1" ).arg( password );
        }
        else if ( uri.startsWith( QStringLiteral( "MSSQL:" ) ) )
        {
          uri += QStringLiteral( ";uid=%1" ).arg( username );
          uri = uri.replace( QLatin1String( ";trusted_connection=yes" ), QString() );

          if ( !password.isEmpty() )
            uri += QStringLiteral( ";pwd=%1" ).arg( password );
        }
        else if ( uri.startsWith( QStringLiteral( "OCI:" ) ) )
        {
          // OCI:userid/password@database_instance:table,table
          uri = uri.replace( QStringLiteral( "OCI:/" ),  QStringLiteral( "OCI:%1/%2" ).arg( username, password ) );
        }
        else if ( uri.startsWith( QStringLiteral( "ODBC:" ) ) )
        {
          if ( password.isEmpty() )
          {
            uri = uri.replace( QRegExp( "^ODBC:[^@]+" ),  "ODBC:" + username + '@' );
          }
          else
          {
            uri = uri.replace( QRegExp( "^ODBC:[^@]+" ), "ODBC:" + username + '/' + password + '@' );
          }
        }
        else if ( uri.startsWith( QStringLiteral( "MSSQL:" ) ) )
        {
          uri += QStringLiteral( ";uid=%1" ).arg( username );
          if ( !password.isEmpty() )
            uri += QStringLiteral( ";pwd=%1" ).arg( password );
        }
        else if ( uri.startsWith( QStringLiteral( "couchdb" ) )
                  || uri.startsWith( QStringLiteral( "DODS" ) )
                  || uri.startsWith( "http://" )
                  || uri.startsWith( "https://" )
                  || uri.startsWith( "ftp://" ) // not really sure that this is supported ...
                )
        {
          uri = uri.replace( QStringLiteral( "://" ), QStringLiteral( "://%1:%2@" ).arg( username, password ) );
        }
      }
      // Handle sub-layers
      if ( fullUri.contains( '|' ) )
      {
        uri += '|' + fullUri.right( fullUri.indexOf( '|' ) );
      }
      connectionItems.replace( 0, uri );
    }
    else
    {
      QgsDebugMsg( QString( "Update URI items FAILED for authcfg: %1: password empty" ).arg( authcfg ) );
    }

  }
  else // Not-ogr
  {
    QString userparam = "user='" + escapeUserPass( username ) + '\'';
    int userindx = connectionItems.indexOf( QRegExp( "^user='.*" ) );
    if ( userindx != -1 )
    {
      connectionItems.replace( userindx, userparam );
    }
    else
    {
      connectionItems.append( userparam );
    }

    QString passparam = "password='" + escapeUserPass( password ) + '\'';
    int passindx = connectionItems.indexOf( QRegExp( "^password='.*" ) );
    if ( passindx != -1 )
    {
      connectionItems.replace( passindx, passparam );
    }
    else
    {
      connectionItems.append( passparam );
    }
  }
  return true;
}

bool QgsAuthBasicMethod::updateNetworkProxy( QNetworkProxy &proxy, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  QgsAuthMethodConfig mconfig = getMethodConfig( authcfg );
  if ( !mconfig.isValid() )
  {
    QgsDebugMsg( QString( "Update proxy config FAILED for authcfg: %1: config invalid" ).arg( authcfg ) );
    return false;
  }

  QString username = mconfig.config( QStringLiteral( "username" ) );
  QString password = mconfig.config( QStringLiteral( "password" ) );

  if ( !username.isEmpty() )
  {
    proxy.setUser( username );
    proxy.setPassword( password );
  }
  return true;
}

void QgsAuthBasicMethod::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( QStringLiteral( "oldconfigstyle" ) ) )
  {
    QgsDebugMsg( "Updating old style auth method config" );

    QStringList conflist = mconfig.config( QStringLiteral( "oldconfigstyle" ) ).split( QStringLiteral( "|||" ) );
    mconfig.setConfig( QStringLiteral( "realm" ), conflist.at( 0 ) );
    mconfig.setConfig( QStringLiteral( "username" ), conflist.at( 1 ) );
    mconfig.setConfig( QStringLiteral( "password" ), conflist.at( 2 ) );
    mconfig.removeConfig( QStringLiteral( "oldconfigstyle" ) );
  }

  // TODO: add updates as method version() increases due to config storage changes
}

void QgsAuthBasicMethod::clearCachedConfig( const QString &authcfg )
{
  removeMethodConfig( authcfg );
}

QgsAuthMethodConfig QgsAuthBasicMethod::getMethodConfig( const QString &authcfg, bool fullconfig )
{
  QMutexLocker locker( &mConfigMutex );
  QgsAuthMethodConfig mconfig;

  // check if it is cached
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    mconfig = sAuthConfigCache.value( authcfg );
    QgsDebugMsg( QString( "Retrieved config for authcfg: %1" ).arg( authcfg ) );
    return mconfig;
  }

  // else build basic bundle
  if ( !QgsApplication::authManager()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugMsg( QString( "Retrieve config FAILED for authcfg: %1" ).arg( authcfg ) );
    return QgsAuthMethodConfig();
  }

  // cache bundle
  locker.unlock();
  putMethodConfig( authcfg, mconfig );

  return mconfig;
}

void QgsAuthBasicMethod::putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig )
{
  QMutexLocker locker( &mConfigMutex );
  QgsDebugMsg( QString( "Putting basic config for authcfg: %1" ).arg( authcfg ) );
  sAuthConfigCache.insert( authcfg, mconfig );
}

void QgsAuthBasicMethod::removeMethodConfig( const QString &authcfg )
{
  QMutexLocker locker( &mConfigMutex );
  if ( sAuthConfigCache.contains( authcfg ) )
  {
    sAuthConfigCache.remove( authcfg );
    QgsDebugMsg( QString( "Removed basic config for authcfg: %1" ).arg( authcfg ) );
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

/**
 * Required class factory to return a pointer to a newly created object
 */
QGISEXTERN QgsAuthBasicMethod *classFactory()
{
  return new QgsAuthBasicMethod();
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
QGISEXTERN QgsAuthBasicEdit *editWidget( QWidget *parent )
{
  return new QgsAuthBasicEdit( parent );
}

/**
 * Required cleanup function
 */
QGISEXTERN void cleanupAuthMethod() // pass QgsAuthMethod *method, then delete method  ?
{
}
