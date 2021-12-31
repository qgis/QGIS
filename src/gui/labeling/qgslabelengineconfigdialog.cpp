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
#include "qgsmessagebar.h"
#include "qgsmapcanvas.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsrendercontext.h"
#include <QAction>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QMenu>

QgsLabelEngineConfigWidget::QgsLabelEngineConfigWidget( QgsMapCanvas *canvas, QWidget *parent )
  : QgsPanelWidget( parent ), mCanvas( canvas )
{
  setupUi( this );

  setPanelTitle( tr( "Placement Engine Settings" ) );

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  verticalLayout->insertWidget( 0,  mMessageBar );

  const QgsLabelingEngineSettings engineSettings = QgsProject::instance()->labelingEngineSettings();

  mTextRenderFormatComboBox->addItem( tr( "Always Render Labels as Paths (Recommended)" ), static_cast< int >( Qgis::TextRenderFormat::AlwaysOutlines ) );
  mTextRenderFormatComboBox->addItem( tr( "Always Render Labels as Text" ), static_cast< int >( Qgis::TextRenderFormat::AlwaysText ) );

  mPlacementVersionComboBox->addItem( tr( "Version 1" ), QgsLabelingEngineSettings::PlacementEngineVersion1 );
  mPlacementVersionComboBox->addItem( tr( "Version 2 (Recommended)" ), QgsLabelingEngineSettings::PlacementEngineVersion2 );

  mPreviousEngineVersion = engineSettings.placementVersion();
  mPlacementVersionComboBox->setCurrentIndex( mPlacementVersionComboBox->findData( mPreviousEngineVersion ) );
  connect( mPlacementVersionComboBox, &QComboBox::currentTextChanged, this, [ = ]()
  {
    if ( static_cast< QgsLabelingEngineSettings::PlacementEngineVersion >( mPlacementVersionComboBox->currentData().toInt() ) != mPreviousEngineVersion )
    {
      mMessageBar->pushMessage( QString(), tr( "Version changes will alter label placement in the project." ), Qgis::MessageLevel::Warning );
    }
  } );

  spinCandLine->setClearValue( 5 );
  spinCandPolygon->setClearValue( 2.5 );

  // candidate numbers
  spinCandLine->setValue( engineSettings.maximumLineCandidatesPerCm() );
  spinCandPolygon->setValue( engineSettings.maximumPolygonCandidatesPerCmSquared() );

  chkShowCandidates->setChecked( engineSettings.testFlag( QgsLabelingEngineSettings::DrawCandidates ) );
  chkShowAllLabels->setChecked( engineSettings.testFlag( QgsLabelingEngineSettings::UseAllLabels ) );
  chkShowUnplaced->setChecked( engineSettings.testFlag( QgsLabelingEngineSettings::DrawUnplacedLabels ) );
  chkShowPartialsLabels->setChecked( engineSettings.testFlag( QgsLabelingEngineSettings::UsePartialCandidates ) );

  mUnplacedColorButton->setColor( engineSettings.unplacedLabelColor() );
  mUnplacedColorButton->setAllowOpacity( false );
  mUnplacedColorButton->setDefaultColor( QColor( 255, 0, 0 ) );
  mUnplacedColorButton->setWindowTitle( tr( "Unplaced Label Color" ) );

  mTextRenderFormatComboBox->setCurrentIndex( mTextRenderFormatComboBox->findData( static_cast< int >( engineSettings.defaultTextRenderFormat() ) ) );

  connect( spinCandLine, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsLabelEngineConfigWidget::widgetChanged );
  connect( spinCandPolygon, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsLabelEngineConfigWidget::widgetChanged );
  connect( chkShowCandidates, &QCheckBox::toggled, this, &QgsLabelEngineConfigWidget::widgetChanged );
  connect( chkShowAllLabels, &QCheckBox::toggled, this, &QgsLabelEngineConfigWidget::widgetChanged );
  connect( chkShowUnplaced, &QCheckBox::toggled, this, &QgsLabelEngineConfigWidget::widgetChanged );
  connect( chkShowPartialsLabels, &QCheckBox::toggled, this, &QgsLabelEngineConfigWidget::widgetChanged );
  connect( mTextRenderFormatComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsLabelEngineConfigWidget::widgetChanged );
  connect( mUnplacedColorButton, &QgsColorButton::colorChanged, this, &QgsLabelEngineConfigWidget::widgetChanged );
  connect( mPlacementVersionComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsLabelEngineConfigWidget::widgetChanged );

  mWidgetMenu = new QMenu( this );
  QAction *resetAction = new QAction( tr( "Restore Defaults" ), this );
  mWidgetMenu->addAction( resetAction );
  connect( resetAction, &QAction::triggered, this, &QgsLabelEngineConfigWidget::setDefaults );
  QAction *helpAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHelpContents.svg" ) ), tr( "Help…" ), this );
  mWidgetMenu->addAction( helpAction );
  connect( helpAction, &QAction::triggered, this, &QgsLabelEngineConfigWidget::showHelp );
}

