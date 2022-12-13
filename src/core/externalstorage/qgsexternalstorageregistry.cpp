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
#include "qgshttpexternalstorage_p.h"

QgsExternalStorageRegistry::QgsExternalStorageRegistry()
{
  registerExternalStorage( new QgsSimpleCopyExternalStorage() );
  registerExternalStorage( new QgsWebDavExternalStorage() );
  registerExternalStorage( new QgsAwsS3ExternalStorage() );
}

QgsExternalStorageRegistry::~QgsExternalStorageRegistry()
{
  qDeleteAll( mBackends );
}

QgsExternalStorage *QgsExternalStorageRegistry::externalStorageFromType( const QString &type ) const
{
  auto it = std::find_if( mBackends.begin(), mBackends.end(), [ = ]( QgsExternalStorage * storage )
  {
    return storage->type() == type;
  } );

  return it != mBackends.end() ? *it : nullptr;
}

QList<QgsExternalStorage *> QgsExternalStorageRegistry::externalStorages() const
{
  return mBackends;
}

void QgsExternalStorageRegistry::registerExternalStorage( QgsExternalStorage *storage )
{
  if ( !mBackends.contains( storage ) )
    mBackends.append( storage );
}

void QgsExternalStorageRegistry::unregisterExternalStorage( QgsExternalStorage *storage )
{
  const int index = mBackends.indexOf( storage );
  if ( index >= 0 )
  {
    delete mBackends.takeAt( index );
  }
}
