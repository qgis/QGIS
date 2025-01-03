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

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalThinByRadiusAlgorithm::name() const
{
  return QStringLiteral( "thinbyradius" );
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
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalThinByRadiusAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,thin,reduce,decrease,size,sampling,radius" ).split( ',' );
}

QString QgsPdalThinByRadiusAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a thinned version of the point cloud by performing sampling by distance point." );
}

QgsPdalThinByRadiusAlgorithm *QgsPdalThinByRadiusAlgorithm::createInstance() const
{
  return new QgsPdalThinByRadiusAlgorithm();
}

void QgsPdalThinByRadiusAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SAMPLING_RADIUS" ), QObject::tr( "Sampling radius (in map units)" ), Qgis::ProcessingNumberParameterType::Double, 1.0, false, 1e-9 ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Thinned (by radius)" ) ) );
}

QStringList QgsPdalThinByRadiusAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  double step = parameterAsDouble( parameters, QStringLiteral( "SAMPLING_RADIUS" ), context );

  QStringList args = { QStringLiteral( "thin" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--mode=sample" ), QStringLiteral( "--step-sample=%1" ).arg( step ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
