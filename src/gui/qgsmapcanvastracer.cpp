/***************************************************************************
    qgsmapcanvastracer.cpp
    ---------------------
    begin                : January 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmapcanvastracer.h"

#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"

#include <QAction>

QHash<QgsMapCanvas*, QgsMapCanvasTracer*> QgsMapCanvasTracer::sTracers;


QgsMapCanvasTracer::QgsMapCanvasTracer( QgsMapCanvas* canvas, QgsMessageBar* messageBar )
    : mCanvas( canvas )
    , mMessageBar( messageBar )
    , mLastMessage( nullptr )
    , mActionEnableTracing( nullptr )
{
  sTracers.insert( canvas, this );

  // when things change we just invalidate the graph - and set up new parameters again only when necessary
  connect( canvas, SIGNAL( destinationCrsChanged() ), this, SLOT( invalidateGraph() ) );
  connect( canvas, SIGNAL( layersChanged() ), this, SLOT( invalidateGraph() ) );
  connect( canvas, SIGNAL( extentsChanged() ), this, SLOT( invalidateGraph() ) );
  connect( canvas, SIGNAL( currentLayerChanged( QgsMapLayer* ) ), this, SLOT( onCurrentLayerChanged() ) );
  connect( canvas->snappingUtils(), SIGNAL( configChanged() ), this, SLOT( invalidateGraph() ) );

  // arbitrarily chosen limit that should allow for fairly fast initialization
  // of the underlying graph structure
  setMaxFeatureCount( QSettings().value( "/qgis/digitizing/tracing_max_feature_count", 10000 ).toInt() );
}

QgsMapCanvasTracer::~QgsMapCanvasTracer()
{
  sTracers.remove( mCanvas );
}

QgsMapCanvasTracer* QgsMapCanvasTracer::tracerForCanvas( QgsMapCanvas* canvas )
{
  return sTracers.value( canvas, 0 );
}

void QgsMapCanvasTracer::reportError( QgsTracer::PathError err, bool addingVertex )
{
  Q_UNUSED( addingVertex );

  if ( !mMessageBar )
    return;

  // remove previous message (if any)
  mMessageBar->popWidget( mLastMessage );
  mLastMessage = nullptr;

  QString message;
  switch ( err )
  {
    case ErrTooManyFeatures:
      message = tr( "Disabled - there are too many features displayed. Try zooming in or disable some layers." );
      break;
    case ErrNone:
    default:
      break;
  }

  if ( message.isEmpty() && hasTopologyProblem() )
  {
    message = tr( "Tracing may not work correctly. Please check topology of the input layers." );
  }

  if ( message.isEmpty() )
    return;

  mLastMessage = new QgsMessageBarItem( tr( "Tracing" ), message, QgsMessageBar::WARNING,
                                        QSettings().value( "/qgis/messageTimeout", 5 ).toInt() );
  mMessageBar->pushItem( mLastMessage );
}

void QgsMapCanvasTracer::configure()
{
  setCrsTransformEnabled( mCanvas->mapSettings().hasCrsTransformEnabled() );
  setDestinationCrs( mCanvas->mapSettings().destinationCrs() );
  setExtent( mCanvas->extent() );

  QList<QgsVectorLayer*> layers;
  QStringList visibleLayerIds = mCanvas->mapSettings().layers();

  switch ( mCanvas->snappingUtils()->snapToMapMode() )
  {
    default:
    case QgsSnappingUtils::SnapCurrentLayer:
    {
      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( mCanvas->currentLayer() );
      if ( vl && visibleLayerIds.contains( vl->id() ) )
        layers << vl;
    }
    break;
    case QgsSnappingUtils::SnapAllLayers:
      Q_FOREACH ( const QString& layerId, visibleLayerIds )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
        if ( vl )
          layers << vl;
      }
      break;
    case QgsSnappingUtils::SnapAdvanced:
      Q_FOREACH ( const QgsSnappingUtils::LayerConfig& cfg, mCanvas->snappingUtils()->layers() )
      {
        if ( visibleLayerIds.contains( cfg.layer->id() ) )
          layers << cfg.layer;
      }
      break;
  }

  setLayers( layers );
}

void QgsMapCanvasTracer::onCurrentLayerChanged()
{
  // no need to bother if we are not snapping
  if ( mCanvas->snappingUtils()->snapToMapMode() == QgsSnappingUtils::SnapCurrentLayer )
    invalidateGraph();
}
