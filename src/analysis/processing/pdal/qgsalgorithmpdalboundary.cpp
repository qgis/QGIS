/***************************************************************************
                         qgsalgorithmpdalboundary.cpp
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

#include "qgsalgorithmpdalboundary.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalBoundaryAlgorithm::name() const
{
  return QStringLiteral( "boundary" );
}

QString QgsPdalBoundaryAlgorithm::displayName() const
{
  return QObject::tr( "Boundary" );
}

QString QgsPdalBoundaryAlgorithm::group() const
{
  return QObject::tr( "Point cloud extraction" );
}

QString QgsPdalBoundaryAlgorithm::groupId() const
{
  return QStringLiteral( "pointcloudextraction" );
}

QStringList QgsPdalBoundaryAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,extent,envelope,bounds,bounding,boundary,vector" ).split( ',' );
}

QString QgsPdalBoundaryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a polygon file containing point cloud layer boundary. It may contain holes and it may be a multi-part polygon." );
}

QgsPdalBoundaryAlgorithm *QgsPdalBoundaryAlgorithm::createInstance() const
{
  return new QgsPdalBoundaryAlgorithm();
}

void QgsPdalBoundaryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "RESOLUTION" ), QObject::tr( "Resolution of cells used to calculate boundary" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-6 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "THRESHOLD" ), QObject::tr( "Minimal number of points in a cell to consider cell occupied" ), Qgis::ProcessingNumberParameterType::Integer, QVariant(), true, 1 ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterVectorDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Boundary" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QStringList QgsPdalBoundaryAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = { QStringLiteral( "boundary" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ) };

  bool hasResolution = parameters.value( QStringLiteral( "RESOLUTION" ) ).isValid();
  bool hasThreshold = parameters.value( QStringLiteral( "THRESHOLD" ) ).isValid();

  if ( hasThreshold && !hasResolution )
  {
    throw QgsProcessingException( QObject::tr( "Resolution parameter must be set when points threshold is set." ) );
  }

  if ( hasResolution )
  {
    args << QStringLiteral( "--resolution=%1" ).arg( parameterAsDouble( parameters, QStringLiteral( "RESOLUTION" ), context ) );
  }

  if ( hasThreshold )
  {
    args << QStringLiteral( "--threshold=%1" ).arg( parameterAsInt( parameters, QStringLiteral( "THRESHOLD" ), context ) );
  }

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
