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
#include "moc_qgsarcgisrestsourcewidget.cpp"

#include "qgsproviderregistry.h"


QgsArcGisRestSourceWidget::QgsArcGisRestSourceWidget( const QString &providerKey, QWidget *parent )
  : QgsProviderSourceWidget( parent )
  , mProviderKey( providerKey )
{
  setupUi( this );
}

void QgsArcGisRestSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( mProviderKey, uri );

  mAuthSettings->setUsername( mSourceParts.value( QStringLiteral( "username" ) ).toString() );
  mAuthSettings->setPassword( mSourceParts.value( QStringLiteral( "password" ) ).toString() );
  mEditReferer->setText( mSourceParts.value( QStringLiteral( "referer" ) ).toString() );

  mAuthSettings->setConfigId( mSourceParts.value( QStringLiteral( "authcfg" ) ).toString() );
}

QString QgsArcGisRestSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;

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
