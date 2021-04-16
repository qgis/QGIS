/***************************************************************************
  qgssettingsregistry.cpp
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

#include "qgssettingsregistry.h"

#include "qgslayout.h"
#include "qgslocator.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsnewsfeedparser.h"
#include "qgsprocessing.h"
#include "qgsapplication.h"
#include "qgsgeometryoptions.h"
#include "qgslocalizeddatapathregistry.h"
#include "qgsmaprendererjob.h"

QgsSettingsRegistry::QgsSettingsRegistry()
  : mSettingsEntriesMap()
  , mDynamicSettingsEntriesMap()
{
}

QgsSettingsRegistry::~QgsSettingsRegistry()
{
}

const QgsSettingsEntryBase *QgsSettingsRegistry::getSettingsEntry( const QString &key )
{
  if ( mSettingsEntriesMap.contains( key ) )
    return mSettingsEntriesMap.value( key );

  const QMap<QString, const QgsSettingsEntryBase *> dynamicSettingsEntriesMap = mDynamicSettingsEntriesMap;
  for ( const QgsSettingsEntryBase *settingsEntry : dynamicSettingsEntriesMap )
  {
    if ( settingsEntry->checkKey( key ) )
      return settingsEntry;
  }

  return nullptr;
}

void QgsSettingsRegistry::addSettingsEntry( const QgsSettingsEntryBase *settingsEntry )
{
  if ( settingsEntry->hasDynamicKey() )
    mDynamicSettingsEntriesMap.insert( settingsEntry->definitionKey(), settingsEntry );
  else
    mSettingsEntriesMap.insert( settingsEntry->key(), settingsEntry );
}
