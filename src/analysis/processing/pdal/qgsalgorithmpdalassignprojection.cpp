/***************************************************************************
                         qgsalgorithmpdalassignprojection.cpp
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

#include "qgsalgorithmpdalassignprojection.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalAssignProjectionAlgorithm::name() const
{
  return QStringLiteral( "assignprojection" );
}

QString QgsPdalAssignProjectionAlgorithm::displayName() const
{
  return QObject::tr( "Assign projection" );
}

QString QgsPdalAssignProjectionAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalAssignProjectionAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalAssignProjectionAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,assign,set,fix,crs,srs" ).split( ',' );
}

QString QgsPdalAssignProjectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm assigns point cloud CRS if it is not present or wrong." );
}

QgsPdalAssignProjectionAlgorithm *QgsPdalAssignProjectionAlgorithm::createInstance() const
{
  return new QgsPdalAssignProjectionAlgorithm();
}

void QgsPdalAssignProjectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Desired CRS" ), QStringLiteral( "EPSG:4326" ) ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ) ) );
}

QStringList QgsPdalAssignProjectionAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );

  QStringList args = { QStringLiteral( "translate" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--assign-crs=%1" ).arg( crs.authid() ) };

  applyThreadsParameter( args, context );
  return args;
}

///@endcond
