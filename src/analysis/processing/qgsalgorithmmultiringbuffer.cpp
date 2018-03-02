/***************************************************************************
                         qgsalgorithmnultitingbuffer.cpp
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

#include "qgsalgorithmmultiringbuffer.h"

///@cond PRIVATE

QString QgsMultiRingBufferAlgorithm::name() const
{
  return QStringLiteral( "multiringbuffer" );
}

QString QgsMultiRingBufferAlgorithm::displayName() const
{
  return QObject::tr( "Multi-ring buffer" );
}

QStringList QgsMultiRingBufferAlgorithm::tags() const
{
  return QObject::tr( "buffer,grow,multiple,rings,distance,donut" ).split( ',' );
}

QString QgsMultiRingBufferAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsMultiRingBufferAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsMultiRingBufferAlgorithm::outputName() const
{
  return QObject::tr( "Multi-ring buffer" );
}

QString QgsMultiRingBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes multi-ring ('donuts') buffer for all the features in an input layer, using a fixed or dynamic distance and rings number." );
}

QgsMultiRingBufferAlgorithm *QgsMultiRingBufferAlgorithm::createInstance() const
{
  return new QgsMultiRingBufferAlgorithm();
}

void QgsMultiRingBufferAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber> rings = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "RINGS" ),
      QObject::tr( "Number of rings" ), QgsProcessingParameterNumber::Integer,
      1, false, 0 );
  rings->setIsDynamic( true );
  rings->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "RINGS" ), QObject::tr( "Number of rings" ), QgsPropertyDefinition::IntegerPositive ) );
  rings->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( rings.release() );

  std::unique_ptr< QgsProcessingParameterNumber> distance = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DISTANCE" ),
      QObject::tr( "Distance between rings" ), QgsProcessingParameterNumber::Double,
      1, false, 0 );
  distance->setIsDynamic( true );
  distance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance between rings" ), QgsPropertyDefinition::DoublePositive ) );
  distance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( distance.release() );
}

bool QgsMultiRingBufferAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mRingsNumber = parameterAsInt( parameters, QStringLiteral( "RINGS" ), context );
  mDynamicRingsNumber = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "RINGS" ) );
  if ( mDynamicRingsNumber )
    mRingsNumberProperty = parameters.value( QStringLiteral( "RINGS" ) ).value< QgsProperty >();

  mDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicDistance )
    mDistanceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();

  return true;
}

QgsFields QgsMultiRingBufferAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "ringId" ), QVariant::Int, QString(), 10, 0 ) );
  fields.append( QgsField( QStringLiteral( "distance" ), QVariant::Double, QString(), 20, 6 ) );
  return fields;
}

QgsFeatureList QgsMultiRingBufferAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
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

  for ( int i = 1; i <= mRingsNumber; ++i )
  {
    QgsFeature out;
    currentDistance = i * distance;
    outputGeometry = feature.geometry().buffer( currentDistance, 40 );
    if ( !outputGeometry )
    {
      feedback->reportError( QObject::tr( "Error calculating buffer for feature %1" ).arg( feature.id() ) );
      continue;
    }

    if ( i == 1 )
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
  return outputs;
}

///@endcond
