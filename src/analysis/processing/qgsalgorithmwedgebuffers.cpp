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
  return QStringLiteral( "wedgebuffers" );
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
  return QStringLiteral( "vectorgeometry" );
}

QString QgsWedgeBuffersAlgorithm::outputName() const
{
  return QObject::tr( "Buffers" );
}

QgsWkbTypes::Type QgsWedgeBuffersAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  QgsWkbTypes::Type out = QgsWkbTypes::CurvePolygon;
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

QList<int> QgsWedgeBuffersAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPoint;
}

QgsProcessing::SourceType QgsWedgeBuffersAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsWedgeBuffersAlgorithm *QgsWedgeBuffersAlgorithm::createInstance() const
{
  return new QgsWedgeBuffersAlgorithm();
}

void QgsWedgeBuffersAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > azimuth = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "AZIMUTH" ), QObject::tr( "Azimuth (degrees from North)" ), QgsProcessingParameterNumber::Double, 0, false );
  azimuth->setIsDynamic( true );
  azimuth->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Azimuth" ), QObject::tr( "Azimuth (degrees from North)" ), QgsPropertyDefinition::Double ) );
  azimuth->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( azimuth.release() );

  std::unique_ptr< QgsProcessingParameterNumber > width = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "WIDTH" ), QObject::tr( "Wedge width (in degrees)" ), QgsProcessingParameterNumber::Double, 45, false, 0, 360.0 );
  width->setIsDynamic( true );
  width->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Width" ), QObject::tr( "Wedge width (in degrees)" ), QgsPropertyDefinition::DoublePositive ) );
  width->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( width.release() );

  std::unique_ptr< QgsProcessingParameterNumber > outerRadius = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "OUTER_RADIUS" ), QObject::tr( "Outer radius" ), QgsProcessingParameterNumber::Double, 1, false, 0 );
  outerRadius->setIsDynamic( true );
  outerRadius->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Outer radius" ), QObject::tr( "Outer radius" ), QgsPropertyDefinition::DoublePositive ) );
  outerRadius->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( outerRadius.release() );

  std::unique_ptr< QgsProcessingParameterNumber > innerRadius = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "INNER_RADIUS" ), QObject::tr( "Inner radius" ), QgsProcessingParameterNumber::Double, 0, true, 0 );
  innerRadius->setIsDynamic( true );
  innerRadius->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Inner radius" ), QObject::tr( "Inner radius" ), QgsPropertyDefinition::DoublePositive ) );
  innerRadius->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( innerRadius.release() );
}

QgsProcessingFeatureSource::Flag QgsWedgeBuffersAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

bool QgsWedgeBuffersAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mAzimuth = parameterAsDouble( parameters, QStringLiteral( "AZIMUTH" ), context );
  mDynamicAzimuth = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "AZIMUTH" ) );
  if ( mDynamicAzimuth )
    mAzimuthProperty = parameters.value( QStringLiteral( "AZIMUTH" ) ).value< QgsProperty >();

  mWidth = parameterAsDouble( parameters, QStringLiteral( "WIDTH" ), context );
  mDynamicWidth = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "WIDTH" ) );
  if ( mDynamicWidth )
    mWidthProperty = parameters.value( QStringLiteral( "WIDTH" ) ).value< QgsProperty >();

  mOuterRadius = parameterAsDouble( parameters, QStringLiteral( "OUTER_RADIUS" ), context );
  mDynamicOuterRadius = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "OUTER_RADIUS" ) );
  if ( mDynamicOuterRadius )
    mOuterRadiusProperty = parameters.value( QStringLiteral( "OUTER_RADIUS" ) ).value< QgsProperty >();

  mInnerRadius = parameterAsDouble( parameters, QStringLiteral( "INNER_RADIUS" ), context );
  mDynamicInnerRadius = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "INNER_RADIUS" ) );
  if ( mDynamicInnerRadius )
    mInnerRadiusProperty = parameters.value( QStringLiteral( "INNER_RADIUS" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsWedgeBuffersAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() && QgsWkbTypes::geometryType( f.geometry().wkbType() ) == QgsWkbTypes::PointGeometry )
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
      const QgsMultiPoint *mp = static_cast< const QgsMultiPoint * >( g.constGet() );
      std::unique_ptr< QgsMultiSurface > result = std::make_unique< QgsMultiSurface >();
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
      const QgsPoint *p = static_cast< const QgsPoint * >( g.constGet() );
      f.setGeometry( QgsGeometry::createWedgeBuffer( *p, azimuth, width, outerRadius, innerRadius ) );
    }
  }

  return QgsFeatureList() << f;
}

///@endcond

