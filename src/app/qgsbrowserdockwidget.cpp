#include "qgsbrowserdockwidget.h"

#include <QTreeView>
#include <QMenu>
#include <QSettings>

#include "qgsbrowsermodel.h"
#include "qgsdataitem.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include <QDragEnterEvent>
/**
Utility class for correct drag&drop handling.

We want to allow user to drag layers to qgis window. At the same time we do not
accept drops of the items on our view - but if we ignore the drag enter action
then qgis application consumes the drag events and it is possible to drop the
items on the tree view although the drop is actually managed by qgis app.
 */
class QgsBrowserTreeView : public QTreeView
{
public:
  QgsBrowserTreeView( QWidget* parent ) : QTreeView(parent)
  {
    setDragDropMode( QTreeView::DragDrop ); // sets also acceptDrops + dragEnabled
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setContextMenuPolicy( Qt::CustomContextMenu );
  }

  void dragEnterEvent(QDragEnterEvent* e)
  {
    // accept drag enter so that our widget will not get ignored
    // and drag events will not get passed to QgisApp
    e->accept();
  }
  void dragMoveEvent(QDragMoveEvent* e)
  {
    // ignore all possibilities where an item could be dropped
    // because we want that user drops the item on canvas / legend / app
    e->ignore();
  }
};

QgsBrowserDockWidget::QgsBrowserDockWidget( QWidget * parent ) :
    QDockWidget( parent ), mModel( NULL )
{
  setWindowTitle( tr( "Browser" ) );

  mBrowserView = new QgsBrowserTreeView( this );
  setWidget( mBrowserView );

  connect( mBrowserView, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showContextMenu( const QPoint & ) ) );
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

void QgsBrowserDockWidget::showContextMenu( const QPoint & pt )
{
  QModelIndex idx = mBrowserView->indexAt( pt );
  QgsDataItem* item = mModel->dataItem( idx );
  if ( !item )
    return;

  QMenu* menu = new QMenu( this );

  if ( item->type() == QgsDataItem::Directory )
  {
    QSettings settings;
    QStringList favDirs = settings.value( "/browser/favourites" ).toStringList();
    bool inFavDirs = favDirs.contains( item->path() );

    if ( item->parent() != NULL && !inFavDirs )
    {
      // only non-root directories can be added as favourites
      menu->addAction( tr( "Add as a favourite" ), this, SLOT( addFavourite() ) );
    }
    else if ( inFavDirs )
    {
      // only favourites can be removed
      menu->addAction( tr( "Remove favourite" ), this, SLOT( removeFavourite() ) );
    }
  }

  if ( menu->actions().count() == 0 )
  {
    delete menu;
    return;
  }

  menu->popup( mBrowserView->mapToGlobal( pt ) );
}

void QgsBrowserDockWidget::addFavourite()
{
  QgsDataItem* item = mModel->dataItem( mBrowserView->currentIndex() );
  if ( !item )
    return;
  if ( item->type() != QgsDataItem::Directory )
    return;

  QString newFavDir = item->path();

  QSettings settings;
  QStringList favDirs = settings.value( "/browser/favourites" ).toStringList();
  favDirs.append( newFavDir );
  settings.setValue( "/browser/favourites", favDirs );

  // reload the browser model so that the newly added favourite directory is shown
  mModel->reload();
}

void QgsBrowserDockWidget::removeFavourite()
{
  QgsDataItem* item = mModel->dataItem( mBrowserView->currentIndex() );
  if ( !item )
    return;
  if ( item->type() != QgsDataItem::Directory )
    return;

  QString favDir  = item->path();

  QSettings settings;
  QStringList favDirs = settings.value( "/browser/favourites" ).toStringList();
  favDirs.removeAll( favDir );
  settings.setValue( "/browser/favourites", favDirs );

  // reload the browser model so that the favourite directory is not shown anymore
  mModel->reload();
}
