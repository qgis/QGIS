/***************************************************************************
                         qgsalgorithmnultitingconstantbuffer.cpp
                         --------------------------
    begin                : February 2018
    copyright            : (C) 2018 by Alexander Bruy
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

#include "qgsalgorithmmultiringconstantbuffer.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsMultiRingConstantBufferAlgorithm::name() const
{
  return QStringLiteral( "multiringconstantbuffer" );
}

QString QgsMultiRingConstantBufferAlgorithm::displayName() const
{
  return QObject::tr( "Multi-ring buffer (constant distance)" );
}

QStringList QgsMultiRingConstantBufferAlgorithm::tags() const
{
  return QObject::tr( "buffer,grow,multiple,rings,distance,donut" ).split( ',' );
}

QString QgsMultiRingConstantBufferAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsMultiRingConstantBufferAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsMultiRingConstantBufferAlgorithm::outputName() const
{
  return QObject::tr( "Multi-ring buffer (constant distance)" );
}

QString QgsMultiRingConstantBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes multi-ring ('donuts') buffer for all the features in an input layer, using a fixed or dynamic distance and rings number." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsMultiRingConstantBufferAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsMultiRingConstantBufferAlgorithm *QgsMultiRingConstantBufferAlgorithm::createInstance() const
{
  return new QgsMultiRingConstantBufferAlgorithm();
}

void QgsMultiRingConstantBufferAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr<QgsProcessingParameterNumber> rings = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "RINGS" ), QObject::tr( "Number of rings" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 0 );
  rings->setIsDynamic( true );
  rings->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "RINGS" ), QObject::tr( "Number of rings" ), QgsPropertyDefinition::IntegerPositive ) );
  rings->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( rings.release() );

  std::unique_ptr<QgsProcessingParameterDistance> distance = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance between rings" ), 1, QStringLiteral( "INPUT" ), false );
  distance->setIsDynamic( true );
  distance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance between rings" ), QgsPropertyDefinition::DoublePositive ) );
  distance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( distance.release() );
}

bool QgsMultiRingConstantBufferAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( l );
  if ( !layer )
    return false;

  if ( !QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;
  // Polygons only
  return layer->wkbType() == Qgis::WkbType::Polygon || layer->wkbType() == Qgis::WkbType::MultiPolygon;
}

bool QgsMultiRingConstantBufferAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mRingsNumber = parameterAsInt( parameters, QStringLiteral( "RINGS" ), context );
  mDynamicRingsNumber = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "RINGS" ) );
  if ( mDynamicRingsNumber )
    mRingsNumberProperty = parameters.value( QStringLiteral( "RINGS" ) ).value<QgsProperty>();

  mDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicDistance )
    mDistanceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value<QgsProperty>();

  return true;
}

QgsFields QgsMultiRingConstantBufferAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "ringId" ), QMetaType::Type::Int, QString(), 10, 0 ) );
  fields.append( QgsField( QStringLiteral( "distance" ), QMetaType::Type::Double, QString(), 20, 6 ) );
  return fields;
}

Qgis::ProcessingFeatureSourceFlags QgsMultiRingConstantBufferAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QgsFeatureSink::SinkFlags QgsMultiRingConstantBufferAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

QgsFeatureList QgsMultiRingConstantBufferAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  double currentDistance = 0;
  QgsGeometry outputGeometry, previousGeometry;

  int rings = mRingsNumber;
  if ( mDynamicRingsNumber )
    rings = mRingsNumberProperty.valueAsInt( context.expressionContext(), rings );

  double distance = mDistance;
  if ( mDynamicDistance )
    distance = mDistanceProperty.valueAsDouble( context.expressionContext(), distance );

  QgsFeatureList outputs;

  // Set previous geometry to a zero-distance buffer of the original geometry,
  // this is needed for negative distance values
  previousGeometry = feature.geometry().buffer( 0.0, 40 );
  previousGeometry.convertToMultiType();

  for ( int i = 1; i <= rings; ++i )
  {
    QgsFeature out;
    currentDistance = i * distance;
    outputGeometry = feature.geometry().buffer( currentDistance, 40 );
    outputGeometry.convertToMultiType();
    if ( outputGeometry.isNull() )
    {
      feedback->reportError( QObject::tr( "Error calculating buffer for feature %1" ).arg( feature.id() ) );
      continue;
    }

    if ( distance < 0.0 )
    {
      out.setGeometry( previousGeometry.symDifference( outputGeometry ) );
    }
    else if ( i == 1 )
    {
      out.setGeometry( outputGeometry );
    }
    else
    {
      out.setGeometry( outputGeometry.symDifference( previousGeometry ) );
    }
    previousGeometry = outputGeometry;
    QgsAttributes attrs = feature.attributes();
    attrs << i << currentDistance;
    out.setAttributes( attrs );
    outputs.append( out );
  }

  // For negative distance values, the last generated buffer geometry needs to be added
  if ( distance < 0.0 )
  {
    QgsFeature out;
    out.setGeometry( previousGeometry );
    QgsAttributes attrs = feature.attributes();
    attrs << 0 << 0.0;
    out.setAttributes( attrs );
    outputs.append( out );
  }

  return outputs;
}

///@endcond
