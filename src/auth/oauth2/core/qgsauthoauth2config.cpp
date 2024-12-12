/***************************************************************************
    begin                : July 30, 2016
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
#include <functional>

#include "qgsauthoauth2config.h"
#include "moc_qgsauthoauth2config.cpp"

#include <QDir>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvariantutils.h"
#include "qgsjsonutils.h"

#include <nlohmann/json.hpp>

QgsAuthOAuth2Config::QgsAuthOAuth2Config( QObject *parent )
  : QObject( parent )
  , mQueryPairs( QVariantMap() )
{
  // internal signal bounces
  connect( this, &QgsAuthOAuth2Config::idChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::versionChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::configTypeChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::grantFlowChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::nameChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::descriptionChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::requestUrlChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::tokenUrlChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::refreshTokenUrlChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::redirectHostChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::redirectUrlChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::redirectPortChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::clientIdChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::clientSecretChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::usernameChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::passwordChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::scopeChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::apiKeyChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::persistTokenChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::accessMethodChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::requestTimeoutChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::queryPairsChanged, this, &QgsAuthOAuth2Config::configChanged );
  connect( this, &QgsAuthOAuth2Config::customHeaderChanged, this, &QgsAuthOAuth2Config::configChanged );

  // always recheck validity on any change
  // this, in turn, may emit validityChanged( bool )
  connect( this, &QgsAuthOAuth2Config::configChanged, this, &QgsAuthOAuth2Config::validateConfig );

  validateConfig();
}


void QgsAuthOAuth2Config::setId( const QString &value )
{
  const QString preval( mId );
  mId = value;
  if ( preval != value )
    emit idChanged( mId );
}

void QgsAuthOAuth2Config::setVersion( int value )
{
  const int preval( mVersion );
  mVersion = value;
  if ( preval != value )
    emit versionChanged( mVersion );
}

void QgsAuthOAuth2Config::setConfigType( QgsAuthOAuth2Config::ConfigType value )
{
  const ConfigType preval( mConfigType );
  mConfigType = value;
  if ( preval != value )
    emit configTypeChanged( mConfigType );
}

void QgsAuthOAuth2Config::setGrantFlow( QgsAuthOAuth2Config::GrantFlow value )
{
  const GrantFlow preval( mGrantFlow );
  mGrantFlow = value;
  if ( preval != value )
    emit grantFlowChanged( mGrantFlow );
}

void QgsAuthOAuth2Config::setName( const QString &value )
{
  const QString preval( mName );
  mName = value;
  if ( preval != value )
    emit nameChanged( mName );
}

void QgsAuthOAuth2Config::setDescription( const QString &value )
{
  const QString preval( mDescription );
  mDescription = value;
  if ( preval != value )
    emit descriptionChanged( mDescription );
}

void QgsAuthOAuth2Config::setRequestUrl( const QString &value )
{
  const QString preval( mRequestUrl );
  mRequestUrl = value.trimmed();
  if ( preval != mRequestUrl )
    emit requestUrlChanged( mRequestUrl );
}

void QgsAuthOAuth2Config::setTokenUrl( const QString &value )
{
  const QString preval( mTokenUrl );
  mTokenUrl = value.trimmed();
  if ( preval != mTokenUrl )
    emit tokenUrlChanged( mTokenUrl );
}

void QgsAuthOAuth2Config::setRefreshTokenUrl( const QString &value )
{
  const QString preval( mRefreshTokenUrl );
  mRefreshTokenUrl = value.trimmed();
  if ( preval != mRefreshTokenUrl )
    emit refreshTokenUrlChanged( mRefreshTokenUrl );
}

void QgsAuthOAuth2Config::setRedirectHost( const QString &host )
{
  const QString preval( mRedirectHost );
  mRedirectHost = host.trimmed();
  if ( preval != mRedirectHost )
    emit redirectHostChanged( mRedirectHost );
}

void QgsAuthOAuth2Config::setRedirectUrl( const QString &value )
{
  const QString preval( mRedirectURL );
  mRedirectURL = value.trimmed();
  if ( preval != mRedirectURL )
    emit redirectUrlChanged( mRedirectURL );
}

void QgsAuthOAuth2Config::setRedirectPort( int value )
{
  const int preval( mRedirectPort );
  mRedirectPort = value;
  if ( preval != mRedirectPort )
    emit redirectPortChanged( mRedirectPort );
}

void QgsAuthOAuth2Config::setClientId( const QString &value )
{
  const QString preval( mClientId );
  mClientId = value.trimmed();
  if ( preval != mClientId )
    emit clientIdChanged( mClientId );
}

void QgsAuthOAuth2Config::setClientSecret( const QString &value )
{
  const QString preval( mClientSecret );
  mClientSecret = value.trimmed();
  if ( preval != mClientSecret )
    emit clientSecretChanged( mClientSecret );
}

void QgsAuthOAuth2Config::setUsername( const QString &value )
{
  const QString preval( mUsername );
  mUsername = value;
  if ( preval != value )
    emit usernameChanged( mUsername );
}

void QgsAuthOAuth2Config::setPassword( const QString &value )
{
  const QString preval( mPassword );
  mPassword = value;
  if ( preval != value )
    emit passwordChanged( mPassword );
}

void QgsAuthOAuth2Config::setScope( const QString &value )
{
  const QString preval( mScope );
  mScope = value;
  if ( preval != value )
    emit scopeChanged( mScope );
}

void QgsAuthOAuth2Config::setApiKey( const QString &value )
{
  const QString preval( mApiKey );
  mApiKey = value;
  if ( preval != value )
    emit apiKeyChanged( mApiKey );
}

void QgsAuthOAuth2Config::setPersistToken( bool persist )
{
  const bool preval( mPersistToken );
  mPersistToken = persist;
  if ( preval != persist )
    emit persistTokenChanged( mPersistToken );
}

void QgsAuthOAuth2Config::setAccessMethod( QgsAuthOAuth2Config::AccessMethod value )
{
  const AccessMethod preval( mAccessMethod );
  mAccessMethod = value;
  if ( preval != value )
    emit accessMethodChanged( mAccessMethod );
}

void QgsAuthOAuth2Config::setCustomHeader( const QString &header )
{
  const QString preval( mCustomHeader );
  mCustomHeader = header;
  if ( preval != header )
    emit customHeaderChanged( mCustomHeader );
}

void QgsAuthOAuth2Config::setRequestTimeout( int value )
{
  const int preval( mRequestTimeout );
  mRequestTimeout = value;
  if ( preval != value )
    emit requestTimeoutChanged( mRequestTimeout );
}

void QgsAuthOAuth2Config::setQueryPairs( const QVariantMap &pairs )
{
  const QVariantMap preval( mQueryPairs );
  mQueryPairs = pairs;
  if ( preval != pairs )
    emit queryPairsChanged( mQueryPairs );
}

void QgsAuthOAuth2Config::setToDefaults()
{
  setId( QString() );
  setVersion( 1 );
  setConfigType( QgsAuthOAuth2Config::ConfigType::Custom );
  setGrantFlow( QgsAuthOAuth2Config::GrantFlow::AuthCode );
  setName( QString() );
  setDescription( QString() );
  setRequestUrl( QString() );
  setTokenUrl( QString() );
  setRefreshTokenUrl( QString() );
  setRedirectHost( QStringLiteral( "127.0.0.1" ) );
  setRedirectUrl( QString() );
  setRedirectPort( 7070 );
  setClientId( QString() );
  setClientSecret( QString() );
  setUsername( QString() );
  setPassword( QString() );
  setScope( QString() );
  setApiKey( QString() );
  setPersistToken( false );
  setAccessMethod( QgsAuthOAuth2Config::AccessMethod::Header );
  setCustomHeader( QString() );
  setRequestTimeout( 30 ); // in seconds
  setQueryPairs( QVariantMap() );
}

bool QgsAuthOAuth2Config::operator==( const QgsAuthOAuth2Config &other ) const
{
  return ( other.version() == this->version() && other.configType() == this->configType() && other.grantFlow() == this->grantFlow() && other.name() == this->name() && other.description() == this->description() && other.requestUrl() == this->requestUrl() && other.tokenUrl() == this->tokenUrl() && other.refreshTokenUrl() == this->refreshTokenUrl() && other.redirectHost() == this->redirectHost() && other.redirectUrl() == this->redirectUrl() && other.redirectPort() == this->redirectPort() && other.clientId() == this->clientId() && other.clientSecret() == this->clientSecret() && other.username() == this->username() && other.password() == this->password() && other.scope() == this->scope() && other.apiKey() == this->apiKey() && other.persistToken() == this->persistToken() && other.accessMethod() == this->accessMethod() && other.customHeader() == this->customHeader() && other.requestTimeout() == this->requestTimeout() && other.queryPairs() == this->queryPairs() );
}

bool QgsAuthOAuth2Config::operator!=( const QgsAuthOAuth2Config &other ) const
{
  return !( *this == other );
}

bool QgsAuthOAuth2Config::isValid() const
{
  return mValid;
}

// slot
void QgsAuthOAuth2Config::validateConfig()
{
  validateConfigId( false );
}

// public
void QgsAuthOAuth2Config::validateConfigId( bool needsId )
{
  const bool oldvalid = mValid;

  if ( mGrantFlow == GrantFlow::AuthCode || mGrantFlow == GrantFlow::Implicit )
  {
    mValid = ( !requestUrl().isEmpty() && !tokenUrl().isEmpty() && !clientId().isEmpty() && ( ( mGrantFlow == GrantFlow::AuthCode || mGrantFlow == GrantFlow::Pkce ) ? !clientSecret().isEmpty() : true ) && redirectPort() > 0 && ( needsId ? !id().isEmpty() : true ) );
  }
  else if ( mGrantFlow == GrantFlow::Pkce ) // No client secret for PKCE
  {
    mValid = ( !requestUrl().isEmpty() && !tokenUrl().isEmpty() && !clientId().isEmpty() && redirectPort() > 0 && ( needsId ? !id().isEmpty() : true ) );
  }
  else if ( mGrantFlow == GrantFlow::ResourceOwner )
  {
    mValid = ( !tokenUrl().isEmpty() && !username().isEmpty() && !password().isEmpty() && ( needsId ? !id().isEmpty() : true ) );
  }

  if ( mValid != oldvalid )
    emit validityChanged( mValid );
}

bool QgsAuthOAuth2Config::loadConfigTxt(
  const QByteArray &configtxt, QgsAuthOAuth2Config::ConfigFormat format
)
{
  QString errStr;

  switch ( format )
  {
    case ConfigFormat::JSON:
    {
      const QVariant variant = QgsJsonUtils::parseJson( configtxt.toStdString(), errStr );
      if ( !errStr.isEmpty() )
      {
        QgsDebugError( QStringLiteral( "Error parsing JSON: %1" ).arg( QString( errStr ) ) );
        return false;
      }
      const QVariantMap variantMap = variant.toMap();

      if ( variantMap.contains( QStringLiteral( "apiKey" ) ) )
        setApiKey( variantMap.value( QStringLiteral( "apiKey" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "clientId" ) ) )
        setClientId( variantMap.value( QStringLiteral( "clientId" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "clientSecret" ) ) )
        setClientSecret( variantMap.value( QStringLiteral( "clientSecret" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "configType" ) ) )
      {
        const int configTypeInt = variantMap.value( QStringLiteral( "configType" ) ).toInt();
        if ( configTypeInt >= 0 && configTypeInt <= static_cast<int>( ConfigType::Last ) )
          setConfigType( static_cast<ConfigType>( configTypeInt ) );
      }
      if ( variantMap.contains( QStringLiteral( "description" ) ) )
        setDescription( variantMap.value( QStringLiteral( "description" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "grantFlow" ) ) )
      {
        const int grantFlowInt = variantMap.value( QStringLiteral( "grantFlow" ) ).toInt();
        if ( grantFlowInt >= 0 && grantFlowInt <= static_cast<int>( GrantFlow::Last ) )
          setGrantFlow( static_cast<GrantFlow>( grantFlowInt ) );
      }
      if ( variantMap.contains( QStringLiteral( "id" ) ) )
        setId( variantMap.value( QStringLiteral( "id" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "name" ) ) )
        setName( variantMap.value( QStringLiteral( "name" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "password" ) ) )
        setPassword( variantMap.value( QStringLiteral( "password" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "persistToken" ) ) )
        setPersistToken( variantMap.value( QStringLiteral( "persistToken" ) ).toBool() );
      if ( variantMap.contains( QStringLiteral( "queryPairs" ) ) )
        setQueryPairs( variantMap.value( QStringLiteral( "queryPairs" ) ).toMap() );
      if ( variantMap.contains( QStringLiteral( "redirectHost" ) ) )
        setRedirectHost( variantMap.value( QStringLiteral( "redirectHost" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "redirectPort" ) ) )
        setRedirectPort( variantMap.value( QStringLiteral( "redirectPort" ) ).toInt() );
      if ( variantMap.contains( QStringLiteral( "redirectUrl" ) ) )
        setRedirectUrl( variantMap.value( QStringLiteral( "redirectUrl" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "refreshTokenUrl" ) ) )
        setRefreshTokenUrl( variantMap.value( QStringLiteral( "refreshTokenUrl" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "accessMethod" ) ) )
      {
        const int accessMethodInt = variantMap.value( QStringLiteral( "accessMethod" ) ).toInt();
        if ( accessMethodInt >= 0 && accessMethodInt <= static_cast<int>( AccessMethod::Last ) )
          setAccessMethod( static_cast<AccessMethod>( accessMethodInt ) );
      }

      if ( variantMap.contains( QStringLiteral( "customHeader" ) ) )
        setCustomHeader( variantMap.value( QStringLiteral( "customHeader" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "requestTimeout" ) ) )
        setRequestTimeout( variantMap.value( QStringLiteral( "requestTimeout" ) ).toInt() );
      if ( variantMap.contains( QStringLiteral( "requestUrl" ) ) )
        setRequestUrl( variantMap.value( QStringLiteral( "requestUrl" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "scope" ) ) )
        setScope( variantMap.value( QStringLiteral( "scope" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "tokenUrl" ) ) )
        setTokenUrl( variantMap.value( QStringLiteral( "tokenUrl" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "username" ) ) )
        setUsername( variantMap.value( QStringLiteral( "username" ) ).toString() );
      if ( variantMap.contains( QStringLiteral( "version" ) ) )
        setVersion( variantMap.value( QStringLiteral( "version" ) ).toInt() );

      break;
    }
    default:
      QgsDebugError( QStringLiteral( "Unsupported output format" ) );
  }
  return true;
}

QByteArray QgsAuthOAuth2Config::saveConfigTxt(
  QgsAuthOAuth2Config::ConfigFormat format, bool pretty, bool *ok
) const
{
  QByteArray out;
  bool res = false;

  if ( !isValid() )
  {
    QgsDebugError( QStringLiteral( "FAILED, config is not valid" ) );
    if ( ok )
      *ok = res;
    return out;
  }

  switch ( format )
  {
    case ConfigFormat::JSON:
    {
      QVariantMap variant;
      variant.insert( "accessMethod", static_cast<int>( accessMethod() ) );
      variant.insert( "apiKey", apiKey() );
      variant.insert( "clientId", clientId() );
      variant.insert( "clientSecret", clientSecret() );
      variant.insert( "configType", static_cast<int>( configType() ) );
      variant.insert( "customHeader", customHeader() );
      variant.insert( "description", description() );
      variant.insert( "grantFlow", static_cast<int>( grantFlow() ) );
      variant.insert( "id", id() );
      variant.insert( "name", name() );
      variant.insert( "objectName", objectName().isEmpty() ? "" : objectName() );
      variant.insert( "password", password() );
      variant.insert( "persistToken", persistToken() );
      variant.insert( "queryPairs", queryPairs() );
      variant.insert( "redirectHost", redirectHost() );
      variant.insert( "redirectPort", redirectPort() );
      variant.insert( "redirectUrl", redirectUrl() );
      variant.insert( "refreshTokenUrl", refreshTokenUrl() );
      variant.insert( "requestTimeout", requestTimeout() );
      variant.insert( "requestUrl", requestUrl() );
      variant.insert( "scope", scope() );
      variant.insert( "tokenUrl", tokenUrl() );
      variant.insert( "username", username() );
      variant.insert( "version", version() );

      out = QByteArray::fromStdString( QgsJsonUtils::jsonFromVariant( variant ).dump( pretty ? 4 : -1 ) );
      res = true;
      break;
    }
    default:
      QgsDebugError( QStringLiteral( "Unsupported output format" ) );
  }

  if ( ok )
    *ok = res;
  return out;
}

QVariantMap QgsAuthOAuth2Config::mappedProperties() const
{
  QVariantMap vmap;
  vmap.insert( QStringLiteral( "apiKey" ), this->apiKey() );
  vmap.insert( QStringLiteral( "clientId" ), this->clientId() );
  vmap.insert( QStringLiteral( "clientSecret" ), this->clientSecret() );
  vmap.insert( QStringLiteral( "configType" ), static_cast<int>( this->configType() ) );
  vmap.insert( QStringLiteral( "description" ), this->description() );
  vmap.insert( QStringLiteral( "grantFlow" ), static_cast<int>( this->grantFlow() ) );
  vmap.insert( QStringLiteral( "id" ), this->id() );
  vmap.insert( QStringLiteral( "name" ), this->name() );
  vmap.insert( QStringLiteral( "password" ), this->password() );
  vmap.insert( QStringLiteral( "persistToken" ), this->persistToken() );
  vmap.insert( QStringLiteral( "queryPairs" ), this->queryPairs() );
  vmap.insert( QStringLiteral( "redirectHost" ), this->redirectHost() );
  vmap.insert( QStringLiteral( "redirectPort" ), this->redirectPort() );
  vmap.insert( QStringLiteral( "redirectUrl" ), this->redirectUrl() );
  vmap.insert( QStringLiteral( "refreshTokenUrl" ), this->refreshTokenUrl() );
  vmap.insert( QStringLiteral( "accessMethod" ), static_cast<int>( this->accessMethod() ) );
  vmap.insert( QStringLiteral( "customHeader" ), this->customHeader() );
  vmap.insert( QStringLiteral( "requestTimeout" ), this->requestTimeout() );
  vmap.insert( QStringLiteral( "requestUrl" ), this->requestUrl() );
  vmap.insert( QStringLiteral( "scope" ), this->scope() );
  vmap.insert( QStringLiteral( "tokenUrl" ), this->tokenUrl() );
  vmap.insert( QStringLiteral( "username" ), this->username() );
  vmap.insert( QStringLiteral( "version" ), this->version() );

  return vmap;
}

// static
QByteArray QgsAuthOAuth2Config::serializeFromVariant(
  const QVariantMap &variant,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool pretty,
  bool *ok
)
{
  QByteArray out;
  bool res = false;
  switch ( format )
  {
    case ConfigFormat::JSON:
      out = QByteArray::fromStdString( QgsJsonUtils::jsonFromVariant( variant ).dump( pretty ? 4 : -1 ) );
      res = true;
      break;
    default:
      QgsDebugError( QStringLiteral( "Unsupported output format" ) );
  }

  if ( ok )
    *ok = res;
  return out;
}

// static
QVariantMap QgsAuthOAuth2Config::variantFromSerialized(
  const QByteArray &serial,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok
)
{
  QVariantMap vmap;
  QString errStr;

  switch ( format )
  {
    case ConfigFormat::JSON:
    {
      const QVariant var = QgsJsonUtils::parseJson( serial.toStdString(), errStr );
      if ( !errStr.isEmpty() )
      {
        QgsDebugError( QStringLiteral( "Error parsing JSON to variant: %1" ).arg( QString( errStr ) ) );
        if ( ok )
          *ok = false;
        return vmap;
      }

      if ( QgsVariantUtils::isNull( var ) )
      {
        QgsDebugError( QStringLiteral( "Error parsing JSON to variant: %1" ).arg( "invalid or null" ) );
        if ( ok )
          *ok = false;
        return vmap;
      }
      vmap = var.toMap();
      if ( vmap.isEmpty() )
      {
        QgsDebugError( QStringLiteral( "Error parsing JSON to variantmap: %1" ).arg( "map empty" ) );
        if ( ok )
          *ok = false;
        return vmap;
      }
      break;
    }
    default:
      QgsDebugError( QStringLiteral( "Unsupported output format" ) );
  }

  if ( ok )
    *ok = true;
  return vmap;
}

//static
bool QgsAuthOAuth2Config::writeOAuth2Config(
  const QString &filepath,
  QgsAuthOAuth2Config *config,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool pretty
)
{
  bool res = false;
  const QByteArray configtxt = config->saveConfigTxt( format, pretty, &res );
  if ( !res )
  {
    QgsDebugError( QStringLiteral( "FAILED to save config to text" ) );
    return false;
  }

  QFile config_file( filepath );
  const QString file_path( config_file.fileName() );

  if ( config_file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) )
  {
    const qint64 bytesWritten = config_file.write( configtxt );
    config_file.close();
    if ( bytesWritten == -1 )
    {
      QgsDebugError( QStringLiteral( "FAILED to write config file: %1" ).arg( file_path ) );
      return false;
    }
  }
  else
  {
    QgsDebugError( QStringLiteral( "FAILED to open for writing config file: %1" ).arg( file_path ) );
    return false;
  }

  if ( !config_file.setPermissions( QFile::ReadOwner | QFile::WriteOwner ) )
  {
    QgsDebugError( QStringLiteral( "FAILED to set permissions config file: %1" ).arg( file_path ) );
    return false;
  }

  return true;
}

// static
QList<QgsAuthOAuth2Config *> QgsAuthOAuth2Config::loadOAuth2Configs(
  const QString &configdirectory,
  QObject *parent,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok
)
{
  QList<QgsAuthOAuth2Config *> configs = QList<QgsAuthOAuth2Config *>();
  const bool res = false;
  QStringList namefilters;

  switch ( format )
  {
    case ConfigFormat::JSON:
      namefilters << QStringLiteral( "*.json" );
      break;
    default:
      QgsDebugError( QStringLiteral( "Unsupported output format" ) );
      if ( ok )
        *ok = res;
      return configs;
  }

  QDir configdir( configdirectory );
  configdir.setNameFilters( namefilters );
  const QStringList configfiles = configdir.entryList( namefilters );

  if ( configfiles.size() > 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Config files found in: %1...\n%2" ).arg( configdir.path(), configfiles.join( QLatin1String( ", " ) ) ), 2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "No config files found in: %1" ).arg( configdir.path() ), 2 );
    if ( ok )
      *ok = res;
    return configs;
  }

  // Add entries
  for ( const auto &configfile : configfiles )
  {
    QByteArray configtxt;
    QFile cfile( configdir.path() + '/' + configfile );
    if ( cfile.exists() )
    {
      const bool ret = cfile.open( QIODevice::ReadOnly | QIODevice::Text );
      if ( ret )
      {
        configtxt = cfile.readAll();
      }
      else
      {
        QgsDebugError( QStringLiteral( "FAILED to open config for reading: %1" ).arg( configfile ) );
      }
      cfile.close();
    }

    if ( configtxt.isEmpty() )
    {
      QgsDebugError( QStringLiteral( "EMPTY read of config: %1" ).arg( configfile ) );
      continue;
    }

    QgsAuthOAuth2Config *config = new QgsAuthOAuth2Config( parent );
    if ( !config->loadConfigTxt( configtxt, format ) )
    {
      QgsDebugError( QStringLiteral( "FAILED to load config: %1" ).arg( configfile ) );
      config->deleteLater();
      continue;
    }
    configs << config;
  }

  if ( ok )
    *ok = true;
  return configs;
}

// static
QgsStringMap QgsAuthOAuth2Config::mapOAuth2Configs(
  const QString &configdirectory,
  QObject *parent,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok
)
{
  QgsStringMap configs = QgsStringMap();
  const bool res = false;
  QStringList namefilters;

  switch ( format )
  {
    case ConfigFormat::JSON:
      namefilters << QStringLiteral( "*.json" );
      break;
    default:
      QgsDebugError( QStringLiteral( "Unsupported output format" ) );
      if ( ok )
        *ok = res;
      return configs;
  }

  QDir configdir( configdirectory );
  configdir.setNameFilters( namefilters );
  const QStringList configfiles = configdir.entryList( namefilters );

  if ( configfiles.size() > 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Config files found in: %1...\n%2" ).arg( configdir.path(), configfiles.join( QLatin1String( ", " ) ) ), 2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "No config files found in: %1" ).arg( configdir.path() ), 2 );
    if ( ok )
      *ok = res;
    return configs;
  }

  // Add entries
  for ( const auto &configfile : configfiles )
  {
    QByteArray configtxt;
    QFile cfile( configdir.path() + '/' + configfile );
    if ( cfile.exists() )
    {
      const bool ret = cfile.open( QIODevice::ReadOnly | QIODevice::Text );
      if ( ret )
      {
        configtxt = cfile.readAll();
      }
      else
      {
        QgsDebugError( QStringLiteral( "FAILED to open config for reading: %1" ).arg( configfile ) );
      }
      cfile.close();
    }

    if ( configtxt.isEmpty() )
    {
      QgsDebugError( QStringLiteral( "EMPTY read of config: %1" ).arg( configfile ) );
      continue;
    }

    // validate the config before caching it
    std::unique_ptr<QgsAuthOAuth2Config, std::function<void( QgsAuthOAuth2Config * )>> config( new QgsAuthOAuth2Config( parent ), []( QgsAuthOAuth2Config *cfg ) { cfg->deleteLater(); } );
    if ( !config->loadConfigTxt( configtxt, format ) )
    {
      QgsDebugError( QStringLiteral( "FAILED to load config: %1" ).arg( configfile ) );
      continue;
    }
    if ( config->id().isEmpty() )
    {
      QgsDebugError( QStringLiteral( "NO ID SET for config: %1" ).arg( configfile ) );
      continue;
    }
    configs.insert( config->id(), configtxt );
  }

  if ( ok )
    *ok = true;
  return configs;
}

QStringList QgsAuthOAuth2Config::configLocations( const QString &extradir )
{
  QStringList dirs;
  // in order of override preference, i.e. user over pkg dir
  dirs << QgsAuthOAuth2Config::oauth2ConfigsPkgDataDir()
       << QgsAuthOAuth2Config::oauth2ConfigsUserSettingsDir();

  if ( !extradir.isEmpty() )
  {
    // configs of similar IDs in this dir will override existing in standard dirs
    dirs << extradir;
  }
  return dirs;
}

QgsStringMap QgsAuthOAuth2Config::mappedOAuth2ConfigsCache( QObject *parent, const QString &extradir )
{
  QgsStringMap configs;
  bool ok = false;

  // Load from default locations
  const QStringList configdirs = configLocations( extradir );
  for ( const auto &configdir : configdirs )
  {
    const QFileInfo configdirinfo( configdir );
    if ( !configdirinfo.exists() || !configdirinfo.isDir() )
    {
      continue;
    }
    const QgsStringMap newconfigs = QgsAuthOAuth2Config::mapOAuth2Configs(
      configdirinfo.canonicalFilePath(), parent, QgsAuthOAuth2Config::ConfigFormat::JSON, &ok
    );
    if ( ok )
    {
      QgsStringMap::const_iterator i = newconfigs.constBegin();
      while ( i != newconfigs.constEnd() )
      {
        configs.insert( i.key(), i.value() );
        ++i;
      }
    }
  }
  return configs;
}

// static
QString QgsAuthOAuth2Config::oauth2ConfigsPkgDataDir()
{
  return QgsApplication::pkgDataPath() + QStringLiteral( "/oauth2_configs" );
}

// static
QString QgsAuthOAuth2Config::oauth2ConfigsUserSettingsDir()
{
  return QgsApplication::qgisSettingsDirPath() + QStringLiteral( "oauth2_configs" );
}

// static
QString QgsAuthOAuth2Config::configTypeString( QgsAuthOAuth2Config::ConfigType configtype )
{
  switch ( configtype )
  {
    case QgsAuthOAuth2Config::ConfigType::Custom:
      return tr( "Custom" );
    case QgsAuthOAuth2Config::ConfigType::Predefined:
      return tr( "Predefined" );
  }
  BUILTIN_UNREACHABLE
}

// static
QString QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::GrantFlow flow )
{
  switch ( flow )
  {
    case QgsAuthOAuth2Config::GrantFlow::AuthCode:
      return tr( "Authorization Code" );
    case QgsAuthOAuth2Config::GrantFlow::Implicit:
      return tr( "Implicit" );
    case QgsAuthOAuth2Config::GrantFlow::Pkce:
      return tr( "Authorization Code PKCE" );
    case QgsAuthOAuth2Config::GrantFlow::ResourceOwner:
      return tr( "Resource Owner" );
  }
  BUILTIN_UNREACHABLE
}

// static
QString QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::AccessMethod method )
{
  switch ( method )
  {
    case QgsAuthOAuth2Config::AccessMethod::Header:
      return tr( "Header" );
    case QgsAuthOAuth2Config::AccessMethod::Form:
      return tr( "Form (POST only)" );
    case QgsAuthOAuth2Config::AccessMethod::Query:
      return tr( "URL Query" );
  }
  BUILTIN_UNREACHABLE
}

// static
QString QgsAuthOAuth2Config::tokenCacheDirectory( bool temporary )
{
  const QDir setdir( QgsApplication::qgisSettingsDirPath() );
  return QStringLiteral( "%1/oauth2-cache" ).arg( temporary ? QDir::tempPath() : setdir.canonicalPath() );
}

// static
QString QgsAuthOAuth2Config::tokenCacheFile( const QString &suffix )
{
  return QStringLiteral( "authcfg-%1.ini" ).arg( !suffix.isEmpty() ? suffix : QStringLiteral( "cache" ) );
}

// static
QString QgsAuthOAuth2Config::tokenCachePath( const QString &suffix, bool temporary )
{
  return QStringLiteral( "%1/%2" ).arg( QgsAuthOAuth2Config::tokenCacheDirectory( temporary ), QgsAuthOAuth2Config::tokenCacheFile( suffix ) );
}
