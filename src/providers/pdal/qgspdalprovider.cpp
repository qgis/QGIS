/***************************************************************************
                         qgspdaldataprovider.cpp
                         -----------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
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
#include "qgspdalprovider.h"
#include "qgspdaldataitems.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"

#include <pdal/io/LasReader.hpp>
#include <pdal/io/LasHeader.hpp>
#include <pdal/Options.hpp>

#define PROVIDER_KEY QStringLiteral( "pdal" )
#define PROVIDER_DESCRIPTION QStringLiteral( "PDAL point cloud data provider" )

QgsPdalProvider::QgsPdalProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags )
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = qgis::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  mIsValid = load( uri );
}

QgsPdalProvider::~QgsPdalProvider() = default;

QgsCoordinateReferenceSystem QgsPdalProvider::crs() const
{
  return mCrs;
}

QgsRectangle QgsPdalProvider::extent() const
{
  return mExtent;
}

bool QgsPdalProvider::isValid() const
{
  return mIsValid;
}

QString QgsPdalProvider::name() const
{
  return QStringLiteral( "pdal" );
}

QString QgsPdalProvider::description() const
{
  return QStringLiteral( "Point Clouds PDAL" );
}

QgsPointCloudIndex *QgsPdalProvider::index() const
{
  // TODO automatically generate EPT index
  return nullptr;
}

bool QgsPdalProvider::load( const QString &uri )
{
  pdal::Option las_opt( "filename", uri.toStdString() );
  pdal::Options las_opts;
  las_opts.add( las_opt );
  pdal::PointTable table;
  pdal::LasReader las_reader;
  las_reader.setOptions( las_opts );
  las_reader.prepare( table );
  pdal::PointViewSet point_view_set = las_reader.execute( table );
  pdal::PointViewPtr point_view = *point_view_set.begin();
  pdal::Dimension::IdList dims = point_view->dims();
  pdal::LasHeader las_header = las_reader.header();

  // extent
  /*
  double scale_x = las_header.scaleX();
  double scale_y = las_header.scaleY();
  double scale_z = las_header.scaleZ();

  double offset_x = las_header.offsetX();
  double offset_y = las_header.offsetY();
  double offset_z = las_header.offsetZ();
  */

  double xmin = las_header.minX();
  double xmax = las_header.maxX();
  double ymin = las_header.minY();
  double ymax = las_header.maxY();
  mExtent = QgsRectangle( xmin, ymin, xmax, ymax );

  // unsigned int nFeatures = las_header.pointCount();

  // projection
  QString wkt = QString::fromStdString( las_reader.getSpatialReference().getWKT() );
  mCrs = QgsCoordinateReferenceSystem::fromWkt( wkt );

  return true;
}

QgsPdalProviderMetadata::QgsPdalProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QgsPdalProvider *QgsPdalProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsPdalProvider( uri, options, flags );
}

QList<QgsDataItemProvider *> QgsPdalProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsPdalDataItemProvider;
  return providers;
}

QVariantMap QgsPdalProviderMetadata::decodeUri( const QString &uri )
{
  const QString path = uri;
  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "path" ), path );
  return uriComponents;
}

QString QgsPdalProviderMetadata::encodeUri( const QVariantMap &parts )
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsPdalProviderMetadata();
}
