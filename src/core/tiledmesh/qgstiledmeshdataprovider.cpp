/***************************************************************************
                         qgstiledmeshdataprovider.cpp
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledmeshdataprovider.h"
#include "qgsthreadingutils.h"

QgsTiledMeshDataProvider::QgsTiledMeshDataProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( uri, options, flags )
{
}

QgsTiledMeshDataProvider::QgsTiledMeshDataProvider( const QgsTiledMeshDataProvider &other )
  : QgsDataProvider( other.dataSourceUri( false ), ProviderOptions(), other.mReadFlags )
{
  setTransformContext( other.transformContext() );
}

QgsTiledMeshDataProvider::~QgsTiledMeshDataProvider() = default;

Qgis::TiledMeshProviderCapabilities QgsTiledMeshDataProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::TiledMeshProviderCapabilities();
}

QString QgsTiledMeshDataProvider::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QString();
}
