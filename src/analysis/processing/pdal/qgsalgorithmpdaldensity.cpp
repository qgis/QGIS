/***************************************************************************
                         qgsalgorithmpdaldensity.cpp
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

#include "qgsalgorithmpdaldensity.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalDensityAlgorithm::name() const
{
  return QStringLiteral( "density" );
}

QString QgsPdalDensityAlgorithm::displayName() const
{
  return QObject::tr( "Density" );
}

QString QgsPdalDensityAlgorithm::group() const
{
  return QObject::tr( "Point cloud extraction" );
}

QString QgsPdalDensityAlgorithm::groupId() const
{
  return QStringLiteral( "pointcloudextraction" );
}

QStringList QgsPdalDensityAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,cell,count,density,raster" ).split( ',' );
}

QString QgsPdalDensityAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a raster file where each cell contains number of points that are in that cell's area." );
}

QgsPdalDensityAlgorithm *QgsPdalDensityAlgorithm::createInstance() const
{
  return new QgsPdalDensityAlgorithm();
}

void QgsPdalDensityAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "RESOLUTION" ), QObject::tr( "Resolution of the density raster" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 1e-6 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TILE_SIZE" ), QObject::tr( "Tile size for parallel runs" ), Qgis::ProcessingNumberParameterType::Integer, 1000, false, 1 ) );

  createCommonParameters();

  std::unique_ptr<QgsProcessingParameterNumber> paramOriginX = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "ORIGIN_X" ), QObject::tr( "X origin of a tile for parallel runs" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 0 );
  paramOriginX->setFlags( paramOriginX->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramOriginX.release() );
  std::unique_ptr<QgsProcessingParameterNumber> paramOriginY = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "ORIGIN_Y" ), QObject::tr( "Y origin of a tile for parallel runs" ), Qgis::ProcessingNumberParameterType::Integer, QVariant(), true, 0 );
  paramOriginY->setFlags( paramOriginY->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( paramOriginY.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Density" ) ) );
}

QStringList QgsPdalDensityAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  const double resolution = parameterAsDouble( parameters, QStringLiteral( "RESOLUTION" ), context );
  const int tileSize = parameterAsInt( parameters, QStringLiteral( "TILE_SIZE" ), context );

  QStringList args = { QStringLiteral( "density" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--resolution=%1" ).arg( resolution ), QStringLiteral( "--tile-size=%1" ).arg( tileSize ) };

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
