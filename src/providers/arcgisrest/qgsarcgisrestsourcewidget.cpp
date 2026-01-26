/***************************************************************************
    qgsarcgisrestsourcewidget.cpp
     --------------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsarcgisrestsourcewidget.h"

#include "qgsproviderregistry.h"

#include "moc_qgsarcgisrestsourcewidget.cpp"

QgsArcGisRestSourceWidget::QgsArcGisRestSourceWidget( const QString &providerKey, QWidget *parent )
  : QgsProviderSourceWidget( parent )
  , mProviderKey( providerKey )
{
  setupUi( this );
}

void QgsArcGisRestSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( mProviderKey, uri );

  mAuthSettings->setUsername( mSourceParts.value( u"username"_s ).toString() );
  mAuthSettings->setPassword( mSourceParts.value( u"password"_s ).toString() );
  mEditReferer->setText( mSourceParts.value( u"referer"_s ).toString() );

  mAuthSettings->setConfigId( mSourceParts.value( u"authcfg"_s ).toString() );
}

QString QgsArcGisRestSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;

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

  return QgsProviderRegistry::instance()->encodeUri( mProviderKey, parts );
}

void QgsArcGisRestSourceWidget::setUsername( const QString &username )
{
  mAuthSettings->setUsername( username );
}

void QgsArcGisRestSourceWidget::setPassword( const QString &password )
{
  mAuthSettings->setPassword( password );
}

void QgsArcGisRestSourceWidget::setAuthCfg( const QString &id )
{
  mAuthSettings->setConfigId( id );
}

QString QgsArcGisRestSourceWidget::username() const
{
  return mAuthSettings->username();
}

QString QgsArcGisRestSourceWidget::password() const
{
  return mAuthSettings->password();
}

QString QgsArcGisRestSourceWidget::authcfg() const
{
  return mAuthSettings->configId();
}

void QgsArcGisRestSourceWidget::setReferer( const QString &referer )
{
  mEditReferer->setText( referer );
}

QString QgsArcGisRestSourceWidget::referer() const
{
  return mEditReferer->text();
}
