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
#include "qgsbrowserdockwidget_p.h"

#include <QAbstractTextDocumentLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QPlainTextDocumentLayout>
#include <QSortFilterProxyModel>

#include "qgsbrowsermodel.h"
#include "qgsbrowsertreeview.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsnewnamedialog.h"

// browser layer properties dialog
#include "qgsapplication.h"
#include "qgsmapcanvas.h"

#include <QDragEnterEvent>

QgsBrowserDockWidget::QgsBrowserDockWidget( const QString &name, QgsBrowserModel *browserModel, QWidget *parent )
  : QgsDockWidget( parent )
  , mModel( browserModel )
  , mPropertiesWidgetEnabled( false )
  , mPropertiesWidgetHeight( 0 )
{
  setupUi( this );

  mContents->layout()->setContentsMargins( 0, 0, 0, 0 );
  mContents->layout()->setMargin( 0 );
  static_cast< QVBoxLayout * >( mContents->layout() )->setSpacing( 0 );

  setWindowTitle( name );

  mBrowserView = new QgsDockBrowserTreeView( this );
  mLayoutBrowser->addWidget( mBrowserView );

  mWidgetFilter->hide();
  mLeFilter->setPlaceholderText( tr( "Type here to filter visible items…" ) );
  // icons from http://www.fatcow.com/free-icons License: CC Attribution 3.0

  QMenu *menu = new QMenu( this );
  menu->setSeparatorsCollapsible( false );
  mBtnFilterOptions->setMenu( menu );
  QAction *action = new QAction( tr( "Case Sensitive" ), menu );
  action->setData( "case" );
  action->setCheckable( true );
  action->setChecked( false );
  connect( action, &QAction::toggled, this, &QgsBrowserDockWidget::setCaseSensitive );
  menu->addAction( action );
  QActionGroup *group = new QActionGroup( menu );
  action = new QAction( tr( "Filter Pattern Syntax" ), group );
  action->setSeparator( true );
  menu->addAction( action );
  action = new QAction( tr( "Normal" ), group );
  action->setData( "normal" );
  action->setCheckable( true );
  action->setChecked( true );
  menu->addAction( action );
  action = new QAction( tr( "Wildcard(s)" ), group );
  action->setData( "wildcard" );
  action->setCheckable( true );
  menu->addAction( action );
  action = new QAction( tr( "Regular Expression" ), group );
  action->setData( "regexp" );
  action->setCheckable( true );
  menu->addAction( action );

  connect( mActionRefresh, &QAction::triggered, this, &QgsBrowserDockWidget::refresh );
  connect( mActionAddLayers, &QAction::triggered, this, &QgsBrowserDockWidget::addSelectedLayers );
  connect( mActionCollapse, &QAction::triggered, mBrowserView, &QgsDockBrowserTreeView::collapseAll );
  connect( mActionShowFilter, &QAction::triggered, this, &QgsBrowserDockWidget::showFilterWidget );
  connect( mActionPropertiesWidget, &QAction::triggered, this, &QgsBrowserDockWidget::enablePropertiesWidget );
  connect( mLeFilter, &QgsFilterLineEdit::returnPressed, this, &QgsBrowserDockWidget::setFilter );
  connect( mLeFilter, &QgsFilterLineEdit::cleared, this, &QgsBrowserDockWidget::setFilter );
  connect( mLeFilter, &QgsFilterLineEdit::textChanged, this, &QgsBrowserDockWidget::setFilter );
  connect( group, &QActionGroup::triggered, this, &QgsBrowserDockWidget::setFilterSyntax );
  connect( mBrowserView, &QgsDockBrowserTreeView::customContextMenuRequested, this, &QgsBrowserDockWidget::showContextMenu );
  connect( mBrowserView, &QgsDockBrowserTreeView::doubleClicked, this, &QgsBrowserDockWidget::itemDoubleClicked );
  connect( mSplitter, &QSplitter::splitterMoved, this, &QgsBrowserDockWidget::splitterMoved );
}

QgsBrowserDockWidget::~QgsBrowserDockWidget()
{
  QgsSettings settings;
  settings.setValue( settingsSection() + "/propertiesWidgetEnabled", mPropertiesWidgetEnabled );
  //settings.setValue(settingsSection() + "/propertiesWidgetHeight", mPropertiesWidget->size().height() );
  settings.setValue( settingsSection() + "/propertiesWidgetHeight", mPropertiesWidgetHeight );
}

