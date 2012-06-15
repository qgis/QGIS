/***************************************************************************
    qgsbrowserdockwidget.cpp
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsbrowserdockwidget.h"

#include <QHeaderView>
#include <QTreeView>
#include <QMenu>
#include <QSettings>
#include <QToolButton>

#include "qgsbrowsermodel.h"
#include "qgsdataitem.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"

// browser layer properties dialog
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include <ui_qgsbrowserlayerpropertiesbase.h>

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
    QgsBrowserTreeView( QWidget* parent ) : QTreeView( parent )
    {
      setDragDropMode( QTreeView::DragDrop ); // sets also acceptDrops + dragEnabled
      setSelectionMode( QAbstractItemView::ExtendedSelection );
      setContextMenuPolicy( Qt::CustomContextMenu );
      setHeaderHidden( true );
      setDropIndicatorShown( true );
    }

    void dragEnterEvent( QDragEnterEvent* e )
    {
      // accept drag enter so that our widget will not get ignored
      // and drag events will not get passed to QgisApp
      e->accept();
    }
    void dragMoveEvent( QDragMoveEvent* e )
    {
      // do not accept drops above/below items
      /*if ( dropIndicatorPosition() != QAbstractItemView::OnItem )
      {
        QgsDebugMsg("drag not on item");
        e->ignore();
        return;
      }*/

      QTreeView::dragMoveEvent( e );

      if ( !e->provides( "application/x-vnd.qgis.qgis.uri" ) )
      {
        e->ignore();
        return;
      }
    }
};

QgsBrowserDockWidget::QgsBrowserDockWidget( QWidget * parent ) :
    QDockWidget( parent ), mModel( NULL )
{
  setWindowTitle( tr( "Browser" ) );

  mBrowserView = new QgsBrowserTreeView( this );

  QToolButton* refreshButton = new QToolButton( this );
  refreshButton->setIcon( QgisApp::instance()->getThemeIcon( "mActionDraw.png" ) );
  // remove this to save space
  refreshButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  refreshButton->setText( tr( "Refresh" ) );
  refreshButton->setToolTip( tr( "Refresh" ) );
  refreshButton->setAutoRaise( true );
  connect( refreshButton, SIGNAL( clicked() ), this, SLOT( refresh() ) );

  QToolButton* addLayersButton = new QToolButton( this );
  addLayersButton->setIcon( QgisApp::instance()->getThemeIcon( "mActionAddLayer.png" ) );
  // remove this to save space
  addLayersButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  addLayersButton->setText( tr( "Add Selection" ) );
  addLayersButton->setToolTip( tr( "Add Selected Layers" ) );
  addLayersButton->setAutoRaise( true );
  connect( addLayersButton, SIGNAL( clicked() ), this, SLOT( addSelectedLayers() ) );

  QToolButton* collapseButton = new QToolButton( this );
  collapseButton->setIcon( QgisApp::instance()->getThemeIcon( "mActionCollapseTree.png" ) );
  collapseButton->setToolTip( tr( "Collapse All" ) );
  collapseButton->setAutoRaise( true );
  connect( collapseButton, SIGNAL( clicked() ), mBrowserView, SLOT( collapseAll() ) );

  QVBoxLayout* layout = new QVBoxLayout();
  QHBoxLayout* hlayout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  hlayout->setContentsMargins( 0, 0, 0, 0 );
  hlayout->setSpacing( 5 );
  hlayout->setAlignment( Qt::AlignLeft );

  hlayout->addSpacing( 5 );
  hlayout->addWidget( refreshButton );
  hlayout->addSpacing( 5 );
  hlayout->addWidget( addLayersButton );
  hlayout->addStretch( );
  hlayout->addWidget( collapseButton );
  layout->addLayout( hlayout );
  layout->addWidget( mBrowserView );

  QWidget* innerWidget = new QWidget( this );
  innerWidget->setLayout( layout );
  setWidget( innerWidget );

  connect( mBrowserView, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showContextMenu( const QPoint & ) ) );
  connect( mBrowserView, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( addLayerAtIndex( const QModelIndex& ) ) );

}

