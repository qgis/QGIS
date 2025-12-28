/***************************************************************************
                         qgsalgorithmpdalexportraster.cpp
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

#include "qgsalgorithmpdalexportraster.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalExportRasterAlgorithm::name() const
{
  return u"exportraster"_s;
}

QString QgsPdalExportRasterAlgorithm::displayName() const
{
  return QObject::tr( "Export point cloud to raster" );
}

QString QgsPdalExportRasterAlgorithm::group() const
{
  return QObject::tr( "Point cloud conversion" );
}

QString QgsPdalExportRasterAlgorithm::groupId() const
{
  return u"pointcloudconversion"_s;
}

QStringList QgsPdalExportRasterAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,dem,export,raster,attribute,create" ).split( ',' );
}

QString QgsPdalExportRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports point cloud data to a 2D raster grid having cell size of given resolution, writing values from the specified attribute." );
}

QString QgsPdalExportRasterAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports a point cloud to a 2D raster grid containing values from the specified attribute." );
}

QgsPdalExportRasterAlgorithm *QgsPdalExportRasterAlgorithm::createInstance() const
{
  return new QgsPdalExportRasterAlgorithm();
}

void QgsPdalExportRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterPointCloudAttribute( u"ATTRIBUTE"_s, QObject::tr( "Attribute" ), u"Z"_s, u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterNumber( u"RESOLUTION"_s, QObject::tr( "Resolution of the density raster" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 1e-6 ) );
  addParameter( new QgsProcessingParameterNumber( u"TILE_SIZE"_s, QObject::tr( "Tile size for parallel runs" ), Qgis::ProcessingNumberParameterType::Integer, 1000, false, 1 ) );

  createCommonParameters();

  auto paramOriginX = std::make_unique<QgsProcessingParameterNumber>( u"ORIGIN_X"_s, QObject::tr( "X origin of a tile for parallel runs" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  paramOriginX->setFlags( paramOriginX->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramOriginX.release() );
  auto paramOriginY = std::make_unique<QgsProcessingParameterNumber>( u"ORIGIN_Y"_s, QObject::tr( "Y origin of a tile for parallel runs" ), Qgis::ProcessingNumberParameterType::Integer, QVariant(), true, 0 );
  paramOriginY->setFlags( paramOriginY->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramOriginY.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Exported" ) ) );
}

QStringList QgsPdalExportRasterAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  const QString attribute = parameterAsString( parameters, u"ATTRIBUTE"_s, context );
  const double resolution = parameterAsDouble( parameters, u"RESOLUTION"_s, context );
  const int tileSize = parameterAsInt( parameters, u"TILE_SIZE"_s, context );

  if ( attribute == 'Z' )
  {
    enableElevationPropertiesPostProcessor( true );
  }

  QStringList args = { u"to_raster"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--attribute=%1"_s.arg( attribute ), u"--resolution=%1"_s.arg( resolution ), u"--tile-size=%1"_s.arg( tileSize ) };

  if ( hasOriginX && hasOriginY )
  {
    args << u"--tile-origin-x=%1"_s.arg( parameterAsInt( parameters, u"ORIGIN_X"_s, context ) );
    args << u"--tile-origin-y=%1"_s.arg( parameterAsInt( parameters, u"ORIGIN_Y"_s, context ) );
  }

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
