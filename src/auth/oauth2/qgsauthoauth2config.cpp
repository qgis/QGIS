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

#include <QDir>

#include "qjsonwrapper/Json.h"

#include "qgsapplication.h"
#include "qgslogger.h"


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

  // always recheck validity on any change
  // this, in turn, may emit validityChanged( bool )
  connect( this, &QgsAuthOAuth2Config::configChanged, this, &QgsAuthOAuth2Config::validateConfig );

  validateConfig();
}


void QgsAuthOAuth2Config::setId( const QString &value )
{
  QString preval( mId );
  mId = value;
  if ( preval != value )
    emit idChanged( mId );
}

void QgsAuthOAuth2Config::setVersion( int value )
{
  int preval( mVersion );
  mVersion = value;
  if ( preval != value )
    emit versionChanged( mVersion );
}

void QgsAuthOAuth2Config::setConfigType( QgsAuthOAuth2Config::ConfigType value )
{
  ConfigType preval( mConfigType );
  mConfigType = value;
  if ( preval != value )
    emit configTypeChanged( mConfigType );
}

void QgsAuthOAuth2Config::setGrantFlow( QgsAuthOAuth2Config::GrantFlow value )
{
  GrantFlow preval( mGrantFlow );
  mGrantFlow = value;
  if ( preval != value )
    emit grantFlowChanged( mGrantFlow );
}

void QgsAuthOAuth2Config::setName( const QString &value )
{
  QString preval( mName );
  mName = value;
  if ( preval != value )
    emit nameChanged( mName );
}

void QgsAuthOAuth2Config::setDescription( const QString &value )
{
  QString preval( mDescription );
  mDescription = value;
  if ( preval != value )
    emit descriptionChanged( mDescription );
}

void QgsAuthOAuth2Config::setRequestUrl( const QString &value )
{
  QString preval( mRequestUrl );
  mRequestUrl = value;
  if ( preval != value )
    emit requestUrlChanged( mRequestUrl );
}

void QgsAuthOAuth2Config::setTokenUrl( const QString &value )
{
  QString preval( mTokenUrl );
  mTokenUrl = value;
  if ( preval != value )
    emit tokenUrlChanged( mTokenUrl );
}

void QgsAuthOAuth2Config::setRefreshTokenUrl( const QString &value )
{
  QString preval( mRefreshTokenUrl );
  mRefreshTokenUrl = value;
  if ( preval != value )
    emit refreshTokenUrlChanged( mRefreshTokenUrl );
}

void QgsAuthOAuth2Config::setRedirectUrl( const QString &value )
{
  QString preval( mRedirectURL );
  mRedirectURL = value;
  if ( preval != value )
    emit redirectUrlChanged( mRedirectURL );
}

void QgsAuthOAuth2Config::setRedirectPort( int value )
{
  int preval( mRedirectPort );
  mRedirectPort = value;
  if ( preval != value )
    emit redirectPortChanged( mRedirectPort );
}

void QgsAuthOAuth2Config::setClientId( const QString &value )
{
  QString preval( mClientId );
  mClientId = value;
  if ( preval != value )
    emit clientIdChanged( mClientId );
}

void QgsAuthOAuth2Config::setClientSecret( const QString &value )
{
  QString preval( mClientSecret );
  mClientSecret = value;
  if ( preval != value )
    emit clientSecretChanged( mClientSecret );
}

void QgsAuthOAuth2Config::setUsername( const QString &value )
{
  QString preval( mUsername );
  mUsername = value;
  if ( preval != value )
    emit usernameChanged( mUsername );
}

void QgsAuthOAuth2Config::setPassword( const QString &value )
{
  QString preval( mPassword );
  mPassword = value;
  if ( preval != value )
    emit passwordChanged( mPassword );
}

void QgsAuthOAuth2Config::setScope( const QString &value )
{
  QString preval( mScope );
  mScope = value;
  if ( preval != value )
    emit scopeChanged( mScope );
}

