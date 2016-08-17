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

#include <QStringList>
#include <QRegExp>
#include <QUrl>

QgsDataSourceUri::QgsDataSourceUri()
    : mSSLmode( SslPrefer )
    , mKeyColumn( "" )
    , mUseEstimatedMetadata( false )
    , mSelectAtIdDisabled( false )
    , mWkbType( QgsWkbTypes::Unknown )
{
  // do nothing
}

QgsDataSourceUri::QgsDataSourceUri( QString uri )
    : mSSLmode( SslPrefer )
    , mKeyColumn( "" )
    , mUseEstimatedMetadata( false )
    , mSelectAtIdDisabled( false )
    , mWkbType( QgsWkbTypes::Unknown )
{
  int i = 0;
  while ( i < uri.length() )
  {
    skipBlanks( uri, i );

    if ( uri[i] == '=' )
    {
      QgsDebugMsg( "parameter name expected before =" );
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
      QgsDebugMsg( QString( "= expected after parameter name, skipping text '%1'" ).arg( pname ) );
      continue;
    }

    i++;

    if ( pname == "sql" )
    {
      // rest of line is a sql where clause
      skipBlanks( uri, i );
      mSql = uri.mid( i );
      break;
    }
    else
    {
      QString pval = getValue( uri, i );

      if ( pname == "table" )
      {
        if ( uri[i] == '.' )
        {
          i++;

          mSchema = pval;
          mTable = getValue( uri, i );
        }
        else
        {
          mSchema = "";
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
            QgsDebugMsg( "closing parenthesis missing" );
          }

          mGeometryColumn = uri.mid( start, i - start );
          mGeometryColumn.replace( "\\)", ")" );
          mGeometryColumn.replace( "\\\\", "\\" );

          i++;
        }
        else
        {
          mGeometryColumn = QString::null;
        }
      }
      else if ( pname == "key" )
      {
        mKeyColumn = pval;
      }
      else if ( pname == "estimatedmetadata" )
      {
        mUseEstimatedMetadata = pval == "true";
      }
      else if ( pname == "srid" )
      {
        mSrid = pval;
      }
      else if ( pname == "type" )
      {
        mWkbType = QgsWkbTypes::parseType( pval );
      }
      else if ( pname == "selectatid" )
      {
        mSelectAtIdDisabled = pval == "false";
      }
      else if ( pname == "service" )
      {
        mService = pval;
      }
      else if ( pname == "authcfg" )
      {
        mAuthConfigId = pval;
      }
      else if ( pname == "user" )
      {
        mUsername = pval;
      }
      else if ( pname == "password" )
      {
        mPassword = pval;
      }
      else if ( pname == "connect_timeout" )
      {
        QgsDebugMsg( "connection timeout ignored" );
      }
      else if ( pname == "dbname" )
      {
        mDatabase = pval;
      }
      else if ( pname == "host" )
      {
        mHost = pval;
      }
      else if ( pname == "hostaddr" )
      {
        QgsDebugMsg( "database host ip address ignored" );
      }
      else if ( pname == "port" )
      {
        mPort = pval;
      }
      else if ( pname == "driver" )
      {
        mDriver = pval;
      }
      else if ( pname == "tty" )
      {
        QgsDebugMsg( "backend debug tty ignored" );
      }
      else if ( pname == "options" )
      {
        QgsDebugMsg( "backend debug options ignored" );
      }
      else if ( pname == "sslmode" )
      {
        if ( pval == "disable" )
          mSSLmode = SslDisable;
        else if ( pval == "allow" )
          mSSLmode = SslAllow;
        else if ( pval == "prefer" )
          mSSLmode = SslPrefer;
        else if ( pval == "require" )
          mSSLmode = SslRequire;
        else if ( pval == "verify-ca" )
          mSSLmode = SslVerifyCa;
        else if ( pval == "verify-full" )
          mSSLmode = SslVerifyFull;
      }
      else if ( pname == "requiressl" )
      {
        if ( pval == "0" )
          mSSLmode = SslDisable;
        else
          mSSLmode = SslPrefer;
      }
      else if ( pname == "krbsrvname" )
      {
        QgsDebugMsg( "kerberos server name ignored" );
      }
      else if ( pname == "gsslib" )
      {
        QgsDebugMsg( "gsslib ignored" );
      }
      else
      {
        QgsDebugMsg( "parameter \"" + pname + "\":\"" + pval + "\" added" );
        setParam( pname, pval );
      }
    }
  }
}

