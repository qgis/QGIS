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
  return u"taperedbuffer"_s;
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
  return u"vectorgeometry"_s;
}

QString QgsTaperedBufferAlgorithm::outputName() const
{
  return QObject::tr( "Buffered" );
}

Qgis::ProcessingSourceType QgsTaperedBufferAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

Qgis::WkbType QgsTaperedBufferAlgorithm::outputWkbType( Qgis::WkbType ) const
{
  return Qgis::WkbType::MultiPolygon;
}

bool QgsTaperedBufferAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mStartWidth = parameterAsDouble( parameters, u"START_WIDTH"_s, context );
  mDynamicStartWidth = QgsProcessingParameters::isDynamic( parameters, u"START_WIDTH"_s );
  if ( mDynamicStartWidth )
    mStartWidthProperty = parameters.value( u"START_WIDTH"_s ).value<QgsProperty>();

  mEndWidth = parameterAsDouble( parameters, u"END_WIDTH"_s, context );
  mDynamicEndWidth = QgsProcessingParameters::isDynamic( parameters, u"END_WIDTH"_s );
  if ( mDynamicEndWidth )
    mEndWidthProperty = parameters.value( u"END_WIDTH"_s ).value<QgsProperty>();

  mSegments = parameterAsInt( parameters, u"Segments"_s, context );
  mDynamicSegments = QgsProcessingParameters::isDynamic( parameters, u"Segments"_s );
  if ( mDynamicSegments )
    mSegmentsProperty = parameters.value( u"Segments"_s ).value<QgsProperty>();

  return true;
}

QString QgsTaperedBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates tapered buffers along line geometries, using a specified start and "
                      "end buffer diameter corresponding to the buffer diameter at the start and end of the linestrings." );
}

QString QgsTaperedBufferAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates tapered buffers along line geometries." );
}

QList<int> QgsTaperedBufferAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

QgsTaperedBufferAlgorithm *QgsTaperedBufferAlgorithm::createInstance() const
{
  return new QgsTaperedBufferAlgorithm();
}

void QgsTaperedBufferAlgorithm::initParameters( const QVariantMap & )
{
  auto startWidth = std::make_unique<QgsProcessingParameterNumber>( u"START_WIDTH"_s, QObject::tr( "Start width" ), Qgis::ProcessingNumberParameterType::Double, 0.0, false, 0.0 );
  startWidth->setIsDynamic( true );
  startWidth->setDynamicPropertyDefinition( QgsPropertyDefinition( u"START_WIDTH"_s, QObject::tr( "Start width" ), QgsPropertyDefinition::DoublePositive ) );
  startWidth->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( startWidth.release() );

  auto endWidth = std::make_unique<QgsProcessingParameterNumber>( u"END_WIDTH"_s, QObject::tr( "End width" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0.0 );
  endWidth->setIsDynamic( true );
  endWidth->setDynamicPropertyDefinition( QgsPropertyDefinition( u"END_WIDTH"_s, QObject::tr( "End width" ), QgsPropertyDefinition::DoublePositive ) );
  endWidth->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( endWidth.release() );

  auto segments = std::make_unique<QgsProcessingParameterNumber>( u"SEGMENTS"_s, QObject::tr( "Segments" ), Qgis::ProcessingNumberParameterType::Integer, 16, false, 1 );
  segments->setIsDynamic( true );
  segments->setDynamicPropertyDefinition( QgsPropertyDefinition( u"SEGMENTS"_s, QObject::tr( "Segments" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  segments->setDynamicLayerParameterName( u"INPUT"_s );
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
  return u"bufferbym"_s;
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
  return u"vectorgeometry"_s;
}

QString QgsVariableWidthBufferByMAlgorithm::outputName() const
{
  return QObject::tr( "Buffered" );
}

Qgis::ProcessingSourceType QgsVariableWidthBufferByMAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

Qgis::WkbType QgsVariableWidthBufferByMAlgorithm::outputWkbType( Qgis::WkbType ) const
{
  return Qgis::WkbType::MultiPolygon;
}

bool QgsVariableWidthBufferByMAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSegments = parameterAsInt( parameters, u"Segments"_s, context );
  mDynamicSegments = QgsProcessingParameters::isDynamic( parameters, u"Segments"_s );
  if ( mDynamicSegments )
    mSegmentsProperty = parameters.value( u"Segments"_s ).value<QgsProperty>();

  return true;
}

QString QgsVariableWidthBufferByMAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates variable width buffers along lines, using the M value of the line geometries "
                      "as the diameter of the buffer at each vertex." );
}

QString QgsVariableWidthBufferByMAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates variable width buffers along lines, using the M value of the line geometries "
                      "as the diameter of the buffer at each vertex." );
}

QList<int> QgsVariableWidthBufferByMAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine );
}

QgsVariableWidthBufferByMAlgorithm *QgsVariableWidthBufferByMAlgorithm::createInstance() const
{
  return new QgsVariableWidthBufferByMAlgorithm();
}

void QgsVariableWidthBufferByMAlgorithm::initParameters( const QVariantMap & )
{
  auto segments = std::make_unique<QgsProcessingParameterNumber>( u"SEGMENTS"_s, QObject::tr( "Segments" ), Qgis::ProcessingNumberParameterType::Integer, 16, false, 1 );
  segments->setIsDynamic( true );
  segments->setDynamicPropertyDefinition( QgsPropertyDefinition( u"SEGMENTS"_s, QObject::tr( "Segments" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  segments->setDynamicLayerParameterName( u"INPUT"_s );
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
