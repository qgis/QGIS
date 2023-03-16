/***************************************************************************
  qgsarcgisvectortileservicedataprovider.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarcgisvectortileservicedataprovider.h"
#include "qgsthreadingutils.h"

///@cond PRIVATE

QgsArcGisVectorTileServiceDataProvider::QgsArcGisVectorTileServiceDataProvider( const QString &uri, const QString &sourcePath, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsXyzVectorTileDataProvider( uri, providerOptions, flags )
  , mSourcePath( sourcePath )
{

}

QgsVectorTileDataProvider *QgsArcGisVectorTileServiceDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  ProviderOptions options;
  options.transformContext = transformContext();
  return new QgsArcGisVectorTileServiceDataProvider( dataSourceUri(), mSourcePath, options, mReadFlags );
}

QString QgsArcGisVectorTileServiceDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSourcePath;
}

bool QgsArcGisVectorTileServiceDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}

///@endcond


