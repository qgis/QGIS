/***************************************************************************
                         qgsalgorithmpdalfilternoisestatistical.cpp
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

#include "qgsalgorithmpdalfilternoisestatistical.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalFilterNoiseStatisticalAlgorithm::name() const
{
  return u"filternoisestatistical"_s;
}

QString QgsPdalFilterNoiseStatisticalAlgorithm::displayName() const
{
  return QObject::tr( "Filter noise" );
}

QString QgsPdalFilterNoiseStatisticalAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalFilterNoiseStatisticalAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalFilterNoiseStatisticalAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,filter,noise,statistical" ).split( ',' );
}

QString QgsPdalFilterNoiseStatisticalAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters noise in a point cloud using a statistical outlier removal algorithm." )
         + u"\n\n"_s
         + QObject::tr( "For each point, the algorithm computes the mean distance to its K nearest neighbors. Points whose mean distance exceeds a threshold (mean distance + multiplier × standard deviation) are classified as noise." );
}

QString QgsPdalFilterNoiseStatisticalAlgorithm::shortDescription() const
{
  return QObject::tr( "Filters noise in a point cloud using a statistical outlier removal algorithm." );
}

QgsPdalFilterNoiseStatisticalAlgorithm *QgsPdalFilterNoiseStatisticalAlgorithm::createInstance() const
{
  return new QgsPdalFilterNoiseStatisticalAlgorithm();
}

void QgsPdalFilterNoiseStatisticalAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"REMOVE_NOISE_POINTS"_s, QObject::tr( "Remove noise points" ), false ) );

  addParameter( new QgsProcessingParameterNumber( u"MEAN_K"_s, QObject::tr( "Mean number of neighbors" ), Qgis::ProcessingNumberParameterType::Integer, 8 ) );
  addParameter( new QgsProcessingParameterNumber( u"MULTIPLIER"_s, QObject::tr( "Standard deviation multiplier" ), Qgis::ProcessingNumberParameterType::Double, 2.0 ) );

  createVpcOutputFormatParameter();

  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Filtered (statistical algorithm)" ) ) );
}

QStringList QgsPdalFilterNoiseStatisticalAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  const int meanK = parameterAsInt( parameters, u"MEAN_K"_s, context );
  const double multiplier = parameterAsDouble( parameters, u"MULTIPLIER"_s, context );

  QString removeNoisePoints = "false";
  if ( parameterAsBoolean( parameters, u"REMOVE_NOISE_POINTS"_s, context ) )
  {
    removeNoisePoints = "true";
  }

  QStringList args = { u"filter_noise"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--algorithm=statistical"_s, u"--remove-noise-points=%1"_s.arg( removeNoisePoints ), u"--statistical-mean-k=%1"_s.arg( meanK ), u"--statistical-multiplier=%1"_s.arg( multiplier ) };

  applyVpcOutputFormatParameter( outputFile, args, parameters, context, feedback );
  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
