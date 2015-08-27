/***************************************************************************
                              qgsgrassoptions.cpp
                             -------------------
    begin                : May, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFileDialog>

#include "qgsrasterprojector.h"

#include "qgsgrass.h"
#include "qgsgrassoptions.h"
#include "ui_qgsgrassoptionsbase.h"

QgsGrassOptions::QgsGrassOptions( QWidget *parent )
    : QgsOptionsDialogBase( "GrassOptions", parent )
    , QgsGrassOptionsBase()
    , mImportSettingsPath( "/GRASS/browser/import" )
    , mModulesSettingsPath( "/GRASS/modules/config" )
{
  setupUi( this );
  initOptionsBase( false );

  connect( this, SIGNAL( accepted() ), SLOT( saveOptions() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), SLOT( saveOptions() ) );

  QSettings settings;

  // Modules
  bool customModules = settings.value( mModulesSettingsPath + "/custom", false ).toBool();
  QString customModulesDir = settings.value( mModulesSettingsPath + "/customDir" ).toString();
  mModulesConfigDefaultRadioButton->setText( tr( "Default" ) + " (" + QgsGrass::modulesConfigDefaultDirPath() + ")" );
  mModulesConfigDefaultRadioButton->setChecked( !customModules );
  mModulesConfigCustomRadioButton->setChecked( customModules );
  mModulesConfigDirLineEdit->setText( customModulesDir );
  mModulesDebugCheckBox->setChecked( QgsGrass::modulesDebug() );

  // Browser
  QgsRasterProjector::Precision crsTransform = ( QgsRasterProjector::Precision ) settings.value( mImportSettingsPath + "/crsTransform", QgsRasterProjector::Approximate ).toInt();
  mCrsTransformationComboBox->addItem( QgsRasterProjector::precisionLabel( QgsRasterProjector::Approximate ), QgsRasterProjector::Approximate );
  mCrsTransformationComboBox->addItem( QgsRasterProjector::precisionLabel( QgsRasterProjector::Exact ), QgsRasterProjector::Exact );
  mCrsTransformationComboBox->setCurrentIndex( mCrsTransformationComboBox->findData( crsTransform ) );

  mImportExternalCheckBox->setChecked( settings.value( mImportSettingsPath + "/external", true ).toBool() );

  // Region
  QPen regionPen = QgsGrass::regionPen();
  mRegionColorButton->setContext( "gui" );
  mRegionColorButton->setColorDialogTitle( tr( "Select color" ) );
  mRegionColorButton->setColor( regionPen.color() );
  mRegionWidthSpinBox->setValue( regionPen.width() );

  restoreOptionsBaseUi();
}

QgsGrassOptions::~QgsGrassOptions()
{
}

void QgsGrassOptions::on_mModulesConfigBrowseButton_clicked()
{
  QString dir = QFileDialog::getExistingDirectory( this,
                tr( "Choose a directory with configuration files (default.qgc, *.qgm)" ),
                mModulesConfigDirLineEdit->text() );

  if ( !dir.isEmpty() )
  {
    mModulesConfigDirLineEdit->setText( dir );
  }
}

void QgsGrassOptions::saveOptions()
{
  QSettings settings;

  // Modules
  bool customModules = mModulesConfigCustomRadioButton->isChecked();
  QString customModulesDir = mModulesConfigDirLineEdit->text();
  QgsGrass::instance()->setModulesConfig( customModules, customModulesDir );
  QgsGrass::instance()->setModulesDebug( mModulesDebugCheckBox->isChecked() );

  // Browser
  settings.setValue( mImportSettingsPath + "/crsTransform",
                     mCrsTransformationComboBox->itemData( mCrsTransformationComboBox->currentIndex() ).toInt() );

  settings.setValue( mImportSettingsPath + "/external", mImportExternalCheckBox->isChecked() );

  // Region
  QPen regionPen = QgsGrass::regionPen();
  regionPen.setColor( mRegionColorButton->color() );
  regionPen.setWidthF( mRegionWidthSpinBox->value() );
  QgsGrass::instance()->setRegionPen( regionPen );
}
