/***************************************************************************
                         qgsalgorithmlinesubstring.cpp
                         ---------------------
    begin                : August 2018
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

#include "qgsalgorithmlinesubstring.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"

///@cond PRIVATE

QString QgsLineSubstringAlgorithm::name() const
{
  return QStringLiteral( "linesubstring" );
}

QString QgsLineSubstringAlgorithm::displayName() const
{
  return QObject::tr( "Line substring" );
}

QStringList QgsLineSubstringAlgorithm::tags() const
{
  return QObject::tr( "linestring,curve,split,shorten,shrink,portion,part,reference,referencing,distance,interpolate" ).split( ',' );
}

QString QgsLineSubstringAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsLineSubstringAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsLineSubstringAlgorithm::outputName() const
{
  return QObject::tr( "Substring" );
}

QString QgsLineSubstringAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm returns the portion of a line (or curve) which falls "
                      "between the specified start and end distances (measured from the "
                      "beginning of the line).\n\n"
                      "Z and M values are linearly interpolated from existing values.\n\n"
                      "If a multipart geometry is encountered, only the first part is considered when "
                      "calculating the substring." );
}

QString QgsLineSubstringAlgorithm::shortDescription() const
{
  return QObject::tr( "Returns the substring of lines which fall between start and end distances." );
}

QList<int> QgsLineSubstringAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsProcessing::SourceType QgsLineSubstringAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

QgsLineSubstringAlgorithm *QgsLineSubstringAlgorithm::createInstance() const
{
  return new QgsLineSubstringAlgorithm();
}

void QgsLineSubstringAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance> startDistance = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "START_DISTANCE" ),
      QObject::tr( "Start distance" ), 0.0, QStringLiteral( "INPUT" ), false, 0 );
  startDistance->setIsDynamic( true );
  startDistance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Start Distance" ), QObject::tr( "Start distance" ), QgsPropertyDefinition::DoublePositive ) );
  startDistance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( startDistance.release() );

  std::unique_ptr< QgsProcessingParameterDistance> endDistance = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "END_DISTANCE" ),
      QObject::tr( "End distance" ), 1.0, QStringLiteral( "INPUT" ), false, 0 );
  endDistance->setIsDynamic( true );
  endDistance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "End Distance" ), QObject::tr( "End distance" ), QgsPropertyDefinition::DoublePositive ) );
  endDistance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( endDistance.release() );
}

QgsProcessingFeatureSource::Flag QgsLineSubstringAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm doesn't care about invalid geometries
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

bool QgsLineSubstringAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mStartDistance = parameterAsDouble( parameters, QStringLiteral( "START_DISTANCE" ), context );
  mDynamicStartDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "START_DISTANCE" ) );
  if ( mDynamicStartDistance )
    mStartDistanceProperty = parameters.value( QStringLiteral( "START_DISTANCE" ) ).value< QgsProperty >();

  mEndDistance = parameterAsDouble( parameters, QStringLiteral( "END_DISTANCE" ), context );
  mDynamicEndDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "END_DISTANCE" ) );
  if ( mDynamicEndDistance )
    mEndDistanceProperty = parameters.value( QStringLiteral( "END_DISTANCE" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsLineSubstringAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    const QgsGeometry geometry = f.geometry();
    double startDistance = mStartDistance;
    if ( mDynamicStartDistance )
      startDistance = mStartDistanceProperty.valueAsDouble( context.expressionContext(), startDistance );

    double endDistance = mEndDistance;
    if ( mDynamicEndDistance )
      endDistance = mEndDistanceProperty.valueAsDouble( context.expressionContext(), endDistance );

    const QgsCurve *curve = nullptr;
    if ( !geometry.isMultipart() )
      curve = qgsgeometry_cast< const QgsCurve * >( geometry.constGet() );
    else
    {
      if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geometry.constGet() ) )
      {
        if ( collection->numGeometries() > 0 )
        {
          curve = qgsgeometry_cast< const QgsCurve * >( collection->geometryN( 0 ) );
        }
      }
    }
    if ( curve )
    {
      std::unique_ptr< QgsCurve > substring( curve->curveSubstring( startDistance, endDistance ) );
      const QgsGeometry result( std::move( substring ) );
      f.setGeometry( result );
    }
    else
    {
      f.clearGeometry();
    }
  }
  return QgsFeatureList() << f;
}

///@endcond


