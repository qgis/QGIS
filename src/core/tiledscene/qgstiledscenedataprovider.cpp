/***************************************************************************
                         qgstiledscenedataprovider.cpp
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

#include "qgstiledscenedataprovider.h"
#include "moc_qgstiledscenedataprovider.cpp"
#include "qgsthreadingutils.h"

QgsTiledSceneDataProvider::QgsTiledSceneDataProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  Qgis::DataProviderReadFlags flags )
  : QgsDataProvider( uri, options, flags )
{
}

QgsTiledSceneDataProvider::QgsTiledSceneDataProvider( const QgsTiledSceneDataProvider &other )
  : QgsDataProvider( other.dataSourceUri( false ), ProviderOptions(), other.mReadFlags )
{
  setTransformContext( other.transformContext() );
}

QgsTiledSceneDataProvider::~QgsTiledSceneDataProvider() = default;

Qgis::TiledSceneProviderCapabilities QgsTiledSceneDataProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::TiledSceneProviderCapabilities();
}

QgsDoubleRange QgsTiledSceneDataProvider::zRange() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsDoubleRange();
}
