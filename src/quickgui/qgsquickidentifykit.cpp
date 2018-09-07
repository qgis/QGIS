/***************************************************************************
  qgsquickidentifykit.cpp
 ---------------------
  Date                 : 30.8.2016
  Copyright            : (C) 2016 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"

#include "qgsquickidentifykit.h"
#include "qgsquickmapsettings.h"

#include "qgis.h"

QgsQuickIdentifyKit::QgsQuickIdentifyKit( QObject *parent )
  : QObject( parent )
{
}

QgsQuickMapSettings *QgsQuickIdentifyKit::mapSettings() const
{
  return mMapSettings;
}

void QgsQuickIdentifyKit::setMapSettings( QgsQuickMapSettings *mapSettings )
{
  if ( mapSettings == mMapSettings )
    return;

  mMapSettings = mapSettings;
  emit mapSettingsChanged();
}

QgsQuickFeatureLayerPairs QgsQuickIdentifyKit::identify( const QPointF &point, QgsVectorLayer *layer )
{
  QgsQuickFeatureLayerPairs results;

  if ( !mMapSettings )
  {
    QgsDebugMsg( QStringLiteral( "Unable to use IdentifyKit without mapSettings property set." ) );
    return results;
  }
  QgsPointXY mapPoint = mMapSettings->mapSettings().mapToPixel().toMapCoordinates( point.toPoint() );

  if ( layer )
  {
    QgsFeatureList featureList = identifyVectorLayer( layer, mapPoint );
    for ( const QgsFeature &feature : featureList )
    {
      results.append( QgsQuickFeatureLayerPair( feature, layer ) );
    }
    QgsDebugMsg( QStringLiteral( "IdentifyKit identified %1 results for layer %2" ).arg( results.count() ).arg( layer->name() ) );
  }
  else
  {
    for ( QgsMapLayer *layer : mMapSettings->mapSettings().layers() )
    {
      if ( mMapSettings->project() && !layer->flags().testFlags( QgsMapLayer::Identifiable ) )
        continue;

      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
      if ( vl )
      {
        QgsFeatureList featureList = identifyVectorLayer( vl, mapPoint );

        for ( const QgsFeature &feature : featureList )
        {
          results.append( QgsQuickFeatureLayerPair( feature, vl ) );
        }
      }
      if ( mIdentifyMode == IdentifyMode::TopDownStopAtFirst && !results.isEmpty() )
      {
        QgsDebugMsg( QStringLiteral( "IdentifyKit identified %1 results with TopDownStopAtFirst mode." ).arg( results.count() ) );
        return results;
      }
    }

    QgsDebugMsg( QStringLiteral( "IdentifyKit identified %1 results" ).arg( results.count() ) );
  }

  return results;
}

static QgsQuickFeatureLayerPair _closestFeature( const QgsQuickFeatureLayerPairs &results, const QgsMapSettings &mapSettings, const QPointF &point )
{
  QgsPointXY mapPoint = mapSettings.mapToPixel().toMapCoordinates( point.toPoint() );
  QgsGeometry mapPointGeom( QgsGeometry::fromPointXY( mapPoint ) );

  double distMin = 1e10;
  int iMin = -1;
  for ( int i = 0; i < results.count(); ++i )
  {
    const QgsQuickFeatureLayerPair &res = results.at( i );
    QgsGeometry geom( res.feature().geometry() );
    try
    {
      geom.transform( mapSettings.layerTransform( res.layer() ) );
    }
    catch ( QgsCsException &e )
    {
      Q_UNUSED( e );
      // Caught an error in transform
      continue;
    }

    double dist = geom.distance( mapPointGeom );
    if ( dist < distMin )
    {
      iMin = i;
      distMin = dist;
    }
  }

  if ( results.empty() )
  {
    return QgsQuickFeatureLayerPair();
  }
  else
  {
    return results.at( iMin );
  }
}

QgsQuickFeatureLayerPair QgsQuickIdentifyKit::identifyOne( const QPointF &point, QgsVectorLayer *layer )
{
  QgsQuickFeatureLayerPairs results = identify( point, layer );
  return _closestFeature( results, mMapSettings->mapSettings(), point );
}

QgsFeatureList QgsQuickIdentifyKit::identifyVectorLayer( QgsVectorLayer *layer, const QgsPointXY &point ) const
{
  QgsFeatureList results;

  if ( !layer || !layer->isSpatial() )
    return results;

  if ( !layer->isInScaleRange( mMapSettings->mapSettings().scale() ) )
    return results;

  QgsFeatureList featureList;

  // toLayerCoordinates will throw an exception for an 'invalid' point.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  try
  {
    // create the search rectangle
    double searchRadius = searchRadiusMU();

    QgsRectangle r;
    r.setXMinimum( point.x() - searchRadius );
    r.setXMaximum( point.x() + searchRadius );
    r.setYMinimum( point.y() - searchRadius );
    r.setYMaximum( point.y() + searchRadius );

    r = toLayerCoordinates( layer, r );

    QgsFeatureRequest req;
    req.setFilterRect( r );
    req.setLimit( mFeaturesLimit );
    req.setFlags( QgsFeatureRequest::ExactIntersect );

    QgsFeatureIterator fit = layer->getFeatures( req );
    QgsFeature f;
    while ( fit.nextFeature( f ) )
      featureList << QgsFeature( f );
  }
  catch ( QgsCsException &cse )
  {
    QgsDebugMsg( QStringLiteral( "Invalid point, proceed without a found features." ) );
    Q_UNUSED( cse );
  }

  bool filter = false;

  QgsRenderContext context( QgsRenderContext::fromMapSettings( mMapSettings->mapSettings() ) );
  context.expressionContext() << QgsExpressionContextUtils::layerScope( layer );
  QgsFeatureRenderer *renderer = layer->renderer();
  if ( renderer && renderer->capabilities() & QgsFeatureRenderer::ScaleDependent )
  {
    // setup scale for scale dependent visibility (rule based)
    renderer->startRender( context, layer->fields() );
    filter = renderer->capabilities() & QgsFeatureRenderer::Filter;
  }

  for ( const QgsFeature &feature : featureList )
  {
    context.expressionContext().setFeature( feature );

    if ( filter && !renderer->willRenderFeature( const_cast<QgsFeature &>( feature ), context ) )
      continue;

    results.append( feature );
  }

  if ( renderer && renderer->capabilities() & QgsFeatureRenderer::ScaleDependent )
  {
    renderer->stopRender( context );
  }

  return results;
}

double QgsQuickIdentifyKit::searchRadiusMU( const QgsRenderContext &context ) const
{
  return mSearchRadiusMm * context.scaleFactor() * context.mapToPixel().mapUnitsPerPixel();
}

double QgsQuickIdentifyKit::searchRadiusMU() const
{
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mMapSettings->mapSettings() );
  return searchRadiusMU( context );
}

QgsRectangle QgsQuickIdentifyKit::toLayerCoordinates( QgsMapLayer *layer, const QgsRectangle &rect ) const
{
  return mMapSettings->mapSettings().mapToLayerCoordinates( layer, rect );
}

double QgsQuickIdentifyKit::searchRadiusMm() const
{
  return mSearchRadiusMm;
}

void QgsQuickIdentifyKit::setSearchRadiusMm( double searchRadiusMm )
{
  if ( qgsDoubleNear( mSearchRadiusMm, searchRadiusMm ) )
    return;

  mSearchRadiusMm = searchRadiusMm;
  emit searchRadiusMmChanged();
}

int QgsQuickIdentifyKit::featuresLimit() const
{
  return mFeaturesLimit;
}

void QgsQuickIdentifyKit::setFeaturesLimit( int limit )
{
  if ( mFeaturesLimit == limit )
    return;

  mFeaturesLimit = limit;
  emit featuresLimitChanged();
}
