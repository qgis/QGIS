/***************************************************************************
                         qgsnativealgorithms.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnativealgorithms.h"
#include "qgsfeatureiterator.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingutils.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgswkbtypes.h"

///@cond PRIVATE

QgsNativeAlgorithms::QgsNativeAlgorithms( QObject *parent )
  : QgsProcessingProvider( parent )
{}

QIcon QgsNativeAlgorithms::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/providerQgis.svg" ) );
}

QString QgsNativeAlgorithms::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "providerQgis.svg" ) );
}

QString QgsNativeAlgorithms::id() const
{
  return QStringLiteral( "native" );
}

QString QgsNativeAlgorithms::name() const
{
  return tr( "QGIS" );
}

bool QgsNativeAlgorithms::supportsNonFileBasedOutput() const
{
  return true;
}

void QgsNativeAlgorithms::loadAlgorithms()
{
  addAlgorithm( new QgsCentroidAlgorithm() );
  addAlgorithm( new QgsBufferAlgorithm() );
}



QgsCentroidAlgorithm::QgsCentroidAlgorithm()
{
  addParameter( new QgsProcessingParameterVector( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
}

QVariantMap QgsCentroidAlgorithm::run( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !layer )
    return QVariantMap();

  QString dest = parameterAsString( parameters, QStringLiteral( "OUTPUT_LAYER" ), context );
  std::unique_ptr< QgsFeatureSink > writer( QgsProcessingUtils::createFeatureSink( dest, QString(), layer->fields(), QgsWkbTypes::Point, layer->crs(), context ) );

  long count = QgsProcessingUtils::featureCount( layer, context );
  if ( count <= 0 )
    return QVariantMap();

  QgsFeature f;
  QgsFeatureIterator it = QgsProcessingUtils::getFeatures( layer, context );

  double step = 100.0 / count;
  int current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeature out = f;
    if ( out.hasGeometry() )
    {
      out.setGeometry( f.geometry().centroid() );
      if ( !out.geometry() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating centroid for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), QgsMessageLog::WARNING );
      }
    }
    writer->addFeature( out );

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT_LAYER" ), dest );
  return outputs;
}

//
// QgsBufferAlgorithm
//

QgsBufferAlgorithm::QgsBufferAlgorithm()
{
  addParameter( new QgsProcessingParameterVector( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), QgsProcessingParameterNumber::Double, 10 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer, 5, false, 1 ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DISSOLVE" ), QObject::tr( "Dissolve result" ), false ) );
}

QVariantMap QgsBufferAlgorithm::run( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !layer )
    return QVariantMap();

  QString dest = parameterAsString( parameters, QStringLiteral( "OUTPUT_LAYER" ), context );
  std::unique_ptr< QgsFeatureSink > writer( QgsProcessingUtils::createFeatureSink( dest, QString(), layer->fields(), QgsWkbTypes::Point, layer->crs(), context ) );

  // fixed parameters
  //bool dissolve = QgsProcessingParameters::parameterAsBool( parameters, QStringLiteral( "DISSOLVE" ), context );
  int segments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  QgsGeometry::EndCapStyle endCapStyle = static_cast< QgsGeometry::EndCapStyle >( parameterAsInt( parameters, QStringLiteral( "END_CAP_STYLE" ), context ) );
  QgsGeometry::JoinStyle joinStyle = static_cast< QgsGeometry::JoinStyle>( parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  double miterLimit = parameterAsDouble( parameters, QStringLiteral( "MITRE_LIMIT" ), context );
  double bufferDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  bool dynamicBuffer = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  const QgsProcessingParameterDefinition *distanceParamDef = parameterDefinition( QStringLiteral( "DISTANCE" ) );

  long count = QgsProcessingUtils::featureCount( layer, context );
  if ( count <= 0 )
    return QVariantMap();

  QgsFeature f;
  QgsFeatureIterator it = QgsProcessingUtils::getFeatures( layer, context );

  double step = 100.0 / count;
  int current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeature out = f;
    if ( out.hasGeometry() )
    {
      if ( dynamicBuffer )
      {
        context.expressionContext().setFeature( f );
        bufferDistance = QgsProcessingParameters::parameterAsDouble( distanceParamDef, parameters, QStringLiteral( "DISTANCE" ), context );
      }

      out.setGeometry( f.geometry().buffer( bufferDistance, segments, endCapStyle, joinStyle, miterLimit ) );
      if ( !out.geometry() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating buffer for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), QgsMessageLog::WARNING );
      }
    }
    writer->addFeature( out );

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT_LAYER" ), dest );
  return outputs;
}

///@endcond
