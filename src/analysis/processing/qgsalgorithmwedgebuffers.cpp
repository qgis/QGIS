/***************************************************************************
                         qgsalgorithmwedgebuffers.cpp
                         ---------------------
    begin                : April 2018
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

#include "qgsalgorithmwedgebuffers.h"

#include "qgsmultipoint.h"
#include "qgsmultisurface.h"

///@cond PRIVATE

QString QgsWedgeBuffersAlgorithm::name() const
{
  return u"wedgebuffers"_s;
}

QString QgsWedgeBuffersAlgorithm::displayName() const
{
  return QObject::tr( "Create wedge buffers" );
}

QStringList QgsWedgeBuffersAlgorithm::tags() const
{
  return QObject::tr( "arc,segment,circular,circle,slice" ).split( ',' );
}

QString QgsWedgeBuffersAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsWedgeBuffersAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsWedgeBuffersAlgorithm::outputName() const
{
  return QObject::tr( "Buffers" );
}

Qgis::WkbType QgsWedgeBuffersAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  Qgis::WkbType out = Qgis::WkbType::CurvePolygon;
  if ( QgsWkbTypes::hasZ( inputWkbType ) )
    out = QgsWkbTypes::addZ( out );
  if ( QgsWkbTypes::hasM( inputWkbType ) )
    out = QgsWkbTypes::addM( out );
  if ( QgsWkbTypes::isMultiType( inputWkbType ) )
    out = QgsWkbTypes::multiType( out );
  return out;
}

QString QgsWedgeBuffersAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates wedge shaped buffers from input points.\n\n"
                      "The azimuth parameter gives the angle (in degrees) for the middle of the wedge to point. "
                      "The buffer width (in degrees) is specified by the width parameter. Note that the "
                      "wedge will extend to half of the angular width either side of the azimuth direction.\n\n"
                      "The outer radius of the buffer is specified via outer radius, and optionally an "
                      "inner radius can also be specified.\n\n"
                      "The native output from this algorithm are CurvePolygon geometries, but these may "
                      "be automatically segmentized to Polygons depending on the output format." );
}

QString QgsWedgeBuffersAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates wedge shaped buffers from input points." );
}

QList<int> QgsWedgeBuffersAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint );
}

Qgis::ProcessingSourceType QgsWedgeBuffersAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

QgsWedgeBuffersAlgorithm *QgsWedgeBuffersAlgorithm::createInstance() const
{
  return new QgsWedgeBuffersAlgorithm();
}

void QgsWedgeBuffersAlgorithm::initParameters( const QVariantMap & )
{
  auto azimuth = std::make_unique<QgsProcessingParameterNumber>( u"AZIMUTH"_s, QObject::tr( "Azimuth (degrees from North)" ), Qgis::ProcessingNumberParameterType::Double, 0, false );
  azimuth->setIsDynamic( true );
  azimuth->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Azimuth"_s, QObject::tr( "Azimuth (degrees from North)" ), QgsPropertyDefinition::Double ) );
  azimuth->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( azimuth.release() );

  auto width = std::make_unique<QgsProcessingParameterNumber>( u"WIDTH"_s, QObject::tr( "Wedge width (in degrees)" ), Qgis::ProcessingNumberParameterType::Double, 45, false, 0, 360.0 );
  width->setIsDynamic( true );
  width->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Width"_s, QObject::tr( "Wedge width (in degrees)" ), QgsPropertyDefinition::DoublePositive ) );
  width->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( width.release() );

  auto outerRadius = std::make_unique<QgsProcessingParameterNumber>( u"OUTER_RADIUS"_s, QObject::tr( "Outer radius" ), Qgis::ProcessingNumberParameterType::Double, 1, false, 0 );
  outerRadius->setIsDynamic( true );
  outerRadius->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Outer radius"_s, QObject::tr( "Outer radius" ), QgsPropertyDefinition::DoublePositive ) );
  outerRadius->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( outerRadius.release() );

  auto innerRadius = std::make_unique<QgsProcessingParameterNumber>( u"INNER_RADIUS"_s, QObject::tr( "Inner radius" ), Qgis::ProcessingNumberParameterType::Double, 0, true, 0 );
  innerRadius->setIsDynamic( true );
  innerRadius->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Inner radius"_s, QObject::tr( "Inner radius" ), QgsPropertyDefinition::DoublePositive ) );
  innerRadius->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( innerRadius.release() );
}

Qgis::ProcessingFeatureSourceFlags QgsWedgeBuffersAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

bool QgsWedgeBuffersAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mAzimuth = parameterAsDouble( parameters, u"AZIMUTH"_s, context );
  mDynamicAzimuth = QgsProcessingParameters::isDynamic( parameters, u"AZIMUTH"_s );
  if ( mDynamicAzimuth )
    mAzimuthProperty = parameters.value( u"AZIMUTH"_s ).value<QgsProperty>();

  mWidth = parameterAsDouble( parameters, u"WIDTH"_s, context );
  mDynamicWidth = QgsProcessingParameters::isDynamic( parameters, u"WIDTH"_s );
  if ( mDynamicWidth )
    mWidthProperty = parameters.value( u"WIDTH"_s ).value<QgsProperty>();

  mOuterRadius = parameterAsDouble( parameters, u"OUTER_RADIUS"_s, context );
  mDynamicOuterRadius = QgsProcessingParameters::isDynamic( parameters, u"OUTER_RADIUS"_s );
  if ( mDynamicOuterRadius )
    mOuterRadiusProperty = parameters.value( u"OUTER_RADIUS"_s ).value<QgsProperty>();

  mInnerRadius = parameterAsDouble( parameters, u"INNER_RADIUS"_s, context );
  mDynamicInnerRadius = QgsProcessingParameters::isDynamic( parameters, u"INNER_RADIUS"_s );
  if ( mDynamicInnerRadius )
    mInnerRadiusProperty = parameters.value( u"INNER_RADIUS"_s ).value<QgsProperty>();

  return true;
}

QgsFeatureList QgsWedgeBuffersAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() && QgsWkbTypes::geometryType( f.geometry().wkbType() ) == Qgis::GeometryType::Point )
  {
    double azimuth = mAzimuth;
    if ( mDynamicAzimuth )
      azimuth = mAzimuthProperty.valueAsDouble( context.expressionContext(), azimuth );

    double width = mWidth;
    if ( mDynamicWidth )
      width = mWidthProperty.valueAsDouble( context.expressionContext(), width );

    double outerRadius = mOuterRadius;
    if ( mDynamicOuterRadius )
      outerRadius = mOuterRadiusProperty.valueAsDouble( context.expressionContext(), outerRadius );

    double innerRadius = mInnerRadius;
    if ( mDynamicInnerRadius )
      innerRadius = mInnerRadiusProperty.valueAsDouble( context.expressionContext(), innerRadius );

    const QgsGeometry g = f.geometry();
    if ( QgsWkbTypes::isMultiType( g.wkbType() ) )
    {
      const QgsMultiPoint *mp = static_cast<const QgsMultiPoint *>( g.constGet() );
      auto result = std::make_unique<QgsMultiSurface>();
      result->reserve( mp->numGeometries() );
      for ( int i = 0; i < mp->numGeometries(); ++i )
      {
        const QgsPoint *p = mp->pointN( i );
        result->addGeometry( QgsGeometry::createWedgeBuffer( *p, azimuth, width, outerRadius, innerRadius ).constGet()->clone() );
      }
      f.setGeometry( QgsGeometry( std::move( result ) ) );
    }
    else
    {
      const QgsPoint *p = static_cast<const QgsPoint *>( g.constGet() );
      f.setGeometry( QgsGeometry::createWedgeBuffer( *p, azimuth, width, outerRadius, innerRadius ) );
    }
  }

  return QgsFeatureList() << f;
}

///@endcond