void QgsBrowserDockWidget::showEvent( QShowEvent *e )
{
  // delayed initialization of the model
  if ( !mModel->initialized( ) )
  {
    mModel->initialize();
  }
  if ( ! mProxyModel )
  {
    mProxyModel = new QgsBrowserTreeFilterProxyModel( this );
    mProxyModel->setBrowserModel( mModel );
    mBrowserView->setSettingsSection( objectName().toLower() ); // to distinguish 2 or more instances of the browser
    mBrowserView->setBrowserModel( mModel );
    mBrowserView->setModel( mProxyModel );
    mBrowserView->setSortingEnabled( true );
    mBrowserView->sortByColumn( 0, Qt::AscendingOrder );
    // provide a horizontal scroll bar instead of using ellipse (...) for longer items
    mBrowserView->setTextElideMode( Qt::ElideNone );
    mBrowserView->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
    mBrowserView->header()->setStretchLastSection( false );

    // selectionModel is created when model is set on tree
    connect( mBrowserView->selectionModel(), &QItemSelectionModel::selectionChanged,
             this, &QgsBrowserDockWidget::selectionChanged );

    // Forward the model changed signals to the widget
    connect( mModel, &QgsBrowserModel::connectionsChanged,
             this, &QgsBrowserDockWidget::connectionsChanged );


    // objectName used by settingsSection() is not yet set in constructor
    QgsSettings settings;
    mPropertiesWidgetEnabled = settings.value( settingsSection() + "/propertiesWidgetEnabled", false ).toBool();
    mActionPropertiesWidget->setChecked( mPropertiesWidgetEnabled );
    mPropertiesWidget->setVisible( false ); // false until item is selected

    mPropertiesWidgetHeight = settings.value( settingsSection() + "/propertiesWidgetHeight" ).toFloat();
    QList<int> sizes = mSplitter->sizes();
    int total = sizes.value( 0 ) + sizes.value( 1 );
    int height = static_cast<int>( total ) * mPropertiesWidgetHeight;
    sizes.clear();
    sizes << total - height << height;
    mSplitter->setSizes( sizes );
  }

  QgsDockWidget::showEvent( e );
}

void QgsBrowserDockWidget::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_F2 )
  {
    renameFavorite();
    event->accept();
  }
}

void QgsBrowserDockWidget::itemDoubleClicked( const QModelIndex &index )
{
  QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( index ) );
  if ( !item )
    return;

  if ( item->handleDoubleClick() )
    return;
  else
    addLayerAtIndex( index ); // default double-click handler
}

void QgsBrowserDockWidget::renameFavorite()
{
  QgsDataItem *dataItem = mModel->dataItem( mProxyModel->mapToSource( mBrowserView->currentIndex() ) );
  if ( !dataItem )
    return;

  QgsFavoriteItem *favorite = qobject_cast< QgsFavoriteItem * >( dataItem );
  if ( !favorite )
    return;

  QgsNewNameDialog dlg( tr( "favorite “%1”" ).arg( favorite->name() ), favorite->name() );
  dlg.setWindowTitle( tr( "Rename Favorite" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == favorite->name() )
    return;

  favorite->rename( dlg.name() );
}

void QgsBrowserDockWidget::showContextMenu( QPoint pt )
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->indexAt( pt ) );
  QgsDataItem *item = mModel->dataItem( index );
  if ( !item )
    return;

  QMenu *menu = new QMenu( this );

  if ( item->type() == QgsDataItem::Directory )
  {
    QgsSettings settings;

    bool inFavDirs = item->parent() && item->parent()->type() == QgsDataItem::Favorites;
    if ( item->parent() && !inFavDirs )
    {
      // only non-root directories can be added as favorites
      QAction *addAsFavorite = new QAction( tr( "Add as a Favorite" ), this );
      addAsFavorite->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFavourites.svg" ) ) );
      menu->addAction( addAsFavorite );
      connect( addAsFavorite, &QAction::triggered, this, &QgsBrowserDockWidget::addFavorite );
    }
    else if ( inFavDirs )
    {
      QAction *actionRename = new QAction( tr( "Rename Favorite…" ), this );
      connect( actionRename, &QAction::triggered, this, &QgsBrowserDockWidget::renameFavorite );
      menu->addAction( actionRename );
      menu->addSeparator();
      menu->addAction( tr( "Remove Favorite" ), this, SLOT( removeFavorite() ) );
      menu->addSeparator();
    }
    menu->addAction( tr( "Properties…" ), this, SLOT( showProperties() ) );
    menu->addAction( tr( "Hide from Browser" ), this, SLOT( hideItem() ) );
    QAction *action = menu->addAction( tr( "Fast Scan this Directory" ), this, SLOT( toggleFastScan() ) );
    action->setCheckable( true );
    action->setChecked( settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                                        QStringList() ).toStringList().contains( item->path() ) );
  }
  else if ( item->type() == QgsDataItem::Layer )
  {
    menu->addAction( tr( "Add Selected Layer(s) to Canvas" ), this, SLOT( addSelectedLayers() ) );
    menu->addAction( tr( "Properties…" ), this, SLOT( showProperties() ) );
  }
  else if ( item->type() == QgsDataItem::Favorites )
  {
    menu->addAction( tr( "Add a Directory…" ), this, SLOT( addFavoriteDirectory() ) );
  }

  const QList<QMenu *> menus = item->menus( menu );
  QList<QAction *> actions = item->actions( menu );

  if ( !menus.isEmpty() )
  {
    for ( QMenu *mn : menus )
    {
      menu->addMenu( mn );
    }
  }

  if ( !actions.isEmpty() )
  {
    if ( !menu->actions().isEmpty() )
      menu->addSeparator();
    // add action to the menu
    menu->addActions( actions );
  }

  if ( menu->actions().isEmpty() )
  {
    delete menu;
    return;
  }

  menu->popup( mBrowserView->mapToGlobal( pt ) );
}

