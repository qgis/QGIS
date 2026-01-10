/***************************************************************************
                         qgsalgorithmpdalfilternoiseradius.cpp
                         ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpdalfilternoiseradius.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalFilterNoiseRadiusAlgorithm::name() const
{
  return QStringLiteral( "filternoiseradius" );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::displayName() const
{
  return QObject::tr( "Filter Noise (by radius algorithm)" );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalFilterNoiseRadiusAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,filter,noise,radius" ).split( ',' );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters noise in a point cloud using radius algorithm." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "Points are marked as noise if they have fewer than the minimum number of neighbors within the specified radius." );
}

QString QgsPdalFilterNoiseRadiusAlgorithm::shortDescription() const
{
  return QObject::tr( "Filters noise in a point cloud using radius algorithm." );
}

QgsPdalFilterNoiseRadiusAlgorithm *QgsPdalFilterNoiseRadiusAlgorithm::createInstance() const
{
  return new QgsPdalFilterNoiseRadiusAlgorithm();
}

void QgsPdalFilterNoiseRadiusAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "REMOVE_NOISE_POINTS" ), QObject::tr( "Remove noise points" ), false ) );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MIN_K" ), QObject::tr( "Minimum number of neighbors in radius" ), Qgis::ProcessingNumberParameterType::Integer, 2 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "RADIUS" ), QObject::tr( "Radius" ), Qgis::ProcessingNumberParameterType::Double, 1.0 ) );

  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Filtered (radius algorithm)" ) ) );
}

QStringList QgsPdalFilterNoiseRadiusAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  const double minK = parameterAsDouble( parameters, QStringLiteral( "MIN_K" ), context );
  const double radius = parameterAsDouble( parameters, QStringLiteral( "RADIUS" ), context );

  QString removeNoisePoints = "false";
  if ( parameterAsBoolean( parameters, QStringLiteral( "REMOVE_NOISE_POINTS" ), context ) )
  {
    removeNoisePoints = "true";
  }

  QStringList args = { QStringLiteral( "filter_noise" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--algorithm=radius" ), QStringLiteral( "--remove-noise-points=%1" ).arg( removeNoisePoints ), QStringLiteral( "--radius-min-k=%1" ).arg( minK ), QStringLiteral( "--radius-radius=%1" ).arg( radius ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
