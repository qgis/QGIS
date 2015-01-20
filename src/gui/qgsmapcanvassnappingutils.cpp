#include "qgsmapcanvassnappingutils.h"

#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"

QgsMapCanvasSnappingUtils::QgsMapCanvasSnappingUtils( QgsMapCanvas* canvas, QObject* parent )
  : QgsSnappingUtils( parent )
  , mCanvas( canvas )
{
  connect( canvas, SIGNAL( extentsChanged() ), this, SLOT( canvasMapSettingsChanged() ) );
  connect( canvas, SIGNAL( destinationCrsChanged() ), this, SLOT( canvasMapSettingsChanged() ) );
  connect( canvas, SIGNAL( currentLayerChanged( QgsMapLayer* ) ), this, SLOT( canvasCurrentLayerChanged() ) );
  canvasMapSettingsChanged();
  canvasCurrentLayerChanged();
}

void QgsMapCanvasSnappingUtils::canvasMapSettingsChanged()
{
  setMapSettings( mCanvas->mapSettings() );
}

void QgsMapCanvasSnappingUtils::canvasCurrentLayerChanged()
{
  setCurrentLayer( qobject_cast<QgsVectorLayer*>( mCanvas->currentLayer() ) );
}
