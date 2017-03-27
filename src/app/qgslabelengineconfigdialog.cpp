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
#include "qgslabelingengine.h"
#include "qgsproject.h"
#include <pal/pal.h>

#include <QPushButton>

QgsLabelEngineConfigDialog::QgsLabelEngineConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsLabelEngineConfigDialog::onOK );
  connect( buttonBox->button( QDialogButtonBox::RestoreDefaults ), &QAbstractButton::clicked,
           this, &QgsLabelEngineConfigDialog::setDefaults );

  QgsLabelingEngine engine;
  engine.readSettingsFromProject( QgsProject::instance() );

  // search method
  cboSearchMethod->setCurrentIndex( engine.searchMethod() );

  // candidate numbers
  int candPoint, candLine, candPolygon;
  engine.numCandidatePositions( candPoint, candLine, candPolygon );
  spinCandPoint->setValue( candPoint );
  spinCandLine->setValue( candLine );
  spinCandPolygon->setValue( candPolygon );

  chkShowCandidates->setChecked( engine.testFlag( QgsLabelingEngine::DrawCandidates ) );
  chkShowAllLabels->setChecked( engine.testFlag( QgsLabelingEngine::UseAllLabels ) );

  chkShowPartialsLabels->setChecked( engine.testFlag( QgsLabelingEngine::UsePartialCandidates ) );
  mDrawOutlinesChkBox->setChecked( engine.testFlag( QgsLabelingEngine::RenderOutlineLabels ) );
}


void QgsLabelEngineConfigDialog::onOK()
{
  QgsLabelingEngine engine;

  // save
  engine.setSearchMethod( ( QgsPalLabeling::Search ) cboSearchMethod->currentIndex() );

  engine.setNumCandidatePositions( spinCandPoint->value(),
                                   spinCandLine->value(),
                                   spinCandPolygon->value() );

  engine.setFlag( QgsLabelingEngine::DrawCandidates, chkShowCandidates->isChecked() );
  engine.setFlag( QgsLabelingEngine::UseAllLabels, chkShowAllLabels->isChecked() );
  engine.setFlag( QgsLabelingEngine::UsePartialCandidates, chkShowPartialsLabels->isChecked() );
  engine.setFlag( QgsLabelingEngine::RenderOutlineLabels, mDrawOutlinesChkBox->isChecked() );

  engine.writeSettingsToProject( QgsProject::instance() );

  accept();
}

void QgsLabelEngineConfigDialog::setDefaults()
{
  pal::Pal p;
  cboSearchMethod->setCurrentIndex( ( int )p.getSearch() );
  spinCandPoint->setValue( p.getPointP() );
  spinCandLine->setValue( p.getLineP() );
  spinCandPolygon->setValue( p.getPolyP() );
  chkShowCandidates->setChecked( false );
  chkShowAllLabels->setChecked( false );
  chkShowPartialsLabels->setChecked( p.getShowPartial() );
  mDrawOutlinesChkBox->setChecked( true );
}
