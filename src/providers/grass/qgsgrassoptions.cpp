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

extern "C"
{
#include <grass/gis.h>
#include <grass/version.h>
}

QgsGrassOptions::QgsGrassOptions( QWidget *parent )
  : QgsOptionsDialogBase( QStringLiteral( "GrassOptions" ), parent )
  , QgsGrassOptionsBase()
  , mImportSettingsPath( QStringLiteral( "/GRASS/browser/import" ) )
  , mModulesSettingsPath( QStringLiteral( "/GRASS/modules/config" ) )
{
  setupUi( this );
  connect( mGisbaseBrowseButton, &QPushButton::clicked, this, &QgsGrassOptions::mGisbaseBrowseButton_clicked );
  connect( mModulesConfigBrowseButton, &QPushButton::clicked, this, &QgsGrassOptions::mModulesConfigBrowseButton_clicked );
  initOptionsBase( false );

  connect( this, &QDialog::accepted, this, &QgsGrassOptions::saveOptions );
  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsGrassOptions::saveOptions );

  QgsSettings settings;

  // General
  QString version = QStringLiteral( GRASS_VERSION_STRING ).remove( QStringLiteral( "@(#)" ) ).trimmed();
  QString revision = QStringLiteral( GIS_H_VERSION ).remove( QStringLiteral( "$" ) ).trimmed();
  mGrassVersionLabel->setText( tr( "GRASS version" ) + " : " + version + " " + revision );

  bool customGisbase = settings.value( QStringLiteral( "GRASS/gidbase/custom" ), false ).toBool();
  QString customGisbaseDir = settings.value( QStringLiteral( "GRASS/gidbase/customDir" ) ).toString();
  mGisbaseDefaultRadioButton->setText( tr( "Default" ) + " (" + QgsGrass::defaultGisbase() + ")" );
  mGisbaseDefaultRadioButton->setChecked( !customGisbase );
  mGisbaseCustomRadioButton->setChecked( customGisbase );
  mGisbaseLineEdit->setText( customGisbaseDir );
  gisbaseChanged();
  connect( mGisbaseDefaultRadioButton, &QAbstractButton::toggled, this, &QgsGrassOptions::gisbaseChanged );
  connect( mGisbaseLineEdit, &QLineEdit::textChanged, this, &QgsGrassOptions::gisbaseChanged );
  connect( mGisbaseLineEdit, &QLineEdit::textEdited, this, &QgsGrassOptions::gisbaseChanged );
  connect( mGisbaseGroupBox, &QgsCollapsibleGroupBoxBasic::collapsedStateChanged, this, &QgsGrassOptions::gisbaseChanged );

  // Modules
  bool customModules = settings.value( mModulesSettingsPath + "/custom", false ).toBool();
  QString customModulesDir = settings.value( mModulesSettingsPath + "/customDir" ).toString();
  mModulesConfigDefaultRadioButton->setText( tr( "Default" ) + " (" + QgsGrass::modulesConfigDefaultDirPath() + ")" );
  mModulesConfigDefaultRadioButton->setChecked( !customModules );
  mModulesConfigCustomRadioButton->setChecked( customModules );
  mModulesConfigDirLineEdit->setText( customModulesDir );
  mModulesDebugCheckBox->setChecked( QgsGrass::modulesDebug() );

  // Browser
  QgsRasterProjector::Precision crsTransform = settings.enumValue( mImportSettingsPath + "/crsTransform", QgsRasterProjector::Approximate );
  mCrsTransformationComboBox->addItem( QgsRasterProjector::precisionLabel( QgsRasterProjector::Approximate ), QgsRasterProjector::Approximate );
  mCrsTransformationComboBox->addItem( QgsRasterProjector::precisionLabel( QgsRasterProjector::Exact ), QgsRasterProjector::Exact );
  mCrsTransformationComboBox->setCurrentIndex( mCrsTransformationComboBox->findData( crsTransform ) );

  mImportExternalCheckBox->setChecked( settings.value( mImportSettingsPath + "/external", true ).toBool() );

  mTopoLayersCheckBox->setChecked( settings.value( QStringLiteral( "GRASS/showTopoLayers" ), false ).toBool() );

  // Region
  QPen regionPen = QgsGrass::regionPen();
  mRegionColorButton->setContext( QStringLiteral( "gui" ) );
  mRegionColorButton->setColorDialogTitle( tr( "Select Color" ) );
  mRegionColorButton->setColor( regionPen.color() );
  mRegionWidthSpinBox->setValue( regionPen.width() );

  restoreOptionsBaseUi();
}

void QgsGrassOptions::mGisbaseBrowseButton_clicked()
{
  QString gisbase = mGisbaseLineEdit->text();
  // For Mac, GISBASE folder may be inside GRASS bundle. Use Qt file dialog
  // since Mac native dialog doesn't allow user to browse inside bundles.
  gisbase = QFileDialog::getExistingDirectory(
              nullptr, QObject::tr( "Choose GRASS installation path (GISBASE)" ), gisbase,
              QFileDialog::DontUseNativeDialog );
  if ( !gisbase.isEmpty() )gisbaseChanged();
  {
    mGisbaseLineEdit->setText( gisbase );
  }
}

void QgsGrassOptions::gisbaseChanged()
{
  QString gisbase;
  if ( mGisbaseDefaultRadioButton->isChecked() )
  {
    gisbase = QgsGrass::defaultGisbase();
  }
  else
  {
    gisbase = mGisbaseLineEdit->text().trimmed();
  }
  QgsDebugMsg( "gisbase = " + gisbase );
  if ( !QgsGrass::isValidGrassBaseDir( gisbase ) )
  {
    mGisbaseErrorLabel->setText( tr( "Currently selected GRASS installation is not valid" ) );
    mGisbaseErrorLabel->show();
  }
  else
  {
    mGisbaseErrorLabel->hide();
  }
}

void QgsGrassOptions::mModulesConfigBrowseButton_clicked()
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
  QgsSettings settings;

  // General
  bool customGisbase = mGisbaseCustomRadioButton->isChecked();
  QString customGisbaseDir = mGisbaseLineEdit->text().trimmed();
  QgsGrass::instance()->setGisbase( customGisbase, customGisbaseDir );

  // Modules
  bool customModules = mModulesConfigCustomRadioButton->isChecked();
  QString customModulesDir = mModulesConfigDirLineEdit->text().trimmed();
  QgsGrass::instance()->setModulesConfig( customModules, customModulesDir );
  QgsGrass::instance()->setModulesDebug( mModulesDebugCheckBox->isChecked() );

  // Browser
  settings.setEnumValue( mImportSettingsPath + "/crsTransform",
                         ( QgsRasterProjector::Precision )mCrsTransformationComboBox->currentData().toInt() );

  settings.setValue( mImportSettingsPath + "/external", mImportExternalCheckBox->isChecked() );

  settings.setValue( QStringLiteral( "GRASS/showTopoLayers" ), mTopoLayersCheckBox->isChecked() );

  // Region
  QPen regionPen = QgsGrass::regionPen();
  regionPen.setColor( mRegionColorButton->color() );
  regionPen.setWidthF( mRegionWidthSpinBox->value() );
  QgsGrass::instance()->setRegionPen( regionPen );
}
