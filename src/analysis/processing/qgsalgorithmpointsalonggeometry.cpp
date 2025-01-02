/***************************************************************************
                         qgsalgorithmpointsalonggeometry.cpp
                         ---------------------
    begin                : June 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmpointsalonggeometry.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"
#include "qgsapplication.h"

///@cond PRIVATE

QString QgsPointsAlongGeometryAlgorithm::name() const
{
  return QStringLiteral( "pointsalonglines" );
}

QString QgsPointsAlongGeometryAlgorithm::displayName() const
{
  return QObject::tr( "Points along geometry" );
}

QStringList QgsPointsAlongGeometryAlgorithm::tags() const
{
  return QObject::tr( "create,interpolate,points,lines,regular,distance,by" ).split( ',' );
}

QString QgsPointsAlongGeometryAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsPointsAlongGeometryAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsPointsAlongGeometryAlgorithm::outputName() const
{
  return QObject::tr( "Interpolated points" );
}

QString QgsPointsAlongGeometryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a points layer, with points distributed along the lines of an "
                      "input vector layer. The distance between points (measured along the line) is defined as a parameter.\n\n"
                      "Start and end offset distances can be defined, so the first and last point will not fall exactly on the line's "
                      "first and last nodes. These start and end offsets are defined as distances, measured along the line from the first and last "
                      "nodes of the lines." );
}

QString QgsPointsAlongGeometryAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates regularly spaced points along line features." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsPointsAlongGeometryAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QList<int> QgsPointsAlongGeometryAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

Qgis::ProcessingSourceType QgsPointsAlongGeometryAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPoint;
}

Qgis::WkbType QgsPointsAlongGeometryAlgorithm::outputWkbType( Qgis::WkbType inputType ) const
{
  Qgis::WkbType out = Qgis::WkbType::Point;
  if ( QgsWkbTypes::hasZ( inputType ) )
    out = QgsWkbTypes::addZ( out );
  if ( QgsWkbTypes::hasM( inputType ) )
    out = QgsWkbTypes::addM( out );
  return out;
}

QgsFields QgsPointsAlongGeometryAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields output = inputFields;
  output.append( QgsField( QStringLiteral( "distance" ), QMetaType::Type::Double ) );
  output.append( QgsField( QStringLiteral( "angle" ), QMetaType::Type::Double ) );
  return output;
}

QgsPointsAlongGeometryAlgorithm *QgsPointsAlongGeometryAlgorithm::createInstance() const
{
  return new QgsPointsAlongGeometryAlgorithm();
}

void QgsPointsAlongGeometryAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr<QgsProcessingParameterDistance> distance = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), 1.0, QStringLiteral( "INPUT" ), false, 0 );
  distance->setIsDynamic( true );
  distance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), QgsPropertyDefinition::DoublePositive ) );
  distance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( distance.release() );

  std::unique_ptr<QgsProcessingParameterDistance> startOffset = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "START_OFFSET" ), QObject::tr( "Start offset" ), 0.0, QStringLiteral( "INPUT" ), false, 0 );
  startOffset->setIsDynamic( true );
  startOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "START_OFFSET" ), QObject::tr( "Start offset" ), QgsPropertyDefinition::DoublePositive ) );
  startOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( startOffset.release() );

  std::unique_ptr<QgsProcessingParameterDistance> endOffset = std::make_unique<QgsProcessingParameterDistance>( QStringLiteral( "END_OFFSET" ), QObject::tr( "End offset" ), 0.0, QStringLiteral( "INPUT" ), false, 0 );
  endOffset->setIsDynamic( true );
  endOffset->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "END_OFFSET" ), QObject::tr( "End offset" ), QgsPropertyDefinition::DoublePositive ) );
  endOffset->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( endOffset.release() );
}

QIcon QgsPointsAlongGeometryAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmExtractVertices.svg" ) );
}

QString QgsPointsAlongGeometryAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmExtractVertices.svg" ) );
}

Qgis::ProcessingFeatureSourceFlags QgsPointsAlongGeometryAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm doesn't care about invalid geometries
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QgsFeatureSink::SinkFlags QgsPointsAlongGeometryAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

bool QgsPointsAlongGeometryAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicDistance )
    mDistanceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value<QgsProperty>();

  mStartOffset = parameterAsDouble( parameters, QStringLiteral( "START_OFFSET" ), context );
  mDynamicStartOffset = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "START_OFFSET" ) );
  if ( mDynamicStartOffset )
    mStartOffsetProperty = parameters.value( QStringLiteral( "START_OFFSET" ) ).value<QgsProperty>();

  mEndOffset = parameterAsDouble( parameters, QStringLiteral( "END_OFFSET" ), context );
  mDynamicEndOffset = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "END_OFFSET" ) );
  if ( mDynamicEndOffset )
    mEndOffsetProperty = parameters.value( QStringLiteral( "END_OFFSET" ) ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsPointsAlongGeometryAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry geometry = f.geometry();

    double distance = mDistance;
    if ( mDynamicDistance )
      distance = mDistanceProperty.valueAsDouble( context.expressionContext(), distance );
    if ( distance <= 0 )
      return QgsFeatureList();

    double startOffset = mStartOffset;
    if ( mDynamicStartOffset )
      startOffset = mStartOffsetProperty.valueAsDouble( context.expressionContext(), startOffset );

    double endOffset = mEndOffset;
    if ( mDynamicEndOffset )
      endOffset = mEndOffsetProperty.valueAsDouble( context.expressionContext(), endOffset );

    const double totalLength = geometry.type() == Qgis::GeometryType::Polygon ? geometry.constGet()->perimeter()
                                                                              : geometry.length() - endOffset;

    double currentDistance = startOffset;
    QgsFeatureList out;
    out.reserve( static_cast<int>( std::ceil( ( totalLength - startOffset ) / distance ) ) );
    while ( currentDistance <= totalLength )
    {
      const QgsGeometry point = geometry.interpolate( currentDistance );
      const double angle = ( 180 / M_PI ) * geometry.interpolateAngle( currentDistance );
      QgsFeature outputFeature;
      outputFeature.setGeometry( point );
      QgsAttributes outAttr = f.attributes();
      outAttr << currentDistance << angle;
      outputFeature.setAttributes( outAttr );
      out.append( outputFeature );
      currentDistance += distance;
      if ( feedback->isCanceled() ) // better check here -- a ridiculously small distance might take forever
        break;
    }
    return out;
  }
  else
  {
    QgsAttributes outAttr = f.attributes();
    outAttr << QVariant() << QVariant();
    f.setAttributes( outAttr );
    return QgsFeatureList() << f;
  }
}

///@endcond
