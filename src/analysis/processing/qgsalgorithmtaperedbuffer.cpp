/***************************************************************************
                         qgsalgorithmtaperedbuffer.cpp
                         ---------------------
    begin                : March 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmtaperedbuffer.h"

///@cond PRIVATE

QString QgsTaperedBufferAlgorithm::name() const
{
  return QStringLiteral( "taperedbuffer" );
}

QString QgsTaperedBufferAlgorithm::displayName() const
{
  return QObject::tr( "Tapered buffers" );
}

QStringList QgsTaperedBufferAlgorithm::tags() const
{
  return QObject::tr( "variable,distance,length,line,buffer" ).split( ',' );
}

QString QgsTaperedBufferAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTaperedBufferAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsTaperedBufferAlgorithm::outputName() const
{
  return QObject::tr( "Buffered" );
}

QgsProcessing::SourceType QgsTaperedBufferAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsWkbTypes::Type QgsTaperedBufferAlgorithm::outputWkbType( QgsWkbTypes::Type ) const
{
  return QgsWkbTypes::MultiPolygon;
}

bool QgsTaperedBufferAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mStartWidth = parameterAsDouble( parameters, QStringLiteral( "START_WIDTH" ), context );
  mDynamicStartWidth = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "START_WIDTH" ) );
  if ( mDynamicStartWidth )
    mStartWidthProperty = parameters.value( QStringLiteral( "START_WIDTH" ) ).value< QgsProperty >();

  mEndWidth = parameterAsDouble( parameters, QStringLiteral( "END_WIDTH" ), context );
  mDynamicEndWidth = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "END_WIDTH" ) );
  if ( mDynamicEndWidth )
    mEndWidthProperty = parameters.value( QStringLiteral( "END_WIDTH" ) ).value< QgsProperty >();

  mSegments = parameterAsInt( parameters, QStringLiteral( "Segments" ), context );
  mDynamicSegments = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "Segments" ) );
  if ( mDynamicSegments )
    mSegmentsProperty = parameters.value( QStringLiteral( "Segments" ) ).value< QgsProperty >();

  return true;
}

QString QgsTaperedBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates tapered buffers along line geometries, using a specified start and "
                      "end buffer diameter corresponding to the buffer diameter at the start and end of the linestrings." );
}

QList<int> QgsTaperedBufferAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsTaperedBufferAlgorithm *QgsTaperedBufferAlgorithm::createInstance() const
{
  return new QgsTaperedBufferAlgorithm();
}

void QgsTaperedBufferAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > startWidth = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "START_WIDTH" ),
      QObject::tr( "Start width" ), QgsProcessingParameterNumber::Double,
      0.0, false, 0.0 );
  startWidth->setIsDynamic( true );
  startWidth->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "START_WIDTH" ), QObject::tr( "Start width" ), QgsPropertyDefinition::DoublePositive ) );
  startWidth->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( startWidth.release() );

  std::unique_ptr< QgsProcessingParameterNumber > endWidth = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "END_WIDTH" ),
      QObject::tr( "End width" ), QgsProcessingParameterNumber::Double,
      1, false, 0.0 );
  endWidth->setIsDynamic( true );
  endWidth->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "END_WIDTH" ), QObject::tr( "End width" ), QgsPropertyDefinition::DoublePositive ) );
  endWidth->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( endWidth.release() );

  std::unique_ptr< QgsProcessingParameterNumber > segments = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SEGMENTS" ),
      QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer,
      16, false, 1 );
  segments->setIsDynamic( true );
  segments->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  segments->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( segments.release() );
}

QgsFeatureList QgsTaperedBufferAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( !feature.hasGeometry() )
    return QgsFeatureList() << feature;

  QgsFeature f = feature;
  int segments = mSegments;
  if ( mDynamicSegments )
    segments = mSegmentsProperty.valueAsInt( context.expressionContext(), segments );

  double startWidth = mStartWidth;
  if ( mDynamicStartWidth )
    startWidth = mStartWidthProperty.valueAsDouble( context.expressionContext(), startWidth );

  double endWidth = mEndWidth;
  if ( mDynamicEndWidth )
    endWidth = mEndWidthProperty.valueAsDouble( context.expressionContext(), endWidth );

  const QgsGeometry outputGeometry = feature.geometry().taperedBuffer( startWidth, endWidth, segments );
  if ( outputGeometry.isNull() )
  {
    feedback->reportError( QObject::tr( "Error buffering geometry %1: %2" ).arg( feature.id() ).arg( outputGeometry.lastError() ) );
  }
  f.setGeometry( outputGeometry );

  return QgsFeatureList() << f;
}




QString QgsVariableWidthBufferByMAlgorithm::name() const
{
  return QStringLiteral( "bufferbym" );
}

QString QgsVariableWidthBufferByMAlgorithm::displayName() const
{
  return QObject::tr( "Variable width buffer (by M value)" );
}

QStringList QgsVariableWidthBufferByMAlgorithm::tags() const
{
  return QObject::tr( "variable,distance,length,line,buffer" ).split( ',' );
}

QString QgsVariableWidthBufferByMAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsVariableWidthBufferByMAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsVariableWidthBufferByMAlgorithm::outputName() const
{
  return QObject::tr( "Buffered" );
}

QgsProcessing::SourceType QgsVariableWidthBufferByMAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsWkbTypes::Type QgsVariableWidthBufferByMAlgorithm::outputWkbType( QgsWkbTypes::Type ) const
{
  return QgsWkbTypes::MultiPolygon;
}

bool QgsVariableWidthBufferByMAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSegments = parameterAsInt( parameters, QStringLiteral( "Segments" ), context );
  mDynamicSegments = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "Segments" ) );
  if ( mDynamicSegments )
    mSegmentsProperty = parameters.value( QStringLiteral( "Segments" ) ).value< QgsProperty >();

  return true;
}

QString QgsVariableWidthBufferByMAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates variable width buffers along lines, using the M value of the line geometries "
                      "as the diameter of the buffer at each vertex." );
}

QList<int> QgsVariableWidthBufferByMAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsVariableWidthBufferByMAlgorithm *QgsVariableWidthBufferByMAlgorithm::createInstance() const
{
  return new QgsVariableWidthBufferByMAlgorithm();
}

void QgsVariableWidthBufferByMAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > segments = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SEGMENTS" ),
      QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer,
      16, false, 1 );
  segments->setIsDynamic( true );
  segments->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  segments->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( segments.release() );
}

QgsFeatureList QgsVariableWidthBufferByMAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( !feature.hasGeometry() )
    return QgsFeatureList() << feature;

  QgsFeature f = feature;
  int segments = mSegments;
  if ( mDynamicSegments )
    segments = mSegmentsProperty.valueAsInt( context.expressionContext(), segments );

  const QgsGeometry outputGeometry = feature.geometry().variableWidthBufferByM( segments );
  if ( outputGeometry.isNull() )
  {
    feedback->reportError( QObject::tr( "Error buffering geometry %1: %2" ).arg( feature.id() ).arg( outputGeometry.lastError() ) );
  }
  f.setGeometry( outputGeometry );

  return QgsFeatureList() << f;
}
///@endcond


