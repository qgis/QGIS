/***************************************************************************
                         qgseptdataprovider.cpp
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
#include "qgseptprovider.h"
#include "qgseptpointcloudindex.h"
#include "qgseptdataitems.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "ept" )
#define PROVIDER_DESCRIPTION QStringLiteral( "EPT point cloud data provider" )

QgsEptProvider::QgsEptProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags )
  , mIndex( new QgsEptPointCloudIndex )
{
  mIsValid = mIndex->load( uri );
}

QgsEptProvider::~QgsEptProvider() = default;

QgsCoordinateReferenceSystem QgsEptProvider::crs() const
{
  return mIndex->crs();
}

QgsRectangle QgsEptProvider::extent() const
{
  return mIndex->extent();
}

bool QgsEptProvider::isValid() const
{
  return mIsValid;
}

QString QgsEptProvider::name() const
{
  return QStringLiteral( "ept" );
}

QString QgsEptProvider::description() const
{
  return QStringLiteral( "Point Clouds EPT" );
}

QgsPointCloudIndex *QgsEptProvider::index() const
{
  return mIndex.get();
}

QgsEptProviderMetadata::QgsEptProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QgsEptProvider *QgsEptProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsEptProvider( uri, options, flags );
}

QList<QgsDataItemProvider *> QgsEptProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsEptDataItemProvider;
  return providers;
}

QVariantMap QgsEptProviderMetadata::decodeUri( const QString &uri )
{
  const QString path = uri;
  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "path" ), path );
  return uriComponents;
}

QString QgsEptProviderMetadata::encodeUri( const QVariantMap &parts )
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

///@endcond
