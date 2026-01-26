/***************************************************************************
                         qgsalgorithmpdalconvertformat.cpp
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

#include "qgsalgorithmpdalconvertformat.h"

#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalConvertFormatAlgorithm::name() const
{
  return u"convertformat"_s;
}

QString QgsPdalConvertFormatAlgorithm::displayName() const
{
  return QObject::tr( "Convert point cloud format" );
}

QString QgsPdalConvertFormatAlgorithm::group() const
{
  return QObject::tr( "Point cloud conversion" );
}

QString QgsPdalConvertFormatAlgorithm::groupId() const
{
  return u"pointcloudconversion"_s;
}

QStringList QgsPdalConvertFormatAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,las,laz,format,convert,translate" ).split( ',' );
}

QString QgsPdalConvertFormatAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm converts point cloud to a different file format, e. g. creates compressed LAZ." );
}

QString QgsPdalConvertFormatAlgorithm::shortDescription() const
{
  return QObject::tr( "Converts a point cloud to a different file format, e.g. creates compressed LAZ." );
}

QgsPdalConvertFormatAlgorithm *QgsPdalConvertFormatAlgorithm::createInstance() const
{
  return new QgsPdalConvertFormatAlgorithm();
}

void QgsPdalConvertFormatAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Converted" ) ) );
}

QStringList QgsPdalConvertFormatAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, u"INPUT"_s, context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, u"INPUT"_s ) );

  const QString outputName = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( u"OUTPUT"_s, outputFile );

  QStringList args = { u"translate"_s, u"--input=%1"_s.arg( layer->source() ), u"--output=%1"_s.arg( outputFile ) };

  applyThreadsParameter( args, context );
  return args;
}

///@endcond
