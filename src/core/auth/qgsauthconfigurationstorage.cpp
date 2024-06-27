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

QgsAuthConfigurationStorage::QgsAuthConfigurationStorage( const QMap<QString, QString> &configuration )
  : mConfiguration( configuration )
{
  // Forward all specific signals to the generic one
  connect( this, &QgsAuthConfigurationStorage::methodConfigChanged, this, &QgsAuthConfigurationStorage::storageChanged );
  connect( this, &QgsAuthConfigurationStorage::masterPasswordChanged, this, &QgsAuthConfigurationStorage::storageChanged );
  connect( this, &QgsAuthConfigurationStorage::authSettingsChanged, this, &QgsAuthConfigurationStorage::storageChanged );

#ifndef QT_NO_SSL
  connect( this, &QgsAuthConfigurationStorage::certIdentityChanged, this, &QgsAuthConfigurationStorage::storageChanged );
  connect( this, &QgsAuthConfigurationStorage::certAuthorityChanged, this, &QgsAuthConfigurationStorage::storageChanged );
  connect( this, &QgsAuthConfigurationStorage::sslCertCustomConfigChanged, this, &QgsAuthConfigurationStorage::storageChanged );
  connect( this, &QgsAuthConfigurationStorage::sslCertTrustPolicyChanged, this, &QgsAuthConfigurationStorage::storageChanged );
#endif

}

QString QgsAuthConfigurationStorage::lastError() const
{
  return mLastError;
}

Qgis::AuthConfigurationStorageCapabilities QgsAuthConfigurationStorage::capabilities() const
{
  return mCapabilities;
}

void QgsAuthConfigurationStorage::setCapabilities( const Qgis::AuthConfigurationStorageCapabilities &newCapabilities )
{
  mCapabilities = newCapabilities;
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

QMap<QString, QString> QgsAuthConfigurationStorage::settings() const
{
  return mConfiguration;
}


QString QgsAuthConfigurationStorage::loggerTag() const
{
  return QStringLiteral( "Auth storage %1" ).arg( name() );
}
