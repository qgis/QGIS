/***************************************************************************
    qgsbrowserdockwidget.cpp
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
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
#include <QFileDialog>
#include <QSortFilterProxyModel>

#include "qgsbrowsermodel.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"
#include "qgsproject.h"

// browser layer properties dialog
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include <ui_qgsbrowserlayerpropertiesbase.h>
#include <ui_qgsbrowserdirectorypropertiesbase.h>


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

/**
Utility class for filtering browser items
 */
class QgsBrowserTreeFilterProxyModel : public QSortFilterProxyModel
{
  public:

    QgsBrowserTreeFilterProxyModel( QObject *parent )
        : QSortFilterProxyModel( parent ), mModel( 0 )
        , mFilter( "" ), mPatternSyntax( QRegExp::Wildcard )
    {
      setDynamicSortFilter( true );
    }

    void setBrowserModel( QgsBrowserModel* model )
    {
      mModel = model;
      setSourceModel( model );
    }

    void setFilterSyntax( const QRegExp::PatternSyntax & syntax )
    {
      QgsDebugMsg( QString( "syntax = %1" ).arg(( int ) mPatternSyntax ) );
      if ( mPatternSyntax == syntax )
        return;
      mPatternSyntax = syntax;
      updateFilter();
    }

    void setFilter( const QString & filter )
    {
      QgsDebugMsg( QString( "filter = %1" ).arg( mFilter ) );
      if ( mFilter == filter )
        return;
      mFilter = filter;
      updateFilter();
    }

    void updateFilter( )
    {
      QgsDebugMsg( QString( "filter = %1 syntax = %2" ).arg( mFilter ).arg(( int ) mPatternSyntax ) );
      mREList.clear();
      if ( mPatternSyntax == QRegExp::Wildcard ||
           mPatternSyntax == QRegExp::WildcardUnix )
      {
        foreach ( QString f, mFilter.split( "|" ) )
        {
          QRegExp rx( f.trimmed() );
          rx.setPatternSyntax( mPatternSyntax );
          mREList.append( rx );
        }
      }
      else
      {
        QRegExp rx( mFilter.trimmed() );
        rx.setPatternSyntax( mPatternSyntax );
        mREList.append( rx );
      }
      invalidateFilter();
    }

  protected:

    QgsBrowserModel* mModel;
    QString mFilter; //filter string provided
    QVector<QRegExp> mREList; //list of filters, separated by "|"
    QRegExp::PatternSyntax mPatternSyntax;

    bool filterAcceptsString( const QString & value ) const
    {
      if ( mPatternSyntax == QRegExp::Wildcard ||
           mPatternSyntax == QRegExp::WildcardUnix )
      {
        foreach ( QRegExp rx, mREList )
        {
          QgsDebugMsg( QString( "value: [%1] rx: [%2] match: %3" ).arg( value ).arg( rx.pattern() ).arg( rx.exactMatch( value ) ) );
          if ( rx.exactMatch( value ) )
            return true;
        }
      }
      else
      {
        foreach ( QRegExp rx, mREList )
        {
          QgsDebugMsg( QString( "value: [%1] rx: [%2] match: %3" ).arg( value ).arg( rx.pattern() ).arg( rx.indexIn( value ) ) );
          if ( rx.indexIn( value ) != -1 )
            return true;
        }
      }
      return false;
    }

