/***************************************************************************
  qgs3dmapexportwidget.cpp
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

#include "ui_map3dexportwidget.h"
#include "qgs3dmapexportwidget.h"

#include "qgis.h"
#include "qgs3dmapexportsettings.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgssettings.h"

#include <QFileDialog>
#include <QPushButton>
#include <QString>
#include <QtGlobal>

#include "moc_qgs3dmapexportwidget.cpp"

using namespace Qt::StringLiterals;

Qgs3DMapExportWidget::Qgs3DMapExportWidget( Qgs3DMapScene *scene, Qgs3DMapExportSettings *exportSettings, QWidget *parent )
  : QWidget( parent ), ui( new Ui::Map3DExportWidget ), mScene( scene ), mExportSettings( exportSettings )
{
  ui->setupUi( this );
  ui->terrainResolutionSpinBox->setClearValue( 128 );
  ui->terrainTextureResolutionSpinBox->setClearValue( 512 );
  ui->scaleSpinBox->setClearValue( 1.0 );

  ui->selectFolderWidget->setStorageMode( QgsFileWidget::StorageMode::GetDirectory );

  ui->exportFormatComboxBox->addItem( tr( "OBJ" ), QVariant::fromValue( Qgis::Export3DSceneFormat::Obj ) );
  ui->exportFormatComboxBox->addItem( tr( "STL" ), QVariant::fromValue( Qgis::Export3DSceneFormat::StlAscii ) );

  loadSettings();

  connect( ui->sceneNameLineEdit, &QLineEdit::textChanged, this, [this]( const QString & ) { settingsChanged(); } );
  connect( ui->selectFolderWidget, &QgsFileWidget::fileChanged, this, [this]( const QString & ) { settingsChanged(); } );
  connect( ui->smoothEdgesCheckBox, &QCheckBox::stateChanged, this, [this]( int ) { settingsChanged(); } );
  connect( ui->terrainResolutionSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [this]( int ) { settingsChanged(); } );
  connect( ui->exportNormalsCheckBox, &QCheckBox::stateChanged, this, [this]( int ) { settingsChanged(); } );
  connect( ui->exportTexturesCheckBox, &QCheckBox::stateChanged, this, [this]( int ) { settingsChanged(); } );
  connect( ui->terrainTextureResolutionSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [this]( int ) { settingsChanged(); } );
  connect( ui->scaleSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( int ) { settingsChanged(); } );
  connect( ui->exportFormatComboxBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &Qgs3DMapExportWidget::exportFormatChanged );

  // sets the export settings to whatever is on the scene
  settingsChanged();
}

Qgs3DMapExportWidget::~Qgs3DMapExportWidget()
{
  delete ui;
}

void Qgs3DMapExportWidget::loadSettings()
{
  ui->sceneNameLineEdit->setText( mExportSettings->sceneName() );
  ui->selectFolderWidget->setFilePath( mExportSettings->sceneFolderPath() );
  ui->terrainResolutionSpinBox->setValue( mExportSettings->terrrainResolution() );
  ui->terrainTextureResolutionSpinBox->setValue( mExportSettings->terrainTextureResolution() );
  ui->smoothEdgesCheckBox->setChecked( mExportSettings->smoothEdges() );
  ui->exportNormalsCheckBox->setChecked( mExportSettings->exportNormals() );
  ui->exportTexturesCheckBox->setChecked( mExportSettings->exportTextures() );
  ui->scaleSpinBox->setValue( mExportSettings->scale() );
  ui->exportFormatComboxBox->setCurrentIndex( ui->exportFormatComboxBox->findData( QVariant::fromValue( mExportSettings->exportFormat() ) ) );

  // Do not enable terrain options if terrain rendering is disabled
  if ( mScene->mapSettings()->terrainRenderingEnabled() )
  {
    ui->terrainGroup->setEnabled( true );
    ui->terrainGroup->setChecked( true );
    ui->terrainTextureResolutionLabel->setEnabled( true );
    ui->terrainTextureResolutionSpinBox->setEnabled( true );

    updateTerrainResolutionWidget();
  }
  else
  {
    ui->terrainGroup->setEnabled( false );
    ui->terrainGroup->setChecked( false );
    ui->terrainGroup->setToolTip( tr( "Enable terrain rendering to use this option." ) );
  }
}

void Qgs3DMapExportWidget::settingsChanged()
{
  mExportSettings->setSceneName( ui->sceneNameLineEdit->text() );
  mExportSettings->setSceneFolderPath( ui->selectFolderWidget->filePath() );
  mExportSettings->setTerrainResolution( ui->terrainResolutionSpinBox->value() );
  mExportSettings->setSmoothEdges( ui->smoothEdgesCheckBox->isChecked() );
  mExportSettings->setExportNormals( ui->exportNormalsCheckBox->isChecked() );
  mExportSettings->setExportTextures( ui->exportTexturesCheckBox->isChecked() );
  mExportSettings->setTerrainTextureResolution( ui->terrainTextureResolutionSpinBox->value() );
  mExportSettings->setScale( ui->scaleSpinBox->value() );
  mExportSettings->setTerrainExportEnabled( ui->terrainGroup->isEnabled() && ui->terrainGroup->isChecked() );
  mExportSettings->setExportFormat( ui->exportFormatComboxBox->currentData().value< Qgis::Export3DSceneFormat >() );
}

void Qgs3DMapExportWidget::exportFormatChanged()
{
  const Qgis::Export3DSceneFormat selectedType = ui->exportFormatComboxBox->currentData().value< Qgis::Export3DSceneFormat >();
  const bool isObjFormat = ( selectedType == Qgis::Export3DSceneFormat::Obj );

  ui->smoothEdgesCheckBox->setEnabled( isObjFormat );
  ui->exportTexturesCheckBox->setEnabled( isObjFormat );
  ui->exportNormalsCheckBox->setEnabled( isObjFormat );

  if ( isObjFormat )
  {
    ui->smoothEdgesCheckBox->setToolTip( "" );
    ui->exportTexturesCheckBox->setToolTip( "" );
    ui->exportNormalsCheckBox->setToolTip( "" );
  }
  else
  {
    ui->smoothEdgesCheckBox->setToolTip( tr( "This option is only available for OBJ export." ) );
    ui->exportTexturesCheckBox->setToolTip( tr( "This option is only available for OBJ export." ) );
    ui->exportNormalsCheckBox->setToolTip( tr( "This option is only available for OBJ export." ) );
  }

  updateTerrainResolutionWidget();
  settingsChanged();
}

bool Qgs3DMapExportWidget::exportScene()
{
  return mScene->exportScene( *mExportSettings );
}

void Qgs3DMapExportWidget::updateTerrainResolutionWidget()
{
  // Terrain resolution is only supported for OBJ export,
  // and only if the terrain type supports tile resolution.
  const Qgis::Export3DSceneFormat selectedType = ui->exportFormatComboxBox->currentData().value< Qgis::Export3DSceneFormat >();
  const bool isObjFormat = ( selectedType == Qgis::Export3DSceneFormat::Obj );

  // Only Dem and Online types handle terrain resolution
  const QgsTerrainGenerator *terrainGenerator = mScene->mapSettings()->terrainGenerator();
  if ( terrainGenerator->capabilities().testFlag( QgsTerrainGenerator::Capability::SupportsTileResolution ) )
  {
    ui->terrainResolutionLabel->setEnabled( isObjFormat );
    ui->terrainResolutionSpinBox->setEnabled( isObjFormat );
    if ( isObjFormat )
    {
      ui->terrainResolutionSpinBox->setToolTip( "" );
    }
    else
    {
      ui->terrainResolutionSpinBox->setToolTip( tr( "This option is only available for OBJ export." ) );
    }
  }
  else
  {
    ui->terrainResolutionLabel->setEnabled( false );
    ui->terrainResolutionSpinBox->setEnabled( false );
    ui->terrainResolutionSpinBox->setToolTip( tr( "This option is unavailable for the %1 terrain type." ).arg( terrainGenerator->typeToString( terrainGenerator->type() ) ) );
  }
}