void QgsAuthOAuth2Config::setApiKey( const QString &value )
{
  QString preval( mApiKey );
  mApiKey = value;
  if ( preval != value )
    emit apiKeyChanged( mApiKey );
}

void QgsAuthOAuth2Config::setPersistToken( bool persist )
{
  bool preval( mPersistToken );
  mPersistToken = persist;
  if ( preval != persist )
    emit persistTokenChanged( mPersistToken );
}

void QgsAuthOAuth2Config::setAccessMethod( QgsAuthOAuth2Config::AccessMethod value )
{
  AccessMethod preval( mAccessMethod );
  mAccessMethod = value;
  if ( preval != value )
    emit accessMethodChanged( mAccessMethod );
}

void QgsAuthOAuth2Config::setRequestTimeout( int value )
{
  int preval( mRequestTimeout );
  mRequestTimeout = value;
  if ( preval != value )
    emit requestTimeoutChanged( mRequestTimeout );
}

void QgsAuthOAuth2Config::setQueryPairs( const QVariantMap &pairs )
{
  QVariantMap preval( mQueryPairs );
  mQueryPairs = pairs;
  if ( preval != pairs )
    emit queryPairsChanged( mQueryPairs );
}

void QgsAuthOAuth2Config::setToDefaults()
{
  setId( QString() );
  setVersion( 1 );
  setConfigType( QgsAuthOAuth2Config::Custom );
  setGrantFlow( QgsAuthOAuth2Config::AuthCode );
  setName( QString() );
  setDescription( QString() );
  setRequestUrl( QString() );
  setTokenUrl( QString() );
  setRefreshTokenUrl( QString() );
  setRedirectUrl( QString() );
  setRedirectPort( 7070 );
  setClientId( QString() );
  setClientSecret( QString() );
  setUsername( QString() );
  setPassword( QString() );
  setScope( QString() );
  setApiKey( QString() );
  setPersistToken( false );
  setAccessMethod( QgsAuthOAuth2Config::Header );
  setRequestTimeout( 30 ); // in seconds
  setQueryPairs( QVariantMap() );
}

bool QgsAuthOAuth2Config::operator==( const QgsAuthOAuth2Config &other ) const
{
  return ( other.version() == this->version()
           && other.configType() == this->configType()
           && other.grantFlow() == this->grantFlow()
           && other.name() == this->name()
           && other.description() == this->description()
           && other.requestUrl() == this->requestUrl()
           && other.tokenUrl() == this->tokenUrl()
           && other.refreshTokenUrl() == this->refreshTokenUrl()
           && other.redirectUrl() == this->redirectUrl()
           && other.redirectPort() == this->redirectPort()
           && other.clientId() == this->clientId()
           && other.clientSecret() == this->clientSecret()
           && other.username() == this->username()
           && other.password() == this->password()
           && other.scope() == this->scope()
           && other.apiKey() == this->apiKey()
           && other.persistToken() == this->persistToken()
           && other.accessMethod() == this->accessMethod()
           && other.requestTimeout() == this->requestTimeout()
           && other.queryPairs() == this->queryPairs() );
}

bool QgsAuthOAuth2Config::operator!=( const QgsAuthOAuth2Config &other ) const
{
  return  !( *this == other );
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
  bool oldvalid = mValid;

  if ( mGrantFlow == AuthCode || mGrantFlow == Implicit )
  {
    mValid = ( !requestUrl().isEmpty()
               && !tokenUrl().isEmpty()
               && !clientId().isEmpty()
               && ( mGrantFlow == AuthCode ? !clientSecret().isEmpty() : true )
               && redirectPort() > 0
               && ( needsId ? !id().isEmpty() : true ) );
  }
  else if ( mGrantFlow == ResourceOwner )
  {
    mValid = ( !tokenUrl().isEmpty()
               && !username().isEmpty()
               && !password().isEmpty()
               && ( needsId ? !id().isEmpty() : true ) );
  }

  if ( mValid != oldvalid )
    emit validityChanged( mValid );
}

