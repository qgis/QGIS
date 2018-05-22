/***************************************************************************
  qgsquickfeature.cpp
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

#include "qgsquickfeature.h"

QgsQuickFeature::QgsQuickFeature( const QgsFeature &feature, QgsVectorLayer *layer )
  : mLayer( layer )
  , mFeature( feature )
{
}

QgsQuickFeature::QgsQuickFeature()
{
  mFeature.setValid( false );
}

QgsVectorLayer *QgsQuickFeature::layer() const
{
  return mLayer;
}

QgsFeature QgsQuickFeature::feature() const
{
  return mFeature;
}

bool QgsQuickFeature::valid() const
{
  return ( mLayer && mFeature.isValid() );
}

void QgsQuickFeature::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
}

void QgsQuickFeature::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}
