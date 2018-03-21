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


#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"

#include "qgsquickidentifykit.h"
#include "qgsquickmapsettings.h"

QgsQuickIdentifyKit::QgsQuickIdentifyKit( QObject *parent )
  : QObject( parent )
  , mMapSettings( nullptr )
  , mSearchRadiusMm( 8 )
  , mFeaturesLimit( 100 )
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


QList<QgsQuickFeature> QgsQuickIdentifyKit::identify( const QPointF &point )
{
  QList<QgsQuickFeature> results;

  if ( !mMapSettings )
  {
    qWarning() << "Unable to use IdentifyKit without mapSettings property set.";
    return results;
  }

  QgsPointXY mapPoint = mMapSettings->mapSettings().mapToPixel().toMapCoordinates( point.toPoint() );

  QStringList noIdentifyLayerIdList;
  if ( mMapSettings->project() )
  {
    noIdentifyLayerIdList = mMapSettings->project()->nonIdentifiableLayers();
  }

  Q_FOREACH ( QgsMapLayer *layer, mMapSettings->mapSettings().layers() )
  {
    if ( mMapSettings->project() && noIdentifyLayerIdList.contains( layer->id() ) )
      continue;

    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
    if ( vl )
    {
      QgsFeatureList featureList = identifyVectorLayer( vl, mapPoint );

      Q_FOREACH ( const QgsFeature &feature, featureList )
      {
        results.append( QgsQuickFeature( feature, vl ) );
      }
    }
  }

  QgsDebugMsg( QString( "IdentifyKit identified %1 results" ).arg( results.count() ) );
  return results;
}

static QgsFeature _closestFeature( const QgsFeatureList &results, const QgsMapSettings &mapSettings, QgsVectorLayer *layer, const QPointF &point )
{
  QgsPointXY mapPoint = mapSettings.mapToPixel().toMapCoordinates( point.toPoint() );
  QgsGeometry mapPointGeom( QgsGeometry::fromPointXY( mapPoint ) );
  QgsCoordinateTransform ctLayerToMap = mapSettings.layerTransform( layer );

  Q_ASSERT( results.count() != 0 );
  double distMin = 1e10;
  int iMin = -1;
  for ( int i = 0; i < results.count(); ++i )
  {
    QgsGeometry geom( results.at( i ).geometry() );
    geom.transform( ctLayerToMap );

    double dist = geom.distance( mapPointGeom );
    if ( dist < distMin )
    {
      iMin = i;
      distMin = dist;
    }
  }
  return results.at( iMin );
}


static QgsQuickFeature _closestFeature( const QList<QgsQuickFeature> &results, const QgsMapSettings &mapSettings, const QPointF &point )
{
  QgsPointXY mapPoint = mapSettings.mapToPixel().toMapCoordinates( point.toPoint() );
  QgsGeometry mapPointGeom( QgsGeometry::fromPointXY( mapPoint ) );

  Q_ASSERT( results.count() != 0 );
  double distMin = 1e10;
  int iMin = -1;
  for ( int i = 0; i < results.count(); ++i )
  {
    const QgsQuickFeature &res = results.at( i );
    QgsGeometry geom( res.feature().geometry() );
    geom.transform( mapSettings.layerTransform( res.layer() ) );

    double dist = geom.distance( mapPointGeom );
    if ( dist < distMin )
    {
      iMin = i;
      distMin = dist;
    }
  }
  return results.at( iMin );
}


QgsFeature QgsQuickIdentifyKit::identifyOne( QgsVectorLayer *layer, const QPointF &point )
{
  QgsFeatureList results = identify( layer, point );
  if ( results.empty() )
  {
    QgsFeature f = QgsFeature();
    f.setValid( false );
    return f;
  }
  else
  {
    return _closestFeature( results, mMapSettings->mapSettings(), layer, point );
  }
}


QgsQuickFeature QgsQuickIdentifyKit::identifyOne( const QPointF &point )
{
  QList<QgsQuickFeature> results = identify( point );
  if ( results.empty() )
  {
    QgsQuickFeature emptyRes;
    return emptyRes;
  }
  else
  {
    return _closestFeature( results, mMapSettings->mapSettings(), point );
  }
}

QgsFeatureList QgsQuickIdentifyKit::identify( QgsVectorLayer *layer, const QPointF &point )
{
  QgsFeatureList results;

  Q_ASSERT( layer );

  if ( !mMapSettings )
  {
    qWarning() << "Unable to use IdentifyKit without mapSettings property set.";
    return results;
  }
  QgsPointXY mapPoint = mMapSettings->mapSettings().mapToPixel().toMapCoordinates( point.toPoint() );

  results = identifyVectorLayer( layer, mapPoint );

  QgsDebugMsg( QString( "IdentifyKit identified %1 results for layer %2" ).arg( results.count() ).arg( layer->name() ) );
  return results;
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
    Q_UNUSED( cse );
    // catch exception for 'invalid' point and proceed with no features found
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

  Q_FOREACH ( const QgsFeature &feature, featureList )
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
  if ( mSearchRadiusMm == searchRadiusMm )
    return;

  mSearchRadiusMm = searchRadiusMm;
  emit searchRadiusMmChanged();
}
