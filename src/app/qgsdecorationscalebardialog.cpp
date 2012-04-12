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

QgsDecorationScaleBarDialog::QgsDecorationScaleBarDialog( QgsDecorationScaleBar& deco, int units, QWidget* parent )
    : QDialog( parent ), mDeco( deco )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/DecorationScaleBar/geometry" ).toByteArray() );

  // set the map units in the spin box
  switch ( units )
  {
    case 0:
      spnSize->setSuffix( tr( " metres/km" ) );
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

  cboPlacement->clear();
  cboPlacement->addItems( mDeco.mPlacementLabels );
  cboPlacement->setCurrentIndex( mDeco.mPlacementIndex );

  chkEnable->setChecked( mDeco.mEnabled );

  cboStyle->clear();
  cboStyle->addItems( mDeco.mStyleLabels );

  cboStyle->setCurrentIndex( mDeco.mStyleIndex );

  pbnChangeColor->setColor( mDeco.mColor );
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

void QgsDecorationScaleBarDialog::on_buttonBox_accepted()
{
  mDeco.mPlacementIndex = cboPlacement->currentIndex();
  mDeco.mPreferredSize = spnSize->value();
  mDeco.mSnapping = chkSnapping->isChecked();
  mDeco.mEnabled = chkEnable->isChecked();
  mDeco.mStyleIndex = cboStyle->currentIndex();
  mDeco.mColor = pbnChangeColor->color();

  accept();
}

void QgsDecorationScaleBarDialog::on_pbnChangeColor_clicked()
{
  QColor color = QColorDialog::getColor( pbnChangeColor->color(), this );

  if ( color.isValid() )
    pbnChangeColor->setColor( color );
}

void QgsDecorationScaleBarDialog::on_buttonBox_rejected()
{
  reject();
}
