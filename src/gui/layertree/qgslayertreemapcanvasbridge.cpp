/***************************************************************************
  qgslayertreemapcanvasbridge.cpp
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreemapcanvasbridge.h"

#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgsmapoverviewcanvas.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsgui.h"

QgsLayerTreeMapCanvasBridge::QgsLayerTreeMapCanvasBridge( QgsLayerTree *root, QgsMapCanvas *canvas, QObject *parent )
  : QObject( parent )
  , mRoot( root )
  , mCanvas( canvas )
  , mPendingCanvasUpdate( false )
  , mAutoSetupOnFirstLayer( true )
  , mHasLayersLoaded( !root->findLayers().isEmpty() )
{
  connect( root, &QgsLayerTreeGroup::customPropertyChanged, this, &QgsLayerTreeMapCanvasBridge::nodeCustomPropertyChanged );
  connect( root, &QgsLayerTreeNode::visibilityChanged, this, &QgsLayerTreeMapCanvasBridge::nodeVisibilityChanged );
  connect( root, &QgsLayerTree::layerOrderChanged, this, &QgsLayerTreeMapCanvasBridge::deferredSetCanvasLayers );

  connect( QgsProject::instance(), &QgsProject::layersAdded, this, &QgsLayerTreeMapCanvasBridge::layersAdded );

  setCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::setCanvasLayers()
{
  QList<QgsMapLayer *> canvasLayers, overviewLayers, allLayerOrder;

  if ( mRoot->hasCustomLayerOrder() )
  {
    const QList<QgsMapLayer *> customOrderLayers = mRoot->customLayerOrder();
    for ( const QgsMapLayer *layer : customOrderLayers )
    {
      QgsLayerTreeLayer *nodeLayer = mRoot->findLayer( layer->id() );
      if ( nodeLayer )
      {
        if ( !nodeLayer->layer()->isSpatial() )
          continue;

        allLayerOrder << nodeLayer->layer();
        if ( nodeLayer->isVisible() )
          canvasLayers << nodeLayer->layer();
        if ( nodeLayer->customProperty( QStringLiteral( "overview" ), 0 ).toInt() )
          overviewLayers << nodeLayer->layer();
      }
    }
  }
  else
  {
    setCanvasLayers( mRoot, canvasLayers, overviewLayers, allLayerOrder );
  }

  const QList<QgsLayerTreeLayer *> layerNodes = mRoot->findLayers();
  int currentSpatialLayerCount = 0;
  int currentValidSpatialLayerCount = 0;
  for ( QgsLayerTreeLayer *layerNode : layerNodes )
  {
    if ( layerNode->layer() && layerNode->layer()->isSpatial() )
    {
      currentSpatialLayerCount++;
      if ( layerNode->layer()->isValid() )
        currentValidSpatialLayerCount++;
    }
  }

  const bool firstLayers = mAutoSetupOnFirstLayer && !mHasLayersLoaded && currentSpatialLayerCount != 0;
  const bool firstValidLayers = mAutoSetupOnFirstLayer && !mHasValidLayersLoaded && currentValidSpatialLayerCount != 0;

  mCanvas->setLayers( canvasLayers );
  if ( mOverviewCanvas )
    mOverviewCanvas->setLayers( overviewLayers );

  if ( firstValidLayers )
  {
    // if we are moving from zero to non-zero layers, let's zoom to those data (only consider valid layers here!)
    mCanvas->zoomToProjectExtent();
  }

  if ( !mFirstCRS.isValid() )
  {
    // find out what is the first used CRS in case we may need to turn on OTF projections later
    for ( const QgsLayerTreeLayer *layerNode : layerNodes )
    {
      if ( layerNode->layer() && layerNode->layer()->crs().isValid() )
      {
        mFirstCRS = layerNode->layer()->crs();
        break;
      }
    }
  }

  if ( mFirstCRS.isValid() && firstLayers )
  {
    const QgsGui::ProjectCrsBehavior projectCrsBehavior = QgsSettings().enumValue( QStringLiteral( "/projections/newProjectCrsBehavior" ),  QgsGui::UseCrsOfFirstLayerAdded, QgsSettings::App );
    switch ( projectCrsBehavior )
    {
      case QgsGui::UseCrsOfFirstLayerAdded:
      {
        const bool planimetric = QgsSettings().value( QStringLiteral( "measure/planimetric" ), true, QgsSettings::Core ).toBool();
        // Only adjust ellipsoid to CRS if it's not set to planimetric
        QgsProject::instance()->setCrs( mFirstCRS, !planimetric );
        break;
      }

      case QgsGui::UsePresetCrs:
        break;
    }
  }

  mHasLayersLoaded = currentSpatialLayerCount;
  mHasValidLayersLoaded = currentValidSpatialLayerCount;
  if ( currentSpatialLayerCount == 0 )
    mFirstCRS = QgsCoordinateReferenceSystem();

  mPendingCanvasUpdate = false;

  emit canvasLayersChanged( canvasLayers );
}

void QgsLayerTreeMapCanvasBridge::setCanvasLayers( QgsLayerTreeNode *node, QList<QgsMapLayer *> &canvasLayers, QList<QgsMapLayer *> &overviewLayers, QList<QgsMapLayer *> &allLayers )
{
  if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
    if ( nodeLayer->layer() && nodeLayer->layer()->isSpatial() )
    {
      allLayers << nodeLayer->layer();
      if ( nodeLayer->isVisible() )
        canvasLayers << nodeLayer->layer();
      if ( nodeLayer->customProperty( QStringLiteral( "overview" ), 0 ).toInt() )
        overviewLayers << nodeLayer->layer();
    }
  }

  const QList<QgsLayerTreeNode *> children = node->children();
  for ( QgsLayerTreeNode *child : children )
  {
    if ( QgsLayerTreeGroup *group = QgsLayerTree::toGroup( node ) )
    {
      if ( QgsGroupLayer *groupLayer = group->groupLayer() )
      {
        canvasLayers << groupLayer;
        continue;
      }
    }
    setCanvasLayers( child, canvasLayers, overviewLayers, allLayers );
  }
}

void QgsLayerTreeMapCanvasBridge::deferredSetCanvasLayers()
{
  if ( mPendingCanvasUpdate )
    return;

  mPendingCanvasUpdate = true;
  QMetaObject::invokeMethod( this, "setCanvasLayers", Qt::QueuedConnection );
}

void QgsLayerTreeMapCanvasBridge::nodeVisibilityChanged()
{
  deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::nodeCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key )
{
  Q_UNUSED( node )
  if ( key == QLatin1String( "overview" ) )
    deferredSetCanvasLayers();
}

void QgsLayerTreeMapCanvasBridge::layersAdded( const QList<QgsMapLayer *> &layers )
{
  for ( QgsMapLayer *l : layers )
  {
    if ( l )
    {
      connect( l, &QgsMapLayer::dataSourceChanged, this, [ this, l ]
      {
        if ( l->isValid() && l->isSpatial() && mAutoSetupOnFirstLayer && !mHasValidLayersLoaded )
        {
          mHasValidLayersLoaded = true;
          // if we are moving from zero valid layers to non-zero VALID layers, let's zoom to those data
          mCanvas->zoomToProjectExtent();
        }
      } );
    }
  }
}
