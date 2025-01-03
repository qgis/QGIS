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

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalConvertFormatAlgorithm::name() const
{
  return QStringLiteral( "convertformat" );
}

QString QgsPdalConvertFormatAlgorithm::displayName() const
{
  return QObject::tr( "Convert format" );
}

QString QgsPdalConvertFormatAlgorithm::group() const
{
  return QObject::tr( "Point cloud conversion" );
}

QString QgsPdalConvertFormatAlgorithm::groupId() const
{
  return QStringLiteral( "pointcloudconversion" );
}

QStringList QgsPdalConvertFormatAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,las,laz,format,convert,translate" ).split( ',' );
}

QString QgsPdalConvertFormatAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm converts point cloud to a different file format, e. g. creates compressed LAZ." );
}

QgsPdalConvertFormatAlgorithm *QgsPdalConvertFormatAlgorithm::createInstance() const
{
  return new QgsPdalConvertFormatAlgorithm();
}

void QgsPdalConvertFormatAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Converted" ) ) );
}

QStringList QgsPdalConvertFormatAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputName = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QString outputFile = fixOutputFileName( layer->source(), outputName, context );
  checkOutputFormat( layer->source(), outputFile );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = { QStringLiteral( "translate" ), QStringLiteral( "--input=%1" ).arg( layer->source() ), QStringLiteral( "--output=%1" ).arg( outputFile ) };

  applyThreadsParameter( args, context );
  return args;
}

///@endcond
