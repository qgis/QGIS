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

#include "qgslabelingenginesettings.h"
#include "qgsproject.h"
#include "pal/pal.h"
#include "qgshelp.h"

#include <QPushButton>

QgsLabelEngineConfigDialog::QgsLabelEngineConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsLabelEngineConfigDialog::onOK );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsLabelEngineConfigDialog::showHelp );
  connect( buttonBox->button( QDialogButtonBox::RestoreDefaults ), &QAbstractButton::clicked,
           this, &QgsLabelEngineConfigDialog::setDefaults );

  QgsLabelingEngineSettings engineSettings = QgsProject::instance()->labelingEngineSettings();

  mTextRenderFormatComboBox->addItem( tr( "Always Render Labels as Paths (Recommended)" ), QgsRenderContext::TextFormatAlwaysOutlines );
  mTextRenderFormatComboBox->addItem( tr( "Always Render Labels as Text" ), QgsRenderContext::TextFormatAlwaysText );

  // candidate numbers
  int candPoint, candLine, candPolygon;
  engineSettings.numCandidatePositions( candPoint, candLine, candPolygon );
  spinCandPoint->setValue( candPoint );
  spinCandLine->setValue( candLine );
  spinCandPolygon->setValue( candPolygon );

  chkShowCandidates->setChecked( engineSettings.testFlag( QgsLabelingEngineSettings::DrawCandidates ) );
  chkShowAllLabels->setChecked( engineSettings.testFlag( QgsLabelingEngineSettings::UseAllLabels ) );
  chkShowUnplaced->setChecked( engineSettings.testFlag( QgsLabelingEngineSettings::DrawUnplacedLabels ) );
  chkShowPartialsLabels->setChecked( engineSettings.testFlag( QgsLabelingEngineSettings::UsePartialCandidates ) );

  mUnplacedColorButton->setColor( engineSettings.unplacedLabelColor() );
  mUnplacedColorButton->setAllowOpacity( false );
  mUnplacedColorButton->setDefaultColor( QColor( 255, 0, 0 ) );
  mUnplacedColorButton->setWindowTitle( tr( "Unplaced Label Color" ) );

  mTextRenderFormatComboBox->setCurrentIndex( mTextRenderFormatComboBox->findData( engineSettings.defaultTextRenderFormat() ) );
}

void QgsLabelEngineConfigDialog::onOK()
{
  QgsLabelingEngineSettings engineSettings;

  // save
  engineSettings.setNumCandidatePositions( spinCandPoint->value(), spinCandLine->value(), spinCandPolygon->value() );

  engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, chkShowCandidates->isChecked() );
  engineSettings.setFlag( QgsLabelingEngineSettings::UseAllLabels, chkShowAllLabels->isChecked() );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, chkShowUnplaced->isChecked() );
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, chkShowPartialsLabels->isChecked() );

  engineSettings.setDefaultTextRenderFormat( static_cast< QgsRenderContext::TextRenderFormat >( mTextRenderFormatComboBox->currentData().toInt() ) );

  engineSettings.setUnplacedLabelColor( mUnplacedColorButton->color() );

  QgsProject::instance()->setLabelingEngineSettings( engineSettings );

  accept();
}

void QgsLabelEngineConfigDialog::setDefaults()
{
  pal::Pal p;
  spinCandPoint->setValue( p.getPointP() );
  spinCandLine->setValue( p.getLineP() );
  spinCandPolygon->setValue( p.getPolyP() );
  chkShowCandidates->setChecked( false );
  chkShowAllLabels->setChecked( false );
  chkShowPartialsLabels->setChecked( p.getShowPartial() );
  mTextRenderFormatComboBox->setCurrentIndex( mTextRenderFormatComboBox->findData( QgsRenderContext::TextFormatAlwaysOutlines ) );
}

void QgsLabelEngineConfigDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#setting-the-automated-placement-engine" ) );
}
