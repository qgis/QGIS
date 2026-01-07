/***************************************************************************
                         qgsalgorithmpdalthinbyradius.cpp
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

#include "qgsalgorithmpdalthinbyradius.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalThinByRadiusAlgorithm::name() const
{
  return u"thinbyradius"_s;
}

QString QgsPdalThinByRadiusAlgorithm::displayName() const
{
  return QObject::tr( "Thin (by sampling radius)" );
}

QString QgsPdalThinByRadiusAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalThinByRadiusAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalThinByRadiusAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,thin,reduce,decrease,size,sampling,radius" ).split( ',' );
}

QString QgsPdalThinByRadiusAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a thinned version of the point cloud by performing sampling by distance point." );
}

QString QgsPdalThinByRadiusAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a thinned version of a point cloud by performing sampling by distance point." );
}

QgsPdalThinByRadiusAlgorithm *QgsPdalThinByRadiusAlgorithm::createInstance() const
{
  return new QgsPdalThinByRadiusAlgorithm();
}

void QgsPdalThinByRadiusAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( u"SAMPLING_RADIUS"_s, QObject::tr( "Sampling radius (in map units)" ), Qgis::ProcessingNumberParameterType::Double, 1.0, false, 1e-9 ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Thinned (by radius)" ) ) );
}

QStringList QgsPdalThinByRadiusAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  double step = parameterAsDouble( parameters, u"SAMPLING_RADIUS"_s, context );

  QStringList args = { u"thin"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--mode=sample"_s, u"--step-sample=%1"_s.arg( step ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
