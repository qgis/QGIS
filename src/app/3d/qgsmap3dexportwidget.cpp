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

#include "qgs3dmapscene.h"
#include "qgssettings.h"

QgsMap3DExportWidget::QgsMap3DExportWidget( Qgs3DMapScene *scene, QWidget *parent ) :
  QWidget( parent ),
  ui( new Ui::Map3DExportWidget ),
  mScene( scene )
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
}

QgsMap3DExportWidget::~QgsMap3DExportWidget()
{
  delete ui;
}

void QgsMap3DExportWidget::exportScene()
{
  QString sceneName = ui->sceneNameLineEdit->text();
  QString sceneFolder = ui->folderPathLineEdit->text();
  int levelOfDetails = ui->levelOfDetailsSpinBox->value();
  bool smoothEdges = ui->smoothEdgesCheckBox->isChecked();
  mScene->exportScene( sceneName, sceneFolder, levelOfDetails, smoothEdges );
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/last3DSceneExportDir" ), sceneFolder );
}
