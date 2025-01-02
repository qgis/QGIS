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
#include "moc_qgsdatasourceuri.cpp"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgswkbtypes.h"
#include "qgsapplication.h"

#include <QStringList>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

#define HIDING_TOKEN QStringLiteral( "XXXXXXXX" )

QgsDataSourceUri::QgsDataSourceUri()
{
  // do nothing
}

QgsDataSourceUri::QgsDataSourceUri( const QString &u )
{
  QString uri = u;
  int i = 0;
  while ( i < uri.length() )
  {
    skipBlanks( uri, i );

    if ( uri[i] == '=' )
    {
      QgsDebugError( QStringLiteral( "parameter name expected before =" ) );
      i++;
      continue;
    }

    int start = i;

    while ( i < uri.length() && uri[i] != '=' && !uri[i].isSpace() )
      i++;

    const QString pname = uri.mid( start, i - start );

    skipBlanks( uri, i );

    if ( i == uri.length() || uri[i] != '=' )
    {
      // no "=", so likely not a parameter name
      continue;
    }

    i++;

    if ( pname == QLatin1String( "sql" ) )
    {
      // rest of line is a sql where clause
      skipBlanks( uri, i );
      mSql = uri.mid( i );

      // handle empty sql specified by a empty '' or "" encapsulated value
      // possibly we should be calling getValue here, but there's a very high risk of regressions
      // if we change that now...
      if ( mSql == QLatin1String( "''" ) || mSql == QLatin1String( "\"\"" ) )
        mSql.clear();
      break;
    }
    else
    {
      const QString pval = getValue( uri, i );

      if ( pname == QLatin1String( "table" ) )
      {
        if ( i < uri.length() && uri[i] == '.' )
        {
          i++;

          mSchema = pval;
          mTable = getValue( uri, i );
        }
        else
        {
          mTable = pval;
        }

        if ( i < uri.length() && uri[i] == '(' )
        {
          i++;

          start = i;
          while ( i < uri.length() && uri[i] != ')' )
          {
            if ( uri[i] == '\\' )
              i++;
            i++;
          }

          if ( i == uri.length() )
          {
            QgsDebugError( QStringLiteral( "closing parenthesis missing" ) );
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
      else if ( pname == QLatin1String( "schema" ) )
      {
        mSchema = pval;
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
        mSelectAtIdDisabledSet = true;
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
        QgsDebugMsgLevel( QStringLiteral( "connection timeout ignored" ), 3 );
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
        QgsDebugMsgLevel( QStringLiteral( "database host ip address ignored" ), 2 );
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
        QgsDebugMsgLevel( QStringLiteral( "backend debug tty ignored" ), 2 );
      }
      else if ( pname == QLatin1String( "options" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "backend debug options ignored" ), 2 );
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
        QgsDebugMsgLevel( QStringLiteral( "kerberos server name ignored" ), 2 );
      }
      else if ( pname == QLatin1String( "gsslib" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "gsslib ignored" ), 2 );
      }
      else if ( pname.startsWith( QgsHttpHeaders::PARAM_PREFIX ) )
      {
        mHttpHeaders.insert( pname, pval );
      }
      else
      {
        QgsDebugMsgLevel( "parameter \"" + pname + "\":\"" + pval + "\" added", 4 );
        setParam( pname, pval );
      }
    }
  }
}

QString QgsDataSourceUri::removePassword( const QString &aUri, bool hide )
{
  QRegularExpression regexp;
  regexp.setPatternOptions( QRegularExpression::InvertedGreedinessOption );
  QString safeName( aUri );
  if ( aUri.contains( QLatin1String( " password=" ) ) )
  {
    regexp.setPattern( QStringLiteral( " password=.* " ) );

    if ( hide )
    {
      safeName.replace( regexp, QStringLiteral( " password=%1 " ).arg( HIDING_TOKEN ) );
    }
    else
    {
      safeName.replace( regexp, QStringLiteral( " " ) );
    }
  }
  else if ( aUri.contains( QLatin1String( ",password=" ) ) )
  {
    regexp.setPattern( QStringLiteral( ",password=.*," ) );

    if ( hide )
    {
      safeName.replace( regexp, QStringLiteral( ",password=%1," ).arg( HIDING_TOKEN ) );
    }
    else
    {
      safeName.replace( regexp, QStringLiteral( "," ) );
    }
  }
  else if ( aUri.contains( QLatin1String( "IDB:" ) ) )
  {
    regexp.setPattern( QStringLiteral( " pass=.* " ) );

    if ( hide )
    {
      safeName.replace( regexp, QStringLiteral( " pass=%1 " ).arg( HIDING_TOKEN ) );
    }
    else
    {
      safeName.replace( regexp, QStringLiteral( " " ) );
    }
  }
  else if ( ( aUri.contains( QLatin1String( "OCI:" ) ) )
            || ( aUri.contains( QLatin1String( "ODBC:" ) ) ) )
  {
    regexp.setPattern( QStringLiteral( "/.*@" ) );

    if ( hide )
    {
      safeName.replace( regexp, QStringLiteral( "/%1@" ).arg( HIDING_TOKEN ) );
    }
    else
    {
      safeName.replace( regexp, QStringLiteral( "/@" ) );
    }
  }
  else if ( aUri.contains( QLatin1String( "postgresql:" ) ) )
  {
    // postgresql://user:pwd@...
    regexp.setPattern( QStringLiteral( "/.*@" ) );
    const QString matched = regexp.match( aUri ).captured();

    QString anonymised = matched;
    const QStringList items = matched.split( QStringLiteral( ":" ) );
    if ( items.size() > 1 )
    {
      anonymised = matched.split( QStringLiteral( ":" ) )[0];
      if ( hide )
      {
        anonymised.append( QStringLiteral( ":%1" ).arg( HIDING_TOKEN ) );
      }
      anonymised.append( QStringLiteral( "@" ) );
    }

    safeName.replace( regexp, anonymised );
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
  mSelectAtIdDisabledSet = true;
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

void QgsDataSourceUri::setGeometryColumn( const QString &geometryColumn )
{
  mGeometryColumn = geometryColumn;
}

void QgsDataSourceUri::setTable( const QString &table )
{
  mTable = table;
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
    const QChar delim = uri[i];

    i++;

    // value is quoted
    for ( ;; )
    {
      if ( i == uri.length() )
      {
        QgsDebugError( QStringLiteral( "unterminated quoted string in connection info string" ) );
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
        QgsDebugError( QStringLiteral( "Data source URI FAILED to update via loading configuration ID '%1'" ).arg( mAuthConfigId ) );
      }
    }
    else
    {
      connectionItems << "authcfg=" + mAuthConfigId;
    }
  }

  return connectionItems.join( QLatin1Char( ' ' ) );
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
    uri += QLatin1String( " estimatedmetadata=true" );
  }

  if ( !mSrid.isEmpty() )
  {
    uri += QStringLiteral( " srid=%1" ).arg( mSrid );
  }

  if ( mWkbType != Qgis::WkbType::Unknown && mWkbType != Qgis::WkbType::NoGeometry )
  {
    uri += QLatin1String( " type=" );
    uri += QgsWkbTypes::displayString( mWkbType );
  }

  if ( mSelectAtIdDisabled )
  {
    uri += QLatin1String( " selectatid=false" );
  }

  for ( auto it = mParams.constBegin(); it != mParams.constEnd(); ++it )
  {
    if ( it.key().contains( '=' ) || it.key().contains( ' ' ) )
    {
      QgsDebugError( QStringLiteral( "invalid uri parameter %1 skipped" ).arg( it.key() ) );
      continue;
    }

    uri += ' ' + it.key() + "='" + escape( it.value() ) + '\'';
  }

  uri += mHttpHeaders.toSpacedString();

  QString columnName( mGeometryColumn );
  columnName.replace( '\\', QLatin1String( "\\\\" ) );
  columnName.replace( ')', QLatin1String( "\\)" ) );

  if ( !mTable.isEmpty() )
  {
    uri += QStringLiteral( " table=%1%2" )
           .arg( quotedTablename(),
                 mGeometryColumn.isEmpty() ? QString() : QStringLiteral( " (%1)" ).arg( columnName ) );
  }
  else if ( !mSchema.isEmpty() )
  {
    uri += QStringLiteral( " schema='%1'" ).arg( escape( mSchema ) );
  }

  if ( !mSql.isEmpty() )
  {
    uri += QStringLiteral( " sql=" ) + mSql;
  }

  return uri;
}

// from qurl.h
QByteArray toLatin1_helper( const QString &string )
{
  if ( string.isEmpty() )
    return string.isNull() ? QByteArray() : QByteArray( "" );
  return string.toLatin1();
}

QByteArray QgsDataSourceUri::encodedUri() const
{
  QUrlQuery url;
  for ( auto it = mParams.constBegin(); it != mParams.constEnd(); ++it )
  {
    url.addQueryItem( it.key(), it.value() );
  }

  if ( !mUsername.isEmpty() )
    url.addQueryItem( QStringLiteral( "username" ), mUsername );

  if ( !mPassword.isEmpty() )
    url.addQueryItem( QStringLiteral( "password" ), mPassword );

  if ( !mAuthConfigId.isEmpty() )
    url.addQueryItem( QStringLiteral( "authcfg" ), mAuthConfigId );

  mHttpHeaders.updateUrlQuery( url );

  return toLatin1_helper( url.toString( QUrl::FullyEncoded ) );
}

void QgsDataSourceUri::setEncodedUri( const QByteArray &uri )
{
  mParams.clear();
  mUsername.clear();
  mPassword.clear();
  mAuthConfigId.clear();

  QUrl url;
  url.setQuery( QString::fromLatin1( uri ) );
  const QUrlQuery query( url );

  mHttpHeaders.setFromUrlQuery( query );

  const auto constQueryItems = query.queryItems();
  for ( const QPair<QString, QString> &item : constQueryItems )
  {
    if ( !item.first.startsWith( QgsHttpHeaders::PARAM_PREFIX ) && item.first != QgsHttpHeaders::KEY_REFERER )
    {
      if ( item.first == QLatin1String( "username" ) )
        mUsername = query.queryItemValue( QStringLiteral( "username" ), QUrl::ComponentFormattingOption::FullyDecoded );
      else if ( item.first == QLatin1String( "password" ) )
        mPassword = query.queryItemValue( QStringLiteral( "password" ), QUrl::ComponentFormattingOption::FullyDecoded );
      else if ( item.first == QLatin1String( "authcfg" ) )
        mAuthConfigId = query.queryItemValue( QStringLiteral( "authcfg" ), QUrl::ComponentFormattingOption::FullyDecoded );
      else
        mParams.insert( item.first, item.second );
    }
  }
}

void QgsDataSourceUri::setEncodedUri( const QString &uri )
{
  QUrl url;
  url.setQuery( uri );
  setEncodedUri( url.query( QUrl::EncodeUnicode ).toLatin1() );
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

Qgis::WkbType QgsDataSourceUri::wkbType() const
{
  return mWkbType;
}

void QgsDataSourceUri::setWkbType( Qgis::WkbType wkbType )
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
  // maintain old API
  if ( key == QLatin1String( "username" ) )
    mUsername = value;
  else if ( key == QLatin1String( "password" ) )
    mPassword = value;
  else if ( key == QLatin1String( "authcfg" ) )
    mAuthConfigId = value;
  else
  {
    // may be multiple
    mParams.insert( key, value );
  }
}

void QgsDataSourceUri::setParam( const QString &key, const QStringList &value )
{
  for ( const QString &val : value )
  {
    setParam( key, val );
  }
}

int QgsDataSourceUri::removeParam( const QString &key )
{
  if ( key == QLatin1String( "username" ) && !mUsername.isEmpty() )
  {
    mUsername.clear();
    return 1;
  }
  else if ( key == QLatin1String( "password" ) && !mPassword.isEmpty() )
  {
    mPassword.clear();
    return 1;
  }
  else if ( key == QLatin1String( "authcfg" ) && !mAuthConfigId.isEmpty() )
  {
    mAuthConfigId.clear();
    return 1;
  }

  return mParams.remove( key );
}

QString QgsDataSourceUri::param( const QString &key ) const
{
  // maintain old api
  if ( key == QLatin1String( "username" ) && !mUsername.isEmpty() )
    return mUsername;
  else if ( key == QLatin1String( "password" ) && !mPassword.isEmpty() )
    return mPassword;
  else if ( key == QLatin1String( "authcfg" ) && !mAuthConfigId.isEmpty() )
    return mAuthConfigId;

  return mParams.value( key );
}

QStringList QgsDataSourceUri::params( const QString &key ) const
{
  // maintain old api
  if ( key == QLatin1String( "username" ) && !mUsername.isEmpty() )
    return QStringList() << mUsername;
  else if ( key == QLatin1String( "password" ) && !mPassword.isEmpty() )
    return QStringList() << mPassword;
  else if ( key == QLatin1String( "authcfg" ) && !mAuthConfigId.isEmpty() )
    return QStringList() << mAuthConfigId;

  return mParams.values( key );
}

bool QgsDataSourceUri::hasParam( const QString &key ) const
{
  // maintain old api
  if ( key == QLatin1String( "username" ) && !mUsername.isEmpty() )
    return true;
  else if ( key == QLatin1String( "password" ) && !mPassword.isEmpty() )
    return true;
  else if ( key == QLatin1String( "authcfg" ) && !mAuthConfigId.isEmpty() )
    return true;

  return mParams.contains( key );
}

QSet<QString> QgsDataSourceUri::parameterKeys() const
{
  QSet<QString> paramKeys;
  for ( auto it = mParams.constBegin(); it != mParams.constEnd(); it++ )
    paramKeys.insert( it.key() );

  if ( !mHost.isEmpty() )
    paramKeys.insert( QLatin1String( "host" ) );
  if ( !mPort.isEmpty() )
    paramKeys.insert( QLatin1String( "port" ) );
  if ( !mDriver.isEmpty() )
    paramKeys.insert( QLatin1String( "driver" ) );
  if ( !mService.isEmpty() )
    paramKeys.insert( QLatin1String( "service" ) );
  if ( !mDatabase.isEmpty() )
    paramKeys.insert( QLatin1String( "dbname" ) );
  if ( !mSchema.isEmpty() )
    paramKeys.insert( QLatin1String( "schema" ) );
  if ( !mTable.isEmpty() )
    paramKeys.insert( QLatin1String( "table" ) );
  // Ignore mGeometryColumn: not a key ==> embedded in table value
  if ( !mSql.isEmpty() )
    paramKeys.insert( QLatin1String( "sql" ) );
  if ( !mAuthConfigId.isEmpty() )
    paramKeys.insert( QLatin1String( "authcfg" ) );
  if ( !mUsername.isEmpty() )
    paramKeys.insert( QLatin1String( "username" ) );
  if ( !mPassword.isEmpty() )
    paramKeys.insert( QLatin1String( "password" ) );
  if ( mSSLmode != SslPrefer )
    paramKeys.insert( QLatin1String( "sslmode" ) );
  if ( !mKeyColumn.isEmpty() )
    paramKeys.insert( QLatin1String( "key" ) );
  if ( mUseEstimatedMetadata )
    paramKeys.insert( QLatin1String( "estimatedmetadata" ) );
  if ( mSelectAtIdDisabledSet )
    paramKeys.insert( QLatin1String( "selectatid" ) );
  if ( mWkbType != Qgis::WkbType::Unknown )
    paramKeys.insert( QLatin1String( "type" ) );
  if ( !mSrid.isEmpty() )
    paramKeys.insert( QLatin1String( "srid" ) );
  return paramKeys;
}
