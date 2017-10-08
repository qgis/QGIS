/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "qgsdecorationscalebardialog.h"
#include "qgsdecorationscalebar.h"
#include "qgslogger.h"
#include "qgshelp.h"
#include "qgssettings.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QPushButton>

QgsDecorationScaleBarDialog::QgsDecorationScaleBarDialog( QgsDecorationScaleBar &deco, int units, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationScaleBarDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationScaleBarDialog::buttonBox_rejected );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationScaleBarDialog::showHelp );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/DecorationScaleBar/geometry" ) ).toByteArray() );

  QPushButton *applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, &QAbstractButton::clicked, this, &QgsDecorationScaleBarDialog::apply );

  // set the map units in the spin box
  spnSize->setShowClearButton( false );
  switch ( units )
  {
    case 0:
      spnSize->setSuffix( tr( " meters/km" ) );
      break;
    case 1:
      spnSize->setSuffix( tr( " feet/miles" ) );
      break;
    case 2:
      spnSize->setSuffix( tr( " degrees" ) );
      break;
    default:
      QgsDebugMsg( QString( "Error: not picked up map units - actual value = %1" ).arg( units ) );
  }
  spnSize->setValue( mDeco.mPreferredSize );

  chkSnapping->setChecked( mDeco.mSnapping );

  // placement
  cboPlacement->addItem( tr( "Top left" ), QgsDecorationItem::TopLeft );
  cboPlacement->addItem( tr( "Top right" ), QgsDecorationItem::TopRight );
  cboPlacement->addItem( tr( "Bottom left" ), QgsDecorationItem::BottomLeft );
  cboPlacement->addItem( tr( "Bottom right" ), QgsDecorationItem::BottomRight );
  cboPlacement->setCurrentIndex( cboPlacement->findData( mDeco.placement() ) );
  spnHorizontal->setValue( mDeco.mMarginHorizontal );
  spnVertical->setValue( mDeco.mMarginVertical );
  wgtUnitSelection->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPercentage << QgsUnitTypes::RenderPixels );
  wgtUnitSelection->setUnit( mDeco.mMarginUnit );

  grpEnable->setChecked( mDeco.enabled() );

  // style
  cboStyle->clear();
  cboStyle->addItems( mDeco.mStyleLabels );

  cboStyle->setCurrentIndex( mDeco.mStyleIndex );

  pbnChangeColor->setAllowOpacity( true );
  pbnChangeColor->setColor( mDeco.mColor );
  pbnChangeColor->setContext( QStringLiteral( "gui" ) );
  pbnChangeColor->setColorDialogTitle( tr( "Select Scale Bar Fill Color" ) );

  pbnChangeOutlineColor->setAllowOpacity( true );
  pbnChangeOutlineColor->setColor( mDeco.mOutlineColor );
  pbnChangeOutlineColor->setContext( QStringLiteral( "gui" ) );
  pbnChangeOutlineColor->setColorDialogTitle( tr( "Select Scale Bar Outline Color" ) );
}

QgsDecorationScaleBarDialog::~QgsDecorationScaleBarDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/DecorationScaleBar/geometry" ), saveGeometry() );
}

void QgsDecorationScaleBarDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#scale-bar" ) );
}

void QgsDecorationScaleBarDialog::apply()
{
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->currentData().toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.mMarginHorizontal = spnHorizontal->value();
  mDeco.mMarginVertical = spnVertical->value();
  mDeco.mPreferredSize = spnSize->value();
  mDeco.mSnapping = chkSnapping->isChecked();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.mStyleIndex = cboStyle->currentIndex();
  mDeco.mColor = pbnChangeColor->color();
  mDeco.mOutlineColor = pbnChangeOutlineColor->color();
  mDeco.update();
}

void QgsDecorationScaleBarDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationScaleBarDialog::buttonBox_rejected()
{
  reject();
}