    bool filterAcceptsRow( int sourceRow,
                           const QModelIndex &sourceParent ) const
    {
      // if ( filterRegExp().pattern() == QString( "" ) ) return true;
      if ( mFilter == "" ) return true;

      QModelIndex index = sourceModel()->index( sourceRow, 0, sourceParent );
      QgsDataItem* item = mModel->dataItem( index );
      QgsDataItem* parentItem = mModel->dataItem( sourceParent );

      // accept "invalid" items and data collections
      if ( ! item )
        return true;
      if ( qobject_cast<QgsDataCollectionItem*>( item ) )
        return true;

      // filter layer items - this could be delegated to the providers but a little overkill
      if ( parentItem && qobject_cast<QgsLayerItem*>( item ) )
      {
        // filter normal files by extension
        if ( qobject_cast<QgsDirectoryItem*>( parentItem ) )
        {
          QFileInfo fileInfo( item->path() );
          return filterAcceptsString( fileInfo.fileName() );
        }
        // filter other items (postgis, etc.) by name
        else if ( qobject_cast<QgsDataCollectionItem*>( parentItem ) )
        {
          return filterAcceptsString( item->name() );
        }
      }

      // accept anything else
      return true;
    }

};
QgsBrowserDockWidget::QgsBrowserDockWidget( QString name, QWidget * parent ) :
    QDockWidget( parent ), mModel( NULL ), mProxyModel( NULL )
{
  setupUi( this );

  setWindowTitle( name );

  mBrowserView = new QgsBrowserTreeView( this );
  mLayoutBrowser->addWidget( mBrowserView );

  mBtnRefresh->setIcon( QgsApplication::getThemeIcon( "mActionRefresh.png" ) );
  mBtnAddLayers->setIcon( QgsApplication::getThemeIcon( "mActionAdd.png" ) );
  mBtnCollapse->setIcon( QgsApplication::getThemeIcon( "mActionCollapseTree.png" ) );

  mWidgetFilter->hide();
  // icons from http://www.fatcow.com/free-icons License: CC Attribution 3.0
  mBtnFilterShow->setIcon( QgsApplication::getThemeIcon( "mActionFilter.png" ) );
  mBtnFilter->setIcon( QgsApplication::getThemeIcon( "mActionFilter.png" ) );

  QMenu* menu = new QMenu( this );
  menu->setSeparatorsCollapsible( false );
  mBtnFilterOptions->setMenu( menu );
  QActionGroup* group = new QActionGroup( menu );
  QAction* action = new QAction( tr( "Filter Pattern Syntax" ), group );
  action->setSeparator( true );
  menu->addAction( action );
  action = new QAction( tr( "Wildcard(s)" ), group );
  action->setData( QVariant(( int ) QRegExp::Wildcard ) );
  action->setCheckable( true );
  action->setChecked( true );
  menu->addAction( action );
  action = new QAction( tr( "Regular Expression" ), group );
  action->setData( QVariant(( int ) QRegExp::RegExp ) );
  action->setCheckable( true );
  menu->addAction( action );

  connect( mBtnRefresh, SIGNAL( clicked() ), this, SLOT( refresh() ) );
  connect( mBtnAddLayers, SIGNAL( clicked() ), this, SLOT( addSelectedLayers() ) );
  connect( mBtnCollapse, SIGNAL( clicked() ), mBrowserView, SLOT( collapseAll() ) );
  connect( mBtnFilterShow, SIGNAL( toggled( bool ) ), this, SLOT( showFilterWidget( bool ) ) );
  connect( mBtnFilter, SIGNAL( clicked() ), this, SLOT( setFilter() ) );
  connect( mLeFilter, SIGNAL( returnPressed() ), this, SLOT( setFilter() ) );
  connect( mLeFilter, SIGNAL( cleared() ), this, SLOT( setFilter() ) );
  // connect( mLeFilter, SIGNAL( textChanged( const QString & ) ), this, SLOT( setFilter() ) );
  connect( group, SIGNAL( triggered( QAction * ) ), this, SLOT( setFilterSyntax( QAction * ) ) );

  connect( mBrowserView, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showContextMenu( const QPoint & ) ) );
  connect( mBrowserView, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( addLayerAtIndex( const QModelIndex& ) ) );
}