bool QgsAuthOAuth2Config::loadConfigTxt(
  const QByteArray &configtxt, QgsAuthOAuth2Config::ConfigFormat format )
{
  QByteArray errStr;
  bool res = false;

  switch ( format )
  {
    case JSON:
    {
      QVariant variant = QJsonWrapper::parseJson( configtxt, &res, &errStr );
      if ( !res )
      {
        QgsDebugMsg( QStringLiteral( "Error parsing JSON: %1" ).arg( QString( errStr ) ) );
        return res;
      }
      QJsonWrapper::qvariant2qobject( variant.toMap(), this );
      break;
    }
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported output format" ) );
  }
  return true;
}

QByteArray QgsAuthOAuth2Config::saveConfigTxt(
  QgsAuthOAuth2Config::ConfigFormat format, bool pretty, bool *ok ) const
{
  QByteArray out;
  QByteArray errStr;
  bool res = false;

  if ( !isValid() )
  {
    QgsDebugMsg( QStringLiteral( "FAILED, config is not valid" ) );
    if ( ok )
      *ok = res;
    return out;
  }

  switch ( format )
  {
    case JSON:
    {
      QVariantMap variant = QJsonWrapper::qobject2qvariant( this );
      out = QJsonWrapper::toJson( variant, &res, &errStr, pretty );
      if ( !res )
      {
        QgsDebugMsg( QStringLiteral( "Error serializing JSON: %1" ).arg( QString( errStr ) ) );
      }
      break;
    }
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported output format" ) );
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
  vmap.insert( QStringLiteral( "redirectPort" ), this->redirectPort() );
  vmap.insert( QStringLiteral( "redirectUrl" ), this->redirectUrl() );
  vmap.insert( QStringLiteral( "refreshTokenUrl" ), this->refreshTokenUrl() );
  vmap.insert( QStringLiteral( "accessMethod" ), static_cast<int>( this->accessMethod() ) );
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
  bool *ok )
{
  QByteArray out;
  QByteArray errStr;
  bool res = false;

  switch ( format )
  {
    case JSON:
      out = QJsonWrapper::toJson( variant, &res, &errStr, pretty );
      if ( !res )
      {
        QgsDebugMsg( QStringLiteral( "Error serializing JSON: %1" ).arg( QString( errStr ) ) );
      }
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported output format" ) );
  }

  if ( ok )
    *ok = res;
  return out;
}

// static
QVariantMap QgsAuthOAuth2Config::variantFromSerialized(
  const QByteArray &serial,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok )
{
  QVariantMap vmap;
  QByteArray errStr;
  bool res = false;

  switch ( format )
  {
    case JSON:
    {
      QVariant var = QJsonWrapper::parseJson( serial, &res, &errStr );
      if ( !res )
      {
        QgsDebugMsg( QStringLiteral( "Error parsing JSON to variant: %1" ).arg( QString( errStr ) ) );
        if ( ok )
          *ok = res;
        return vmap;
      }

      if ( var.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "Error parsing JSON to variant: %1" ).arg( "invalid or null" ) );
        if ( ok )
          *ok = res;
        return vmap;
      }
      vmap = var.toMap();
      if ( vmap.isEmpty() )
      {
        QgsDebugMsg( QStringLiteral( "Error parsing JSON to variantmap: %1" ).arg( "map empty" ) );
        if ( ok )
          *ok = res;
        return vmap;
      }
      break;
    }
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported output format" ) );
  }

  if ( ok )
    *ok = res;
  return vmap;
}

