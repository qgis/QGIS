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
  mSettingsEntries.layout.initialize( "layout", nullptr, "Layout group description" );

  mSettingsEntries.layout.searchPathForTemplates = QgsSettingsEntryStringList( "searchPathsForTemplates", &mSettingsEntries.layout, QStringList(), QObject::tr( "Search path for templates" ) );
  mSettingsEntries.layout.anotherNumericSettings = QgsSettingsEntryInteger( "anotherNumericSettings", &mSettingsEntries.layout, 1234, "Example settings", 100, 9999 );

  mSettingsEntries.layout.subLayout.initialize( "sub_layout", &mSettingsEntries.layout, "Description..." );


  mSettingsEntries.measure.initialize( "measure", nullptr );

  mSettingsEntries.measure.planimetric = QgsSettingsEntryBool( "planimetric", &mSettingsEntries.measure, false, "Planimetric" );


//  mSettingsEntries.locatorFilters.initialize("locator_filters", nullptr, "Example of a map of settings, could also be a map of settings groups");

}

QgsSettingsRegistryCore::~QgsSettingsRegistryCore()
{
}

QgsSettingsRegistryCore::SettingsEntries QgsSettingsRegistryCore::settingsEntries() const
{
  return mSettingsEntries;
}