void QgsBrowserDockWidget::showEvent( QShowEvent * e )
{
  // delayed initialization of the model
  if ( mModel == NULL )
  {
    mModel = new QgsBrowserModel( mBrowserView );
    mBrowserView->setModel( mModel );

    // provide a horizontal scroll bar instead of using ellipse (...) for longer items
    mBrowserView->setTextElideMode( Qt::ElideNone );
    mBrowserView->header()->setResizeMode( 0, QHeaderView::ResizeToContents );
    mBrowserView->header()->setStretchLastSection( false );

    // find root favourites item
    for ( int i = 0; i < mModel->rowCount(); i++ )
    {
      QModelIndex index = mModel->index( i, 0 );
      QgsDataItem* item = mModel->dataItem( index );
      if ( item && item->type() == QgsDataItem::Favourites )
        mBrowserView->expand( index );
    }
  }

  QDockWidget::showEvent( e );
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

  else if ( item->type() == QgsDataItem::Layer )
  {
    menu->addAction( tr( "Add Layer" ), this, SLOT( addCurrentLayer( ) ) );
    menu->addAction( tr( "Add Selected Layers" ), this, SLOT( addSelectedLayers() ) );
    menu->addAction( tr( "Properties" ), this, SLOT( showProperties( ) ) );
  }

  QList<QAction*> actions = item->actions();
  if ( !actions.isEmpty() )
  {
    if ( !menu->actions().isEmpty() )
      menu->addSeparator();
    // add action to the menu
    menu->addActions( actions );
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

void QgsBrowserDockWidget::refresh()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  refreshModel( QModelIndex() );
  QApplication::restoreOverrideCursor();
}

void QgsBrowserDockWidget::refreshModel( const QModelIndex& index )
{
  QgsDebugMsg( "Entered" );
  if ( index.isValid() )
  {
    QgsDataItem *item = mModel->dataItem( index );
    if ( item )
    {
      QgsDebugMsg( "path = " + item->path() );
    }
    else
    {
      QgsDebugMsg( "invalid item" );
    }
  }

  mModel->refresh( index );

  for ( int i = 0 ; i < mModel->rowCount( index ); i++ )
  {
    QModelIndex idx = mModel->index( i, 0, index );
    if ( mBrowserView->isExpanded( idx ) || !mModel->hasChildren( idx ) )
    {
      refreshModel( idx );
    }
  }
}

void QgsBrowserDockWidget::addLayer( QgsLayerItem *layerItem )
{
  if ( layerItem == NULL )
    return;

  QString uri = layerItem->uri();
  if ( uri.isEmpty() )
    return;

  QgsMapLayer::LayerType type = layerItem->mapLayerType();
  QString providerKey = layerItem->providerKey();

  QgsDebugMsg( providerKey + " : " + uri );
  if ( type == QgsMapLayer::VectorLayer )
  {
    QgisApp::instance()->addVectorLayer( uri, layerItem->layerName(), providerKey );
  }
  if ( type == QgsMapLayer::RasterLayer )
  {
    QgisApp::instance()->addRasterLayer( uri, layerItem->layerName(), providerKey );
  }
}

void QgsBrowserDockWidget::addLayerAtIndex( const QModelIndex& index )
{
  QgsDataItem *dataItem = mModel->dataItem( index );

  if ( dataItem != NULL && dataItem->type() == QgsDataItem::Layer )
  {
    QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( dataItem );
    if ( layerItem != NULL )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      addLayer( layerItem );
      QApplication::restoreOverrideCursor();
    }
  }
}

void QgsBrowserDockWidget::addCurrentLayer( )
{
  addLayerAtIndex( mBrowserView->currentIndex() );
}

