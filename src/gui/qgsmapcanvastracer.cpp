#include "qgsmapcanvastracer.h"

#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"

#include <QAction>

QHash<QgsMapCanvas*, QgsMapCanvasTracer*> QgsMapCanvasTracer::sTracers;


QgsMapCanvasTracer::QgsMapCanvasTracer( QgsMapCanvas* canvas )
    : mCanvas( canvas )
{
  sTracers.insert( canvas, this );

  connect( canvas, SIGNAL( destinationCrsChanged() ), this, SLOT( updateSettings() ) );
  connect( canvas, SIGNAL( layersChanged() ), this, SLOT( updateLayerSettings() ) );
  connect( canvas, SIGNAL( extentsChanged() ), this, SLOT( updateSettings() ) );
  connect( canvas, SIGNAL( currentLayerChanged( QgsMapLayer* ) ), this, SLOT( onCurrentLayerChanged() ) );
  connect( canvas->snappingUtils(), SIGNAL( configChanged() ), this, SLOT( updateLayerSettings() ) );

  mActionEnableTracing = new QAction( QIcon( QgsApplication::getThemeIcon( "/mActionTracing.png" ) ), tr( "Enable Tracing" ), this );
  mActionEnableTracing->setShortcut( Qt::Key_T );
  mActionEnableTracing->setCheckable( true );

  updateSettings(); // initialize
  updateLayerSettings();
}

QgsMapCanvasTracer::~QgsMapCanvasTracer()
{
  sTracers.remove( mCanvas );
}

QgsMapCanvasTracer* QgsMapCanvasTracer::tracerForCanvas( QgsMapCanvas* canvas )
{
  return sTracers.value( canvas, 0 );
}

void QgsMapCanvasTracer::updateSettings()
{
  setDestinationCrs( mCanvas->mapSettings().destinationCrs() );
  setExtent( mCanvas->extent() );
}

void QgsMapCanvasTracer::updateLayerSettings()
{
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
      foreach ( const QString& layerId, visibleLayerIds )
      {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
        if ( vl )
          layers << vl;
      }
      break;
    case QgsSnappingUtils::SnapAdvanced:
      foreach ( const QgsSnappingUtils::LayerConfig& cfg, mCanvas->snappingUtils()->layers() )
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
    updateLayerSettings();
}
