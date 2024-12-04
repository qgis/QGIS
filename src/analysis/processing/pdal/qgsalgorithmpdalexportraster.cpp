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

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalExportRasterAlgorithm::name() const
{
  return QStringLiteral( "exportraster" );
}

QString QgsPdalExportRasterAlgorithm::displayName() const
{
  return QObject::tr( "Export to raster" );
}

QString QgsPdalExportRasterAlgorithm::group() const
{
  return QObject::tr( "Point cloud conversion" );
}

QString QgsPdalExportRasterAlgorithm::groupId() const
{
  return QStringLiteral( "pointcloudconversion" );
}

QStringList QgsPdalExportRasterAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,dem,export,raster,attribute,create" ).split( ',' );
}

QString QgsPdalExportRasterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports point cloud data to a 2D raster grid having cell size of given resolution, writing values from the specified attribute." );
}

QgsPdalExportRasterAlgorithm *QgsPdalExportRasterAlgorithm::createInstance() const
{
  return new QgsPdalExportRasterAlgorithm();
}

void QgsPdalExportRasterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterPointCloudAttribute( QStringLiteral( "ATTRIBUTE" ), QObject::tr( "Attribute" ), QStringLiteral( "Z" ), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "RESOLUTION" ), QObject::tr( "Resolution of the density raster" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 1e-6 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TILE_SIZE" ), QObject::tr( "Tile size for parallel runs" ), Qgis::ProcessingNumberParameterType::Integer, 1000, false, 1 ) );

  createCommonParameters();

  std::unique_ptr<QgsProcessingParameterNumber> paramOriginX = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "ORIGIN_X" ), QObject::tr( "X origin of a tile for parallel runs" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  paramOriginX->setFlags( paramOriginX->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramOriginX.release() );
  std::unique_ptr<QgsProcessingParameterNumber> paramOriginY = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "ORIGIN_Y" ), QObject::tr( "Y origin of a tile for parallel runs" ), Qgis::ProcessingNumberParameterType::Integer, QVariant(), true, 0 );
  paramOriginY->setFlags( paramOriginY->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramOriginY.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Exported" ) ) );
}

QStringList QgsPdalExportRasterAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  bool hasOriginX = parameters.value( QStringLiteral( "ORIGIN_X" ) ).isValid();
  bool hasOriginY = parameters.value( QStringLiteral( "ORIGIN_Y" ) ).isValid();

  if ( ( hasOriginX && !hasOriginY ) || ( !hasOriginX && hasOriginY ) )
  {
    throw QgsProcessingException( QObject::tr( "Specify both X and Y tile origin or don't set any of them." ) );
  }

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  const QString attribute = parameterAsString( parameters, QStringLiteral( "ATTRIBUTE" ), context );
  const double resolution = parameterAsDouble( parameters, QStringLiteral( "RESOLUTION" ), context );
  const int tileSize = parameterAsInt( parameters, QStringLiteral( "TILE_SIZE" ), context );

  if ( attribute == 'Z' )
  {
    enableElevationPropertiesPostProcessor( true );
  }

  QStringList args = { QStringLiteral( "to_raster" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--attribute=%1" ).arg( attribute ), QStringLiteral( "--resolution=%1" ).arg( resolution ), QStringLiteral( "--tile-size=%1" ).arg( tileSize ) };

  if ( hasOriginX && hasOriginY )
  {
    args << QStringLiteral( "--tile-origin-x=%1" ).arg( parameterAsInt( parameters, QStringLiteral( "ORIGIN_X" ), context ) );
    args << QStringLiteral( "--tile-origin-y=%1" ).arg( parameterAsInt( parameters, QStringLiteral( "ORIGIN_Y" ), context ) );
  }

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
