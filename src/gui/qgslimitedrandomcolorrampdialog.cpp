/***************************************************************************
    qgslimitedrandomcolorrampdialog.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslimitedrandomcolorrampdialog.h"

#include "qgssymbollayerutils.h"
#include "qgscolorramp.h"
#include "qgshelp.h"

#include <QColorDialog>
#include <QDialogButtonBox>


QgsLimitedRandomColorRampWidget::QgsLimitedRandomColorRampWidget( const QgsLimitedRandomColorRamp &ramp, QWidget *parent )
  : QgsPanelWidget( parent )
  , mRamp( ramp )
{
  setupUi( this );
  spinCount->setClearValue( 10 );
  spinHue1->setClearValue( 0 );
  spinHue2->setClearValue( 359 );
  spinSat1->setClearValue( 100 );
  spinSat2->setClearValue( 240 );
  spinVal1->setClearValue( 200 );
  spinVal2->setClearValue( 240 );

  updateUi();

  connect( spinCount, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLimitedRandomColorRampWidget::setCount );
  connect( spinHue1, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLimitedRandomColorRampWidget::setHue1 );
  connect( spinHue2, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLimitedRandomColorRampWidget::setHue2 );
  connect( spinSat1, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLimitedRandomColorRampWidget::setSat1 );
  connect( spinSat2, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLimitedRandomColorRampWidget::setSat2 );
  connect( spinVal1, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLimitedRandomColorRampWidget::setVal1 );
  connect( spinVal2, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLimitedRandomColorRampWidget::setVal2 );
}

void QgsLimitedRandomColorRampWidget::setRamp( const QgsLimitedRandomColorRamp &ramp )
{
  mRamp = ramp;
  updateUi();
  emit changed();
}

void QgsLimitedRandomColorRampWidget::updatePreview()
{
  mRamp.updateColors();

  const QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( &mRamp, size ) );
}

void QgsLimitedRandomColorRampWidget::updateUi()
{
  spinCount->setValue( mRamp.count() );
  spinHue1->setValue( mRamp.hueMin() );
  spinHue2->setValue( mRamp.hueMax() );
  spinSat1->setValue( mRamp.satMin() );
  spinSat2->setValue( mRamp.satMax() );
  spinVal1->setValue( mRamp.valMin() );
  spinVal2->setValue( mRamp.valMax() );
  updatePreview();
}

void QgsLimitedRandomColorRampWidget::setCount( int val )
{
  mRamp.setCount( val );
  updatePreview();
  emit changed();
}

void QgsLimitedRandomColorRampWidget::setHue1( int val )
{
  mRamp.setHueMin( val );
  updatePreview();
  emit changed();
}

void QgsLimitedRandomColorRampWidget::setHue2( int val )
{
  mRamp.setHueMax( val );
  updatePreview();
  emit changed();
}

void QgsLimitedRandomColorRampWidget::setSat1( int val )
{
  mRamp.setSatMin( val );
  updatePreview();
  emit changed();
}

void QgsLimitedRandomColorRampWidget::setSat2( int val )
{
  mRamp.setSatMax( val );
  updatePreview();
  emit changed();
}

void QgsLimitedRandomColorRampWidget::setVal1( int val )
{
  mRamp.setValMin( val );
  updatePreview();
  emit changed();
}

void QgsLimitedRandomColorRampWidget::setVal2( int val )
{
  mRamp.setValMax( val );
  updatePreview();
  emit changed();
}

QgsLimitedRandomColorRampDialog::QgsLimitedRandomColorRampDialog( const QgsLimitedRandomColorRamp &ramp, QWidget *parent )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsLimitedRandomColorRampWidget( ramp );

  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );

  vLayout->addWidget( mWidget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsLimitedRandomColorRampDialog::showHelp );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );
  setWindowTitle( tr( "Random Color Ramp" ) );
  connect( mWidget, &QgsLimitedRandomColorRampWidget::changed, this, &QgsLimitedRandomColorRampDialog::changed );
}

QDialogButtonBox *QgsLimitedRandomColorRampDialog::buttonBox() const
{
  return mButtonBox;
}

void QgsLimitedRandomColorRampDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "style_library/style_manager.html#setting-a-color-ramp" ) );
}