void QgsBrowserDockWidget::addFavorite()
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem *item = mModel->dataItem( index );
  if ( !item )
    return;

  QgsDirectoryItem *dirItem = dynamic_cast<QgsDirectoryItem *>( item );
  if ( !dirItem )
    return;

  addFavoriteDirectory( dirItem->dirPath() );
}

void QgsBrowserDockWidget::addFavoriteDirectory()
{
  QString directory = QFileDialog::getExistingDirectory( this, tr( "Add directory to favorites" ) );
  if ( !directory.isEmpty() )
  {
    addFavoriteDirectory( directory );
  }
}

void QgsBrowserDockWidget::addFavoriteDirectory( const QString &favDir, const QString &name )
{
  mModel->addFavoriteDirectory( favDir, name );
}

void QgsBrowserDockWidget::removeFavorite()
{
  mModel->removeFavorite( mProxyModel->mapToSource( mBrowserView->currentIndex() ) );
}

void QgsBrowserDockWidget::refresh()
{
  refreshModel( QModelIndex() );
}

void QgsBrowserDockWidget::refreshModel( const QModelIndex &index )
{
  if ( mModel && mProxyModel )
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

    if ( item && ( item->capabilities2() & QgsDataItem::Fertile ) )
    {
      mModel->refresh( index );
    }

    for ( int i = 0; i < mModel->rowCount( index ); i++ )
    {
      QModelIndex idx = mModel->index( i, 0, index );
      QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
      QgsDataItem *child = mModel->dataItem( idx );

      // Check also expanded descendants so that the whole expanded path does not get collapsed if one item is collapsed.
      // Fast items (usually root items) are refreshed so that when collapsed, it is obvious they are if empty (no expand symbol).
      if ( mBrowserView->isExpanded( proxyIdx ) || mBrowserView->hasExpandedDescendant( proxyIdx ) || ( child && child->capabilities2() & QgsDataItem::Fast ) )
      {
        refreshModel( idx );
      }
      else
      {
        if ( child && ( child->capabilities2() & QgsDataItem::Fertile ) )
        {
          child->depopulate();
        }
      }
    }
  }
}

void QgsBrowserDockWidget::addLayer( QgsLayerItem *layerItem )
{
  if ( !layerItem )
    return;

  QgsMimeDataUtils::UriList list;
  list << layerItem->mimeUri();
  emit handleDropUriList( list );
}

void QgsBrowserDockWidget::addLayerAtIndex( const QModelIndex &index )
{
  QgsDebugMsg( QString( "rowCount() = %1" ).arg( mModel->rowCount( mProxyModel->mapToSource( index ) ) ) );
  QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( index ) );

  if ( item && item->type() == QgsDataItem::Project )
  {
    QgsProjectItem *projectItem = qobject_cast<QgsProjectItem *>( item );
    if ( projectItem )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      emit openFile( projectItem->path(), QStringLiteral( "project" ) );
      QApplication::restoreOverrideCursor();
    }
  }
  if ( item && item->type() == QgsDataItem::Layer )
  {
    QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
    if ( layerItem )
    {
      QApplication::setOverrideCursor( Qt::WaitCursor );
      addLayer( layerItem );
      QApplication::restoreOverrideCursor();
    }
  }
}

void QgsBrowserDockWidget::addSelectedLayers()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // get a sorted list of selected indexes
  QModelIndexList list = mBrowserView->selectionModel()->selectedIndexes();
  std::sort( list.begin(), list.end() );

  // If any of the layer items are QGIS we just open and exit the loop
  Q_FOREACH ( const QModelIndex &index, list )
  {
    QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( index ) );
    if ( item && item->type() == QgsDataItem::Project )
    {
      QgsProjectItem *projectItem = qobject_cast<QgsProjectItem *>( item );
      if ( projectItem )
        emit openFile( projectItem->path(), QStringLiteral( "project" ) );

      QApplication::restoreOverrideCursor();
      return;
    }
  }

  // add items in reverse order so they are in correct order in the layers dock
  for ( int i = list.size() - 1; i >= 0; i-- )
  {
    QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( list[i] ) );
    if ( item && item->type() == QgsDataItem::Layer )
    {
      QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
      if ( layerItem )
        addLayer( layerItem );
    }
  }

  QApplication::restoreOverrideCursor();
}

