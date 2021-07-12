/***************************************************************************
                              qgsidwinterpolator.cpp
                              ----------------------
  begin                : Marco 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsidwinterpolator.h"
#include "qgsfeedback.h"
#include "qgsgeometry.h"
#include "qgsfeaturesource.h"
#include "qgsfeatureiterator.h"
#include "qgis.h"
#include <cmath>
#include <limits>

QgsIDWInterpolator::QgsIDWInterpolator( const QList<LayerData> &layerData )
  : QgsInterpolator( layerData )
{
  cacheBaseData( );
}

QgsInterpolator::Result QgsIDWInterpolator::cacheBaseData( QgsFeedback *feedback )
{
  if ( mLayerData.empty() )
  {
    return Success;
  }

  //reserve initial memory for 100000 vertices
  mCachedBaseData.clear();
  mCachedBaseData.reserve( 100000 );

  const QgsCoordinateReferenceSystem crs = !mLayerData.empty() ? mLayerData.at( 0 ).source->sourceCrs() : QgsCoordinateReferenceSystem();

  double layerStep = !mLayerData.empty() ? 100.0 / mLayerData.count() : 1;
  int layerCount = 0;
  for ( const LayerData &layer : std::as_const( mLayerData ) )
  {
    if ( feedback && feedback->isCanceled() )
      return Canceled;

    QgsFeatureSource *source = layer.source;
    if ( !source )
    {
      return InvalidSource;
    }

    QgsAttributeList attList;
    switch ( layer.valueSource )
    {
      case ValueAttribute:
        attList.push_back( layer.interpolationAttribute );
        break;

      case ValueZ:
      case ValueM:
        break;
    }

    double attributeValue = 0.0;
    bool attributeConversionOk = false;
    double progress = layerCount * layerStep;

    QgsFeatureIterator fit = source->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( attList ).setDestinationCrs( crs, layer.transformContext ) );
    double featureStep = source->featureCount() > 0 ? layerStep / source->featureCount() : layerStep;

    QgsFeature feature;
    while ( fit.nextFeature( feature ) )
    {
      if ( feedback && feedback->isCanceled() )
        return Canceled;

      progress += featureStep;
      if ( feedback )
        feedback->setProgress( progress );

      switch ( layer.valueSource )
      {
        case ValueAttribute:
        {
          QVariant attributeVariant = feature.attribute( layer.interpolationAttribute );
          if ( !attributeVariant.isValid() || attributeVariant.isNull() ) //attribute not found, something must be wrong (e.g. NULL value)
          {
            continue;
          }
          attributeValue = attributeVariant.toDouble( &attributeConversionOk );
          if ( !attributeConversionOk || std::isnan( attributeValue ) ) //don't consider vertices with attributes like 'nan' for the interpolation
          {
            continue;
          }
          break;
        }

        case ValueZ:
        case ValueM:
          break;
      }

      if ( !addVerticesToCache( feature.geometry(), layer.valueSource, attributeValue ) )
        return FeatureGeometryError;
    }
    layerCount++;
  }

  return Success;
}

bool QgsIDWInterpolator::addVerticesToCache( const QgsGeometry &geom, ValueSource source, double attributeValue )
{

  if ( geom.isNull() || geom.isEmpty() )
    return true; // nothing to do

  //validate source
  switch ( source )
  {
    case ValueAttribute:
      break;

    case ValueM:
      if ( !geom.constGet()->isMeasure() )
        return false;
      else
        break;

    case ValueZ:
      if ( !geom.constGet()->is3D() )
        return false;
      else
        break;
  }

  for ( auto point = geom.vertices_begin(); point != geom.vertices_end(); ++point )
  {
    switch ( source )
    {
      case ValueM:
        mCachedBaseData.push_back( QgsInterpolatorVertexData( ( *point ).x(), ( *point ).y(), ( *point ).m() ) );
        break;

      case ValueZ:
        mCachedBaseData.push_back( QgsInterpolatorVertexData( ( *point ).x(), ( *point ).y(), ( *point ).z() ) );
        break;

      case ValueAttribute:
        mCachedBaseData.push_back( QgsInterpolatorVertexData( ( *point ).x(), ( *point ).y(), attributeValue ) );
        break;
    }
  }
  return true;
}

QgsIDWInterpolator::QgsIDWInterpolator( const QList<LayerData> &layerData, QgsFeedback *feedback )
  : QgsInterpolator( layerData )
{
  cacheBaseData( feedback );
}

double QgsIDWInterpolator::interpolatedPoint( const QgsPointXY &point, QgsFeedback *feedback ) const
{
  double sumCounter = 0;
  double sumDenominator = 0;

  for ( const QgsInterpolatorVertexData &vertex : std::as_const( mCachedBaseData ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return 1;
    }
    double distance = std::sqrt( ( vertex.x - point.x() ) * ( vertex.x - point.x() ) + ( vertex.y - point.y() ) * ( vertex.y - point.y() ) );
    if ( qgsDoubleNear( distance, 0.0 ) )
    {
      return vertex.z;
    }
    double currentWeight = 1 / ( std::pow( distance, mDistanceCoefficient ) );
    sumCounter += ( currentWeight * vertex.z );
    sumDenominator += currentWeight;
  }

  if ( sumDenominator == 0.0 )
  {
    return -9999;
  }

  return sumCounter / sumDenominator;
}

int QgsIDWInterpolator::interpolatePoint( double x, double y, double &result, QgsFeedback *feedback )
{

  double sumCounter = 0;
  double sumDenominator = 0;

  for ( const QgsInterpolatorVertexData &vertex : std::as_const( mCachedBaseData ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      return 1;
    }
    double distance = std::sqrt( ( vertex.x - x ) * ( vertex.x - x ) + ( vertex.y - y ) * ( vertex.y - y ) );
    if ( qgsDoubleNear( distance, 0.0 ) )
    {
      result = vertex.z;
      return 0;
    }
    double currentWeight = 1 / ( std::pow( distance, mDistanceCoefficient ) );
    sumCounter += ( currentWeight * vertex.z );
    sumDenominator += currentWeight;
  }

  if ( sumDenominator == 0.0 )
  {
    return 1;
  }

  result = sumCounter / sumDenominator;
  return 0;
}