//static
bool QgsAuthOAuth2Config::writeOAuth2Config(
  const QString &filepath,
  QgsAuthOAuth2Config *config,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool pretty )
{
  bool res = false;
  QByteArray configtxt = config->saveConfigTxt( format, pretty, &res );
  if ( !res )
  {
    QgsDebugMsg( QStringLiteral( "FAILED to save config to text" ) );
    return false;
  }

  QFile config_file( filepath );
  QString file_path( config_file.fileName() );

  if ( config_file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) )
  {
    qint64 bytesWritten = config_file.write( configtxt );
    config_file.close();
    if ( bytesWritten == -1 )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to write config file: %1" ).arg( file_path ) );
      return false;
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "FAILED to open for writing config file: %1" ).arg( file_path ) );
    return false;
  }

  if ( !config_file.setPermissions( QFile::ReadOwner | QFile::WriteOwner ) )
  {
    QgsDebugMsg( QStringLiteral( "FAILED to set permissions config file: %1" ).arg( file_path ) );
    return false;
  }

  return true;
}

// static
QList<QgsAuthOAuth2Config *> QgsAuthOAuth2Config::loadOAuth2Configs(
  const QString &configdirectory,
  QObject *parent,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok )
{
  QList<QgsAuthOAuth2Config *> configs = QList<QgsAuthOAuth2Config *>();
  bool res = false;
  QStringList namefilters;

  switch ( format )
  {
    case JSON:
      namefilters << QStringLiteral( "*.json" );
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported output format" ) );
      if ( ok )
        *ok = res;
      return configs;
  }

  QDir configdir( configdirectory );
  configdir.setNameFilters( namefilters );
  const QStringList configfiles = configdir.entryList( namefilters );

  if ( configfiles.size() > 0 )
  {
    QgsDebugMsg( QStringLiteral( "Config files found in: %1...\n%2" )
                 .arg( configdir.path(), configfiles.join( QStringLiteral( ", " ) ) ) );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "No config files found in: %1" ).arg( configdir.path() ) );
    if ( ok ) *ok = res;
    return configs;
  }

  // Add entries
  for ( const auto &configfile : configfiles )
  {
    QByteArray configtxt;
    QFile cfile( configdir.path() + '/' + configfile );
    if ( cfile.exists() )
    {
      bool ret = cfile.open( QIODevice::ReadOnly | QIODevice::Text );
      if ( ret )
      {
        configtxt = cfile.readAll();
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "FAILED to open config for reading: %1" ).arg( configfile ) );
      }
      cfile.close();
    }

    if ( configtxt.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "EMPTY read of config: %1" ).arg( configfile ) );
      continue;
    }

    QgsAuthOAuth2Config *config = new QgsAuthOAuth2Config( parent );
    if ( !config->loadConfigTxt( configtxt, format ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load config: %1" ).arg( configfile ) );
      config->deleteLater();
      continue;
    }
    configs << config;
  }

  if ( ok ) *ok = true;
  return configs;
}

// static
QgsStringMap QgsAuthOAuth2Config::mapOAuth2Configs(
  const QString &configdirectory,
  QObject *parent,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok )
{
  QgsStringMap configs = QgsStringMap();
  bool res = false;
  QStringList namefilters;

  switch ( format )
  {
    case JSON:
      namefilters << QStringLiteral( "*.json" );
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Unsupported output format" ) );
      if ( ok )
        *ok = res;
      return configs;
  }

  QDir configdir( configdirectory );
  configdir.setNameFilters( namefilters );
  const QStringList configfiles = configdir.entryList( namefilters );

  if ( configfiles.size() > 0 )
  {
    QgsDebugMsg( QStringLiteral( "Config files found in: %1...\n%2" )
                 .arg( configdir.path(), configfiles.join( QStringLiteral( ", " ) ) ) );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "No config files found in: %1" ).arg( configdir.path() ) );
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
      bool ret = cfile.open( QIODevice::ReadOnly | QIODevice::Text );
      if ( ret )
      {
        configtxt = cfile.readAll();
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "FAILED to open config for reading: %1" ).arg( configfile ) );
      }
      cfile.close();
    }

    if ( configtxt.isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "EMPTY read of config: %1" ).arg( configfile ) );
      continue;
    }

    // validate the config before caching it
    std::unique_ptr<QgsAuthOAuth2Config, std::function<void( QgsAuthOAuth2Config * )> > config( new QgsAuthOAuth2Config( parent ), []( QgsAuthOAuth2Config * cfg ) { cfg->deleteLater( );} );
    if ( !config->loadConfigTxt( configtxt, format ) )
    {
      QgsDebugMsg( QStringLiteral( "FAILED to load config: %1" ).arg( configfile ) );
      continue;
    }
    if ( config->id().isEmpty() )
    {
      QgsDebugMsg( QStringLiteral( "NO ID SET for config: %1" ).arg( configfile ) );
      continue;
    }
    configs.insert( config->id(), configtxt );
  }

  if ( ok )
    *ok = true;
  return configs;
}

