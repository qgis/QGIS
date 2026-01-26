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

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalAssignProjectionAlgorithm::name() const
{
  return u"assignprojection"_s;
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
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalAssignProjectionAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,assign,set,fix,crs,srs" ).split( ',' );
}

QString QgsPdalAssignProjectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm assigns point cloud CRS if it is not present or wrong." );
}

QString QgsPdalAssignProjectionAlgorithm::shortDescription() const
{
  return QObject::tr( "Assigns a new CRS to a point cloud, without transforming points." );
}

QgsPdalAssignProjectionAlgorithm *QgsPdalAssignProjectionAlgorithm::createInstance() const
{
  return new QgsPdalAssignProjectionAlgorithm();
}

void QgsPdalAssignProjectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "Desired CRS" ), u"EPSG:4326"_s ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Output layer" ) ) );
}

QStringList QgsPdalAssignProjectionAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"CRS"_s, context );

  QStringList args = { u"translate"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--assign-crs=%1"_s.arg( crs.authid() ) };

  applyThreadsParameter( args, context );
  return args;
}

///@endcond
