/***************************************************************************
   qgsprojectstorageguiregistry.cpp
                             -------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at google dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectstorageguiregistry.h"
#include "qgsprojectstorageguiprovider.h"
#include "qgsproviderguiregistry.h"

QgsProjectStorageGuiRegistry::QgsProjectStorageGuiRegistry() = default;

QgsProjectStorageGuiRegistry::~QgsProjectStorageGuiRegistry()
{
  qDeleteAll( mBackends );
}

QgsProjectStorageGuiProvider *QgsProjectStorageGuiRegistry::projectStorageFromType( const QString &type )
{
  return mBackends.value( type, nullptr );
}

QgsProjectStorageGuiProvider *QgsProjectStorageGuiRegistry::projectStorageFromUri( const QString &uri )
{
  for ( auto it = mBackends.constBegin(); it != mBackends.constEnd(); ++it )
  {
    QgsProjectStorageGuiProvider *storage = it.value();
    const QString scheme = storage->type() + ':';
    if ( uri.startsWith( scheme ) )
      return storage;
  }
  return nullptr;
}

QList<QgsProjectStorageGuiProvider *> QgsProjectStorageGuiRegistry::projectStorages() const
{
  return mBackends.values();
}

void QgsProjectStorageGuiRegistry::registerProjectStorage( QgsProjectStorageGuiProvider *storage )
{
  mBackends.insert( storage->type(), storage );
}

void QgsProjectStorageGuiRegistry::unregisterProjectStorage( QgsProjectStorageGuiProvider *storage )
{
  delete mBackends.take( storage->type() );
}

void QgsProjectStorageGuiRegistry::initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry )
{
  if ( !providerGuiRegistry )
    return;

  const QStringList providersList = providerGuiRegistry->providerList();
  for ( const QString &key : providersList )
  {
    const QList<QgsProjectStorageGuiProvider *> providerList = providerGuiRegistry->projectStorageGuiProviders( key );
    // the function is a factory - we keep ownership of the returned providers
    for ( QgsProjectStorageGuiProvider *provider : providerList )
    {
      mBackends[key] = provider;
    }
  }
}
