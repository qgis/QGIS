#include "qgsmapcanvastracer.h"

#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"

#include <QAction>

QHash<QgsMapCanvas*, QgsMapCanvasTracer*> QgsMapCanvasTracer::sTracers;


QgsMapCanvasTracer::QgsMapCanvasTracer( QgsMapCanvas* canvas )
    : mCanvas( canvas )
{
  sTracers.insert( canvas, this );

  connect( canvas, SIGNAL( destinationCrsChanged() ), this, SLOT( updateSettings() ) );
  connect( canvas, SIGNAL( layersChanged() ), this, SLOT( updateSettings() ) );
  connect( canvas, SIGNAL( extentsChanged() ), this, SLOT( updateSettings() ) );
  // TODO: watch for snapping changes

  mActionEnableTracing = new QAction( QIcon( QgsApplication::getThemeIcon( "/mActionTracing.png" ) ), tr( "Enable Tracing" ), this );
  mActionEnableTracing->setShortcut( Qt::Key_T );
  mActionEnableTracing->setCheckable( true );

  updateSettings(); // initialize
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
  QList<QgsVectorLayer*> layers;
  foreach ( const QString& layerId, mCanvas->mapSettings().layers() )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerId ) );
    if ( vl )
      layers << vl;
  }
  setLayers( layers );

  setDestinationCrs( mCanvas->mapSettings().destinationCrs() );
  setExtent( mCanvas->extent() );
}
