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

#include "qgspointcloudexpression.h"
#include "qgspointcloudlayer.h"
#include "qgsrunprocess.h"

///@cond PRIVATE

QString QgsPdalFilterAlgorithm::name() const
{
  return u"filter"_s;
}

QString QgsPdalFilterAlgorithm::displayName() const
{
  return QObject::tr( "Filter point cloud" );
}

QString QgsPdalFilterAlgorithm::group() const
{
  return QObject::tr( "Point cloud extraction" );
}

QString QgsPdalFilterAlgorithm::groupId() const
{
  return u"pointcloudextraction"_s;
}

QStringList QgsPdalFilterAlgorithm::tags() const
{
  return QObject::tr( "pdal,lidar,filter,subset,extract,dimension,attribute,extent,bounds,rectangle" ).split( ',' );
}

QString QgsPdalFilterAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts point from the input point cloud which match PDAL expression and/or are inside of a cropping rectangle." );
}

QString QgsPdalFilterAlgorithm::shortDescription() const
{
  return QObject::tr( "Extracts points from the input point cloud which match specific conditions." );
}

QgsPdalFilterAlgorithm *QgsPdalFilterAlgorithm::createInstance() const
{
  return new QgsPdalFilterAlgorithm();
}

void QgsPdalFilterAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExpression( u"FILTER_EXPRESSION"_s, QObject::tr( "Filter expression" ), QVariant(), u"INPUT"_s, false, Qgis::ExpressionType::PointCloud ) );
  addParameter( new QgsProcessingParameterExtent( u"FILTER_EXTENT"_s, QObject::tr( "Cropping extent" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( u"OUTPUT"_s, QObject::tr( "Filtered" ) ) );
}

QStringList QgsPdalFilterAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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


  const QString filterExpression = parameterAsString( parameters, u"FILTER_EXPRESSION"_s, context ).trimmed();
  if ( !filterExpression.isEmpty() )
  {
    QgsPointCloudExpression exp( filterExpression );
    args << u"--filter=%1"_s.arg( exp.asPdalExpression() );
  }

  if ( parameters.value( u"FILTER_EXTENT"_s ).isValid() )
  {
    if ( layer->crs().isValid() )
    {
      const QgsRectangle extent = parameterAsExtent( parameters, u"FILTER_EXTENT"_s, context, layer->crs() );
      args << u"--bounds=([%1, %2], [%3, %4])"_s
                .arg( extent.xMinimum() )
                .arg( extent.xMaximum() )
                .arg( extent.yMinimum() )
                .arg( extent.yMaximum() );
    }
    else
    {
      const QgsRectangle extent = parameterAsExtent( parameters, u"FILTER_EXTENT"_s, context );
      args << u"--bounds=([%1, %2], [%3, %4])"_s
                .arg( extent.xMinimum() )
                .arg( extent.xMaximum() )
                .arg( extent.yMinimum() )
                .arg( extent.yMaximum() );
    }
  }

  applyThreadsParameter( args, context );
  return args;
}

///@endcond
