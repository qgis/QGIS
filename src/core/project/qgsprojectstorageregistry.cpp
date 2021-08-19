/***************************************************************************
  qgsprojectstorageregistry.cpp
  --------------------------------------
  Date                 : March 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectstorageregistry.h"

#include "qgsprojectstorage.h"


QgsProjectStorageRegistry::~QgsProjectStorageRegistry()
{
  qDeleteAll( mBackends );
}

QgsProjectStorage *QgsProjectStorageRegistry::projectStorageFromType( const QString &type )
{
  return mBackends.value( type, nullptr );
}

QgsProjectStorage *QgsProjectStorageRegistry::projectStorageFromUri( const QString &uri )
{
  for ( auto it = mBackends.constBegin(); it != mBackends.constEnd(); ++it )
  {
    QgsProjectStorage *storage = it.value();
    const QString scheme = storage->type() + ':';
    if ( uri.startsWith( scheme ) )
      return storage;
  }

  // second chance -- use isSupportedUri to determine if a uri is supported by a backend
  for ( auto it = mBackends.constBegin(); it != mBackends.constEnd(); ++it )
  {
    QgsProjectStorage *storage = it.value();
    if ( storage->isSupportedUri( uri ) )
      return storage;
  }
  return nullptr;
}

QList<QgsProjectStorage *> QgsProjectStorageRegistry::projectStorages() const
{
  return mBackends.values();
}

void QgsProjectStorageRegistry::registerProjectStorage( QgsProjectStorage *storage )
{
  mBackends.insert( storage->type(), storage );
}

void QgsProjectStorageRegistry::unregisterProjectStorage( QgsProjectStorage *storage )
{
  delete mBackends.take( storage->type() );
}
