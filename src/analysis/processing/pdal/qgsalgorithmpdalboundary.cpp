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

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalBoundaryAlgorithm::name() const
{
  return u"boundary"_s;
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
  return u"pointcloudextraction"_s;
}

QStringList QgsPdalBoundaryAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,extent,envelope,bounds,bounding,boundary,vector" ).split( ',' );
}

QString QgsPdalBoundaryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a polygon file containing point cloud layer boundary. It may contain holes and it may be a multi-part polygon." );
}

QString QgsPdalBoundaryAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a polygon layer containing a point cloud's boundary." );
}

QgsPdalBoundaryAlgorithm *QgsPdalBoundaryAlgorithm::createInstance() const
{
  return new QgsPdalBoundaryAlgorithm();
}

void QgsPdalBoundaryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( u"RESOLUTION"_s, QObject::tr( "Resolution of cells used to calculate boundary" ), Qgis::ProcessingNumberParameterType::Double, QVariant(), true, 1e-6 ) );
  addParameter( new QgsProcessingParameterNumber( u"THRESHOLD"_s, QObject::tr( "Minimal number of points in a cell to consider cell occupied" ), Qgis::ProcessingNumberParameterType::Integer, QVariant(), true, 1 ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterVectorDestination( u"OUTPUT"_s, QObject::tr( "Boundary" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QStringList QgsPdalBoundaryAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  setOutputValue( u"OUTPUT"_s, outputFile );

  QStringList args = { u"boundary"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ) };

  bool hasResolution = parameters.value( u"RESOLUTION"_s ).isValid();
  bool hasThreshold = parameters.value( u"THRESHOLD"_s ).isValid();

  if ( hasThreshold && !hasResolution )
  {
    throw QgsProcessingException( QObject::tr( "Resolution parameter must be set when points threshold is set." ) );
  }

  if ( hasResolution )
  {
    args << u"--resolution=%1"_s.arg( parameterAsDouble( parameters, u"RESOLUTION"_s, context ) );
  }

  if ( hasThreshold )
  {
    args << u"--threshold=%1"_s.arg( parameterAsInt( parameters, u"THRESHOLD"_s, context ) );
  }

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
