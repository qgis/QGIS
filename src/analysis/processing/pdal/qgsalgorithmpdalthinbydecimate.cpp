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

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalThinByDecimateAlgorithm::name() const
{
  return QStringLiteral( "thinbydecimate" );
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
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalThinByDecimateAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,thin,reduce,decrease,size,decimate,skip" ).split( ',' );
}

QString QgsPdalThinByDecimateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a thinned version of the point cloud by keeping only every N-th point." );
}

QgsPdalThinByDecimateAlgorithm *QgsPdalThinByDecimateAlgorithm::createInstance() const
{
  return new QgsPdalThinByDecimateAlgorithm();
}

void QgsPdalThinByDecimateAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "POINTS_NUMBER" ), QObject::tr( "Number of points to skip" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1 ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Thinned (by decimation)" ) ) );
}

QStringList QgsPdalThinByDecimateAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  int step = parameterAsInt( parameters, QStringLiteral( "POINTS_NUMBER" ), context );

  QStringList args = { QStringLiteral( "thin" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--mode=every-nth" ), QStringLiteral( "--step-every-nth=%1" ).arg( step ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