void QgsBrowserDockWidget::hideItem()
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem *item = mModel->dataItem( index );
  if ( ! item )
    return;

  if ( item->type() == QgsDataItem::Directory )
  {
    mModel->hidePath( item );
  }
}

void QgsBrowserDockWidget::showProperties()
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem *item = mModel->dataItem( index );
  if ( ! item )
    return;

  if ( item->type() == QgsDataItem::Layer || item->type() == QgsDataItem::Directory )
  {
    QgsBrowserPropertiesDialog *dialog = new QgsBrowserPropertiesDialog( settingsSection(), this );
    dialog->setItem( item );
    dialog->show();
  }
}

void QgsBrowserDockWidget::toggleFastScan()
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem *item = mModel->dataItem( index );
  if ( ! item )
    return;

  if ( item->type() == QgsDataItem::Directory )
  {
    QgsSettings settings;
    QStringList fastScanDirs = settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
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
    settings.setValue( QStringLiteral( "qgis/scanItemsFastScanUris" ), fastScanDirs );
  }
}

void QgsBrowserDockWidget::showFilterWidget( bool visible )
{
  mWidgetFilter->setVisible( visible );
  if ( ! visible )
  {
    mLeFilter->setText( QString() );
    setFilter();
  }
  else
  {
    mLeFilter->setFocus();
  }
}

void QgsBrowserDockWidget::setFilter()
{
  QString filter = mLeFilter->text();
  if ( mProxyModel )
    mProxyModel->setFilter( filter );
}

void QgsBrowserDockWidget::updateProjectHome()
{
  if ( mModel )
    mModel->updateProjectHome();
}

void QgsBrowserDockWidget::setFilterSyntax( QAction *action )
{
  if ( !action || ! mProxyModel )
    return;
  mProxyModel->setFilterSyntax( action->data().toString() );
}

void QgsBrowserDockWidget::setCaseSensitive( bool caseSensitive )
{
  if ( ! mProxyModel )
    return;
  mProxyModel->setCaseSensitive( caseSensitive );
}

int QgsBrowserDockWidget::selectedItemsCount()
{
  QItemSelectionModel *selectionModel = mBrowserView->selectionModel();
  if ( selectionModel )
  {
    return selectionModel->selectedIndexes().size();
  }
  return 0;
}

void QgsBrowserDockWidget::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected );
  Q_UNUSED( deselected );
  if ( mPropertiesWidgetEnabled )
  {
    setPropertiesWidget();
  }
}

void QgsBrowserDockWidget::clearPropertiesWidget()
{
  while ( mPropertiesLayout->count() > 0 )
  {
    delete mPropertiesLayout->itemAt( 0 )->widget();
  }
  mPropertiesWidget->setVisible( false );
}

void QgsBrowserDockWidget::setPropertiesWidget()
{
  clearPropertiesWidget();
  QItemSelectionModel *selectionModel = mBrowserView->selectionModel();
  if ( selectionModel )
  {
    QModelIndexList indexes = selectionModel->selectedIndexes();
    if ( indexes.size() == 1 )
    {
      QModelIndex index = mProxyModel->mapToSource( indexes.value( 0 ) );
      QgsDataItem *item = mModel->dataItem( index );
      QgsBrowserPropertiesWidget *propertiesWidget = QgsBrowserPropertiesWidget::createWidget( item, mPropertiesWidget );
      if ( propertiesWidget )
      {
        propertiesWidget->setCondensedMode( true );
        mPropertiesLayout->addWidget( propertiesWidget );
      }
    }
  }
  mPropertiesWidget->setVisible( mPropertiesLayout->count() > 0 );
}

void QgsBrowserDockWidget::enablePropertiesWidget( bool enable )
{
  mPropertiesWidgetEnabled = enable;
  if ( enable && selectedItemsCount() == 1 )
  {
    setPropertiesWidget();
  }
  else
  {
    clearPropertiesWidget();
  }
}

void QgsBrowserDockWidget::splitterMoved()
{
  QList<int> sizes = mSplitter->sizes();
  float total = sizes.value( 0 ) + sizes.value( 1 );
  mPropertiesWidgetHeight = total > 0 ? sizes.value( 1 ) / total : 0;
}
