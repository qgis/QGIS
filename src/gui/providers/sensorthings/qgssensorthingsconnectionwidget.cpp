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

#include "qgsproviderregistry.h"
#include "qgssensorthingsprovider.h"

#include "moc_qgssensorthingsconnectionwidget.cpp"

///@cond PRIVATE
QgsSensorThingsConnectionWidget::QgsSensorThingsConnectionWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsSensorThingsConnectionWidget::validate );
  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsSensorThingsConnectionWidget::changed );

  // only auth config supported, not basic auth
  mAuthSettings->removeBasicSettings();

  connect( mAuthSettings, &QgsAuthSettingsWidget::configIdChanged, this, &QgsSensorThingsConnectionWidget::changed );
  connect( mAuthSettings, &QgsAuthSettingsWidget::usernameChanged, this, &QgsSensorThingsConnectionWidget::changed );
  connect( mAuthSettings, &QgsAuthSettingsWidget::passwordChanged, this, &QgsSensorThingsConnectionWidget::changed );
  connect( mEditReferer, &QLineEdit::textChanged, this, &QgsSensorThingsConnectionWidget::changed );
}

void QgsSensorThingsConnectionWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY, uri );

  mEditUrl->setText( mSourceParts.value( u"url"_s ).toString() );
  mAuthSettings->setUsername( mSourceParts.value( u"username"_s ).toString() );
  mAuthSettings->setPassword( mSourceParts.value( u"password"_s ).toString() );
  mEditReferer->setText( mSourceParts.value( u"referer"_s ).toString() );

  mAuthSettings->setConfigId( mSourceParts.value( u"authcfg"_s ).toString() );
}

QString QgsSensorThingsConnectionWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;

  parts.insert( u"url"_s, mEditUrl->text() );

  if ( !mAuthSettings->username().isEmpty() )
    parts.insert( u"username"_s, mAuthSettings->username() );
  else
    parts.remove( u"username"_s );
  if ( !mAuthSettings->password().isEmpty() )
    parts.insert( u"password"_s, mAuthSettings->password() );
  else
    parts.remove( u"password"_s );

  if ( !mEditReferer->text().isEmpty() )
    parts.insert( u"referer"_s, mEditReferer->text() );
  else
    parts.remove( u"referer"_s );

  if ( !mAuthSettings->configId().isEmpty() )
    parts.insert( u"authcfg"_s, mAuthSettings->configId() );
  else
    parts.remove( u"authcfg"_s );

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
