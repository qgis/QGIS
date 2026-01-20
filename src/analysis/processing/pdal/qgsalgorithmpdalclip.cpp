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

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

QString QgsPdalClipAlgorithm::name() const
{
  return u"clip"_s;
}

QString QgsPdalClipAlgorithm::displayName() const
{
  return QObject::tr( "Clip point cloud" );
}

QString QgsPdalClipAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalClipAlgorithm::groupId() const
{
  return u"pointclouddatamanagement"_s;
}

QStringList QgsPdalClipAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,clip,intersect,intersection,mask" ).split( ',' );
}

QString QgsPdalClipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm clips point cloud with clipping polygons, the resulting point cloud contains points that are inside these polygons." );
}

QString QgsPdalClipAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a point cloud layer with points intersecting clipping polygons." );
}

QgsPdalClipAlgorithm *QgsPdalClipAlgorithm::createInstance() const
{
  return new QgsPdalClipAlgorithm();
}

void QgsPdalClipAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterVectorLayer( u"OVERLAY"_s, QObject::tr( "Clipping polygons" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  createCommonParameters();
  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Clipped" ) ) );
}

QStringList QgsPdalClipAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  QString overlayPath = parameterAsCompatibleSourceLayerPath( parameters, u"OVERLAY"_s, context, QgsVectorFileWriter::supportedFormatExtensions(), QgsVectorFileWriter::supportedFormatExtensions()[0], feedback );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  QStringList args = { u"clip"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ), u"--polygon=%1"_s.arg( overlayPath ) };

  applyCommonParameters( args, layer->crs(), parameters, context );
  applyThreadsParameter( args, context );
  return args;
}

///@endcond
