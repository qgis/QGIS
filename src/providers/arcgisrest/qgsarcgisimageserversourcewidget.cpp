/***************************************************************************
    qgsarcgisimageserversourcewidget.cpp
     --------------------------------------
    Date                 : April 2026
    Copyright            : (C) 2026 by Nyall Dawson
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

#include "qgsarcgisimageserversourcewidget.h"

#include "qgsgdalutils.h"
#include "qgsimageserverprovider.h"
#include "qgsmaplayer.h"
#include "qgsproviderregistry.h"
#include "qgsrasterlayer.h"

#include <QString>

#include "moc_qgsarcgisimageserversourcewidget.cpp"

using namespace Qt::StringLiterals;

QgsArcGisImageServerSourceWidget::QgsArcGisImageServerSourceWidget( QgsMapLayer *layer, QWidget *parent )
  : QgsProviderSourceWidget( parent )
  , mProviderKey( layer->providerType() )
{
  setupUi( this );

  QgsRasterLayer *rasterLayer = qobject_cast< QgsRasterLayer * >( layer );
  if ( QgsRasterDataProvider *dataProvider = rasterLayer->dataProvider() )
  {
    QgsImageServerProvider *imageServerProvider = qobject_cast< QgsImageServerProvider * >( dataProvider );
    // this widget should ONLY be used for image server provider!
    Q_ASSERT( imageServerProvider );
    if ( imageServerProvider->serviceCapabilities().testFlag( Qgis::ArcGisRestServiceCapability::TilesOnly ) )
    {
      mUseTilesCheckBox->setChecked( true );
      mUseTilesCheckBox->setEnabled( false );
      mUseTilesCheckBox->setToolTip( tr( "This service only supports data retrieval via tiles" ) );
    }
    else if ( !imageServerProvider->supportsTiles() )
    {
      mUseTilesCheckBox->setChecked( false );
      mUseTilesCheckBox->setEnabled( false );
      mUseTilesCheckBox->setToolTip( tr( "This service does not support data retrieval via tiles" ) );
    }
    else
    {
      mUseTilesCheckBox->setEnabled( true );
    }
  }

  mImageFormatCombo->addItem( tr( "Default" ) );
  mImageFormatCombo->addItem( tr( "TIFF" ), u"tiff"_s );
  if ( QgsGdalUtils::supportsMrfLercCompression() )
  {
    mImageFormatCombo->addItem( tr( "LERC" ), u"lerc"_s );
  }
  mImageFormatCombo->addItem( tr( "Automatic (JPEG or PNG only)" ), u"jpgpng"_s );
  mImageFormatCombo->addItem( tr( "JPEG" ), u"jpg"_s );
  mImageFormatCombo->addItem( tr( "PNG" ), u"png"_s );
  mImageFormatCombo->addItem( tr( "PNG8" ), u"png8"_s );
  mImageFormatCombo->addItem( tr( "PNG24" ), u"png24"_s );
  mImageFormatCombo->addItem( tr( "PNG32" ), u"png32"_s );
}

void QgsArcGisImageServerSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( mProviderKey, uri );

  mAuthSettings->setUsername( mSourceParts.value( u"username"_s ).toString() );
  mAuthSettings->setPassword( mSourceParts.value( u"password"_s ).toString() );
  mEditReferer->setText( mSourceParts.value( u"referer"_s ).toString() );

  mAuthSettings->setConfigId( mSourceParts.value( u"authcfg"_s ).toString() );

  mImageFormatCombo->setCurrentIndex( mImageFormatCombo->findData( mSourceParts.value( u"format"_s ).toString() ) );
  if ( mImageFormatCombo->currentIndex() < 0 )
  {
    mImageFormatCombo->setCurrentIndex( 0 );
  }

  if ( mUseTilesCheckBox->isEnabled() )
  {
    // always default to tiled mode, it's opt-out
    mUseTilesCheckBox->setChecked( mSourceParts.value( u"tiled"_s, true ).toBool() );
  }
}

QString QgsArcGisImageServerSourceWidget::sourceUri() const
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

  const QString format = mImageFormatCombo->currentData().toString();
  if ( !format.isEmpty() )
    parts.insert( u"format"_s, format );
  else
    parts.remove( u"format"_s );

  if ( mUseTilesCheckBox->isEnabled() && !mUseTilesCheckBox->isChecked() )
    parts.insert( u"tiled"_s, false );
  else
    parts.remove( u"tiled"_s );

  return QgsProviderRegistry::instance()->encodeUri( mProviderKey, parts );
}

void QgsArcGisImageServerSourceWidget::setUsername( const QString &username )
{
  mAuthSettings->setUsername( username );
}

void QgsArcGisImageServerSourceWidget::setPassword( const QString &password )
{
  mAuthSettings->setPassword( password );
}

void QgsArcGisImageServerSourceWidget::setAuthCfg( const QString &id )
{
  mAuthSettings->setConfigId( id );
}

QString QgsArcGisImageServerSourceWidget::username() const
{
  return mAuthSettings->username();
}

QString QgsArcGisImageServerSourceWidget::password() const
{
  return mAuthSettings->password();
}

QString QgsArcGisImageServerSourceWidget::authcfg() const
{
  return mAuthSettings->configId();
}

void QgsArcGisImageServerSourceWidget::setReferer( const QString &referer )
{
  mEditReferer->setText( referer );
}

QString QgsArcGisImageServerSourceWidget::referer() const
{
  return mEditReferer->text();
}
