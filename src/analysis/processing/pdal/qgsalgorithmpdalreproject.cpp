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

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalReprojectAlgorithm::name() const
{
  return QStringLiteral( "reproject" );
}

QString QgsPdalReprojectAlgorithm::displayName() const
{
  return QObject::tr( "Reproject" );
}

QString QgsPdalReprojectAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalReprojectAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalReprojectAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,assign,set,transform,reproject,crs,srs" ).split( ',' );
}

QString QgsPdalReprojectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reprojects point cloud to a different coordinate reference system." );
}

QgsPdalReprojectAlgorithm *QgsPdalReprojectAlgorithm::createInstance() const
{
  return new QgsPdalReprojectAlgorithm();
}

void QgsPdalReprojectAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "EPSG:4326" ) ) );

  std::unique_ptr<QgsProcessingParameterCoordinateOperation> crsOpParam = std::make_unique<QgsProcessingParameterCoordinateOperation>( QStringLiteral( "OPERATION" ), QObject::tr( "Coordinate operation" ), QVariant(), QStringLiteral( "INPUT" ), QStringLiteral( "CRS" ), QVariant(), QVariant(), true );
  crsOpParam->setFlags( crsOpParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( crsOpParam.release() );

  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Reprojected" ) ) );
}

QStringList QgsPdalReprojectAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  QStringList args = { QStringLiteral( "translate" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--transform-crs=%1" ).arg( crs.authid() ) };

  const QString coordOp = parameterAsString( parameters, QStringLiteral( "OPERATION" ), context );
  if ( !coordOp.isEmpty() )
  {
    args << QStringLiteral( "--transform-coord-op=%1" ).arg( coordOp );
  }

  applyThreadsParameter( args, context );
  return args;
}

///@endcond
