/***************************************************************************
  qgsexternalstorageregistry.cpp
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalstorageregistry.h"

#include "qgsexternalstorage.h"
#include "qgssimplecopyexternalstorage_p.h"

QgsExternalStorageRegistry::QgsExternalStorageRegistry()
{
}

QgsExternalStorageRegistry::~QgsExternalStorageRegistry()
{
  qDeleteAll( mBackends );
}

QgsExternalStorage *QgsExternalStorageRegistry::externalStorageFromType( const QString &type )
{
  return mBackends.value( type );
}

QList<QgsExternalStorage *> QgsExternalStorageRegistry::externalStorages() const
{
  return mBackends.values();
}

void QgsExternalStorageRegistry::registerExternalStorage( QgsExternalStorage *storage )
{
  mBackends.insert( storage->type(), storage );
}

void QgsExternalStorageRegistry::unregisterExternalStorage( QgsExternalStorage *storage )
{
  delete mBackends.take( storage->type() );
}
