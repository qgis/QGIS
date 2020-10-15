/***************************************************************************
                         qgspointcloudprovidermetadata.cpp
                         --------------------
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

#include "qgspointcloudprovidermetadata.h"
#include "qgspointclouddataitems.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "pointcloud" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Point cloud provider" )

QgsPointCloudProviderMetadata::QgsPointCloudProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QList<QgsDataItemProvider *> QgsPointCloudProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsPointCloudDataItemProvider;
  return providers;
}

///@endcond
