/***************************************************************************
                         qgspointclouddataprovider.cpp
                         -----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgspointclouddataprovider.h"
#include "qgspointcloudindex.h"

QgsPointCloudDataProvider::QgsPointCloudDataProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( uri, options, flags )
  , mIndex( new QgsPointCloudIndex )
{
  mIsValid = mIndex->load( uri );
}

QgsPointCloudDataProvider::~QgsPointCloudDataProvider() = default;

QgsCoordinateReferenceSystem QgsPointCloudDataProvider::crs() const
{
  return QgsCoordinateReferenceSystem::fromWkt( mIndex->wkt() );
}

QgsRectangle QgsPointCloudDataProvider::extent() const
{
  return mIndex->extent();
}

bool QgsPointCloudDataProvider::isValid() const
{
  return mIsValid;
}

QString QgsPointCloudDataProvider::name() const
{
  return QStringLiteral( "pointclouds" );
}

QString QgsPointCloudDataProvider::description() const
{
  return QStringLiteral( "Point Clouds" );
}

QgsPointCloudIndex *QgsPointCloudDataProvider::index()
{
  return mIndex.get();
}
