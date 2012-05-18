/***************************************************************************
    qgslabelengineconfigdialog.cpp
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslabelengineconfigdialog.h"

#include "qgspallabeling.h"

QgsLabelEngineConfigDialog::QgsLabelEngineConfigDialog( QgsPalLabeling* lbl, QWidget* parent )
    : QDialog( parent ), mLBL( lbl )
{
  setupUi( this );

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOK() ) );

  // search method
  cboSearchMethod->setCurrentIndex( mLBL->searchMethod() );

  // candidate numbers
  int candPoint, candLine, candPolygon;
  mLBL->numCandidatePositions( candPoint, candLine, candPolygon );
  spinCandPoint->setValue( candPoint );
  spinCandLine->setValue( candLine );
  spinCandPolygon->setValue( candPolygon );

  chkShowCandidates->setChecked( mLBL->isShowingCandidates() );

  chkShowAllLabels->setChecked( mLBL->isShowingAllLabels() );
}


void QgsLabelEngineConfigDialog::onOK()
{
  // save
  mLBL->setSearchMethod(( QgsPalLabeling::Search ) cboSearchMethod->currentIndex() );

  mLBL->setNumCandidatePositions( spinCandPoint->value(),
                                  spinCandLine->value(),
                                  spinCandPolygon->value() );

  mLBL->setShowingCandidates( chkShowCandidates->isChecked() );

  mLBL->setShowingAllLabels( chkShowAllLabels->isChecked() );

  accept();
}
