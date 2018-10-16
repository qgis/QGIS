/***************************************************************************
      qgsdatasourceuri.h  -  Structure to contain the component parts
                             of a data source URI
                             -------------------
    begin                : Dec 5, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatasourceuri.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgswkbtypes.h"
#include "qgsapplication.h"

#include <QStringList>
#include <QRegExp>
#include <QUrl>

QgsDataSourceUri::QgsDataSourceUri()
{
  // do nothing
}

QgsDataSourceUri::QgsDataSourceUri( QString uri )
{
  int i = 0;
  while ( i < uri.length() )
  {
    skipBlanks( uri, i );

    if ( uri[i] == '=' )
    {
      QgsDebugMsg( QStringLiteral( "parameter name expected before =" ) );
      i++;
      continue;
    }

    int start = i;

    while ( i < uri.length() && uri[i] != '=' && !uri[i].isSpace() )
      i++;

    QString pname = uri.mid( start, i - start );

    skipBlanks( uri, i );

    if ( i == uri.length() || uri[i] != '=' )
    {
      QgsDebugMsg( QStringLiteral( "= expected after parameter name, skipping text '%1'" ).arg( pname ) );
      continue;
    }

    i++;

    if ( pname == QLatin1String( "sql" ) )
    {
      // rest of line is a sql where clause
      skipBlanks( uri, i );
      mSql = uri.mid( i );
      break;
    }
    else
    {
      QString pval = getValue( uri, i );

      if ( pname == QLatin1String( "table" ) )
      {
        if ( uri[i] == '.' )
        {
          i++;

          mSchema = pval;
          mTable = getValue( uri, i );
        }
        else
        {
          mSchema.clear();
          mTable = pval;
        }

        if ( uri[i] == '(' )
        {
          i++;

          int start = i;
          while ( i < uri.length() && uri[i] != ')' )
          {
            if ( uri[i] == '\\' )
              i++;
            i++;
          }

          if ( i == uri.length() )
          {
            QgsDebugMsg( QStringLiteral( "closing parenthesis missing" ) );
          }

          mGeometryColumn = uri.mid( start, i - start );
          mGeometryColumn.replace( QLatin1String( "\\)" ), QLatin1String( ")" ) );
          mGeometryColumn.replace( QLatin1String( "\\\\" ), QLatin1String( "\\" ) );

          i++;
        }
        else
        {
          mGeometryColumn = QString();
        }
      }
      else if ( pname == QLatin1String( "key" ) )
      {
        mKeyColumn = pval;
      }
      else if ( pname == QLatin1String( "estimatedmetadata" ) )
      {
        mUseEstimatedMetadata = pval == QLatin1String( "true" );
      }
      else if ( pname == QLatin1String( "srid" ) )
      {
        mSrid = pval;
      }
      else if ( pname == QLatin1String( "type" ) )
      {
        mWkbType = QgsWkbTypes::parseType( pval );
      }
      else if ( pname == QLatin1String( "selectatid" ) )
      {
        mSelectAtIdDisabled = pval == QLatin1String( "false" );
      }
      else if ( pname == QLatin1String( "service" ) )
      {
        mService = pval;
      }
      else if ( pname == QLatin1String( "authcfg" ) )
      {
        mAuthConfigId = pval;
      }
      else if ( pname == QLatin1String( "user" ) || pname == QLatin1String( "username" ) ) // Also accepts new WFS provider naming
      {
        mUsername = pval;
      }
      else if ( pname == QLatin1String( "password" ) )
      {
        mPassword = pval;
      }
      else if ( pname == QLatin1String( "connect_timeout" ) )
      {
        QgsDebugMsg( QStringLiteral( "connection timeout ignored" ) );
      }
      else if ( pname == QLatin1String( "dbname" ) )
      {
        mDatabase = pval;
      }
      else if ( pname == QLatin1String( "host" ) )
      {
        mHost = pval;
      }
      else if ( pname == QLatin1String( "hostaddr" ) )
      {
        QgsDebugMsg( QStringLiteral( "database host ip address ignored" ) );
      }
      else if ( pname == QLatin1String( "port" ) )
      {
        mPort = pval;
      }
      else if ( pname == QLatin1String( "driver" ) )
      {
        mDriver = pval;
      }
      else if ( pname == QLatin1String( "tty" ) )
      {
        QgsDebugMsg( QStringLiteral( "backend debug tty ignored" ) );
      }
      else if ( pname == QLatin1String( "options" ) )
      {
        QgsDebugMsg( QStringLiteral( "backend debug options ignored" ) );
      }
      else if ( pname == QLatin1String( "sslmode" ) )
      {
        mSSLmode = decodeSslMode( pval );
      }
      else if ( pname == QLatin1String( "requiressl" ) )
      {
        if ( pval == QLatin1String( "0" ) )
          mSSLmode = SslDisable;
        else
          mSSLmode = SslPrefer;
      }
      else if ( pname == QLatin1String( "krbsrvname" ) )
      {
        QgsDebugMsg( QStringLiteral( "kerberos server name ignored" ) );
      }
      else if ( pname == QLatin1String( "gsslib" ) )
      {
        QgsDebugMsg( QStringLiteral( "gsslib ignored" ) );
      }
      else
      {
        QgsDebugMsgLevel( "parameter \"" + pname + "\":\"" + pval + "\" added", 4 );
        setParam( pname, pval );
      }
    }
  }
}

QString QgsDataSourceUri::removePassword( const QString &aUri )
{
  QRegExp regexp;
  regexp.setMinimal( true );
  QString safeName( aUri );
  if ( aUri.contains( QLatin1String( " password=" ) ) )
  {
    regexp.setPattern( QStringLiteral( " password=.* " ) );
    safeName.replace( regexp, QStringLiteral( " " ) );
  }
  else if ( aUri.contains( QLatin1String( ",password=" ) ) )
  {
    regexp.setPattern( QStringLiteral( ",password=.*," ) );
    safeName.replace( regexp, QStringLiteral( "," ) );
  }
  else if ( aUri.contains( QLatin1String( "IDB:" ) ) )
  {
    regexp.setPattern( QStringLiteral( " pass=.* " ) );
    safeName.replace( regexp, QStringLiteral( " " ) );
  }
  else if ( ( aUri.contains( QLatin1String( "OCI:" ) ) )
            || ( aUri.contains( QLatin1String( "ODBC:" ) ) ) )
  {
    regexp.setPattern( QStringLiteral( "/.*@" ) );
    safeName.replace( regexp, QStringLiteral( "/@" ) );
  }
  else if ( aUri.contains( QLatin1String( "SDE:" ) ) )
  {
    QStringList strlist = aUri.split( ',' );
    safeName = strlist[0] + ',' + strlist[1] + ',' + strlist[2] + ',' + strlist[3];
  }
  return safeName;
}

QString QgsDataSourceUri::authConfigId() const
{
  return mAuthConfigId;
}

QString QgsDataSourceUri::username() const
{
  return mUsername;
}

void QgsDataSourceUri::setUsername( const QString &username )
{
  mUsername = username;
}

QString QgsDataSourceUri::service() const
{
  return mService;
}

QString QgsDataSourceUri::host() const
{
  return mHost;
}

QString QgsDataSourceUri::database() const
{
  return mDatabase;
}

QString QgsDataSourceUri::password() const
{
  return mPassword;
}

void QgsDataSourceUri::setPassword( const QString &password )
{
  mPassword = password;
}

QString QgsDataSourceUri::port() const
{
  return mPort;
}

QString QgsDataSourceUri::driver() const
{
  return mDriver;
}

QgsDataSourceUri::SslMode QgsDataSourceUri::sslMode() const
{
  return mSSLmode;
}

QString QgsDataSourceUri::schema() const
{
  return mSchema;
}

QString QgsDataSourceUri::table() const
{
  return mTable;
}

QString QgsDataSourceUri::sql() const
{
  return mSql;
}

QString QgsDataSourceUri::geometryColumn() const
{
  return mGeometryColumn;
}

QString QgsDataSourceUri::keyColumn() const
{
  return mKeyColumn;
}


void QgsDataSourceUri::setDriver( const QString &driver )
{
  mDriver = driver;
}


void QgsDataSourceUri::setKeyColumn( const QString &column )
{
  mKeyColumn = column;
}


void QgsDataSourceUri::setUseEstimatedMetadata( bool flag )
{
  mUseEstimatedMetadata = flag;
}

bool QgsDataSourceUri::useEstimatedMetadata() const
{
  return mUseEstimatedMetadata;
}

void QgsDataSourceUri::disableSelectAtId( bool flag )
{
  mSelectAtIdDisabled = flag;
}

bool QgsDataSourceUri::selectAtIdDisabled() const
{
  return mSelectAtIdDisabled;
}

void QgsDataSourceUri::setSql( const QString &sql )
{
  mSql = sql;
}

void QgsDataSourceUri::clearSchema()
{
  mSchema.clear();
}

void QgsDataSourceUri::setSchema( const QString &schema )
{
  mSchema = schema;
}

QString QgsDataSourceUri::escape( const QString &val, QChar delim = '\'' ) const
{
  QString escaped = val;

  escaped.replace( '\\', QLatin1String( "\\\\" ) );
  escaped.replace( delim, QStringLiteral( "\\%1" ).arg( delim ) );

  return escaped;
}

void QgsDataSourceUri::skipBlanks( const QString &uri, int &i )
{
  // skip space before value
  while ( i < uri.length() && uri[i].isSpace() )
    i++;
}

QString QgsDataSourceUri::getValue( const QString &uri, int &i )
{
  skipBlanks( uri, i );

  // Get the parameter value
  QString pval;
  if ( i < uri.length() && ( uri[i] == '\'' || uri[i] == '"' ) )
  {
    QChar delim = uri[i];

    i++;

    // value is quoted
    for ( ;; )
    {
      if ( i == uri.length() )
      {
        QgsDebugMsg( QStringLiteral( "unterminated quoted string in connection info string" ) );
        return pval;
      }

      if ( uri[i] == '\\' )
      {
        i++;
        if ( i == uri.length() )
          continue;
        if ( uri[i] != delim && uri[i] != '\\' )
          i--;
      }
      else if ( uri[i] == delim )
      {
        i++;
        break;
      }

      pval += uri[i++];
    }
  }
  else
  {
    // value is not quoted
    while ( i < uri.length() )
    {
      if ( uri[i].isSpace() )
      {
        // end of value
        break;
      }

      if ( uri[i] == '\\' )
      {
        i++;
        if ( i == uri.length() )
          break;
        if ( uri[i] != '\\' && uri[i] != '\'' )
          i--;
      }

      pval += uri[i++];
    }
  }

  skipBlanks( uri, i );

  return pval;
}

QString QgsDataSourceUri::connectionInfo( bool expandAuthConfig ) const
{
  QStringList connectionItems;

  if ( !mDatabase.isEmpty() )
  {
    connectionItems << "dbname='" + escape( mDatabase ) + '\'';
  }

  if ( !mService.isEmpty() )
  {
    connectionItems << "service='" + escape( mService ) + '\'';
  }
  else if ( !mHost.isEmpty() )
  {
    connectionItems << "host=" + mHost;
  }

  if ( mService.isEmpty() )
  {
    if ( !mPort.isEmpty() )
      connectionItems << "port=" + mPort;
  }

  if ( !mDriver.isEmpty() )
  {
    connectionItems << "driver='" + escape( mDriver ) + '\'';
  }

  if ( !mUsername.isEmpty() )
  {
    connectionItems << "user='" + escape( mUsername ) + '\'';

    if ( !mPassword.isEmpty() )
    {
      connectionItems << "password='" + escape( mPassword ) + '\'';
    }
  }

  if ( mSSLmode != SslPrefer )  // no need to output the default
  {
    connectionItems << QStringLiteral( "sslmode=" ) + encodeSslMode( mSSLmode );
  }

  if ( !mAuthConfigId.isEmpty() )
  {
    if ( expandAuthConfig )
    {
      if ( !QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, mAuthConfigId ) )
      {
        QgsDebugMsg( QStringLiteral( "Data source URI FAILED to update via loading configuration ID '%1'" ).arg( mAuthConfigId ) );
      }
    }
    else
    {
      connectionItems << "authcfg=" + mAuthConfigId;
    }
  }

  return connectionItems.join( QStringLiteral( " " ) );
}

QString QgsDataSourceUri::uri( bool expandAuthConfig ) const
{
  QString uri = connectionInfo( expandAuthConfig );

  if ( !mKeyColumn.isEmpty() )
  {
    uri += QStringLiteral( " key='%1'" ).arg( escape( mKeyColumn ) );
  }

  if ( mUseEstimatedMetadata )
  {
    uri += QStringLiteral( " estimatedmetadata=true" );
  }

  if ( !mSrid.isEmpty() )
  {
    uri += QStringLiteral( " srid=%1" ).arg( mSrid );
  }

  if ( mWkbType != QgsWkbTypes::Unknown && mWkbType != QgsWkbTypes::NoGeometry )
  {
    uri += QLatin1String( " type=" );
    uri += QgsWkbTypes::displayString( mWkbType );
  }

  if ( mSelectAtIdDisabled )
  {
    uri += QStringLiteral( " selectatid=false" );
  }

  for ( QMap<QString, QString>::const_iterator it = mParams.begin(); it != mParams.end(); ++it )
  {
    if ( it.key().contains( '=' ) || it.key().contains( ' ' ) )
    {
      QgsDebugMsg( QStringLiteral( "invalid uri parameter %1 skipped" ).arg( it.key() ) );
      continue;
    }

    uri += ' ' + it.key() + "='" + escape( it.value() ) + '\'';
  }

  QString columnName( mGeometryColumn );
  columnName.replace( '\\', QLatin1String( "\\\\" ) );
  columnName.replace( ')', QLatin1String( "\\)" ) );

  uri += QStringLiteral( " table=%1%2 sql=%3" )
         .arg( quotedTablename(),
               mGeometryColumn.isNull() ? QString() : QStringLiteral( " (%1)" ).arg( columnName ),
               mSql );

  return uri;
}

QByteArray QgsDataSourceUri::encodedUri() const
{
  QUrl url;
  for ( auto it = mParams.constBegin(); it != mParams.constEnd(); ++it )
  {
    url.addQueryItem( it.key(), it.value() );
  }
  return url.encodedQuery();
}

void QgsDataSourceUri::setEncodedUri( const QByteArray &uri )
{
  mParams.clear();
  QUrl url;
  url.setEncodedQuery( uri );
  QPair<QString, QString> item;
  Q_FOREACH ( item, url.queryItems() )
  {
    mParams.insertMulti( item.first, item.second );
  }
}

void QgsDataSourceUri::setEncodedUri( const QString &uri )
{
  setEncodedUri( uri.toLatin1() );
}

QString QgsDataSourceUri::quotedTablename() const
{
  if ( !mSchema.isEmpty() )
    return QStringLiteral( "\"%1\".\"%2\"" )
           .arg( escape( mSchema, '"' ),
                 escape( mTable, '"' ) );
  else
    return QStringLiteral( "\"%1\"" )
           .arg( escape( mTable, '"' ) );
}

void QgsDataSourceUri::setConnection( const QString &host,
                                      const QString &port,
                                      const QString &database,
                                      const QString &username,
                                      const QString &password,
                                      SslMode sslmode,
                                      const QString &authConfigId )
{
  mHost = host;
  mDatabase = database;
  mPort = port;
  mUsername = username;
  mPassword = password;
  mSSLmode = sslmode;
  mAuthConfigId = authConfigId;
}

void QgsDataSourceUri::setConnection( const QString &service,
                                      const QString &database,
                                      const QString &username,
                                      const QString &password,
                                      SslMode sslmode,
                                      const QString &authConfigId )
{
  mService = service;
  mDatabase = database;
  mUsername = username;
  mPassword = password;
  mSSLmode = sslmode;
  mAuthConfigId = authConfigId;
}

void QgsDataSourceUri::setDataSource( const QString &schema,
                                      const QString &table,
                                      const QString &geometryColumn,
                                      const QString &sql,
                                      const QString &keyColumn )
{
  mSchema = schema;
  mTable = table;
  mGeometryColumn = geometryColumn;
  mSql = sql;
  mKeyColumn = keyColumn;
}

void QgsDataSourceUri::setAuthConfigId( const QString &authcfg )
{
  mAuthConfigId = authcfg;
}

void QgsDataSourceUri::setDatabase( const QString &database )
{
  mDatabase = database;
}

QgsWkbTypes::Type QgsDataSourceUri::wkbType() const
{
  return mWkbType;
}

void QgsDataSourceUri::setWkbType( QgsWkbTypes::Type wkbType )
{
  mWkbType = wkbType;
}

QString QgsDataSourceUri::srid() const
{
  return mSrid;
}

void QgsDataSourceUri::setSrid( const QString &srid )
{
  mSrid = srid;
}

QgsDataSourceUri::SslMode QgsDataSourceUri::decodeSslMode( const QString &sslMode )
{
  if ( sslMode == QLatin1String( "prefer" ) )
    return SslPrefer;
  else if ( sslMode == QLatin1String( "disable" ) )
    return SslDisable;
  else if ( sslMode == QLatin1String( "allow" ) )
    return SslAllow;
  else if ( sslMode == QLatin1String( "require" ) )
    return SslRequire;
  else if ( sslMode == QLatin1String( "verify-ca" ) )
    return SslVerifyCa;
  else if ( sslMode == QLatin1String( "verify-full" ) )
    return SslVerifyFull;
  else
    return SslPrefer;  // default
}

QString QgsDataSourceUri::encodeSslMode( QgsDataSourceUri::SslMode sslMode )
{
  switch ( sslMode )
  {
    case SslPrefer: return QStringLiteral( "prefer" );
    case SslDisable: return QStringLiteral( "disable" );
    case SslAllow: return QStringLiteral( "allow" );
    case SslRequire: return QStringLiteral( "require" );
    case SslVerifyCa: return QStringLiteral( "verify-ca" );
    case SslVerifyFull: return QStringLiteral( "verify-full" );
  }
  return QString();
}

void QgsDataSourceUri::setParam( const QString &key, const QString &value )
{
  // may be multiple
  mParams.insertMulti( key, value );
}

void QgsDataSourceUri::setParam( const QString &key, const QStringList &value )
{
  Q_FOREACH ( const QString &val, value )
  {
    mParams.insertMulti( key, val );
  }
}

int QgsDataSourceUri::removeParam( const QString &key )
{
  return mParams.remove( key );
}

QString QgsDataSourceUri::param( const QString &key ) const
{
  return mParams.value( key );
}

QStringList QgsDataSourceUri::params( const QString &key ) const
{
  return mParams.values( key );
}

bool QgsDataSourceUri::hasParam( const QString &key ) const
{
  return mParams.contains( key );
}
