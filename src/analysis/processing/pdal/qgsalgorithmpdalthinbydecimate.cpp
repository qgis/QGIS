/***************************************************************************
                         qgsalgorithmpdalthinbydecimate.cpp
                         ---------------------
    begin                : May 2023
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

#include "qgsalgorithmpdalthinbydecimate.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalThinByDecimateAlgorithm::name() const
{
  return u"thinbydecimate"_s;
}

QString QgsPdalThinByDecimateAlgorithm::displayName() const
{
  return QObject::tr( "Thin (by skipping points)" );
}

QString QgsPdalThinByDecimateAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalThinByDecimateAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalThinByDecimateAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,thin,reduce,decrease,size,decimate,skip" ).split( ',' );
}

QString QgsPdalThinByDecimateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a thinned version of the point cloud by keeping only every N-th point." );
}

QString QgsPdalThinByDecimateAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a thinned version of a point cloud by keeping only every N-th point." );
}

QgsPdalThinByDecimateAlgorithm *QgsPdalThinByDecimateAlgorithm::createInstance() const
{
  return new QgsPdalThinByDecimateAlgorithm();
}

void QgsPdalThinByDecimateAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( u"POINTS_NUMBER"_s, QObject::tr( "Number of points to skip" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1 ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Thinned (by decimation)" ) ) );
}

QStringList QgsPdalThinByDecimateAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  int step = parameterAsInt( parameters, u"POINTS_NUMBER"_s, context );

  QStringList args = { u"thin"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--mode=every-nth"_s, u"--step-every-nth=%1"_s.arg( step ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
