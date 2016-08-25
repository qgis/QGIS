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

#include <QColorDialog>
#include <QDialogButtonBox>


QgsLimitedRandomColorRampWidget::QgsLimitedRandomColorRampWidget( const QgsLimitedRandomColorRamp& ramp, QWidget* parent )
    : QgsPanelWidget( parent )
    , mRamp( ramp )
{
  setupUi( this );

  updateUi();

  connect( spinCount, SIGNAL( valueChanged( int ) ), this, SLOT( setCount( int ) ) );
  connect( spinHue1, SIGNAL( valueChanged( int ) ), this, SLOT( setHue1( int ) ) );
  connect( spinHue2, SIGNAL( valueChanged( int ) ), this, SLOT( setHue2( int ) ) );
  connect( spinSat1, SIGNAL( valueChanged( int ) ), this, SLOT( setSat1( int ) ) );
  connect( spinSat2, SIGNAL( valueChanged( int ) ), this, SLOT( setSat2( int ) ) );
  connect( spinVal1, SIGNAL( valueChanged( int ) ), this, SLOT( setVal1( int ) ) );
  connect( spinVal2, SIGNAL( valueChanged( int ) ), this, SLOT( setVal2( int ) ) );
}

void QgsLimitedRandomColorRampWidget::setRamp( const QgsLimitedRandomColorRamp& ramp )
{
  mRamp = ramp;
  updateUi();
  emit changed();
}

void QgsLimitedRandomColorRampWidget::updatePreview()
{
  mRamp.updateColors();

  QSize size( 300, 40 );
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

QgsLimitedRandomColorRampDialog::QgsLimitedRandomColorRampDialog( const QgsLimitedRandomColorRamp& ramp, QWidget* parent )
    : QDialog( parent )
{
  QVBoxLayout* vLayout = new QVBoxLayout();
  mWidget = new QgsLimitedRandomColorRampWidget( ramp );
  vLayout->addWidget( mWidget );
  QDialogButtonBox* bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( bbox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  connect( mWidget, SIGNAL( changed() ), this, SIGNAL( changed() ) );
}