void QgsBrowserDockWidget::showEvent( QShowEvent * e )
{
  // delayed initialization of the model
  if ( !mModel )
  {
    mModel = new QgsBrowserModel( mBrowserView );

    connect( QgisApp::instance(), SIGNAL( newProject() ), mModel, SLOT( updateProjectHome() ) );

    mProxyModel = new QgsBrowserTreeFilterProxyModel( this );
    mProxyModel->setBrowserModel( mModel );
    mBrowserView->setModel( mProxyModel );
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
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->indexAt( pt ) );
  QgsDataItem* item = mModel->dataItem( index );
  if ( !item )
    return;

  QMenu *menu = new QMenu( this );

  if ( item->type() == QgsDataItem::Directory )
  {
    QSettings settings;
    QStringList favDirs = settings.value( "/browser/favourites" ).toStringList();
    bool inFavDirs = favDirs.contains( item->path() );

    if ( item->parent() && !inFavDirs )
    {
      // only non-root directories can be added as favourites
      menu->addAction( tr( "Add as a favourite" ), this, SLOT( addFavourite() ) );
    }
    else if ( inFavDirs )
    {
      // only favourites can be removed
      menu->addAction( tr( "Remove favourite" ), this, SLOT( removeFavourite() ) );
    }
    menu->addAction( tr( "Properties" ), this, SLOT( showProperties( ) ) );
    QAction *action = menu->addAction( tr( "Fast scan this dir." ), this, SLOT( toggleFastScan( ) ) );
    action->setCheckable( true );
    action->setChecked( settings.value( "/qgis/scanItemsFastScanUris",
                                        QStringList() ).toStringList().contains( item->path() ) );
  }
  else if ( item->type() == QgsDataItem::Layer )
  {
    menu->addAction( tr( "Add Layer" ), this, SLOT( addCurrentLayer( ) ) );
    menu->addAction( tr( "Add Selected Layers" ), this, SLOT( addSelectedLayers() ) );
    menu->addAction( tr( "Properties" ), this, SLOT( showProperties( ) ) );
  }
  else if ( item->type() == QgsDataItem::Favourites )
  {
    menu->addAction( tr( "Add a directory" ), this, SLOT( addFavouriteDirectory() ) );

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
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem* item = mModel->dataItem( index );
  if ( !item )
    return;

  if ( item->type() != QgsDataItem::Directory )
    return;

  addFavouriteDirectory( item->path() );
}

void QgsBrowserDockWidget::addFavouriteDirectory()
{
  QString directory = QFileDialog::getExistingDirectory( this, tr( "Add directory to favourites" ) );
  if ( !directory.isEmpty() )
  {
    addFavouriteDirectory( directory );
  }
}

void QgsBrowserDockWidget::addFavouriteDirectory( QString favDir )
{
  mModel->addFavouriteDirectory( favDir );
}

void QgsBrowserDockWidget::removeFavourite()
{
  mModel->removeFavourite( mProxyModel->mapToSource( mBrowserView->currentIndex() ) );
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
    QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
    if ( mBrowserView->isExpanded( proxyIdx ) || !mModel->hasChildren( proxyIdx ) )
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
  QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( index ) );

  if ( item != NULL && item->type() == QgsDataItem::Layer )
  {
    QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( item );
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
    QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( list[i] ) );
    if ( item && item->type() == QgsDataItem::Layer )
    {
      QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( item );
      if ( layerItem )
        addLayer( layerItem );
    }
  }

  QApplication::restoreOverrideCursor();
}

void QgsBrowserDockWidget::showProperties( )
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem* item = mModel->dataItem( index );
  if ( ! item )
    return;

  if ( item->type() == QgsDataItem::Layer )
  {
    QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( item );
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
  else if ( item->type() == QgsDataItem::Directory )
  {
    // initialize dialog
    QDialog *dialog = new QDialog( this );
    Ui::QgsBrowserDirectoryPropertiesBase ui;
    ui.setupUi( dialog );

    dialog->setWindowTitle( tr( "Directory Properties" ) );
    ui.leSource->setText( item->path() );
    QgsDirectoryParamWidget *paramWidget = new QgsDirectoryParamWidget( item->path(), dialog );
    ui.lytWidget->addWidget( paramWidget );

    dialog->show();
  }
}

void QgsBrowserDockWidget::toggleFastScan( )
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem* item = mModel->dataItem( index );
  if ( ! item )
    return;

  if ( item->type() == QgsDataItem::Directory )
  {
    QSettings settings;
    QStringList fastScanDirs = settings.value( "/qgis/scanItemsFastScanUris",
                               QStringList() ).toStringList();
    int idx = fastScanDirs.indexOf( item->path() );
    if ( idx != -1 )
    {
      fastScanDirs.removeAt( idx );
    }
    else
    {
      fastScanDirs << item->path();
    }
    settings.setValue( "/qgis/scanItemsFastScanUris", fastScanDirs );
  }
}



void QgsBrowserDockWidget::showFilterWidget( bool visible )
{
  mWidgetFilter->setVisible( visible );
  if ( ! visible )
  {
    mLeFilter->setText( "" );
    setFilter();
  }
}

void QgsBrowserDockWidget::setFilter( )
{
  QString filter = mLeFilter->text();
  if ( mProxyModel )
    mProxyModel->setFilter( filter );
}

void QgsBrowserDockWidget::setFilterSyntax( QAction * action )
{
  if ( !action || ! mProxyModel )
    return;
  mProxyModel->setFilterSyntax(( QRegExp::PatternSyntax ) action->data().toInt() );
}
