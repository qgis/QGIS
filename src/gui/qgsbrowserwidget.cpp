/***************************************************************************
    qgsbrowserwidget.cpp
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
#include "qgsbrowserwidget.h"

#include <QAbstractTextDocumentLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QPlainTextDocumentLayout>
#include <QSortFilterProxyModel>
#include <QActionGroup>

#include "qgsbrowserguimodel.h"
#include "qgsbrowsertreeview.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsnewnamedialog.h"
#include "qgsbrowserproxymodel.h"
#include "qgsgui.h"
#include "qgswindowmanagerinterface.h"
#include "qgsnative.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgsdataitemguiprovider.h"
#include "qgsdirectoryitem.h"
#include "qgslayeritem.h"
#include "qgsprojectitem.h"
#include "qgsbrowserdockwidget_p.h"

// browser layer properties dialog
#include "qgsapplication.h"
#include "qgsmapcanvas.h"

#include <QDragEnterEvent>
#include <functional>

QgsBrowserWidget::QgsBrowserWidget( QgsBrowserGuiModel *browserModel, QWidget *parent )
  : QgsPanelWidget( parent )
  , mModel( browserModel )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );
  qgis::down_cast< QVBoxLayout * >( layout() )->setSpacing( 0 );

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
  connect( action, &QAction::toggled, this, &QgsBrowserWidget::setCaseSensitive );
  menu->addAction( action );
  QActionGroup *group = new QActionGroup( menu );
  action = new QAction( tr( "Filter Pattern Syntax" ), group );
  action->setSeparator( true );
  menu->addAction( action );
  action = new QAction( tr( "Normal" ), group );
  action->setData( QgsBrowserProxyModel::Normal );
  action->setCheckable( true );
  action->setChecked( true );
  menu->addAction( action );
  action = new QAction( tr( "Wildcard(s)" ), group );
  action->setData( QgsBrowserProxyModel::Wildcards );
  action->setCheckable( true );
  menu->addAction( action );
  action = new QAction( tr( "Regular Expression" ), group );
  action->setData( QgsBrowserProxyModel::RegularExpression );
  action->setCheckable( true );
  menu->addAction( action );

  mBrowserView->setExpandsOnDoubleClick( false );

  connect( mActionRefresh, &QAction::triggered, this, &QgsBrowserWidget::refresh );
  connect( mActionAddLayers, &QAction::triggered, this, &QgsBrowserWidget::addSelectedLayers );
  connect( mActionCollapse, &QAction::triggered, mBrowserView, &QgsDockBrowserTreeView::collapseAll );
  connect( mActionShowFilter, &QAction::triggered, this, &QgsBrowserWidget::showFilterWidget );
  connect( mActionPropertiesWidget, &QAction::triggered, this, &QgsBrowserWidget::propertiesWidgetToggled );
  connect( mLeFilter, &QgsFilterLineEdit::returnPressed, this, &QgsBrowserWidget::setFilter );
  connect( mLeFilter, &QgsFilterLineEdit::cleared, this, &QgsBrowserWidget::setFilter );
  connect( mLeFilter, &QgsFilterLineEdit::textChanged, this, &QgsBrowserWidget::setFilter );
  connect( group, &QActionGroup::triggered, this, &QgsBrowserWidget::setFilterSyntax );
  connect( mBrowserView, &QgsDockBrowserTreeView::customContextMenuRequested, this, &QgsBrowserWidget::showContextMenu );
  connect( mBrowserView, &QgsDockBrowserTreeView::doubleClicked, this, &QgsBrowserWidget::itemDoubleClicked );
  connect( mSplitter, &QSplitter::splitterMoved, this, &QgsBrowserWidget::splitterMoved );

  connect( QgsGui::instance(), &QgsGui::optionsChanged, this, &QgsBrowserWidget::onOptionsChanged );
}

QgsBrowserWidget::~QgsBrowserWidget() = default;

void QgsBrowserWidget::showEvent( QShowEvent *e )
{
  // delayed initialization of the model
  if ( !mModel->initialized( ) )
  {
    mModel->initialize();
  }
  if ( ! mProxyModel )
  {
    mProxyModel = new QgsBrowserProxyModel( this );
    mProxyModel->setBrowserModel( mModel );
    mProxyModel->setHiddenDataItemProviderKeyFilter( mDisabledDataItemsKeys );
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
             this, &QgsBrowserWidget::selectionChanged );

    // Forward the model changed signals to the widget
    connect( mModel, &QgsBrowserModel::connectionsChanged,
             this, &QgsBrowserWidget::connectionsChanged );

    // objectName used by settingsSection() is not yet set in constructor
    QgsSettings settings;
    mActionPropertiesWidget->setChecked( settings.value( settingsSection() + "/propertiesWidgetEnabled", false ).toBool() );
    mPropertiesWidget->setVisible( false ); // false until item is selected

    mSplitter->restoreState( settings.value( QStringLiteral( "%1/splitterState" ).arg( settingsSection() ) ).toByteArray() );
  }

  QWidget::showEvent( e );
}

void QgsBrowserWidget::itemDoubleClicked( const QModelIndex &index )
{
  QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( index ) );
  if ( !item )
    return;

  QgsDataItemGuiContext context = createContext();

  const QList< QgsDataItemGuiProvider * > providers = QgsGui::dataItemGuiProviderRegistry()->providers();
  for ( QgsDataItemGuiProvider *provider : providers )
  {
    if ( provider->handleDoubleClick( item, context ) )
      return;
  }

  // if no providers overrode the double-click handling for this item, we give the item itself a chance
  if ( !item->handleDoubleClick() )
  {
    // double-click not handled by browser model, so use as default view expand behavior
    if ( mBrowserView->isExpanded( index ) )
      mBrowserView->collapse( index );
    else
      mBrowserView->expand( index );
  }
}

void QgsBrowserWidget::onOptionsChanged()
{
  std::function< void( const QModelIndex &index ) > updateItem;
  updateItem = [this, &updateItem]( const QModelIndex & index )
  {
    if ( QgsDirectoryItem *dirItem = qobject_cast< QgsDirectoryItem * >( mModel->dataItem( index ) ) )
    {
      dirItem->reevaluateMonitoring();
    }

    const int rowCount = mModel->rowCount( index );
    for ( int i = 0; i < rowCount; ++i )
    {
      const QModelIndex child = mModel->index( i, 0, index );
      updateItem( child );
    }
  };

  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    updateItem( mModel->index( i, 0 ) );
  }
}

void QgsBrowserWidget::showContextMenu( QPoint pt )
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->indexAt( pt ) );
  QgsDataItem *item = mModel->dataItem( index );
  if ( !item )
    return;

  const QModelIndexList selection = mBrowserView->selectionModel()->selectedIndexes();
  QList< QgsDataItem * > selectedItems;
  selectedItems.reserve( selection.size() );
  for ( const QModelIndex &selectedIndex : selection )
  {
    QgsDataItem *selectedItem = mProxyModel->dataItem( selectedIndex );
    if ( selectedItem )
      selectedItems << selectedItem;
  }

  QMenu *menu = new QMenu( this );

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

  QgsDataItemGuiContext context = createContext();

  QList< QgsDataItemGuiProvider * > providers = QgsGui::dataItemGuiProviderRegistry()->providers();
  std::sort( providers.begin(), providers.end(), []( QgsDataItemGuiProvider * a, QgsDataItemGuiProvider * b )
  {
    return a->precedenceWhenPopulatingMenus() < b->precedenceWhenPopulatingMenus();
  } );
  for ( QgsDataItemGuiProvider *provider : std::as_const( providers ) )
  {
    provider->populateContextMenu( item, menu, selectedItems, context );
  }

  if ( menu->actions().isEmpty() )
  {
    delete menu;
    return;
  }

  menu->popup( mBrowserView->mapToGlobal( pt ) );
}

void QgsBrowserWidget::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
  mModel->setMessageBar( bar );
}

QgsMessageBar *QgsBrowserWidget::messageBar()
{
  return mMessageBar;
}

void QgsBrowserWidget::setDisabledDataItemsKeys( const QStringList &filter )
{
  mDisabledDataItemsKeys = filter;

  if ( !mProxyModel )
    return;

  mProxyModel->setHiddenDataItemProviderKeyFilter( mDisabledDataItemsKeys );
}

void QgsBrowserWidget::refresh()
{
  refreshModel( QModelIndex() );
}

void QgsBrowserWidget::refreshModel( const QModelIndex &index )
{
  if ( mModel && mProxyModel )
  {
    QgsDataItem *item = mModel->dataItem( index );
    if ( item )
    {
      QgsDebugMsgLevel( "path = " + item->path(), 4 );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "invalid item" ), 4 );
    }

    if ( item && ( item->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
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
      if ( mBrowserView->isExpanded( proxyIdx ) || mBrowserView->hasExpandedDescendant( proxyIdx ) || ( child && child->capabilities2() & Qgis::BrowserItemCapability::Fast ) )
      {
        refreshModel( idx );
      }
      else
      {
        if ( child && ( child->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
        {
          child->depopulate();
        }
      }
    }
  }
}

void QgsBrowserWidget::addLayer( QgsLayerItem *layerItem )
{
  if ( !layerItem )
    return;

  emit handleDropUriList( layerItem->mimeUris() );
}

void QgsBrowserWidget::addSelectedLayers()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // get a sorted list of selected indexes
  QModelIndexList list = mBrowserView->selectionModel()->selectedIndexes();
  std::sort( list.begin(), list.end() );

  // If any of the layer items are QGIS we just open and exit the loop
  const auto constList = list;
  for ( const QModelIndex &index : constList )
  {
    QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( index ) );
    if ( item && item->type() == Qgis::BrowserItemType::Project )
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
    if ( item && item->type() == Qgis::BrowserItemType::Layer )
    {
      QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
      if ( layerItem )
        addLayer( layerItem );
    }
  }

  QApplication::restoreOverrideCursor();
}

void QgsBrowserWidget::hideItem()
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem *item = mModel->dataItem( index );
  if ( ! item )
    return;

  if ( item->type() == Qgis::BrowserItemType::Directory )
  {
    mModel->hidePath( item );
  }
}

void QgsBrowserWidget::showProperties()
{
  QModelIndex index = mProxyModel->mapToSource( mBrowserView->currentIndex() );
  QgsDataItem *item = mModel->dataItem( index );
  if ( ! item )
    return;

  if ( item->type() == Qgis::BrowserItemType::Layer || item->type() == Qgis::BrowserItemType::Directory )
  {
    QgsBrowserPropertiesDialog *dialog = new QgsBrowserPropertiesDialog( settingsSection(), this );
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->setItem( item, createContext() );
    dialog->show();
  }
}

void QgsBrowserWidget::showFilterWidget( bool visible )
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

void QgsBrowserWidget::setFilter()
{
  QString filter = mLeFilter->text();
  if ( mProxyModel )
    mProxyModel->setFilterString( filter );
}

void QgsBrowserWidget::updateProjectHome()
{
  if ( mModel )
    mModel->updateProjectHome();
}

void QgsBrowserWidget::setFilterSyntax( QAction *action )
{
  if ( !action || ! mProxyModel )
    return;

  mProxyModel->setFilterSyntax( static_cast< QgsBrowserProxyModel::FilterSyntax >( action->data().toInt() ) );
}

void QgsBrowserWidget::setCaseSensitive( bool caseSensitive )
{
  if ( ! mProxyModel )
    return;
  mProxyModel->setFilterCaseSensitivity( caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive );
}

int QgsBrowserWidget::selectedItemsCount()
{
  QItemSelectionModel *selectionModel = mBrowserView->selectionModel();
  if ( selectionModel )
  {
    return selectionModel->selectedIndexes().size();
  }
  return 0;
}

QgsDataItemGuiContext QgsBrowserWidget::createContext()
{
  QgsDataItemGuiContext context;
  context.setMessageBar( mMessageBar );
  return context;
}

void QgsBrowserWidget::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  if ( mActionPropertiesWidget->isChecked() )
  {
    setPropertiesWidget();
  }
}

void QgsBrowserWidget::clearPropertiesWidget()
{
  while ( mPropertiesLayout->count() > 0 )
  {
    delete mPropertiesLayout->itemAt( 0 )->widget();
  }
  mPropertiesWidget->setVisible( false );
}

void QgsBrowserWidget::setPropertiesWidget()
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
      QgsDataItemGuiContext context = createContext();
      QgsBrowserPropertiesWidget *propertiesWidget = QgsBrowserPropertiesWidget::createWidget( item, context, mPropertiesWidget );
      if ( propertiesWidget )
      {
        propertiesWidget->setCondensedMode( true );
        mPropertiesLayout->addWidget( propertiesWidget );
      }
    }
  }
  mPropertiesWidget->setVisible( mPropertiesLayout->count() > 0 );
}

void QgsBrowserWidget::enablePropertiesWidget( bool enable )
{
  mActionPropertiesWidget->setChecked( enable );
  propertiesWidgetToggled( enable );
}

void QgsBrowserWidget::propertiesWidgetToggled( bool enabled )
{
  if ( enabled && selectedItemsCount() == 1 )
  {
    setPropertiesWidget();
  }
  else
  {
    clearPropertiesWidget();
  }

  QgsSettings settings;
  settings.setValue( settingsSection() + "/propertiesWidgetEnabled", enabled );
}

void QgsBrowserWidget::setActiveIndex( const QModelIndex &index )
{
  if ( index.isValid() )
  {
    QModelIndex proxyIndex = mProxyModel->mapFromSource( index );
    mBrowserView->expand( proxyIndex );
    mBrowserView->setCurrentIndex( proxyIndex );
  }
}

void QgsBrowserWidget::splitterMoved()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "%1/splitterState" ).arg( settingsSection() ), mSplitter->saveState() );
}
