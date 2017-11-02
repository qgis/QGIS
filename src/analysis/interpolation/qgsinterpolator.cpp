/***************************************************************************
                              qgsinterpolator.cpp
                              -------------------
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

#include "qgsinterpolator.h"
#include "qgsfeatureiterator.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsfeedback.h"

QgsInterpolator::QgsInterpolator( const QList<LayerData> &layerData )
  : mLayerData( layerData )
{

}

QgsInterpolator::Result QgsInterpolator::cacheBaseData( QgsFeedback *feedback )
{
  if ( mLayerData.empty() )
  {
    return Success;
  }

  //reserve initial memory for 100000 vertices
  mCachedBaseData.clear();
  mCachedBaseData.reserve( 100000 );

  double layerStep = !mLayerData.empty() ? 100.0 / mLayerData.count() : 1;
  int layerCount = 0;
  for ( const LayerData &layer : qgis::as_const( mLayerData ) )
  {
    if ( feedback && feedback->isCanceled() )
      return Canceled;

    QgsFeatureSource *source = layer.source;
    if ( !source )
    {
      return InvalidSource;
    }

    QgsAttributeList attList;
    if ( !layer.useZValue )
    {
      attList.push_back( layer.interpolationAttribute );
    }

    double attributeValue = 0.0;
    bool attributeConversionOk = false;
    double progress = layerCount * layerStep;

    QgsFeatureIterator fit = source->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( attList ) );
    double featureStep = source->featureCount() > 0 ? layerStep / source->featureCount() : layerStep;

    QgsFeature feature;
    while ( fit.nextFeature( feature ) )
    {
      if ( feedback && feedback->isCanceled() )
        return Canceled;

      progress += featureStep;
      if ( feedback )
        feedback->setProgress( progress );

      if ( !layer.useZValue )
      {
        QVariant attributeVariant = feature.attribute( layer.interpolationAttribute );
        if ( !attributeVariant.isValid() ) //attribute not found, something must be wrong (e.g. NULL value)
        {
          continue;
        }
        attributeValue = attributeVariant.toDouble( &attributeConversionOk );
        if ( !attributeConversionOk || std::isnan( attributeValue ) ) //don't consider vertices with attributes like 'nan' for the interpolation
        {
          continue;
        }
      }

      if ( !addVerticesToCache( feature.geometry(), layer.useZValue, attributeValue ) )
        return FeatureGeometryError;
    }
    layerCount++;
  }

  return Success;
}

bool QgsInterpolator::addVerticesToCache( const QgsGeometry &geom, bool zCoord, double attributeValue )
{
  if ( !geom || geom.isEmpty() )
    return true; // nothing to do

  bool hasZ = geom.constGet()->is3D();
  for ( auto point = geom.vertices_begin(); point != geom.vertices_end(); ++point )
  {
    if ( hasZ && zCoord )
    {
      mCachedBaseData.push_back( QgsInterpolatorVertexData( ( *point ).x(), ( *point ).y(), ( *point ).z() ) );
    }
    else
    {
      mCachedBaseData.push_back( QgsInterpolatorVertexData( ( *point ).x(), ( *point ).y(), attributeValue ) );
    }
  }
  mDataIsCached = true;
  return true;
}
