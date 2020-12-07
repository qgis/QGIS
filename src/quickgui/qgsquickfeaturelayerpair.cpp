/***************************************************************************
  qgsquickfeaturelayerpair.cpp
 ---------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayer.h"
#include "qgsfeature.h"

#include "qgsquickfeaturelayerpair.h"

QgsQuickFeatureLayerPair::QgsQuickFeatureLayerPair() = default;

QgsQuickFeatureLayerPair::QgsQuickFeatureLayerPair( const QgsFeature &feature, QgsVectorLayer *layer )
  : mLayer( layer )
  , mFeature( feature )
{
}

QgsVectorLayer *QgsQuickFeatureLayerPair::layer() const
{
  return mLayer;
}

QgsFeature QgsQuickFeatureLayerPair::feature() const
{
  return mFeature;
}

QgsFeature &QgsQuickFeatureLayerPair::featureRef()
{
  return mFeature;
}

bool QgsQuickFeatureLayerPair::isValid() const
{
  return ( mLayer && mFeature.isValid() && hasValidGeometry() );
}

bool QgsQuickFeatureLayerPair::operator==( const QgsQuickFeatureLayerPair &other ) const
{
  return ( mLayer == other.layer() ) && ( mFeature == other.feature() );
}

bool QgsQuickFeatureLayerPair::operator!=( const QgsQuickFeatureLayerPair &other ) const
{
  return ( mLayer != other.layer() ) || ( mFeature != other.feature() );
}

bool QgsQuickFeatureLayerPair::hasValidGeometry() const
{
  Q_ASSERT( mLayer );

  if ( !mFeature.hasGeometry() )
    return false;

  if ( mFeature.geometry().type() != mLayer->geometryType() )
    return false;

  return true;
}
