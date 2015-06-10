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
#include <pal/pal.h>

#include <QPushButton>

QgsLabelEngineConfigDialog::QgsLabelEngineConfigDialog( QWidget* parent )
    : QDialog( parent )
{
  setupUi( this );

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( onOK() ) );
  connect( buttonBox->button( QDialogButtonBox::RestoreDefaults ), SIGNAL( clicked() ),
           this, SLOT( setDefaults() ) );

  QgsPalLabeling lbl;
  lbl.loadEngineSettings();

  // search method
  cboSearchMethod->setCurrentIndex( lbl.searchMethod() );

  // candidate numbers
  int candPoint, candLine, candPolygon;
  lbl.numCandidatePositions( candPoint, candLine, candPolygon );
  spinCandPoint->setValue( candPoint );
  spinCandLine->setValue( candLine );
  spinCandPolygon->setValue( candPolygon );

  chkShowCandidates->setChecked( lbl.isShowingCandidates() );
  chkShowAllLabels->setChecked( lbl.isShowingAllLabels() );
  mShadowDebugRectChkBox->setChecked( lbl.isShowingShadowRectangles() );

  chkShowPartialsLabels->setChecked( lbl.isShowingPartialsLabels() );
  mDrawOutlinesChkBox->setChecked( lbl.isDrawingOutlineLabels() );
}


void QgsLabelEngineConfigDialog::onOK()
{
  QgsPalLabeling lbl;

  // save
  lbl.setSearchMethod(( QgsPalLabeling::Search ) cboSearchMethod->currentIndex() );

  lbl.setNumCandidatePositions( spinCandPoint->value(),
                                spinCandLine->value(),
                                spinCandPolygon->value() );

  lbl.setShowingCandidates( chkShowCandidates->isChecked() );
  lbl.setShowingShadowRectangles( mShadowDebugRectChkBox->isChecked() );
  lbl.setShowingAllLabels( chkShowAllLabels->isChecked() );
  lbl.setShowingPartialsLabels( chkShowPartialsLabels->isChecked() );
  lbl.setDrawingOutlineLabels( mDrawOutlinesChkBox->isChecked() );

  lbl.saveEngineSettings();

  accept();
}

void QgsLabelEngineConfigDialog::setDefaults()
{
  pal::Pal p;
  cboSearchMethod->setCurrentIndex(( int )p.getSearch() );
  spinCandPoint->setValue( p.getPointP() );
  spinCandLine->setValue( p.getLineP() );
  spinCandPolygon->setValue( p.getPolyP() );
  chkShowCandidates->setChecked( false );
  chkShowAllLabels->setChecked( false );
  mShadowDebugRectChkBox->setChecked( false );
  chkShowPartialsLabels->setChecked( p.getShowPartial() );
  mDrawOutlinesChkBox->setChecked( true );
}
