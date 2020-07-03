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

  QgsSettings settings;
  QString initialPath = settings.value( QStringLiteral( "UI/last3DSceneExportDir" ), QDir::homePath() ).toString();
  ui->folderPathLineEdit->setText( initialPath );

  connect( ui->selectFolderBtn, &QPushButton::clicked, [ = ]( bool )
  {
    QString initialPath = ui->folderPathLineEdit->text();
    QString outputDir = QFileDialog::getExistingDirectory( this, QString( "Export scane" ), initialPath );
    ui->folderPathLineEdit->setText( outputDir );
  } );

  connect( ui->sceneNameLineEdit, &QLineEdit::textChanged, [ = ]( const QString & ) { settingsChanged(); } );
  connect( ui->folderPathLineEdit, &QLineEdit::textChanged, [ = ]( const QString & ) { settingsChanged(); } );
  connect( ui->smoothEdgesCheckBox, &QCheckBox::stateChanged, [ = ]( int ) { settingsChanged(); } );
  connect( ui->levelOfDetailsSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), [ = ]( int ) { settingsChanged(); } );

  // sets the export settings to whatever is on the scene
  settingsChanged();
}

QgsMap3DExportWidget::~QgsMap3DExportWidget()
{
  delete ui;
}

void QgsMap3DExportWidget::settingsChanged()
{
  mExportSettings->setSceneName( ui->sceneNameLineEdit->text() );
  mExportSettings->setSceneFolderPath( ui->folderPathLineEdit->text() );
  mExportSettings->setLevelOfDetails( ui->levelOfDetailsSpinBox->value() );
  mExportSettings->setSmoothEdges( ui->smoothEdgesCheckBox->isChecked() );
}

void QgsMap3DExportWidget::exportScene()
{
  mScene->exportScene( *mExportSettings );
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/last3DSceneExportDir" ), mExportSettings->sceneFolderPath() );
}