QMenu *QgsLabelEngineConfigWidget::menuButtonMenu()
{
  return mWidgetMenu;
}

QString QgsLabelEngineConfigWidget::menuButtonTooltip() const
{
  return tr( "Additional Options" );
}

void QgsLabelEngineConfigWidget::apply()
{
  QgsLabelingEngineSettings engineSettings;

  // save
  engineSettings.setMaximumLineCandidatesPerCm( spinCandLine->value() );
  engineSettings.setMaximumPolygonCandidatesPerCmSquared( spinCandPolygon->value() );

  engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, chkShowCandidates->isChecked() );
  engineSettings.setFlag( QgsLabelingEngineSettings::UseAllLabels, chkShowAllLabels->isChecked() );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, chkShowUnplaced->isChecked() );
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, chkShowPartialsLabels->isChecked() );

  engineSettings.setDefaultTextRenderFormat( static_cast< Qgis::TextRenderFormat >( mTextRenderFormatComboBox->currentData().toInt() ) );

  engineSettings.setUnplacedLabelColor( mUnplacedColorButton->color() );

  engineSettings.setPlacementVersion( static_cast< QgsLabelingEngineSettings::PlacementEngineVersion >( mPlacementVersionComboBox->currentData().toInt() ) );

  QgsProject::instance()->setLabelingEngineSettings( engineSettings );
  mCanvas->refreshAllLayers();
}

void QgsLabelEngineConfigWidget::setDefaults()
{
  const pal::Pal p;
  spinCandLine->setValue( 5 );
  spinCandPolygon->setValue( 10 );
  chkShowCandidates->setChecked( false );
  chkShowAllLabels->setChecked( false );
  chkShowPartialsLabels->setChecked( p.showPartialLabels() );
  mTextRenderFormatComboBox->setCurrentIndex( mTextRenderFormatComboBox->findData( static_cast< int >( Qgis::TextRenderFormat::AlwaysOutlines ) ) );
  mPlacementVersionComboBox->setCurrentIndex( mPlacementVersionComboBox->findData( QgsLabelingEngineSettings::PlacementEngineVersion2 ) );
}

void QgsLabelEngineConfigWidget::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#setting-the-automated-placement-engine" ) );
}

//
// QgsLabelEngineConfigDialog
//

QgsLabelEngineConfigDialog::QgsLabelEngineConfigDialog( QgsMapCanvas *canvas, QWidget *parent )
  : QDialog( parent )
{
  mWidget = new QgsLabelEngineConfigWidget( canvas );
  setWindowTitle( mWidget->windowTitle() );
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::RestoreDefaults, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( bbox, &QDialogButtonBox::helpRequested, mWidget, &QgsLabelEngineConfigWidget::showHelp );
  connect( bbox->button( QDialogButtonBox::RestoreDefaults ), &QAbstractButton::clicked,
           mWidget, &QgsLabelEngineConfigWidget::setDefaults );
  vLayout->addWidget( bbox );
  setLayout( vLayout );

  setObjectName( QStringLiteral( "QgsLabelSettingsWidgetDialog" ) );
  QgsGui::enableAutoGeometryRestore( this );
}

void QgsLabelEngineConfigDialog::accept()
{
  mWidget->apply();
  QDialog::accept();
}
