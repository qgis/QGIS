/***************************************************************************
                         qgsalgorithmpdalclip.cpp
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

#include "qgsalgorithmpdalclip.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsPdalClipAlgorithm::name() const
{
  return QStringLiteral( "clip" );
}

QString QgsPdalClipAlgorithm::displayName() const
{
  return QObject::tr( "Clip" );
}

QString QgsPdalClipAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalClipAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalClipAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,clip,intersect,intersection,mask" ).split( ',' );
}

QString QgsPdalClipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm clips point cloud with clipping polygons, the resulting point cloud contains points that are inside these polygons." );
}

QgsPdalClipAlgorithm *QgsPdalClipAlgorithm::createInstance() const
{
  return new QgsPdalClipAlgorithm();
}

void QgsPdalClipAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "OVERLAY" ), QObject::tr( "Clipping polygons" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Clipped" ) ) );
}

QStringList QgsPdalClipAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  QString overlayPath = parameterAsCompatibleSourceLayerPath( parameters, QStringLiteral( "OVERLAY" ), context, QgsVectorFileWriter::supportedFormatExtensions(), QgsVectorFileWriter::supportedFormatExtensions()[0], feedback );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = { QStringLiteral( "clip" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ), QStringLiteral( "--polygon=%1" ).arg( overlayPath ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
