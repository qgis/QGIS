/***************************************************************************
                         qgsalgorithmpdalfilter.cpp
                         ---------------------
    begin                : March 2023
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

#include "qgsalgorithmpdalfilter.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalFilterAlgorithm::name() const
{
  return QStringLiteral( "filter" );
}

QString QgsPdalFilterAlgorithm::displayName() const
{
  return QObject::tr( "Filter" );
}

QString QgsPdalFilterAlgorithm::group() const
{
  return QObject::tr( "Point cloud extraction" );
}

QString QgsPdalFilterAlgorithm::groupId() const
{
  return QStringLiteral( "pointcloudextraction" );
}

QStringList QgsPdalFilterAlgorithm::tags() const
{
  return QObject::tr( "filter,subset,extract,dimension,attribute" ).split( ',' );
}

QString QgsPdalFilterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts point from the input point cloud which match PDAL expression." );
}

QgsPdalFilterAlgorithm *QgsPdalFilterAlgorithm::createInstance() const
{
  return new QgsPdalFilterAlgorithm();
}

void QgsPdalFilterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "FILTER" ), QObject::tr( "Filter expression" ) ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output" ) ) );
}

QStringList QgsPdalFilterAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = { QStringLiteral( "translate" ),
                       QStringLiteral( "--input=%1" ).arg( layer->source() ),
                       QStringLiteral( "--filter=%1" ).arg( parameterAsString( parameters, QStringLiteral( "FILTER" ), context ) ),
                       QStringLiteral( "--output=%1" ).arg( outputFile )
                     };

  applyThreadsParameter( args );
  return args;
}

///@endcond