QString QgsDataSourceUri::removePassword( const QString& aUri )
{
  QRegExp regexp;
  regexp.setMinimal( true );
  QString safeName( aUri );
  if ( aUri.contains( " password=" ) )
  {
    regexp.setPattern( " password=.* " );
    safeName.replace( regexp, " " );
  }
  else if ( aUri.contains( ",password=" ) )
  {
    regexp.setPattern( ",password=.*," );
    safeName.replace( regexp, "," );
  }
  else if ( aUri.contains( "IDB:" ) )
  {
    regexp.setPattern( " pass=.* " );
    safeName.replace( regexp, " " );
  }
  else if (( aUri.contains( "OCI:" ) )
           || ( aUri.contains( "ODBC:" ) ) )
  {
    regexp.setPattern( "/.*@" );
    safeName.replace( regexp, "/@" );
  }
  else if ( aUri.contains( "SDE:" ) )
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

void QgsDataSourceUri::setUsername( const QString& username )
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

void QgsDataSourceUri::setPassword( const QString& password )
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


void QgsDataSourceUri::setDriver( const QString& driver )
{
  mDriver = driver;
}


void QgsDataSourceUri::setKeyColumn( const QString& column )
{
  mKeyColumn = column;
}


void QgsDataSourceUri::setUseEstimatedMetadata( bool theFlag )
{
  mUseEstimatedMetadata = theFlag;
}

bool QgsDataSourceUri::useEstimatedMetadata() const
{
  return mUseEstimatedMetadata;
}

void QgsDataSourceUri::disableSelectAtId( bool theFlag )
{
  mSelectAtIdDisabled = theFlag;
}

bool QgsDataSourceUri::selectAtIdDisabled() const
{
  return mSelectAtIdDisabled;
}

void QgsDataSourceUri::setSql( const QString& sql )
{
  mSql = sql;
}

void QgsDataSourceUri::clearSchema()
{
  mSchema = "";
}

void QgsDataSourceUri::setSchema( const QString& schema )
{
  mSchema = schema;
}

QString QgsDataSourceUri::escape( const QString &theVal, QChar delim = '\'' ) const
{
  QString val = theVal;

  val.replace( '\\', "\\\\" );
  val.replace( delim, QString( "\\%1" ).arg( delim ) );

  return val;
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
        QgsDebugMsg( "unterminated quoted string in connection info string" );
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

  if ( mDatabase != "" )
  {
    connectionItems << "dbname='" + escape( mDatabase ) + '\'';
  }

  if ( mService != "" )
  {
    connectionItems << "service='" + escape( mService ) + '\'';
  }
  else if ( mHost != "" )
  {
    connectionItems << "host=" + mHost;
  }

  if ( mService.isEmpty() )
  {
    if ( mPort != "" )
      connectionItems << "port=" + mPort;
  }

  if ( mDriver != "" )
  {
    connectionItems << "driver='" + escape( mDriver ) + '\'';
  }

  if ( mUsername != "" )
  {
    connectionItems << "user='" + escape( mUsername ) + '\'';

    if ( mPassword != "" )
    {
      connectionItems << "password='" + escape( mPassword ) + '\'';
    }
  }

  if ( mSSLmode == SslDisable )
    connectionItems << "sslmode=disable";
  else if ( mSSLmode == SslAllow )
    connectionItems << "sslmode=allow";
  else if ( mSSLmode == SslRequire )
    connectionItems << "sslmode=require";
#if 0
  else if ( mSSLmode == SSLprefer ) // no need to output the default
    connectionItems << "sslmode=prefer";
#endif
  else if ( mSSLmode == SslVerifyCa )
    connectionItems << "sslmode=verify-ca";
  else if ( mSSLmode == SslVerifyFull )
    connectionItems << "sslmode=verify-full";

  if ( !mAuthConfigId.isEmpty() )
  {
    if ( expandAuthConfig )
    {
      if ( !QgsAuthManager::instance()->updateDataSourceUriItems( connectionItems, mAuthConfigId ) )
      {
        QgsDebugMsg( QString( "Data source URI FAILED to update via loading configuration ID '%1'" ).arg( mAuthConfigId ) );
      }
    }
    else
    {
      connectionItems << "authcfg=" + mAuthConfigId;
    }
  }

  return connectionItems.join( " " );
}

QString QgsDataSourceUri::uri( bool expandAuthConfig ) const
{
  QString theUri = connectionInfo( expandAuthConfig );

  if ( !mKeyColumn.isEmpty() )
  {
    theUri += QString( " key='%1'" ).arg( escape( mKeyColumn ) );
  }

  if ( mUseEstimatedMetadata )
  {
    theUri += QString( " estimatedmetadata=true" );
  }

  if ( !mSrid.isEmpty() )
  {
    theUri += QString( " srid=%1" ).arg( mSrid );
  }

  if ( mWkbType != QgsWkbTypes::Unknown && mWkbType != QgsWkbTypes::NoGeometry )
  {
    theUri += " type=";
    theUri += QgsWkbTypes::displayString( mWkbType );
  }

  if ( mSelectAtIdDisabled )
  {
    theUri += QString( " selectatid=false" );
  }

  for ( QMap<QString, QString>::const_iterator it = mParams.begin(); it != mParams.end(); ++it )
  {
    if ( it.key().contains( '=' ) || it.key().contains( ' ' ) )
    {
      QgsDebugMsg( QString( "invalid uri parameter %1 skipped" ).arg( it.key() ) );
      continue;
    }

    theUri += ' ' + it.key() + "='" + escape( it.value() ) + '\'';
  }

  QString columnName( mGeometryColumn );
  columnName.replace( '\\', "\\\\" );
  columnName.replace( ')', "\\)" );

  theUri += QString( " table=%1%2 sql=%3" )
            .arg( quotedTablename(),
                  mGeometryColumn.isNull() ? QString() : QString( " (%1)" ).arg( columnName ),
                  mSql );

  return theUri;
}

QByteArray QgsDataSourceUri::encodedUri() const
{
  QUrl url;
  Q_FOREACH ( const QString& key, mParams.uniqueKeys() )
  {
    Q_FOREACH ( const QString& value, mParams.values( key ) )
    {
      url.addQueryItem( key, value );
    }
  }
  return url.encodedQuery();
}

void QgsDataSourceUri::setEncodedUri( const QByteArray & uri )
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

void QgsDataSourceUri::setEncodedUri( const QString & uri )
{
  setEncodedUri( uri.toLatin1() );
}

QString QgsDataSourceUri::quotedTablename() const
{
  if ( !mSchema.isEmpty() )
    return QString( "\"%1\".\"%2\"" )
           .arg( escape( mSchema, '"' ),
                 escape( mTable, '"' ) );
  else
    return QString( "\"%1\"" )
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

void QgsDataSourceUri::setSrid( const QString& srid )
{
  mSrid = srid;
}

void QgsDataSourceUri::setParam( const QString &key, const QString &value )
{
  // may be multiple
  mParams.insertMulti( key, value );
}

void QgsDataSourceUri::setParam( const QString &key, const QStringList &value )
{
  Q_FOREACH ( const QString& val, value )
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
