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


QgsLimitedRandomColorRampDialog::QgsLimitedRandomColorRampDialog( QgsLimitedRandomColorRamp* ramp, QWidget* parent )
    : QDialog( parent )
    , mRamp( ramp )
{
  setupUi( this );

  spinCount->setValue( ramp->count() );
  spinHue1->setValue( ramp->hueMin() );
  spinHue2->setValue( ramp->hueMax() );
  spinSat1->setValue( ramp->satMin() );
  spinSat2->setValue( ramp->satMax() );
  spinVal1->setValue( ramp->valMin() );
  spinVal2->setValue( ramp->valMax() );

  connect( spinCount, SIGNAL( valueChanged( int ) ), this, SLOT( setCount( int ) ) );
  connect( spinHue1, SIGNAL( valueChanged( int ) ), this, SLOT( setHue1( int ) ) );
  connect( spinHue2, SIGNAL( valueChanged( int ) ), this, SLOT( setHue2( int ) ) );
  connect( spinSat1, SIGNAL( valueChanged( int ) ), this, SLOT( setSat1( int ) ) );
  connect( spinSat2, SIGNAL( valueChanged( int ) ), this, SLOT( setSat2( int ) ) );
  connect( spinVal1, SIGNAL( valueChanged( int ) ), this, SLOT( setVal1( int ) ) );
  connect( spinVal2, SIGNAL( valueChanged( int ) ), this, SLOT( setVal2( int ) ) );

  updatePreview();
}

void QgsLimitedRandomColorRampDialog::updatePreview()
{
  mRamp->updateColors();

  QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( mRamp, size ) );
}

void QgsLimitedRandomColorRampDialog::setCount( int val )
{
  mRamp->setCount( val );
  updatePreview();
}

void QgsLimitedRandomColorRampDialog::setHue1( int val )
{
  mRamp->setHueMin( val );
  updatePreview();
}

void QgsLimitedRandomColorRampDialog::setHue2( int val )
{
  mRamp->setHueMax( val );
  updatePreview();
}

void QgsLimitedRandomColorRampDialog::setSat1( int val )
{
  mRamp->setSatMin( val );
  updatePreview();
}

void QgsLimitedRandomColorRampDialog::setSat2( int val )
{
  mRamp->setSatMax( val );
  updatePreview();
}

void QgsLimitedRandomColorRampDialog::setVal1( int val )
{
  mRamp->setValMin( val );
  updatePreview();
}

void QgsLimitedRandomColorRampDialog::setVal2( int val )
{
  mRamp->setValMax( val );
  updatePreview();
}
