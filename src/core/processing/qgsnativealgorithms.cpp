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
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Centroids" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Centroids" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
}

QString QgsCentroidAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new point layer, with points representing the centroid of the geometries in an input layer.\n\n"
                      "The attributes associated to each point in the output layer are the same ones associated to the original features." );
}

QVariantMap QgsCentroidAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LAYER" ), context, QString(), layer->fields(), QgsWkbTypes::Point, layer->crs(), dest ) );

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
    sink->addFeature( out );

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
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), QgsProcessingParameterNumber::Double, 10 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer, 5, false, 1 ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "END_CAP_STYLE" ), QObject::tr( "End cap style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Flat" ) << QObject::tr( "Square" ), false ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "JOIN_STYLE" ), QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MITRE_LIMIT" ), QObject::tr( "Miter limit" ), QgsProcessingParameterNumber::Double, 2, false, 1 ) );

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DISSOLVE" ), QObject::tr( "Dissolve result" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Buffered" ), QgsProcessingParameterDefinition::TypeVectorPolygon ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Buffered" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
}

QString QgsBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a buffer area for all the features in an input layer, using a fixed or dynamic distance.\n\n"
                      "The segments parameter controls the number of line segments to use to approximate a quarter circle when creating rounded offsets.\n\n"
                      "The end cap style parameter controls how line endings are handled in the buffer.\n\n"
                      "The join style parameter specifies whether round, mitre or beveled joins should be used when offsetting corners in a line.\n\n"
                      "The mitre limit parameter is only applicable for mitre join styles, and controls the maximum distance from the offset curve to use when creating a mitred join." );
}

QVariantMap QgsBufferAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LAYER" ), context, QString(), layer->fields(), QgsWkbTypes::Polygon, layer->crs(), dest ) );

  // fixed parameters
  //bool dissolve = QgsProcessingParameters::parameterAsBool( parameters, QStringLiteral( "DISSOLVE" ), context );
  int segments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  QgsGeometry::EndCapStyle endCapStyle = static_cast< QgsGeometry::EndCapStyle >( 1 + parameterAsInt( parameters, QStringLiteral( "END_CAP_STYLE" ), context ) );
  QgsGeometry::JoinStyle joinStyle = static_cast< QgsGeometry::JoinStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
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
        bufferDistance = QgsProcessingParameters::parameterAsDouble( distanceParamDef, parameters, context );
      }

      out.setGeometry( f.geometry().buffer( bufferDistance, segments, endCapStyle, joinStyle, miterLimit ) );
      if ( !out.geometry() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating buffer for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), QgsMessageLog::WARNING );
      }
    }
    sink->addFeature( out );

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT_LAYER" ), dest );
  return outputs;
}

///@endcond
