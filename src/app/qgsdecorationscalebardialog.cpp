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
#include "qgscontexthelp.h"

#include <QColorDialog>
#include <QSettings>
#include <QDialogButtonBox>
#include <QPushButton>

QgsDecorationScaleBarDialog::QgsDecorationScaleBarDialog( QgsDecorationScaleBar& deco, int units, QWidget* parent )
    : QDialog( parent )
    , mDeco( deco )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/DecorationScaleBar/geometry" ).toByteArray() );

  QPushButton* applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, SIGNAL( clicked() ), this, SLOT( apply() ) );

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
  wgtUnitSelection->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::Percentage << QgsSymbolV2::Pixel );
  wgtUnitSelection->setUnit( mDeco.mMarginUnit );

  grpEnable->setChecked( mDeco.enabled() );

  // style
  cboStyle->clear();
  cboStyle->addItems( mDeco.mStyleLabels );

  cboStyle->setCurrentIndex( mDeco.mStyleIndex );

  pbnChangeColor->setColor( mDeco.mColor );
  pbnChangeColor->setContext( "gui" );
  pbnChangeColor->setColorDialogTitle( tr( "Select scalebar color" ) );
}

QgsDecorationScaleBarDialog::~QgsDecorationScaleBarDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/DecorationScaleBar/geometry", saveGeometry() );
}

void QgsDecorationScaleBarDialog::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}

void QgsDecorationScaleBarDialog::apply()
{
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->itemData( cboPlacement->currentIndex() ).toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.mMarginHorizontal = spnHorizontal->value();
  mDeco.mMarginVertical = spnVertical->value();
  mDeco.mPreferredSize = spnSize->value();
  mDeco.mSnapping = chkSnapping->isChecked();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.mStyleIndex = cboStyle->currentIndex();
  mDeco.mColor = pbnChangeColor->color();
  mDeco.update();
}

void QgsDecorationScaleBarDialog::on_buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationScaleBarDialog::on_buttonBox_rejected()
{
  reject();
}
