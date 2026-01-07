/***************************************************************************
                         qgsalgorithmremovepartsbyarea.cpp
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

#include "qgsalgorithmremovepartsbyarea.h"

#include "qgsgeometrycollection.h"
#include "qgssurface.h"

///@cond PRIVATE

QString QgsRemovePartsByAreaAlgorithm::name() const
{
  return u"removepartsbyarea"_s;
}

QString QgsRemovePartsByAreaAlgorithm::displayName() const
{
  return QObject::tr( "Remove parts by area" );
}

QStringList QgsRemovePartsByAreaAlgorithm::tags() const
{
  return QObject::tr( "remove,delete,drop,filter,polygon,size" ).split( ',' );
}

QString QgsRemovePartsByAreaAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsRemovePartsByAreaAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsRemovePartsByAreaAlgorithm::outputName() const
{
  return QObject::tr( "Cleaned" );
}

QList<int> QgsRemovePartsByAreaAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast< int >( Qgis::ProcessingSourceType::VectorPolygon );
}

Qgis::ProcessingSourceType QgsRemovePartsByAreaAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

QString QgsRemovePartsByAreaAlgorithm::shortDescription() const
{
  return QObject::tr( "Removes polygons which are smaller than a specified area." );
}

QString QgsRemovePartsByAreaAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon layer and removes polygons which are smaller than a specified area.\n\n"
                      "If the input geometry is a multipart geometry, then the parts will be filtered by their individual areas. If no parts match the "
                      "required minimum area, then the feature will be skipped and omitted from the output layer.\n\n"
                      "If the input geometry is a singlepart geometry, then the feature will be skipped if the geometry's "
                      "area is below the required size and omitted from the output layer.\n\n"
                      "The area will be calculated using Cartesian calculations in the source layer's coordinate reference system.\n\n"
                      "Attributes are not modified." );
}

QgsRemovePartsByAreaAlgorithm *QgsRemovePartsByAreaAlgorithm::createInstance() const
{
  return new QgsRemovePartsByAreaAlgorithm();
}

Qgis::ProcessingFeatureSourceFlags QgsRemovePartsByAreaAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm can be used to repair geometries
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

void QgsRemovePartsByAreaAlgorithm::initParameters( const QVariantMap & )
{
  auto minArea = std::make_unique< QgsProcessingParameterArea >( u"MIN_AREA"_s, QObject::tr( "Remove parts with area less than" ), 0.0, u"INPUT"_s, false, 0 );
  minArea->setIsDynamic( true );
  minArea->setDynamicPropertyDefinition( QgsPropertyDefinition( u"MIN_AREA"_s, QObject::tr( "Remove parts with area less than" ), QgsPropertyDefinition::DoublePositive ) );
  minArea->setDynamicLayerParameterName( u"INPUT"_s );
  addParameter( minArea.release() );
}

bool QgsRemovePartsByAreaAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mMinArea = parameterAsDouble( parameters, u"MIN_AREA"_s, context );
  mDynamicMinArea = QgsProcessingParameters::isDynamic( parameters, u"MIN_AREA"_s );
  if ( mDynamicMinArea )
    mMinAreaProperty = parameters.value( u"MIN_AREA"_s ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsRemovePartsByAreaAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double minArea = mMinArea;
    if ( mDynamicMinArea )
      minArea = mMinAreaProperty.valueAsDouble( context.expressionContext(), minArea );

    const QgsGeometry geometry = f.geometry();
    QgsGeometry outputGeometry;
    if ( const QgsGeometryCollection *inputCollection = qgsgeometry_cast< const QgsGeometryCollection * >( geometry.constGet() ) )
    {
      std::unique_ptr< QgsGeometryCollection> filteredGeometry( inputCollection->createEmptyWithSameType() );
      const int size = inputCollection->numGeometries();
      filteredGeometry->reserve( size );
      for ( int i = 0; i < size; ++i )
      {
        if ( const QgsSurface *surface = qgsgeometry_cast< const QgsSurface * >( inputCollection->geometryN( i ) ) )
        {
          if ( surface->area() >= minArea )
          {
            filteredGeometry->addGeometry( surface->clone() );
          }
        }
      }
      if ( filteredGeometry->numGeometries() == 0 )
      {
        // skip empty features
        return {};
      }
      outputGeometry = QgsGeometry( std::move( filteredGeometry ) );
      f.setGeometry( outputGeometry );
    }
    else if ( const QgsSurface *surface = qgsgeometry_cast< const QgsSurface * >( geometry.constGet() ) )
    {
      if ( surface->area() < minArea )
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
