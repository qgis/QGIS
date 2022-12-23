/***************************************************************************
  qgsmap3dexportwidget.cpp
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmap3dexportwidget.h"
#include "ui_map3dexportwidget.h"

#include <QPushButton>
#include <QFileDialog>
#include <QtGlobal>

#include "qgis.h"
#include "qgs3dmapscene.h"
#include "qgssettings.h"
#include "qgs3dmapexportsettings.h"

QgsMap3DExportWidget::QgsMap3DExportWidget( Qgs3DMapScene *scene, Qgs3DMapExportSettings *exportSettings, QWidget *parent ) :
  QWidget( parent ),
  ui( new Ui::Map3DExportWidget ),
  mScene( scene ),
  mExportSettings( exportSettings )
{
  ui->setupUi( this );
  ui->terrainResolutionSpinBox->setClearValue( 128 );
  ui->terrainTextureResolutionSpinBox->setClearValue( 512 );
  ui->scaleSpinBox->setClearValue( 1.0 );

  ui->selectFolderWidget->setStorageMode( QgsFileWidget::StorageMode::GetDirectory );

  loadSettings();

  connect( ui->sceneNameLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & ) { settingsChanged(); } );
  connect( ui->selectFolderWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & ) { settingsChanged(); } );
  connect( ui->smoothEdgesCheckBox, &QCheckBox::stateChanged, this, [ = ]( int ) { settingsChanged(); } );
  connect( ui->terrainResolutionSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int ) { settingsChanged(); } );
  connect( ui->exportNormalsCheckBox, &QCheckBox::stateChanged, this, [ = ]( int ) { settingsChanged(); } );
  connect( ui->exportTexturesCheckBox, &QCheckBox::stateChanged, this, [ = ]( int ) { settingsChanged(); } );
  connect( ui->terrainTextureResolutionSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]( int ) { settingsChanged(); } );
  connect( ui->scaleSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [ = ]( int ) { settingsChanged(); } );

  // sets the export settings to whatever is on the scene
  settingsChanged();
}

QgsMap3DExportWidget::~QgsMap3DExportWidget()
{
  delete ui;
}

void QgsMap3DExportWidget::loadSettings()
{
  ui->sceneNameLineEdit->setText( mExportSettings->sceneName() );
  ui->selectFolderWidget->setFilePath( mExportSettings->sceneFolderPath() );
  ui->terrainResolutionSpinBox->setValue( mExportSettings->terrrainResolution() );
  ui->terrainTextureResolutionSpinBox->setValue( mExportSettings->terrainTextureResolution() );
  ui->smoothEdgesCheckBox->setChecked( mExportSettings->smoothEdges() );
  ui->exportNormalsCheckBox->setChecked( mExportSettings->exportNormals() );
  ui->exportTexturesCheckBox->setChecked( mExportSettings->exportTextures() );
  ui->scaleSpinBox->setValue( mExportSettings->scale() );
}

void QgsMap3DExportWidget::settingsChanged()
{
  mExportSettings->setSceneName( ui->sceneNameLineEdit->text() );
  mExportSettings->setSceneFolderPath( ui->selectFolderWidget->filePath() );
  mExportSettings->setTerrainResolution( ui->terrainResolutionSpinBox->value() );
  mExportSettings->setSmoothEdges( ui->smoothEdgesCheckBox->isChecked() );
  mExportSettings->setExportNormals( ui->exportNormalsCheckBox->isChecked() );
  mExportSettings->setExportTextures( ui->exportTexturesCheckBox->isChecked() );
  mExportSettings->setTerrainTextureResolution( ui->terrainTextureResolutionSpinBox->value() );
  mExportSettings->setScale( ui->scaleSpinBox->value() );
}

void QgsMap3DExportWidget::exportScene()
{
  mScene->exportScene( *mExportSettings );
}
