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

bool QgsCentroidAlgorithm::run( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback, QVariantMap &outputs ) const
{
  Q_UNUSED( outputs );
  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( QgsProcessingParameters::parameterAsLayer( parameters, QStringLiteral( "INPUT_LAYER" ), context ) );
  if ( !layer )
    return false;

  QString dest;
  QgsVectorLayer *outputLayer = nullptr;
  std::unique_ptr< QgsFeatureSink > writer( QgsProcessingUtils::createFeatureSink( dest, QString(), layer->fields(), QgsWkbTypes::Point, layer->crs(), context, outputLayer ) );

  QgsFeature f;
  QgsFeatureIterator it = QgsProcessingUtils::getFeatures( layer, context );

  double step = 100.0 / QgsProcessingUtils::featureCount( layer, context );
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

  return true;
}

bool QgsBufferAlgorithm::run( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback, QVariantMap &outputs ) const
{
  Q_UNUSED( outputs );

  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( QgsProcessingParameters::parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !layer )
    return false;

  QString dest;
  QgsVectorLayer *outputLayer = nullptr;
  std::unique_ptr< QgsFeatureSink > writer( QgsProcessingUtils::createFeatureSink( dest, QString(), layer->fields(), QgsWkbTypes::Point, layer->crs(), context, outputLayer ) );

  // fixed parameters
  bool dissolve = QgsProcessingParameters::parameterAsBool( parameters, QStringLiteral( "DISSOLVE" ), context );
  int segments = QgsProcessingParameters::parameterAsInt( parameters, QStringLiteral( "DISSOLVE" ), context );
  QgsGeometry::EndCapStyle endCapStyle = static_cast< QgsGeometry::EndCapStyle >( QgsProcessingParameters::parameterAsInt( parameters, QStringLiteral( "END_CAP_STYLE" ), context ) );
  QgsGeometry::JoinStyle joinStyle = static_cast< QgsGeometry::JoinStyle>( QgsProcessingParameters::parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  double miterLimit = QgsProcessingParameters::parameterAsDouble( parameters, QStringLiteral( "MITRE_LIMIT" ), context );
  double bufferDistance = QgsProcessingParameters::parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  bool dynamicBuffer = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );

  QgsFeature f;
  QgsFeatureIterator it = QgsProcessingUtils::getFeatures( layer, context );

  double step = 100.0 / QgsProcessingUtils::featureCount( layer, context );
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
        bufferDistance = QgsProcessingParameters::parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
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

  return true;
}
