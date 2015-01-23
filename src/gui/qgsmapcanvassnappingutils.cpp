#include "qgsmapcanvassnappingutils.h"

#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"

#include <QProgressDialog>

QgsMapCanvasSnappingUtils::QgsMapCanvasSnappingUtils( QgsMapCanvas* canvas, QObject* parent )
    : QgsSnappingUtils( parent )
    , mCanvas( canvas )
{
  connect( canvas, SIGNAL( extentsChanged() ), this, SLOT( canvasMapSettingsChanged() ) );
  connect( canvas, SIGNAL( destinationCrsChanged() ), this, SLOT( canvasMapSettingsChanged() ) );
  connect( canvas, SIGNAL( layersChanged() ), this, SLOT( canvasMapSettingsChanged() ) );
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

void QgsMapCanvasSnappingUtils::prepareIndexStarting( int count )
{
  mProgress = new QProgressDialog( tr( "Indexing data..." ), QString(), 0, count, mCanvas->topLevelWidget() );
  mProgress->setWindowModality( Qt::WindowModal );
}

void QgsMapCanvasSnappingUtils::prepareIndexProgress( int index )
{
  mProgress->setValue( index );
  if ( index == mProgress->maximum() )
  {
    delete mProgress;
    mProgress = 0;
  }
}
