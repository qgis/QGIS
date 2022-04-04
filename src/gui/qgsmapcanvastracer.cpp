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
#include "qgsproject.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgssnappingconfig.h"
#include "qgssettingsregistrycore.h"
#include "qgsrendercontext.h"

#include <QAction>

#include <QGlobalStatic>

typedef QHash<QgsMapCanvas *, QgsMapCanvasTracer *> TracerCanvasHash;
Q_GLOBAL_STATIC( TracerCanvasHash, sTracers );

QgsMapCanvasTracer::QgsMapCanvasTracer( QgsMapCanvas *canvas, QgsMessageBar *messageBar )
  : mCanvas( canvas )
  , mMessageBar( messageBar )

{
  sTracers()->insert( canvas, this );

  // when things change we just invalidate the graph - and set up new parameters again only when necessary
  connect( canvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMapCanvasTracer::invalidateGraph );
  connect( canvas, &QgsMapCanvas::transformContextChanged, this, &QgsMapCanvasTracer::invalidateGraph );
  connect( canvas, &QgsMapCanvas::layersChanged, this, &QgsMapCanvasTracer::invalidateGraph );
  connect( canvas, &QgsMapCanvas::extentsChanged, this, &QgsMapCanvasTracer::invalidateGraph );
  connect( canvas, &QgsMapCanvas::currentLayerChanged, this, &QgsMapCanvasTracer::onCurrentLayerChanged );
  connect( canvas->snappingUtils(), &QgsSnappingUtils::configChanged, this, &QgsMapCanvasTracer::invalidateGraph );
  connect( canvas, &QObject::destroyed, this, [this]() { mCanvas = nullptr; } );

  // arbitrarily chosen limit that should allow for fairly fast initialization
  // of the underlying graph structure
  setMaxFeatureCount( QgsSettingsRegistryCore::settingsDigitizingTracingMaxFeatureCount.value() );
}

QgsMapCanvasTracer::~QgsMapCanvasTracer()
{
  sTracers->remove( mCanvas );
}

QgsMapCanvasTracer *QgsMapCanvasTracer::tracerForCanvas( QgsMapCanvas *canvas )
{
  return sTracers->value( canvas, nullptr );
}

void QgsMapCanvasTracer::reportError( QgsTracer::PathError err, bool addingVertex )
{
  Q_UNUSED( addingVertex )

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

  mLastMessage = new QgsMessageBarItem( tr( "Tracing" ), message, Qgis::MessageLevel::Warning, QgsMessageBar::defaultMessageTimeout( Qgis::MessageLevel::Info ) );
  mMessageBar->pushItem( mLastMessage );
}

void QgsMapCanvasTracer::configure()
{
  setDestinationCrs( mCanvas->mapSettings().destinationCrs(), mCanvas->mapSettings().transformContext() );
  const QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mCanvas->mapSettings() );
  setRenderContext( &ctx );
  setExtent( mCanvas->extent() );

  QList<QgsVectorLayer *> layers;
  const QList<QgsMapLayer *> visibleLayers = mCanvas->mapSettings().layers( true );

  switch ( mCanvas->snappingUtils()->config().mode() )
  {
    default:
    case Qgis::SnappingMode::ActiveLayer:
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
      if ( vl && visibleLayers.contains( vl ) )
        layers << vl;
    }
    break;
    case Qgis::SnappingMode::AllLayers:
    {
      const auto constVisibleLayers = visibleLayers;
      for ( QgsMapLayer *layer : constVisibleLayers )
      {
        QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
        if ( vl )
          layers << vl;
      }
    }
    break;
    case Qgis::SnappingMode::AdvancedConfiguration:
    {
      const auto constLayers = mCanvas->snappingUtils()->layers();
      for ( const QgsSnappingUtils::LayerConfig &cfg : constLayers )
      {
        if ( visibleLayers.contains( cfg.layer ) )
          layers << cfg.layer;
      }
    }
    break;
  }

  setLayers( layers );
}

void QgsMapCanvasTracer::onCurrentLayerChanged()
{
  // no need to bother if we are not snapping
  if ( mCanvas->snappingUtils()->config().mode() == Qgis::SnappingMode::ActiveLayer )
    invalidateGraph();
}
