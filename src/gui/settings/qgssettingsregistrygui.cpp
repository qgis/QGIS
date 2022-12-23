/***************************************************************************
  qgssettingsregistrygui.cpp
  --------------------------------------
  Date                 : July 2021
  Copyright            : (C) 2021 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettingsregistrygui.h"

#include "qgsapplication.h"
#include "qgssettingsregistrycore.h"
#include "qgsstylemanagerdialog.h"

QgsSettingsRegistryGui::QgsSettingsRegistryGui()
  : QgsSettingsRegistry()
{
  addSettingsEntry( &settingsRespectScreenDPI );
  addSettingsEntry( &settingsAutomaticallyCheckForPluginUpdates );
  addSettingsEntry( &QgsStyleManagerDialog::settingLastStyleDatabaseFolder );

  QgsApplication::settingsRegistryCore()->addSubRegistry( this );
}

QgsSettingsRegistryGui::~QgsSettingsRegistryGui()
{
  QgsApplication::settingsRegistryCore()->removeSubRegistry( this );
}