QgsStringMap QgsAuthOAuth2Config::mappedOAuth2ConfigsCache( QObject *parent, const QString &extradir )
{
  QgsStringMap configs;
  bool ok = false;

  // Load from default locations
  QStringList configdirs;
  // in order of override preference, i.e. user over pkg dir
  configdirs << QgsAuthOAuth2Config::oauth2ConfigsPkgDataDir()
             << QgsAuthOAuth2Config::oauth2ConfigsUserSettingsDir();

  if ( !extradir.isEmpty() )
  {
    // configs of similar IDs in this dir will override existing in standard dirs
    configdirs << extradir;
  }

  for ( const auto &configdir : qgis::as_const( configdirs ) )
  {
    QFileInfo configdirinfo( configdir );
    if ( !configdirinfo.exists() || !configdirinfo.isDir() )
    {
      continue;
    }
    QgsStringMap newconfigs = QgsAuthOAuth2Config::mapOAuth2Configs(
                                configdirinfo.canonicalFilePath(), parent, QgsAuthOAuth2Config::JSON, &ok );
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
  return QgsApplication::qgisSettingsDirPath() + QStringLiteral( "/oauth2_configs" );
}

// static
QString QgsAuthOAuth2Config::configTypeString( QgsAuthOAuth2Config::ConfigType configtype )
{
  switch ( configtype )
  {
    case QgsAuthOAuth2Config::Custom:
      return tr( "Custom" );
    case QgsAuthOAuth2Config::Predefined:
    default:
      return tr( "Predefined" );
  }
}

// static
QString QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::GrantFlow flow )
{
  switch ( flow )
  {
    case QgsAuthOAuth2Config::AuthCode:
      return tr( "Authorization Code" );
    case QgsAuthOAuth2Config::Implicit:
      return tr( "Implicit" );
    case QgsAuthOAuth2Config::ResourceOwner:
    default:
      return tr( "Resource Owner" );
  }
}

// static
QString QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::AccessMethod method )
{
  switch ( method )
  {
    case QgsAuthOAuth2Config::Header:
      return tr( "Header" );
    case QgsAuthOAuth2Config::Form:
      return tr( "Form (POST only)" );
    case QgsAuthOAuth2Config::Query:
    default:
      return tr( "URL Query" );
  }
}

// static
QString QgsAuthOAuth2Config::tokenCacheDirectory( bool temporary )
{
  QDir setdir( QgsApplication::qgisSettingsDirPath() );
  return  QStringLiteral( "%1/oauth2-cache" ).arg( temporary ? QDir::tempPath() : setdir.canonicalPath() );
}

// static
QString QgsAuthOAuth2Config::tokenCacheFile( const QString &suffix )
{
  return QStringLiteral( "authcfg-%1.ini" ).arg( !suffix.isEmpty() ? suffix : QStringLiteral( "cache" ) );
}

// static
QString QgsAuthOAuth2Config::tokenCachePath( const QString &suffix, bool temporary )
{
  return QStringLiteral( "%1/%2" ).arg( QgsAuthOAuth2Config::tokenCacheDirectory( temporary ),
                                        QgsAuthOAuth2Config::tokenCacheFile( suffix ) );
}
