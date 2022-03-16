/***************************************************************************
                         qgssettingsregistryapp.h
                         ----------------------
    begin                : January 2022
    copyright            : (C) 2022 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettingsregistrycore.h"
#include "qgssettingsregistryapp.h"
#include "qgsapplication.h"

#include "qgsmaptoolsdigitizingtechniquemanager.h"
#include "georeferencer/qgsgeorefmainwindow.h"
#include "georeferencer/qgstransformsettingsdialog.h"

QgsSettingsRegistryApp::QgsSettingsRegistryApp()
  : QgsSettingsRegistry()
{
  addSettingsEntry( &QgsMapToolsDigitizingTechniqueManager::settingsDigitizingTechnique );
  addSettingsEntry( &QgsMapToolsDigitizingTechniqueManager::settingMapToolShapeDefaultForShape );
  addSettingsEntry( &QgsMapToolsDigitizingTechniqueManager::settingMapToolShapeCurrent );

  addSettingsEntry( &QgsGeoreferencerMainWindow::settingResamplingMethod );
  addSettingsEntry( &QgsGeoreferencerMainWindow::settingCompressionMethod );
  addSettingsEntry( &QgsGeoreferencerMainWindow::settingUseZeroForTransparent );
  addSettingsEntry( &QgsGeoreferencerMainWindow::settingTransformMethod );
  addSettingsEntry( &QgsGeoreferencerMainWindow::settingSaveGcps );
  addSettingsEntry( &QgsGeoreferencerMainWindow::settingLoadInProject );
  addSettingsEntry( &QgsGeoreferencerMainWindow::settingLastSourceFolder );
  addSettingsEntry( &QgsGeoreferencerMainWindow::settingLastRasterFileFilter );

  addSettingsEntry( &QgsTransformSettingsDialog::settingLastDestinationFolder );
  addSettingsEntry( &QgsTransformSettingsDialog::settingLastPdfFolder );

  QgsApplication::settingsRegistryCore()->addSubRegistry( this );
}

QgsSettingsRegistryApp::~QgsSettingsRegistryApp()
{
  QgsApplication::settingsRegistryCore()->removeSubRegistry( this );
}
