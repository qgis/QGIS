/***************************************************************************
  qgsauthconfigurationstorageregistry.cpp - QgsAuthConfigurationStorageRegistry

 ---------------------
 begin                : 20.6.2024
 copyright            : (C) 2024 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsauthconfigurationstorageregistry.h"
#include "qgsauthconfigurationstorage.h"

QgsAuthConfigurationStorageRegistry *QgsAuthConfigurationStorageRegistry::instance()
{
  static QgsAuthConfigurationStorageRegistry *const registry = new QgsAuthConfigurationStorageRegistry( );
  return registry;
}

QgsAuthConfigurationStorageRegistry::QgsAuthConfigurationStorageRegistry()
{
}

QgsAuthConfigurationStorageRegistry::~QgsAuthConfigurationStorageRegistry()
{
  qDeleteAll( mStorages );
}

bool QgsAuthConfigurationStorageRegistry::addStorage( QgsAuthConfigurationStorage *storage,  QgsAuthConfigurationStorage *after )
{
  if ( mStorages.contains( storage ) || ! storage || storage == after )
  {
    return false;
  }
  else
  {
    if ( after && mStorages.contains( after ) )
    {
      mStorages.insert( mStorages.indexOf( after ) + 1, storage );
    }
    else
    {
      mStorages.append( storage );
    }
    connect( storage, &QgsAuthConfigurationStorage::storageChanged, this, [this, storage] { emit storageChanged( storage ); } );
    emit storageAdded( storage );
    return true;
  }
}

QList<QgsAuthConfigurationStorage *> QgsAuthConfigurationStorageRegistry::storages() const
{
  return mStorages;
}

QList<QgsAuthConfigurationStorage *> QgsAuthConfigurationStorageRegistry::readyStorages() const
{
  QList<QgsAuthConfigurationStorage *> readyStorages;
  for ( QgsAuthConfigurationStorage *storage : std::as_const( mStorages ) )
  {
    if ( storage->isReady() && storage->isEnabled() )
    {
      readyStorages.append( storage );
    }
  }
  return readyStorages;
}
