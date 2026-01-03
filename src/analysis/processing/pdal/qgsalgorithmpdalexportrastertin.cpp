/***************************************************************************
                         qgsalgorithmpdalexportrastertin.cpp
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpdalexportrastertin.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalExportRasterTinAlgorithm::name() const
{
  return u"exportrastertin"_s;
}

QString QgsPdalExportRasterTinAlgorithm::displayName() const
{
  return QObject::tr( "Export point cloud to raster (using triangulation)" );
}

QString QgsPdalExportRasterTinAlgorithm::group() const
{
  return QObject::tr( "Point cloud conversion" );
}

QString QgsPdalExportRasterTinAlgorithm::groupId() const
{
  return u"pointcloudconversion"_s;
}

QStringList QgsPdalExportRasterTinAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,dem,export,raster,tin,create" ).split( ',' );
}

QString QgsPdalExportRasterTinAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports point cloud data to a 2D raster grid using a triangulation of points and then interpolating cell values from triangles." );
}

QString QgsPdalExportRasterTinAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports a point cloud to a 2D raster grid using a triangulation of points." );
}

QgsPdalExportRasterTinAlgorithm *QgsPdalExportRasterTinAlgorithm::createInstance() const
{
  return new QgsPdalExportRasterTinAlgorithm();
}

void QgsPdalExportRasterTinAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( u"RESOLUTION"_s, QObject::tr( "Resolution of the density raster" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 1e-6 ) );
  addParameter( new QgsProcessingParameterNumber( u"TILE_SIZE"_s, QObject::tr( "Tile size for parallel runs" ), Qgis::ProcessingNumberParameterType::Integer, 1000, false, 1 ) );

#ifdef HAVE_PDAL_QGIS
#if PDAL_VERSION_MAJOR_INT > 2 || ( PDAL_VERSION_MAJOR_INT == 2 && PDAL_VERSION_MINOR_INT >= 6 )
  addParameter( new QgsProcessingParameterNumber( u"MAX_EDGE_LENGTH"_s, QObject::tr( "Maximum triangle edge length" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0, 1e8 ) );
#endif
#endif

  createCommonParameters();

  auto paramOriginX = std::make_unique<QgsProcessingParameterNumber>( u"ORIGIN_X"_s, QObject::tr( "X origin of a tile for parallel runs" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  paramOriginX->setFlags( paramOriginX->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramOriginX.release() );
  auto paramOriginY = std::make_unique<QgsProcessingParameterNumber>( u"ORIGIN_Y"_s, QObject::tr( "Y origin of a tile for parallel runs" ), Qgis::ProcessingNumberParameterType::Integer, QVariant(), true, 0 );
  paramOriginY->setFlags( paramOriginY->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramOriginY.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Exported (using triangulation)" ) ) );
}

QStringList QgsPdalExportRasterTinAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  bool hasOriginX = parameters.value( u"ORIGIN_X"_s ).isValid();
  bool hasOriginY = parameters.value( u"ORIGIN_Y"_s ).isValid();

  if ( ( hasOriginX && !hasOriginY ) || ( !hasOriginX && hasOriginY ) )
  {
    throw QgsProcessingException( QObject::tr( "Specify both X and Y tile origin or don't set any of them." ) );
  }

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  setOutputValue( u"OUTPUT"_s, outputFile );

  const double resolution = parameterAsDouble( parameters, u"RESOLUTION"_s, context );
  const int tileSize = parameterAsInt( parameters, u"TILE_SIZE"_s, context );

  enableElevationPropertiesPostProcessor( true );

  QStringList args = { u"to_raster_tin"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--resolution=%1"_s.arg( resolution ), u"--tile-size=%1"_s.arg( tileSize ) };

  if ( hasOriginX && hasOriginY )
  {
    args << u"--tile-origin-x=%1"_s.arg( parameterAsInt( parameters, u"ORIGIN_X"_s, context ) );
    args << u"--tile-origin-y=%1"_s.arg( parameterAsInt( parameters, u"ORIGIN_Y"_s, context ) );
  }

#ifdef HAVE_PDAL_QGIS
#if PDAL_VERSION_MAJOR_INT > 2 || ( PDAL_VERSION_MAJOR_INT == 2 && PDAL_VERSION_MINOR_INT >= 6 )
  if ( parameters.value( u"MAX_EDGE_LENGTH"_s ).isValid() )
  {
    args << u"--max_triangle_edge_length=%1"_s.arg( parameterAsDouble( parameters, u"MAX_EDGE_LENGTH"_s, context ) );
  }
#endif
#endif

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
