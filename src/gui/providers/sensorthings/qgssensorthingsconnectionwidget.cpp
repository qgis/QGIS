/***************************************************************************
    qgssensorthingsconnectionwidget.cpp
     --------------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingsconnectionwidget.h"
#include "moc_qgssensorthingsconnectionwidget.cpp"
#include "qgsproviderregistry.h"
#include "qgssensorthingsprovider.h"

///@cond PRIVATE
QgsSensorThingsConnectionWidget::QgsSensorThingsConnectionWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsSensorThingsConnectionWidget::validate );
  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsSensorThingsConnectionWidget::changed );

  connect( mAuthSettings, &QgsAuthSettingsWidget::configIdChanged, this, &QgsSensorThingsConnectionWidget::changed );
  connect( mAuthSettings, &QgsAuthSettingsWidget::usernameChanged, this, &QgsSensorThingsConnectionWidget::changed );
  connect( mAuthSettings, &QgsAuthSettingsWidget::passwordChanged, this, &QgsSensorThingsConnectionWidget::changed );
  connect( mEditReferer, &QLineEdit::textChanged, this, &QgsSensorThingsConnectionWidget::changed );
}

void QgsSensorThingsConnectionWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, uri );

  mEditUrl->setText( mSourceParts.value( QStringLiteral( "url" ) ).toString() );
  mAuthSettings->setUsername( mSourceParts.value( QStringLiteral( "username" ) ).toString() );
  mAuthSettings->setPassword( mSourceParts.value( QStringLiteral( "password" ) ).toString() );
  mEditReferer->setText( mSourceParts.value( QStringLiteral( "referer" ) ).toString() );

  mAuthSettings->setConfigId( mSourceParts.value( QStringLiteral( "authcfg" ) ).toString() );
}

QString QgsSensorThingsConnectionWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;

  parts.insert( QStringLiteral( "url" ), mEditUrl->text() );

  if ( !mAuthSettings->username().isEmpty() )
    parts.insert( QStringLiteral( "username" ), mAuthSettings->username() );
  else
    parts.remove( QStringLiteral( "username" ) );
  if ( !mAuthSettings->password().isEmpty() )
    parts.insert( QStringLiteral( "password" ), mAuthSettings->password() );
  else
    parts.remove( QStringLiteral( "password" ) );

  if ( !mEditReferer->text().isEmpty() )
    parts.insert( QStringLiteral( "referer" ), mEditReferer->text() );
  else
    parts.remove( QStringLiteral( "referer" ) );

  if ( !mAuthSettings->configId().isEmpty() )
    parts.insert( QStringLiteral( "authcfg" ), mAuthSettings->configId() );
  else
    parts.remove( QStringLiteral( "authcfg" ) );

  return QgsProviderRegistry::instance()->encodeUri( QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, parts );
}

void QgsSensorThingsConnectionWidget::setUrl( const QString &url )
{
  mEditUrl->setText( url );
}

QString QgsSensorThingsConnectionWidget::url() const
{
  return mEditUrl->text();
}

void QgsSensorThingsConnectionWidget::setUsername( const QString &username )
{
  mAuthSettings->setUsername( username );
}

void QgsSensorThingsConnectionWidget::setPassword( const QString &password )
{
  mAuthSettings->setPassword( password );
}

void QgsSensorThingsConnectionWidget::setAuthCfg( const QString &id )
{
  mAuthSettings->setConfigId( id );
}

QString QgsSensorThingsConnectionWidget::username() const
{
  return mAuthSettings->username();
}

QString QgsSensorThingsConnectionWidget::password() const
{
  return mAuthSettings->password();
}

QString QgsSensorThingsConnectionWidget::authcfg() const
{
  return mAuthSettings->configId();
}

void QgsSensorThingsConnectionWidget::setReferer( const QString &referer )
{
  mEditReferer->setText( referer );
}

QString QgsSensorThingsConnectionWidget::referer() const
{
  return mEditReferer->text();
}

void QgsSensorThingsConnectionWidget::validate()
{
  const bool valid = !mEditUrl->text().isEmpty();
  if ( valid == mIsValid )
    return;

  mIsValid = valid;
  emit validChanged( mIsValid );
}

///@endcond PRIVATE
