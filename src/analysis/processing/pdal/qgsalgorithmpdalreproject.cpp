/***************************************************************************
                         qgsalgorithmpdalreproject.cpp
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

#include "qgsalgorithmpdalreproject.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalReprojectAlgorithm::name() const
{
  return u"reproject"_s;
}

QString QgsPdalReprojectAlgorithm::displayName() const
{
  return QObject::tr( "Reproject point cloud" );
}

QString QgsPdalReprojectAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalReprojectAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalReprojectAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,assign,set,transform,reproject,crs,srs" ).split( ',' );
}

QString QgsPdalReprojectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reprojects point cloud to a different coordinate reference system." );
}

QString QgsPdalReprojectAlgorithm::shortDescription() const
{
  return QObject::tr( "Reprojects a point cloud to a different coordinate reference system." );
}

QgsPdalReprojectAlgorithm *QgsPdalReprojectAlgorithm::createInstance() const
{
  return new QgsPdalReprojectAlgorithm();
}

void QgsPdalReprojectAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "Target CRS" ), u"EPSG:4326"_s ) );

  auto crsOpParam = std::make_unique<QgsProcessingParameterCoordinateOperation>( u"OPERATION"_s, QObject::tr( "Coordinate operation" ), QVariant(), u"INPUT"_s, u"CRS"_s, QVariant(), QVariant(), true );
  crsOpParam->setFlags( crsOpParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( crsOpParam.release() );

  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Reprojected" ) ) );
}

QStringList QgsPdalReprojectAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  QStringList args = { u"translate"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--transform-crs=%1"_s.arg( crs.authid() ) };

  const QString coordOp = parameterAsString( parameters, u"OPERATION"_s, context );
  if ( !coordOp.isEmpty() )
  {
    args << u"--transform-coord-op=%1"_s.arg( coordOp );
  }

  applyThreadsParameter( args, context );
  return args;
}

///@endcond
