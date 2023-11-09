/***************************************************************************
                         qgsalgorithmremoveduplicatevertices.cpp
                         ---------------------
    begin                : November 2017
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

#include "qgsalgorithmremoveduplicatevertices.h"

///@cond PRIVATE

QString QgsAlgorithmRemoveDuplicateVertices::name() const
{
  return QStringLiteral( "removeduplicatevertices" );
}

QString QgsAlgorithmRemoveDuplicateVertices::displayName() const
{
  return QObject::tr( "Remove duplicate vertices" );
}

QStringList QgsAlgorithmRemoveDuplicateVertices::tags() const
{
  return QObject::tr( "points,valid,overlapping,vertex,nodes,invalid,error,repair" ).split( ',' );
}

QString QgsAlgorithmRemoveDuplicateVertices::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsAlgorithmRemoveDuplicateVertices::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsAlgorithmRemoveDuplicateVertices::outputName() const
{
  return QObject::tr( "Cleaned" );
}

QString QgsAlgorithmRemoveDuplicateVertices::shortHelpString() const
{
  return QObject::tr( "This algorithm removes duplicate vertices from features, wherever removing the vertices does "
                      "not result in a degenerate geometry.\n\n"
                      "The tolerance parameter specifies the tolerance for coordinates when determining whether "
                      "vertices are identical.\n\n"
                      "By default, z values are not considered when detecting duplicate vertices. E.g. two vertices "
                      "with the same x and y coordinate but different z values will still be considered "
                      "duplicate and one will be removed. If the Use Z Value parameter is true, then the z values are "
                      "also tested and vertices with the same x and y but different z will be maintained.\n\n"
                      "Note that duplicate vertices are not tested between different parts of a multipart geometry. E.g. "
                      "a multipoint geometry with overlapping points will not be changed by this method." );
}

QgsAlgorithmRemoveDuplicateVertices *QgsAlgorithmRemoveDuplicateVertices::createInstance() const
{
  return new QgsAlgorithmRemoveDuplicateVertices();
}

void QgsAlgorithmRemoveDuplicateVertices::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance> tolerance = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "TOLERANCE" ),
      QObject::tr( "Tolerance" ), 0.000001, QStringLiteral( "INPUT" ), false, 0, 10000000.0 );
  tolerance->setIsDynamic( true );
  tolerance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Tolerance" ), QObject::tr( "Tolerance distance" ), QgsPropertyDefinition::DoublePositive ) );
  tolerance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( tolerance.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > useZ = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "USE_Z_VALUE" ),
      QObject::tr( "Use Z Value" ), false );
  useZ->setIsDynamic( true );
  useZ->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "UseZ" ), QObject::tr( "Use Z Value" ), QgsPropertyDefinition::Boolean ) );
  useZ->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( useZ.release() );
}

QgsProcessingFeatureSource::Flag QgsAlgorithmRemoveDuplicateVertices::sourceFlags() const
{
  // skip geometry checks - this algorithm can be used to repair geometries
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

bool QgsAlgorithmRemoveDuplicateVertices::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  mDynamicTolerance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "TOLERANCE" ) );
  if ( mDynamicTolerance )
    mToleranceProperty = parameters.value( QStringLiteral( "TOLERANCE" ) ).value< QgsProperty >();

  mUseZValues = parameterAsBoolean( parameters, QStringLiteral( "USE_Z_VALUE" ), context );
  mDynamicUseZ = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "USE_Z_VALUE" ) );
  if ( mDynamicUseZ )
    mUseZProperty = parameters.value( QStringLiteral( "USE_Z_VALUE" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsAlgorithmRemoveDuplicateVertices::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry geometry = f.geometry();
    double tolerance = mTolerance;
    if ( mDynamicTolerance )
      tolerance = mToleranceProperty.valueAsDouble( context.expressionContext(), tolerance );

    bool useZValue = mUseZValues;
    if ( mDynamicUseZ )
      useZValue = mUseZProperty.valueAsBool( context.expressionContext(), useZValue );

    geometry.removeDuplicateNodes( tolerance, useZValue );
    f.setGeometry( geometry );
  }
  return QgsFeatureList() << f;
}

///@endcond


