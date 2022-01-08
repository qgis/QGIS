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
#include "qgswmssourceselect.h"
#include "qgsproviderregistry.h"

#include "qgswmsprovider.h"

QgsXyzSourceWidget::QgsXyzSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  setupUi( this );

  // Behavior for min and max zoom checkbox
  connect( mCheckBoxZMin, &QCheckBox::toggled, mSpinZMin, &QSpinBox::setEnabled );
  connect( mCheckBoxZMax, &QCheckBox::toggled, mSpinZMax, &QSpinBox::setEnabled );
  mSpinZMax->setClearValue( 18 );

  connect( mEditUrl, &QLineEdit::textChanged, this, &QgsXyzSourceWidget::validate );
  mInterpretationCombo = new QgsWmsInterpretationComboBox( this );
  mInterpretationLayout->addWidget( mInterpretationCombo );
}

void QgsXyzSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "wms" ), uri );

  mEditUrl->setText( mSourceParts.value( QStringLiteral( "url" ) ).toString() );
  mCheckBoxZMin->setChecked( mSourceParts.value( QStringLiteral( "zmin" ) ).isValid() );
  mSpinZMin->setValue( mCheckBoxZMin->isChecked() ? mSourceParts.value( QStringLiteral( "zmin" ) ).toInt() : 0 );
  mCheckBoxZMax->setChecked( mSourceParts.value( QStringLiteral( "zmax" ) ).isValid() );
  mSpinZMax->setValue( mCheckBoxZMax->isChecked() ? mSourceParts.value( QStringLiteral( "zmax" ) ).toInt() : 18 );
  mAuthSettings->setUsername( mSourceParts.value( QStringLiteral( "username" ) ).toString() );
  mAuthSettings->setPassword( mSourceParts.value( QStringLiteral( "password" ) ).toString() );
  mEditReferer->setText( mSourceParts.value( QStringLiteral( "referer" ) ).toString() );

  int index = 0;  // default is "unknown"
  if ( mSourceParts.value( QStringLiteral( "tilePixelRatio" ) ).toInt() == 2. )
    index = 2;  // high-res
  else if ( mSourceParts.value( QStringLiteral( "tilePixelRatio" ) ).toInt() == 1. )
    index = 1;  // normal-res
  mComboTileResolution->setCurrentIndex( index );

  mAuthSettings->setConfigId( mSourceParts.value( QStringLiteral( "authcfg" ) ).toString() );

  setInterpretation( mSourceParts.value( QStringLiteral( "interpretation" ) ).toString() );

  mIsValid = true;
}

QString QgsXyzSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;

  parts.insert( QStringLiteral( "url" ), mEditUrl->text() );
  if ( mCheckBoxZMin->isChecked() )
    parts.insert( QStringLiteral( "zmin" ), mSpinZMin->value() );
  else
    parts.remove( QStringLiteral( "zmin" ) );
  if ( mCheckBoxZMax->isChecked() )
    parts.insert( QStringLiteral( "zmax" ), mSpinZMax->value() );
  else
    parts.remove( QStringLiteral( "zmax" ) );

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

  if ( mComboTileResolution->currentIndex() > 0 )
    parts.insert( QStringLiteral( "tilePixelRatio" ), mComboTileResolution->currentIndex() );
  else
    parts.remove( QStringLiteral( "tilePixelRatio" ) );

  if ( !mAuthSettings->configId().isEmpty() )
    parts.insert( QStringLiteral( "authcfg" ), mAuthSettings->configId() );
  else
    parts.remove( QStringLiteral( "authcfg" ) );

  if ( !mInterpretationCombo->interpretation().isEmpty() )
    parts.insert( QStringLiteral( "interpretation" ), mInterpretationCombo->interpretation() );
  else
    parts.remove( QStringLiteral( "interpretation" ) );

  return QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "wms" ), parts );
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
  int index = 0;  // default is "unknown"
  if ( ratio == 2. )
    index = 2;  // high-res
  else if ( ratio == 1. )
    index = 1;  // normal-res
  mComboTileResolution->setCurrentIndex( index );
}

int QgsXyzSourceWidget::tilePixelRatio() const
{
  if ( mComboTileResolution->currentIndex() == 1 )
    return 1.;  // normal-res
  else if ( mComboTileResolution->currentIndex() == 2 )
    return 2.;  // high-res
  else
    return 0;  // unknown
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
