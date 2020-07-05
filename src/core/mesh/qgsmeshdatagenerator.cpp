/***************************************************************************
                         qgsmeshdatagenerator.cpp
                         ---------------------
    begin                : June 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshdatagenerator.h"
#include "qgsmeshdataset.h"

QgsMeshDataGeneratorInterface::QgsMeshDataGeneratorInterface() = default;

QgsMeshDataGeneratorInterface::~QgsMeshDataGeneratorInterface() = default;


QgsMeshDataGeneratorRegistry::~QgsMeshDataGeneratorRegistry()
{
  qDeleteAll( mMeshDataGenerators );
}

void QgsMeshDataGeneratorRegistry::addMeshDataGenerator( QgsMeshDataGeneratorInterface *generator )
{
  mMeshDataGenerators[generator->key()] = generator;
}

QgsMeshDataGeneratorInterface *QgsMeshDataGeneratorRegistry::meshDataGenerator( const QString &key ) const
{
  if ( mMeshDataGenerators.contains( key ) )
    return mMeshDataGenerators[key];
  else
    return nullptr;
}
