#include "qgsbrowserdockwidget.h"

#include <QTreeView>

#include "qgsbrowsermodel.h"
#include "qgsdataitem.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

QgsBrowserDockWidget::QgsBrowserDockWidget( QWidget * parent ) :
    QDockWidget( parent ), mModel( NULL )
{
  setWindowTitle( tr( "Browser" ) );

  mBrowserView = new QTreeView( this );
  mBrowserView->setDragEnabled( true );
  mBrowserView->setDragDropMode( QTreeView::DragOnly );
  setWidget( mBrowserView );

  //connect( mBrowserView, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( itemClicked( const QModelIndex& ) ) );
  connect( mBrowserView, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( itemClicked( const QModelIndex& ) ) );
}

void QgsBrowserDockWidget::showEvent( QShowEvent * e )
{
  // delayed initialization of the model
  if ( mModel == NULL )
  {
    mModel = new QgsBrowserModel( mBrowserView );
    mBrowserView->setModel( mModel );
  }

  QDockWidget::showEvent( e );
}


void QgsBrowserDockWidget::itemClicked( const QModelIndex& index )
{
  QgsDataItem *item = mModel->dataItem( index );
  if ( !item )
    return;

  QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( mModel->dataItem( index ) );
  if ( layerItem == NULL )
    return;

  QString uri = layerItem->uri();
  if ( uri.isEmpty() )
    return;

  QgsMapLayer::LayerType type = layerItem->mapLayerType();
  QString providerKey = layerItem->providerKey();

  QgsDebugMsg( providerKey + " : " + uri );
  QgsMapLayer* layer = NULL;
  if ( type == QgsMapLayer::VectorLayer )
  {
    layer = new QgsVectorLayer( uri, layerItem->name(), providerKey );
  }
  if ( type == QgsMapLayer::RasterLayer )
  {
    // This should go to WMS provider
    QStringList URIParts = uri.split( "|" );
    QString rasterLayerPath = URIParts.at( 0 );
    QStringList layers;
    QStringList styles;
    QString format;
    QString crs;
    for ( int i = 1 ; i < URIParts.size(); i++ )
    {
      QString part = URIParts.at( i );
      int pos = part.indexOf( "=" );
      QString field = part.left( pos );
      QString value = part.mid( pos + 1 );

      if ( field == "layers" )
        layers = value.split( "," );
      if ( field == "styles" )
        styles = value.split( "," );
      if ( field == "format" )
        format = value;
      if ( field == "crs" )
        crs = value;
    }
    QgsDebugMsg( "rasterLayerPath = " + rasterLayerPath );
    QgsDebugMsg( "layers = " + layers.join( " " ) );

    layer = new QgsRasterLayer( 0, rasterLayerPath, layerItem->name(), providerKey, layers, styles, format, crs );
  }

  if ( !layer || !layer->isValid() )
  {
    qDebug( "No layer" );
    delete layer;
    return;
  }

  // add layer to the application
  QgsMapLayerRegistry::instance()->addMapLayer( layer );
}
