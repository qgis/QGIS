/***************************************************************************
                             qgslayoutreportcontext.cpp
                             --------------------
    begin                : July 2017
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

#include "qgslayoutreportcontext.h"
#include "qgsfeature.h"
#include "qgslayout.h"
#include "qgsvectorlayer.h"

QgsLayoutReportContext::QgsLayoutReportContext( QgsLayout *layout )
  : QObject( layout )
  , mLayout( layout )
{}

void QgsLayoutReportContext::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
  mGeometryCache.clear();
  emit changed();
}

QgsGeometry QgsLayoutReportContext::currentGeometry( const QgsCoordinateReferenceSystem &crs ) const
{
  if ( !crs.isValid() )
  {
    // no projection, return the native geometry
    return mFeature.geometry();
  }

  if ( !mLayer || !mFeature.isValid() || !mFeature.hasGeometry() )
  {
    return QgsGeometry();
  }

  if ( mLayer->crs() == crs )
  {
    // no projection, return the native geometry
    return mFeature.geometry();
  }

  const auto it = mGeometryCache.constFind( crs.srsid() );
  if ( it != mGeometryCache.constEnd() )
  {
    // we have it in cache, return it
    return it.value();
  }

  QgsGeometry transformed = mFeature.geometry();
  transformed.transform( QgsCoordinateTransform( mLayer->crs(), crs, mLayout->project() ) );
  mGeometryCache[crs.srsid()] = transformed;
  return transformed;
}

QgsVectorLayer *QgsLayoutReportContext::layer() const
{
  return mLayer;
}

void QgsLayoutReportContext::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
  emit layerChanged( layer );
  emit changed();
}

void QgsLayoutReportContext::setPredefinedScales( const QVector<qreal> &scales )
{
  mPredefinedScales = scales;
  // make sure the list is sorted
  std::sort( mPredefinedScales.begin(), mPredefinedScales.end() ); // clazy:exclude=detaching-member
}
