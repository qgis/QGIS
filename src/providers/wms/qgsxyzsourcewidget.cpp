/***************************************************************************
    qgsxyzsourcewidget.cpp
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

#include "qgsxyzsourcewidget.h"

#include "qgsproviderregistry.h"
#include "qgswmssourceselect.h"

#include "moc_qgsxyzsourcewidget.cpp"

QgsXyzSourceWidget::QgsXyzSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  setupUi( this );

  // Behavior for min and max zoom checkbox
  connect( mCheckBoxZMin, &QCheckBox::toggled, mSpinZMin, &QSpinBox::setEnabled );
  connect( mCheckBoxZMax, &QCheckBox::toggled, mSpinZMax, &QSpinBox::setEnabled );
  mSpinZMax->setClearValue( 18 );

  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsXyzSourceWidget::validate );
  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsProviderSourceWidget::changed );

  connect( mCheckBoxZMin, &QCheckBox::toggled, this, &QgsProviderSourceWidget::changed );
  connect( mSpinZMin, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsProviderSourceWidget::changed );
  connect( mCheckBoxZMax, &QCheckBox::toggled, this, &QgsProviderSourceWidget::changed );
  connect( mSpinZMax, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsProviderSourceWidget::changed );
  connect( mAuthSettings, &QgsAuthSettingsWidget::configIdChanged, this, &QgsProviderSourceWidget::changed );
  connect( mAuthSettings, &QgsAuthSettingsWidget::usernameChanged, this, &QgsProviderSourceWidget::changed );
  connect( mAuthSettings, &QgsAuthSettingsWidget::passwordChanged, this, &QgsProviderSourceWidget::changed );
  connect( mEditReferer, &QLineEdit::textChanged, this, &QgsProviderSourceWidget::changed );
  connect( mComboTileResolution, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsProviderSourceWidget::changed );

  mInterpretationCombo = new QgsWmsInterpretationComboBox( this );
  mInterpretationLayout->addWidget( mInterpretationCombo );

  connect( mInterpretationCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsProviderSourceWidget::changed );
}

void QgsXyzSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( u"wms"_s, uri );

  mEditUrl->setText( mSourceParts.value( u"url"_s ).toString() );
  mCheckBoxZMin->setChecked( mSourceParts.value( u"zmin"_s ).isValid() );
  mSpinZMin->setValue( mCheckBoxZMin->isChecked() ? mSourceParts.value( u"zmin"_s ).toInt() : 0 );
  mCheckBoxZMax->setChecked( mSourceParts.value( u"zmax"_s ).isValid() );
  mSpinZMax->setValue( mCheckBoxZMax->isChecked() ? mSourceParts.value( u"zmax"_s ).toInt() : 18 );
  mAuthSettings->setUsername( mSourceParts.value( u"username"_s ).toString() );
  mAuthSettings->setPassword( mSourceParts.value( u"password"_s ).toString() );
  mEditReferer->setText( mSourceParts.value( u"http-header:referer"_s ).toString() );

  int index = 0; // default is "unknown"
  if ( mSourceParts.value( u"tilePixelRatio"_s ).toInt() == 2. )
    index = 2; // high-res
  else if ( mSourceParts.value( u"tilePixelRatio"_s ).toInt() == 1. )
    index = 1; // normal-res
  mComboTileResolution->setCurrentIndex( index );

  mAuthSettings->setConfigId( mSourceParts.value( u"authcfg"_s ).toString() );

  setInterpretation( mSourceParts.value( u"interpretation"_s ).toString() );
}

QString QgsXyzSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;

  parts.insert( u"url"_s, mEditUrl->text() );
  if ( mCheckBoxZMin->isChecked() )
    parts.insert( u"zmin"_s, mSpinZMin->value() );
  else
    parts.remove( u"zmin"_s );
  if ( mCheckBoxZMax->isChecked() )
    parts.insert( u"zmax"_s, mSpinZMax->value() );
  else
    parts.remove( u"zmax"_s );

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

  if ( mComboTileResolution->currentIndex() > 0 )
    parts.insert( u"tilePixelRatio"_s, mComboTileResolution->currentIndex() );
  else
    parts.remove( u"tilePixelRatio"_s );

  if ( !mAuthSettings->configId().isEmpty() )
    parts.insert( u"authcfg"_s, mAuthSettings->configId() );
  else
    parts.remove( u"authcfg"_s );

  if ( !mInterpretationCombo->interpretation().isEmpty() )
    parts.insert( u"interpretation"_s, mInterpretationCombo->interpretation() );
  else
    parts.remove( u"interpretation"_s );

  return QgsProviderRegistry::instance()->encodeUri( u"wms"_s, parts );
}

void QgsXyzSourceWidget::setUrl( const QString &url )
{
  mEditUrl->setText( url );
}

QString QgsXyzSourceWidget::url() const
{
  return mEditUrl->text();
}

void QgsXyzSourceWidget::setZMin( int zMin )
{
  mCheckBoxZMin->setChecked( zMin != -1 );
  mSpinZMin->setValue( zMin != -1 ? zMin : 0 );
}

int QgsXyzSourceWidget::zMin() const
{
  return mCheckBoxZMin->isChecked() ? mSpinZMin->value() : -1;
}

void QgsXyzSourceWidget::setZMax( int zMax )
{
  mCheckBoxZMax->setChecked( zMax != -1 );
  mSpinZMax->setValue( zMax != -1 ? zMax : 0 );
}

int QgsXyzSourceWidget::zMax() const
{
  return mCheckBoxZMax->isChecked() ? mSpinZMax->value() : -1;
}

void QgsXyzSourceWidget::setUsername( const QString &username )
{
  mAuthSettings->setUsername( username );
}

void QgsXyzSourceWidget::setPassword( const QString &password )
{
  mAuthSettings->setPassword( password );
}

void QgsXyzSourceWidget::setAuthCfg( const QString &id )
{
  mAuthSettings->setConfigId( id );
}

QString QgsXyzSourceWidget::username() const
{
  return mAuthSettings->username();
}

QString QgsXyzSourceWidget::password() const
{
  return mAuthSettings->password();
}

QString QgsXyzSourceWidget::authcfg() const
{
  return mAuthSettings->configId();
}

void QgsXyzSourceWidget::setReferer( const QString &referer )
{
  mEditReferer->setText( referer );
}

QString QgsXyzSourceWidget::referer() const
{
  return mEditReferer->text();
}

void QgsXyzSourceWidget::setTilePixelRatio( int ratio )
{
  int index = 0; // default is "unknown"
  if ( ratio == 2. )
    index = 2; // high-res
  else if ( ratio == 1. )
    index = 1; // normal-res
  mComboTileResolution->setCurrentIndex( index );
}

int QgsXyzSourceWidget::tilePixelRatio() const
{
  if ( mComboTileResolution->currentIndex() == 1 )
    return 1.; // normal-res
  else if ( mComboTileResolution->currentIndex() == 2 )
    return 2.; // high-res
  else
    return 0; // unknown
}

void QgsXyzSourceWidget::setInterpretation( const QString &interpretation )
{
  mInterpretationCombo->setInterpretation( interpretation );
}

QString QgsXyzSourceWidget::interpretation() const
{
  return mInterpretationCombo->interpretation();
}

void QgsXyzSourceWidget::validate()
{
  const bool valid = !mEditUrl->text().isEmpty();
  if ( valid != mIsValid )
    emit validChanged( valid );
  mIsValid = valid;
}
