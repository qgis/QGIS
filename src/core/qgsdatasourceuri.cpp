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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgswkbtypes.h"

#include <QRegularExpression>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

#include "moc_qgsdatasourceuri.cpp"

#define HIDING_TOKEN u"XXXXXXXX"_s

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
      QgsDebugError( u"parameter name expected before ="_s );
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

    if ( pname == "sql"_L1 )
    {
      // rest of line is a sql where clause
      skipBlanks( uri, i );
      mSql = uri.mid( i );

      // handle empty sql specified by a empty '' or "" encapsulated value
      // possibly we should be calling getValue here, but there's a very high risk of regressions
      // if we change that now...
      if ( mSql == "''"_L1 || mSql == "\"\""_L1 )
        mSql.clear();
      break;
    }
    else
    {
      const QString pval = getValue( uri, i );

      if ( pname == "table"_L1 )
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
            QgsDebugError( u"closing parenthesis missing"_s );
          }

          mGeometryColumn = uri.mid( start, i - start );
          mGeometryColumn.replace( "\\)"_L1, ")"_L1 );
          mGeometryColumn.replace( "\\\\"_L1, "\\"_L1 );

          i++;
        }
        else
        {
          mGeometryColumn = QString();
        }
      }
      else if ( pname == "schema"_L1 )
      {
        mSchema = pval;
      }
      else if ( pname == "key"_L1 )
      {
        mKeyColumn = pval;
      }
      else if ( pname == "estimatedmetadata"_L1 )
      {
        mUseEstimatedMetadata = pval == "true"_L1;
      }
      else if ( pname == "srid"_L1 )
      {
        mSrid = pval;
      }
      else if ( pname == "type"_L1 )
      {
        mWkbType = QgsWkbTypes::parseType( pval );
      }
      else if ( pname == "selectatid"_L1 )
      {
        mSelectAtIdDisabledSet = true;
        mSelectAtIdDisabled = pval == "false"_L1;
      }
      else if ( pname == "service"_L1 )
      {
        mService = pval;
      }
      else if ( pname == "authcfg"_L1 )
      {
        mAuthConfigId = pval;
      }
      else if ( pname == "user"_L1 || pname == "username"_L1 ) // Also accepts new WFS provider naming
      {
        mUsername = pval;
      }
      else if ( pname == "password"_L1 )
      {
        mPassword = pval;
      }
      else if ( pname == "connect_timeout"_L1 )
      {
        QgsDebugMsgLevel( u"connection timeout ignored"_s, 3 );
      }
      else if ( pname == "dbname"_L1 )
      {
        mDatabase = pval;
      }
      else if ( pname == "host"_L1 )
      {
        mHost = pval;
      }
      else if ( pname == "hostaddr"_L1 )
      {
        QgsDebugMsgLevel( u"database host ip address ignored"_s, 2 );
      }
      else if ( pname == "port"_L1 )
      {
        mPort = pval;
      }
      else if ( pname == "driver"_L1 )
      {
        mDriver = pval;
      }
      else if ( pname == "tty"_L1 )
      {
        QgsDebugMsgLevel( u"backend debug tty ignored"_s, 2 );
      }
      else if ( pname == "options"_L1 )
      {
        QgsDebugMsgLevel( u"backend debug options ignored"_s, 2 );
      }
      else if ( pname == "sslmode"_L1 )
      {
        mSSLmode = decodeSslMode( pval );
      }
      else if ( pname == "requiressl"_L1 )
      {
        if ( pval == "0"_L1 )
          mSSLmode = SslDisable;
        else
          mSSLmode = SslPrefer;
      }
      else if ( pname == "krbsrvname"_L1 )
      {
        QgsDebugMsgLevel( u"kerberos server name ignored"_s, 2 );
      }
      else if ( pname == "gsslib"_L1 )
      {
        QgsDebugMsgLevel( u"gsslib ignored"_s, 2 );
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
  if ( aUri.contains( " password="_L1 ) )
  {
    regexp.setPattern( u" password=.* "_s );

    if ( hide )
    {
      safeName.replace( regexp, u" password=%1 "_s.arg( HIDING_TOKEN ) );
    }
    else
    {
      safeName.replace( regexp, u" "_s );
    }
  }
  else if ( aUri.contains( ",password="_L1 ) )
  {
    regexp.setPattern( u",password=.*,"_s );

    if ( hide )
    {
      safeName.replace( regexp, u",password=%1,"_s.arg( HIDING_TOKEN ) );
    }
    else
    {
      safeName.replace( regexp, u","_s );
    }
  }
  else if ( aUri.contains( "IDB:"_L1 ) )
  {
    regexp.setPattern( u" pass=.* "_s );

    if ( hide )
    {
      safeName.replace( regexp, u" pass=%1 "_s.arg( HIDING_TOKEN ) );
    }
    else
    {
      safeName.replace( regexp, u" "_s );
    }
  }
  else if ( ( aUri.contains( "OCI:"_L1 ) )
            || ( aUri.contains( "ODBC:"_L1 ) ) )
  {
    regexp.setPattern( u"/.*@"_s );

    if ( hide )
    {
      safeName.replace( regexp, u"/%1@"_s.arg( HIDING_TOKEN ) );
    }
    else
    {
      safeName.replace( regexp, u"/@"_s );
    }
  }
  else if ( aUri.contains( "postgresql:"_L1 ) )
  {
    // postgresql://user:pwd@...
    regexp.setPattern( u"/.*@"_s );
    const QString matched = regexp.match( aUri ).captured();

    QString anonymised = matched;
    const QStringList items = matched.split( u":"_s );
    if ( items.size() > 1 )
    {
      anonymised = matched.split( u":"_s )[0];
      if ( hide )
      {
        anonymised.append( u":%1"_s.arg( HIDING_TOKEN ) );
      }
      anonymised.append( u"@"_s );
    }

    safeName.replace( regexp, anonymised );
  }
  else if ( aUri.contains( "SDE:"_L1 ) )
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

void QgsDataSourceUri::setPort( const QString &port )
{
  mPort = port;
}

QString QgsDataSourceUri::password() const
{
  return mPassword;
}

void QgsDataSourceUri::setSslMode( SslMode mode )
{
  mSSLmode = mode;
}

void QgsDataSourceUri::setService( const QString &service )
{
  mService = service;
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

void QgsDataSourceUri::setHost( const QString &host )
{
  mHost = host;
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

  escaped.replace( '\\', "\\\\"_L1 );
  escaped.replace( delim, u"\\%1"_s.arg( delim ) );

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
        QgsDebugError( u"unterminated quoted string in connection info string"_s );
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
    connectionItems << u"sslmode="_s + encodeSslMode( mSSLmode );
  }

  if ( !mAuthConfigId.isEmpty() )
  {
    if ( expandAuthConfig )
    {
      if ( !QgsApplication::authManager()->updateDataSourceUriItems( connectionItems, mAuthConfigId ) )
      {
        QgsDebugError( u"Data source URI FAILED to update via loading configuration ID '%1'"_s.arg( mAuthConfigId ) );
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
    uri += u" key='%1'"_s.arg( escape( mKeyColumn ) );
  }

  if ( mUseEstimatedMetadata )
  {
    uri += " estimatedmetadata=true"_L1;
  }

  if ( !mSrid.isEmpty() )
  {
    uri += u" srid=%1"_s.arg( mSrid );
  }

  if ( mWkbType != Qgis::WkbType::Unknown && mWkbType != Qgis::WkbType::NoGeometry )
  {
    uri += " type="_L1;
    uri += QgsWkbTypes::displayString( mWkbType );
  }

  if ( mSelectAtIdDisabled )
  {
    uri += " selectatid=false"_L1;
  }

  for ( auto it = mParams.constBegin(); it != mParams.constEnd(); ++it )
  {
    if ( it.key().contains( '=' ) || it.key().contains( ' ' ) )
    {
      QgsDebugError( u"invalid uri parameter %1 skipped"_s.arg( it.key() ) );
      continue;
    }

    uri += ' ' + it.key() + "='" + escape( it.value() ) + '\'';
  }

  uri += mHttpHeaders.toSpacedString();

  QString columnName( mGeometryColumn );
  columnName.replace( '\\', "\\\\"_L1 );
  columnName.replace( ')', "\\)"_L1 );

  if ( !mTable.isEmpty() )
  {
    uri += u" table=%1%2"_s
           .arg( quotedTablename(),
                 mGeometryColumn.isEmpty() ? QString() : u" (%1)"_s.arg( columnName ) );
  }
  else if ( !mSchema.isEmpty() )
  {
    uri += u" schema='%1'"_s.arg( escape( mSchema ) );
  }

  if ( !mSql.isEmpty() )
  {
    uri += u" sql="_s + mSql;
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
    url.addQueryItem( it.key(), QUrl::toPercentEncoding( it.value() ) );
  }

  if ( !mUsername.isEmpty() )
    url.addQueryItem( u"username"_s, QUrl::toPercentEncoding( mUsername ) );

  if ( !mPassword.isEmpty() )
    url.addQueryItem( u"password"_s, QUrl::toPercentEncoding( mPassword ) );

  if ( !mAuthConfigId.isEmpty() )
    url.addQueryItem( u"authcfg"_s, QUrl::toPercentEncoding( mAuthConfigId ) );

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

  const auto constQueryItems = query.queryItems( QUrl::ComponentFormattingOption::FullyDecoded );
  for ( const QPair<QString, QString> &item : constQueryItems )
  {
    if ( !item.first.startsWith( QgsHttpHeaders::PARAM_PREFIX ) && item.first != QgsHttpHeaders::KEY_REFERER )
    {
      if ( item.first == "username"_L1 )
        mUsername = query.queryItemValue( u"username"_s, QUrl::ComponentFormattingOption::FullyDecoded );
      else if ( item.first == "password"_L1 )
        mPassword = query.queryItemValue( u"password"_s, QUrl::ComponentFormattingOption::FullyDecoded );
      else if ( item.first == "authcfg"_L1 )
        mAuthConfigId = query.queryItemValue( u"authcfg"_s, QUrl::ComponentFormattingOption::FullyDecoded );
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
    return u"\"%1\".\"%2\""_s
           .arg( escape( mSchema, '"' ),
                 escape( mTable, '"' ) );
  else
    return u"\"%1\""_s
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
  if ( sslMode == "prefer"_L1 )
    return SslPrefer;
  else if ( sslMode == "disable"_L1 )
    return SslDisable;
  else if ( sslMode == "allow"_L1 )
    return SslAllow;
  else if ( sslMode == "require"_L1 )
    return SslRequire;
  else if ( sslMode == "verify-ca"_L1 )
    return SslVerifyCa;
  else if ( sslMode == "verify-full"_L1 )
    return SslVerifyFull;
  else
    return SslPrefer;  // default
}

QString QgsDataSourceUri::encodeSslMode( QgsDataSourceUri::SslMode sslMode )
{
  switch ( sslMode )
  {
    case SslPrefer: return u"prefer"_s;
    case SslDisable: return u"disable"_s;
    case SslAllow: return u"allow"_s;
    case SslRequire: return u"require"_s;
    case SslVerifyCa: return u"verify-ca"_s;
    case SslVerifyFull: return u"verify-full"_s;
  }
  return QString();
}

void QgsDataSourceUri::setParam( const QString &key, const QString &value )
{
  // maintain old API
  if ( key == "username"_L1 )
    mUsername = value;
  else if ( key == "password"_L1 )
    mPassword = value;
  else if ( key == "authcfg"_L1 )
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
  if ( key == "username"_L1 && !mUsername.isEmpty() )
  {
    mUsername.clear();
    return 1;
  }
  else if ( key == "password"_L1 && !mPassword.isEmpty() )
  {
    mPassword.clear();
    return 1;
  }
  else if ( key == "authcfg"_L1 && !mAuthConfigId.isEmpty() )
  {
    mAuthConfigId.clear();
    return 1;
  }

  return mParams.remove( key );
}

QString QgsDataSourceUri::param( const QString &key ) const
{
  // maintain old api
  if ( key == "username"_L1 && !mUsername.isEmpty() )
    return mUsername;
  else if ( key == "password"_L1 && !mPassword.isEmpty() )
    return mPassword;
  else if ( key == "authcfg"_L1 && !mAuthConfigId.isEmpty() )
    return mAuthConfigId;

  return mParams.value( key );
}

QStringList QgsDataSourceUri::params( const QString &key ) const
{
  // maintain old api
  if ( key == "username"_L1 && !mUsername.isEmpty() )
    return QStringList() << mUsername;
  else if ( key == "password"_L1 && !mPassword.isEmpty() )
    return QStringList() << mPassword;
  else if ( key == "authcfg"_L1 && !mAuthConfigId.isEmpty() )
    return QStringList() << mAuthConfigId;

  return mParams.values( key );
}

bool QgsDataSourceUri::hasParam( const QString &key ) const
{
  // maintain old api
  if ( key == "username"_L1 && !mUsername.isEmpty() )
    return true;
  else if ( key == "password"_L1 && !mPassword.isEmpty() )
    return true;
  else if ( key == "authcfg"_L1 && !mAuthConfigId.isEmpty() )
    return true;

  return mParams.contains( key );
}

QSet<QString> QgsDataSourceUri::parameterKeys() const
{
  QSet<QString> paramKeys;
  for ( auto it = mParams.constBegin(); it != mParams.constEnd(); it++ )
    paramKeys.insert( it.key() );

  if ( !mHost.isEmpty() )
    paramKeys.insert( "host"_L1 );
  if ( !mPort.isEmpty() )
    paramKeys.insert( "port"_L1 );
  if ( !mDriver.isEmpty() )
    paramKeys.insert( "driver"_L1 );
  if ( !mService.isEmpty() )
    paramKeys.insert( "service"_L1 );
  if ( !mDatabase.isEmpty() )
    paramKeys.insert( "dbname"_L1 );
  if ( !mSchema.isEmpty() )
    paramKeys.insert( "schema"_L1 );
  if ( !mTable.isEmpty() )
    paramKeys.insert( "table"_L1 );
  // Ignore mGeometryColumn: not a key ==> embedded in table value
  if ( !mSql.isEmpty() )
    paramKeys.insert( "sql"_L1 );
  if ( !mAuthConfigId.isEmpty() )
    paramKeys.insert( "authcfg"_L1 );
  if ( !mUsername.isEmpty() )
    paramKeys.insert( "username"_L1 );
  if ( !mPassword.isEmpty() )
    paramKeys.insert( "password"_L1 );
  if ( mSSLmode != SslPrefer )
    paramKeys.insert( "sslmode"_L1 );
  if ( !mKeyColumn.isEmpty() )
    paramKeys.insert( "key"_L1 );
  if ( mUseEstimatedMetadata )
    paramKeys.insert( "estimatedmetadata"_L1 );
  if ( mSelectAtIdDisabledSet )
    paramKeys.insert( "selectatid"_L1 );
  if ( mWkbType != Qgis::WkbType::Unknown )
    paramKeys.insert( "type"_L1 );
  if ( !mSrid.isEmpty() )
    paramKeys.insert( "srid"_L1 );
  return paramKeys;
}

bool QgsDataSourceUri::operator==( const QgsDataSourceUri &other ) const
{
  // cheap comparisons first:
  if ( mUseEstimatedMetadata != other.mUseEstimatedMetadata ||
       mSelectAtIdDisabled != other.mSelectAtIdDisabled ||
       mSelectAtIdDisabledSet != other.mSelectAtIdDisabledSet ||
       mSSLmode != other.mSSLmode ||
       mWkbType != other.mWkbType )
  {
    return false;
  }

  if ( mHost != other.mHost ||
       mPort != other.mPort ||
       mDriver != other.mDriver ||
       mService != other.mService ||
       mDatabase != other.mDatabase ||
       mSchema != other.mSchema ||
       mTable != other.mTable ||
       mGeometryColumn != other.mGeometryColumn ||
       mSql != other.mSql ||
       mAuthConfigId != other.mAuthConfigId ||
       mUsername != other.mUsername ||
       mPassword != other.mPassword ||
       mKeyColumn != other.mKeyColumn ||
       mSrid != other.mSrid ||
       mParams != other.mParams ||
       mHttpHeaders != other.mHttpHeaders )
  {
    return false;
  }

  return true;
}

bool QgsDataSourceUri::operator!=( const QgsDataSourceUri &other ) const
{
  return !( *this == other );
}
