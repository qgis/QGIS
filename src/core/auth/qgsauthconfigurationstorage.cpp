/***************************************************************************
  qgsauthconfigurationstorage.cpp - QgsAuthConfigurationStorage

 ---------------------
 begin                : 20.6.2024
 copyright            : (C) 2024 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsauthconfigurationstorage.h"
#include "moc_qgsauthconfigurationstorage.cpp"

QgsAuthConfigurationStorage::QgsAuthConfigurationStorage( const QMap<QString, QVariant> &configuration )
  : mConfiguration( configuration )
{
  // Forward all specific signals to the generic one
  connect( this, &QgsAuthConfigurationStorage::methodConfigChanged, this, [ this ]
  {
    emit storageChanged( id() );
  } );
  connect( this, &QgsAuthConfigurationStorage::masterPasswordChanged, this, [ this ]
  {
    emit storageChanged( id() );
  } );
  connect( this, &QgsAuthConfigurationStorage::authSettingsChanged, this, [ this ]
  {
    emit storageChanged( id() );
  } );

#ifndef QT_NO_SSL
  connect( this, &QgsAuthConfigurationStorage::certIdentityChanged, this, [ this ]
  {
    emit storageChanged( id() );
  } );
  connect( this, &QgsAuthConfigurationStorage::certAuthorityChanged, this, [ this ]
  {
    emit storageChanged( id() );
  } );
  connect( this, &QgsAuthConfigurationStorage::sslCertCustomConfigChanged, this, [ this ]
  {
    emit storageChanged( id() );
  } );
  connect( this, &QgsAuthConfigurationStorage::sslCertTrustPolicyChanged, this, [ this ]
  {
    emit storageChanged( id() );
  } );
#endif
}

void QgsAuthConfigurationStorage::setReadOnly( bool readOnly )
{
  if ( mIsReadOnly != readOnly )
  {
    mIsReadOnly = readOnly;
    emit readOnlyChanged( readOnly );
  }
}

bool QgsAuthConfigurationStorage::isReadOnly() const
{
  return mIsReadOnly;
}


QString QgsAuthConfigurationStorage::lastError() const
{
  return mLastError;
}

Qgis::AuthConfigurationStorageCapabilities QgsAuthConfigurationStorage::capabilities() const
{
  return mCapabilities;
}

void QgsAuthConfigurationStorage::setCapabilities( Qgis::AuthConfigurationStorageCapabilities capabilities )
{
  mCapabilities = capabilities;
}

void QgsAuthConfigurationStorage::setError( const QString &error, Qgis::MessageLevel level )
{
  mLastError = error;
  emit messageLog( error, loggerTag(), level );
}

bool QgsAuthConfigurationStorage::isEnabled() const
{
  return mIsEnabled;
}

void QgsAuthConfigurationStorage::setEnabled( bool enabled )
{
  mIsEnabled = enabled;
}

bool QgsAuthConfigurationStorage::isEncrypted() const
{
  return mIsEncrypted;
}

QMap<QString, QVariant> QgsAuthConfigurationStorage::settings() const
{
  return mConfiguration;
}


QString QgsAuthConfigurationStorage::loggerTag() const
{
  return tr( "Auth storage %1" ).arg( name() );
}

void QgsAuthConfigurationStorage::checkCapability( Qgis::AuthConfigurationStorageCapability capability ) const
{
  const auto caps { capabilities() };
  if ( !caps.testFlag( capability ) )
  {
    throw QgsNotSupportedException( tr( "Capability %1 is not supported by storage %2" ).arg( qgsEnumValueToKey<Qgis::AuthConfigurationStorageCapability>( capability ), name() ) );
  }
}
