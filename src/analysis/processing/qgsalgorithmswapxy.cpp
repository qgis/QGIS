/***************************************************************************
                         qgsalgorithmswapxy.cpp
                         ------------------------
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

#include "qgsalgorithmswapxy.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsSwapXYAlgorithm::name() const
{
  return QStringLiteral( "swapxy" );
}

QString QgsSwapXYAlgorithm::displayName() const
{
  return QObject::tr( "Swap X and Y coordinates" );
}

QStringList QgsSwapXYAlgorithm::tags() const
{
  return QObject::tr( "invert,flip,swap,latitude,longitude" ).split( ',' );
}

QString QgsSwapXYAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsSwapXYAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsSwapXYAlgorithm::outputName() const
{
  return QObject::tr( "Swapped" );
}

QString QgsSwapXYAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm swaps the X and Y coordinate values in input geometries. It can be used to repair geometries "
                      "which have accidentally had their latitude and longitude values reversed." );
}

QgsSwapXYAlgorithm *QgsSwapXYAlgorithm::createInstance() const
{
  return new QgsSwapXYAlgorithm();
}

bool QgsSwapXYAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  if ( ! QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;

  return layer->isSpatial();
}

QgsProcessingFeatureSource::Flag QgsSwapXYAlgorithm::sourceFlags() const
{
  // this algorithm doesn't care about invalid geometries
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QgsFeatureList QgsSwapXYAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeatureList list;
  QgsFeature feature = f;
  if ( feature.hasGeometry() )
  {
    const QgsGeometry geom = feature.geometry();
    std::unique_ptr< QgsAbstractGeometry > swappedGeom( geom.constGet()->clone() );
    swappedGeom->swapXy();
    feature.setGeometry( QgsGeometry( std::move( swappedGeom ) ) );
    list << feature;
  }
  else
  {
    list << feature;
  }
  return list;
}

///@endcond
