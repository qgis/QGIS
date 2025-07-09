/***************************************************************************
                         qgsalgorithmremovepartsbylength.cpp
                         ---------------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#include "qgsalgorithmremovepartsbylength.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"

///@cond PRIVATE

QString QgsRemovePartsByLengthAlgorithm::name() const
{
  return QStringLiteral( "removepartsbylength" );
}

QString QgsRemovePartsByLengthAlgorithm::displayName() const
{
  return QObject::tr( "Remove parts by length" );
}

QStringList QgsRemovePartsByLengthAlgorithm::tags() const
{
  return QObject::tr( "remove,delete,drop,filter,lines,linestring,polyline,size" ).split( ',' );
}

QString QgsRemovePartsByLengthAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsRemovePartsByLengthAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsRemovePartsByLengthAlgorithm::outputName() const
{
  return QObject::tr( "Cleaned" );
}

QList<int> QgsRemovePartsByLengthAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast< int >( Qgis::ProcessingSourceType::VectorLine );
}

Qgis::ProcessingSourceType QgsRemovePartsByLengthAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorLine;
}

QString QgsRemovePartsByLengthAlgorithm::shortDescription() const
{
  return QObject::tr( "Removes lines which are shorter than a specified length." );
}

QString QgsRemovePartsByLengthAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a line layer and removes lines which are shorter than a specified length.\n\n"
                      "If the input geometry is a multipart geometry, then the parts will be filtered by their individual lengths. If no parts match the "
                      "required minimum length, then the feature will be skipped and omitted from the output layer.\n\n"
                      "If the input geometry is a singlepart geometry, then the feature will be skipped if the geometry's "
                      "length is below the required size and omitted from the output layer.\n\n"
                      "The length will be calculated using Cartesian calculations in the source layer's coordinate reference system.\n\n"
                      "Attributes are not modified." );
}

QgsRemovePartsByLengthAlgorithm *QgsRemovePartsByLengthAlgorithm::createInstance() const
{
  return new QgsRemovePartsByLengthAlgorithm();
}

Qgis::ProcessingFeatureSourceFlags QgsRemovePartsByLengthAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm can be used to repair geometries
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

void QgsRemovePartsByLengthAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterDistance > minLength = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "MIN_LENGTH" ), QObject::tr( "Remove parts with lengths less than" ), 0.0, QStringLiteral( "INPUT" ), false, 0 );
  minLength->setIsDynamic( true );
  minLength->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "MIN_LENGTH" ), QObject::tr( "Remove parts with length less than" ), QgsPropertyDefinition::DoublePositive ) );
  minLength->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( minLength.release() );
}

bool QgsRemovePartsByLengthAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mMinLength = parameterAsDouble( parameters, QStringLiteral( "MIN_LENGTH" ), context );
  mDynamicMinLength = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "MIN_LENGTH" ) );
  if ( mDynamicMinLength )
    mMinLengthProperty = parameters.value( QStringLiteral( "MIN_LENGTH" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsRemovePartsByLengthAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double minLength = mMinLength;
    if ( mDynamicMinLength )
      minLength = mMinLengthProperty.valueAsDouble( context.expressionContext(), minLength );

    const QgsGeometry geometry = f.geometry();
    QgsGeometry outputGeometry;
    if ( const QgsGeometryCollection *inputCollection = qgsgeometry_cast< const QgsGeometryCollection * >( geometry.constGet() ) )
    {
      std::unique_ptr< QgsAbstractGeometry> filteredGeometry( geometry.constGet()->createEmptyWithSameType() );
      QgsGeometryCollection *collection = qgsgeometry_cast< QgsGeometryCollection * >( filteredGeometry.get() );
      const int size = inputCollection->numGeometries();
      collection->reserve( size );
      for ( int i = 0; i < size; ++i )
      {
        if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( inputCollection->geometryN( i ) ) )
        {
          if ( curve->length() >= minLength )
          {
            collection->addGeometry( curve->clone() );
          }
        }
      }
      if ( collection->numGeometries() == 0 )
      {
        // skip empty features
        return {};
      }
      outputGeometry = QgsGeometry( std::move( filteredGeometry ) );
      f.setGeometry( outputGeometry );
    }
    else if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( geometry.constGet() ) )
    {
      if ( curve->length() < minLength )
      {
        return {};
      }
    }
    else
    {
      return {};
    }
  }
  return { f };
}


///@endcond
