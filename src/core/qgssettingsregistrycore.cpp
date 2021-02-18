/***************************************************************************
  qgssettingsregistrycore.cpp
  --------------------------------------
  Date                 : February 2021
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

#include "qgssettingsregistrycore.h"

QgsSettingsRegistryCore::QgsSettingsRegistryCore()
  : QgsSettingsRegistry( QgsSettings::Core )
  , mSettingsEntries()
{
  // Register settings for core here:
  mSettingsEntries.layout.searchPathForTemplates = QgsSettingsRegistryCore::registerSettingsStringList( QStringLiteral( "layout/searchPathsForTemplates" ),
      QStringList(),
      QObject::tr( "Search path for templates" ) );
  mSettingsEntries.measure.planimetric = QgsSettingsRegistry::registerSettingsBool( QStringLiteral( "measure/planimetric" ),
                                         false,
                                         QObject::tr( "Planimetric" ) );
}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
}

QgsSettingsRegistryCore::SettingsEntries QgsSettingsRegistryCore::settingsEntries() const
{
  return mSettingsEntries;
}

