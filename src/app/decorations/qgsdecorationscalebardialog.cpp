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
#include "moc_qgsdecorationscalebardialog.cpp"
#include "qgsdecorationscalebar.h"
#include "qgslogger.h"
#include "qgshelp.h"
#include "qgsgui.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QPushButton>

QgsDecorationScaleBarDialog::QgsDecorationScaleBarDialog( QgsDecorationScaleBar &deco, Qgis::DistanceUnit units, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationScaleBarDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationScaleBarDialog::buttonBox_rejected );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationScaleBarDialog::showHelp );

  QPushButton *applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, &QAbstractButton::clicked, this, &QgsDecorationScaleBarDialog::apply );

  // set the map units in the spin box
  spnSize->setShowClearButton( false );
  switch ( units )
  {
    case Qgis::DistanceUnit::Meters:
      spnSize->setSuffix( tr( " meters/km" ) );
      break;
    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::Miles:
      spnSize->setSuffix( tr( " feet/miles" ) );
      break;
    case Qgis::DistanceUnit::Degrees:
      spnSize->setSuffix( tr( " degrees" ) );
      break;
    default:
      QgsDebugError( QStringLiteral( "Error: not picked up map units - actual value = %1" ).arg( qgsEnumValueToKey( units ) ) );
  }
  spnSize->setValue( mDeco.mPreferredSize );

  chkSnapping->setChecked( mDeco.mSnapping );

  // placement
  cboPlacement->addItem( tr( "Top Left" ), QgsDecorationItem::TopLeft );
  cboPlacement->addItem( tr( "Top Center" ), QgsDecorationItem::TopCenter );
  cboPlacement->addItem( tr( "Top Right" ), QgsDecorationItem::TopRight );
  cboPlacement->addItem( tr( "Bottom Left" ), QgsDecorationItem::BottomLeft );
  cboPlacement->addItem( tr( "Bottom Center" ), QgsDecorationItem::BottomCenter );
  cboPlacement->addItem( tr( "Bottom Right" ), QgsDecorationItem::BottomRight );
  connect( cboPlacement, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( int ) {
    spnHorizontal->setMinimum( cboPlacement->currentData() == QgsDecorationItem::TopCenter || cboPlacement->currentData() == QgsDecorationItem::BottomCenter ? -100 : 0 );
  } );
  cboPlacement->setCurrentIndex( cboPlacement->findData( mDeco.placement() ) );

  spnHorizontal->setClearValue( 0 );
  spnHorizontal->setValue( mDeco.mMarginHorizontal );
  spnVertical->setValue( mDeco.mMarginVertical );
  wgtUnitSelection->setUnits(
    { Qgis::RenderUnit::Millimeters,
      Qgis::RenderUnit::Percentage,
      Qgis::RenderUnit::Pixels
    }
  );
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

  mButtonFontStyle->setMode( QgsFontButton::ModeTextRenderer );
  mButtonFontStyle->setTextFormat( mDeco.mTextFormat );
}

void QgsDecorationScaleBarDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "map_views/map_view.html#scalebar-decoration" ) );
}

void QgsDecorationScaleBarDialog::apply()
{
  mDeco.setPlacement( static_cast<QgsDecorationItem::Placement>( cboPlacement->currentData().toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.mMarginHorizontal = spnHorizontal->value();
  mDeco.mMarginVertical = spnVertical->value();
  mDeco.mPreferredSize = spnSize->value();
  mDeco.mSnapping = chkSnapping->isChecked();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.mStyleIndex = cboStyle->currentIndex();
  mDeco.mColor = pbnChangeColor->color();
  mDeco.mOutlineColor = pbnChangeOutlineColor->color();
  mDeco.mTextFormat = mButtonFontStyle->textFormat();
  mDeco.setupScaleBar();
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