void QgsBrowserDockWidget::addSelectedLayers()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // get a sorted list of selected indexes
  QModelIndexList list = mBrowserView->selectionModel()->selectedIndexes();
  qSort( list );

  // add items in reverse order so they are in correct order in the layers dock
  for ( int i = list.size() - 1; i >= 0; i-- )
  {
    QModelIndex index = list[i];
    QgsDataItem *dataItem = mModel->dataItem( index );
    if ( dataItem && dataItem->type() == QgsDataItem::Layer )
    {
      QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( dataItem );
      if ( layerItem )
        addLayer( layerItem );
    }
  }

  QApplication::restoreOverrideCursor();
}

void QgsBrowserDockWidget::showProperties( )
{
  QgsDebugMsg( "Entered" );
  QgsDataItem* dataItem = mModel->dataItem( mBrowserView->currentIndex() );

  if ( dataItem != NULL && dataItem->type() == QgsDataItem::Layer )
  {
    QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( dataItem );
    if ( layerItem != NULL )
    {
      QgsMapLayer::LayerType type = layerItem->mapLayerType();
      QString layerMetadata = tr( "Error" );
      QgsCoordinateReferenceSystem layerCrs;
      QString notice;

      // temporarily override /Projections/defaultBehaviour to avoid dialog prompt
      QSettings settings;
      QString defaultProjectionOption = settings.value( "/Projections/defaultBehaviour", "prompt" ).toString();
      if ( settings.value( "/Projections/defaultBehaviour", "prompt" ).toString() == "prompt" )
      {
        settings.setValue( "/Projections/defaultBehaviour", "useProject" );
      }

      // find root item
      // we need to create a temporary layer to get metadata
      // we could use a provider but the metadata is not as complete and "pretty"  and this is easier
      QgsDebugMsg( QString( "creating temporary layer using path %1" ).arg( layerItem->path() ) );
      if ( type == QgsMapLayer::RasterLayer )
      {
        QgsDebugMsg( "creating raster layer" );
        // should copy code from addLayer() to split uri ?
        QgsRasterLayer* layer = new QgsRasterLayer( layerItem->uri(), layerItem->uri(), layerItem->providerKey() );
        if ( layer != NULL )
        {
          layerCrs = layer->crs();
          layerMetadata = layer->metadata();
          delete layer;
        }
      }
      else if ( type == QgsMapLayer::VectorLayer )
      {
        QgsDebugMsg( "creating vector layer" );
        QgsVectorLayer* layer = new QgsVectorLayer( layerItem->uri(), layerItem->name(), layerItem->providerKey() );
        if ( layer != NULL )
        {
          layerCrs = layer->crs();
          layerMetadata = layer->metadata();
          delete layer;
        }
      }

      // restore /Projections/defaultBehaviour
      if ( defaultProjectionOption == "prompt" )
      {
        settings.setValue( "/Projections/defaultBehaviour", defaultProjectionOption );
      }

      // initialize dialog
      QDialog *dialog = new QDialog( this );
      Ui::QgsBrowserLayerPropertiesBase ui;
      ui.setupUi( dialog );

      dialog->setWindowTitle( tr( "Layer Properties" ) );
      ui.leName->setText( layerItem->name() );
      ui.leSource->setText( layerItem->path() );
      ui.leProvider->setText( layerItem->providerKey() );
      QString myStyle = QgsApplication::reportStyleSheet();
      ui.txtbMetadata->document()->setDefaultStyleSheet( myStyle );
      ui.txtbMetadata->setHtml( layerMetadata );

      // report if layer was set to to project crs without prompt (may give a false positive)
      if ( defaultProjectionOption == "prompt" )
      {
        QgsCoordinateReferenceSystem defaultCrs =
          QgisApp::instance()->mapCanvas()->mapRenderer()->destinationCrs();
        if ( layerCrs == defaultCrs )
          ui.lblNotice->setText( "NOTICE: Layer srs set from project (" + defaultCrs.authid() + ")" );
      }

      dialog->show();
    }
  }
}
