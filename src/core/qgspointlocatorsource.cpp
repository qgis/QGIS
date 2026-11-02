/***************************************************************************
  qgspointlocatorsource.cpp
  --------------------------------------
  Date                 : September 2026
  Copyright            : (C) 2026 by the QGIS project
  Email                : info at qgis dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointlocatorsource.h"

#include "qgsannotationitem.h"
#include "qgsannotationlayer.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfeedback.h"
#include "qgslogger.h"
#include "qgsrendercontext.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#include <QString>

using namespace Qt::StringLiterals;

/// @cond PRIVATE

//
// QgsPointLocatorVectorSource
//

QgsPointLocatorVectorSource::QgsPointLocatorVectorSource( QgsVectorLayer *layer, const QgsRenderContext *context )
{
  // this has to be done on the main thread, see QgsVectorLayerFeatureSource
  mSource = std::make_unique<QgsVectorLayerFeatureSource>( layer );

  if ( context )
  {
    mContext = std::make_unique<QgsRenderContext>( *context );
    mRenderer.reset( layer->renderer() ? layer->renderer()->clone() : nullptr );
    mContext->expressionContext() << QgsExpressionContextUtils::layerScope( layer );
  }
}

QgsPointLocatorVectorSource::~QgsPointLocatorVectorSource() = default;

QString QgsPointLocatorVectorSource::id() const
{
  return mSource ? mSource->id() : QString();
}

void QgsPointLocatorVectorSource::releaseIndexingResources()
{
  mRenderer.reset();
  mContext.reset();
  mSource.reset();
}

QVector<QgsPointLocatorSource::Geometry> QgsPointLocatorVectorSource::snappableGeometries(
  const QgsRectangle *extent, const QgsCoordinateTransform &transform, int maxFeaturesToIndex, bool &ok, QgsFeedback *feedback
)
{
  ok = true;
  QVector<Geometry> geometries;

  QgsFeature f;
  QgsFeatureRequest request;
  request.setNoAttributes();

  if ( extent )
  {
    QgsRectangle rect = *extent;
    if ( !transform.isShortCircuited() )
    {
      QgsCoordinateTransform rectTransform = transform;
      rectTransform.setBallparkTransformsAreAppropriate( true );
      try
      {
        rect = rectTransform.transformBoundingBox( rect, Qgis::TransformDirection::Reverse );
      }
      catch ( const QgsException &e )
      {
        Q_UNUSED( e )
        // See https://github.com/qgis/QGIS/issues/20749
        QgsDebugError( u"could not transform bounding box to map, skipping the snap filter (%1)"_s.arg( e.what() ) );
      }
    }
    request.setFilterRect( rect );
  }

  bool filter = false;
  QgsRenderContext *ctx = nullptr;
  if ( mContext )
  {
    ctx = mContext.get();
    if ( mRenderer )
    {
      // setup scale for scale dependent visibility (rule based)
      mRenderer->startRender( *ctx, mSource->fields() );
      filter = mRenderer->capabilities() & QgsFeatureRenderer::Filter;
      request.setSubsetOfAttributes( mRenderer->usedAttributes( *ctx ), mSource->fields() );
    }
  }

  QgsFeatureIterator fi = mSource->getFeatures( request );
  int indexedCount = 0;

  while ( fi.nextFeature( f ) )
  {
    if ( feedback && feedback->isCanceled() )
    {
      ok = false;
      break;
    }

    if ( !f.hasGeometry() )
      continue;

    if ( filter && ctx && mRenderer )
    {
      ctx->expressionContext().setFeature( f );
      if ( !mRenderer->willRenderFeature( f, *ctx ) )
      {
        continue;
      }
    }

    if ( transform.isValid() )
    {
      try
      {
        QgsGeometry transformedGeometry = f.geometry();
        transformedGeometry.transform( transform );
        f.setGeometry( transformedGeometry );
      }
      catch ( const QgsException &e )
      {
        Q_UNUSED( e )
        // See https://github.com/qgis/QGIS/issues/20749
        QgsDebugError( u"could not transform geometry to map, skipping the snap for it (%1)"_s.arg( e.what() ) );
        continue;
      }
    }

    const QgsRectangle bbox = f.geometry().boundingBox();
    if ( bbox.isFinite() )
    {
      geometries.append( Geometry { f.id(), f.geometry() } );
      ++indexedCount;
    }

    if ( maxFeaturesToIndex != -1 && indexedCount > maxFeaturesToIndex )
    {
      ok = false;
      break;
    }
  }

  if ( ctx && mRenderer )
  {
    mRenderer->stopRender( *ctx );
  }

  if ( !ok )
    return QVector<Geometry>();

  return geometries;
}


//
// QgsPointLocatorAnnotationSource
//

QgsPointLocatorAnnotationSource::QgsPointLocatorAnnotationSource( QgsAnnotationLayer *layer )
  : mLayerId( layer->id() )
{
  // Snapshot the snappable items on the main thread. The live layer must not be read once indexing
  // moves to a worker thread.
  const QMap<QString, QgsAnnotationItem *> items = layer->items();

  QgsFeatureId nextId = 0;
  for ( auto it = items.constBegin(); it != items.constEnd(); ++it )
  {
    QgsAnnotationItem *item = it.value();
    if ( !item )
      continue;

    // a disabled item is not drawn, so it should not be snappable
    if ( !item->enabled() )
      continue;

    const QgsAbstractGeometry *snapGeom = item->snapGeometry();
    if ( !snapGeom || snapGeom->isEmpty() )
      continue; // item type does not support snapping

    // the index needs its own copy it can reproject and that survives the item changing
    const QgsFeatureId fid = nextId++;
    mSnapshot.insert( fid, QgsGeometry( snapGeom->clone() ) );
    mItemIds.insert( fid, it.key() );
  }
}

void QgsPointLocatorAnnotationSource::releaseIndexingResources()
{
  // keep mItemIds, it is still needed to stamp matches at query time
  mSnapshot.clear();
}

QVector<QgsPointLocatorSource::Geometry> QgsPointLocatorAnnotationSource::snappableGeometries(
  const QgsRectangle *extent, const QgsCoordinateTransform &transform, int maxFeaturesToIndex, bool &ok, QgsFeedback *feedback
)
{
  ok = true;
  QVector<Geometry> geometries;

  int indexedCount = 0;
  for ( auto it = mSnapshot.constBegin(); it != mSnapshot.constEnd(); ++it )
  {
    if ( feedback && feedback->isCanceled() )
    {
      ok = false;
      break;
    }

    QgsGeometry geom = it.value();
    if ( transform.isValid() )
    {
      try
      {
        geom.transform( transform );
      }
      catch ( const QgsException &e )
      {
        Q_UNUSED( e )
        QgsDebugError( u"could not transform annotation geometry to map, skipping the snap for it (%1)"_s.arg( e.what() ) );
        continue;
      }
    }

    if ( !geom.boundingBox().isFinite() )
      continue;

    // geom and extent are both in destination CRS now
    if ( extent && !geom.boundingBoxIntersects( *extent ) )
      continue;

    geometries.append( Geometry { it.key(), geom } );
    ++indexedCount;

    if ( maxFeaturesToIndex != -1 && indexedCount > maxFeaturesToIndex )
    {
      ok = false;
      break;
    }
  }

  if ( !ok )
    return QVector<Geometry>();

  return geometries;
}

/// @endcond
