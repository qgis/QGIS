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
  , mSettingsRegistryChildList()
{
}

QgsSettingsRegistry::~QgsSettingsRegistry()
{
}

bool QgsSettingsRegistry::addSettingsEntry( const QgsSettingsEntryBase *settingsEntry )
{
  if ( !settingsEntry )
  {
    QgsDebugMsg( QStringLiteral( "Trying to register a nullptr settings entry." ) );
    return false;
  }

  if ( mSettingsEntriesMap.contains( settingsEntry->definitionKey() ) )
  {
    QgsDebugMsg( QStringLiteral( "Settings with key '%1' is already registered." ).arg( settingsEntry->definitionKey() ) );
    return false;
  }

  mSettingsEntriesMap.insert( settingsEntry->definitionKey(), settingsEntry );
  return true;
}

void QgsSettingsRegistry::addSettingsEntryGroup( const QgsSettingsEntryGroup *settingsGroup )
{
  for ( const auto *setting : settingsGroup->settings() )
  {
    if ( addSettingsEntry( setting ) )
    {
      mSettingsEntriesGroupMap.insert( setting, settingsGroup );
    }
  }
}

QList<const QgsSettingsEntryBase *> QgsSettingsRegistry::settingEntries() const
{
  return mSettingsEntriesMap.values();
}

const QgsSettingsEntryBase *QgsSettingsRegistry::settingsEntry( const QString &key, bool searchChildRegistries ) const
{
  // Search in this registry
  const QMap<QString, const QgsSettingsEntryBase *> settingsEntriesMap = mSettingsEntriesMap;
  for ( const QgsSettingsEntryBase *settingsEntry : settingsEntriesMap )
  {
    if ( settingsEntry->keyIsValid( key ) )
      return settingsEntry;
  }

  // Search in child registries
  if ( searchChildRegistries )
  {
    for ( const QgsSettingsRegistry *settingsRegistry : std::as_const( mSettingsRegistryChildList ) )
    {
      const QgsSettingsEntryBase *settingsEntry = settingsRegistry->settingsEntry( key, true );
      if ( settingsEntry )
        return settingsEntry;
    }
  }

  return nullptr;
}

void QgsSettingsRegistry::addSubRegistry( const QgsSettingsRegistry *settingsRegistry )
{
  if ( !settingsRegistry )
  {
    QgsDebugMsg( QStringLiteral( "Trying to register a nullptr child settings registry." ) );
    return;
  }

  if ( mSettingsRegistryChildList.contains( settingsRegistry ) )
  {
    QgsDebugMsg( QStringLiteral( "Child register is already registered." ) );
    return;
  }

  mSettingsRegistryChildList.append( settingsRegistry );
}

void QgsSettingsRegistry::removeSubRegistry( const QgsSettingsRegistry *settingsRegistry )
{
  if ( !settingsRegistry )
  {
    QgsDebugMsg( QStringLiteral( "Trying to unregister a nullptr child settings registry." ) );
    return;
  }

  if ( mSettingsRegistryChildList.contains( settingsRegistry ) )
  {
    QgsDebugMsg( QStringLiteral( "Child register is not registered." ) );
    return;
  }

  mSettingsRegistryChildList.removeAll( settingsRegistry );
}

QList<const QgsSettingsRegistry *> QgsSettingsRegistry::subRegistries() const
{
  return mSettingsRegistryChildList;
}
