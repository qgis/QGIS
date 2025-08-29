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
#include "moc_qgsbrowserwidget.cpp"

#include <QAbstractTextDocumentLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QPlainTextDocumentLayout>
#include <QSortFilterProxyModel>
#include <QActionGroup>
#include <QClipboard>
#include <QApplication>
#include <QDir>
#include <QFileInfo>

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
#include "qgsmessagebar.h"

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
  qgis::down_cast<QVBoxLayout *>( layout() )->setSpacing( 0 );

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
  
  // Location bar connections
  connect( mBtnNavigateToPath, &QToolButton::clicked, this, &QgsBrowserWidget::navigateToPath );
  connect( mBtnCopyPath, &QToolButton::clicked, this, &QgsBrowserWidget::copySelectedPath );
  connect( mLeLocationBar, &QLineEdit::returnPressed, this, &QgsBrowserWidget::navigateToPath );
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
  if ( !mModel->initialized() )
  {
    mModel->initialize();
  }
  if ( !mProxyModel )
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

    if ( mBrowserView->selectionModel() )
    {
      connect( mBrowserView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsBrowserWidget::selectionChanged );
      connect( mBrowserView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsBrowserWidget::updateLocationBar );
    }

    // Forward the model changed signals to the widget
    connect( mModel, &QgsBrowserModel::connectionsChanged, this, &QgsBrowserWidget::connectionsChanged );

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

  const QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
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
  std::function<void( const QModelIndex &index )> updateItem;
  updateItem = [this, &updateItem]( const QModelIndex &index ) {
    if ( QgsDirectoryItem *dirItem = qobject_cast<QgsDirectoryItem *>( mModel->dataItem( index ) ) )
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
  QList<QgsDataItem *> selectedItems;
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

  QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
  std::sort( providers.begin(), providers.end(), []( QgsDataItemGuiProvider *a, QgsDataItemGuiProvider *b ) {
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

void QgsBrowserWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
  mModel->setMapCanvas( canvas );
}

QgsMapCanvas *QgsBrowserWidget::mapCanvas()
{
  return mMapCanvas;
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
  if ( !item )
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
  if ( !item )
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
  if ( !visible )
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
  if ( !action || !mProxyModel )
    return;

  mProxyModel->setFilterSyntax( static_cast<QgsBrowserProxyModel::FilterSyntax>( action->data().toInt() ) );
}

void QgsBrowserWidget::setCaseSensitive( bool caseSensitive )
{
  if ( !mProxyModel )
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
  context.setMapCanvas( mMapCanvas );
  context.setView( mBrowserView );
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
  
  if ( !mProxyModel || !mBrowserView )
    return;

  if ( index.isValid() )
  {
    QModelIndex proxyIndex = mProxyModel->mapFromSource( index );
    if ( proxyIndex.isValid() )
    {
      mBrowserView->expand( proxyIndex );
      mBrowserView->setCurrentIndex( proxyIndex );
    }
  }
}

void QgsBrowserWidget::splitterMoved()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "%1/splitterState" ).arg( settingsSection() ), mSplitter->saveState() );
}

void QgsBrowserWidget::navigateToPath()
{
  if ( !mLeLocationBar || !mModel || !mBrowserView )
    return;

  const QString path = mLeLocationBar->text().trimmed();
  if ( path.isEmpty() )
    return;

  // Validate path format first
  if ( path.endsWith( ":" ) && path.length() == 2 )
  {
    if ( mMessageBar )
    {
      mMessageBar->pushWarning( tr( "Path does not exist: %1" ).arg( path ) );
    }
    return;
  }

  QString normalizedPath = QDir::cleanPath( path );
  const QString displayPath = QDir::toNativeSeparators( normalizedPath );
  
  // Check if path exists with exact case sensitivity
  QFileInfo pathInfo( normalizedPath );
  if ( !pathInfo.exists() )
  {
    if ( mMessageBar )
    {
      mMessageBar->pushWarning( tr( "Path does not exist: %1" ).arg( displayPath ) );
    }
    return;
  }

  // Verify case sensitivity by comparing canonical paths
  QString canonicalPath = pathInfo.canonicalFilePath();
  if ( !canonicalPath.isEmpty() && QDir::toNativeSeparators( canonicalPath ) != displayPath )
  {
    if ( mMessageBar )
    {
      mMessageBar->pushWarning( tr( "Path does not exist: %1" ).arg( displayPath ) );
    }
    return;
  }

  // Make sure we have an absolute path
  if ( pathInfo.isRelative() )
  {
    normalizedPath = pathInfo.absoluteFilePath();
    normalizedPath = QDir::cleanPath( normalizedPath );
  }

  // Use the existing expandPath functionality to properly expand the browser tree
  QString targetPath = normalizedPath;
  if ( pathInfo.isFile() )
  {
    // For files, expand to the parent directory
    targetPath = pathInfo.absolutePath();
  }

  try
  {
    mBrowserView->expandPath( targetPath, true );
    
    // Clear the location bar on successful navigation
    mLeLocationBar->clear();
  }
  catch ( ... )
  {
    if ( mMessageBar )
    {
      mMessageBar->pushCritical( tr( "Navigate to Path" ), tr( "Failed to navigate to path: %1. The folder may contain problematic files." ).arg( displayPath ) );
    }
  }
}

void QgsBrowserWidget::copySelectedPath()
{
  if ( !mBrowserView || !mBrowserView->selectionModel() || !mModel || !mLeLocationBar )
    return;

  const QModelIndexList selection = mBrowserView->selectionModel()->selectedIndexes();
  if ( selection.isEmpty() )
  {
    if ( mMessageBar )
    {
      mMessageBar->pushInfo( tr( "Copy Path" ), tr( "No item selected" ) );
    }
    return;
  }

  const QModelIndex index = selection.first();
  if ( !index.isValid() )
    return;

  QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( index ) );
  if ( !item )
    return;

  QString path;
  
  if ( QgsDirectoryItem *dirItem = qobject_cast<QgsDirectoryItem *>( item ) )
  {
    path = dirItem->dirPath();
  }
  else if ( QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item ) )
  {
    const QString uri = layerItem->uri();
    QFileInfo fileInfo( uri );
    if ( fileInfo.exists() )
    {
      path = fileInfo.absoluteFilePath();
    }
    else
    {
      path = uri; 
    }
  }
  else
  {

    path = item->path();
  }

  if ( !path.isEmpty() )
  {
    // Convert to native separators for clipboard (backslashes on Windows)
    QString nativePath = QDir::toNativeSeparators( path );
    QApplication::clipboard()->setText( nativePath );
    
    mLeLocationBar->setText( nativePath );
    
    if ( mMessageBar )
    {
      mMessageBar->pushSuccess( tr( "Copy Path" ), tr( "Path copied to clipboard: %1" ).arg( nativePath ) );
    }
  }
  else
  {
    if ( mMessageBar )
    {
      mMessageBar->pushWarning( tr( "Copy Path" ), tr( "Could not determine path for selected item" ) );
    }
  }
}

bool QgsBrowserWidget::ensurePathInModel( const QString &targetPath )
{
  if ( !mModel || !mBrowserView )
    return false;

  QString normalizedPath = QDir::cleanPath( targetPath );
  QFileInfo pathInfo( normalizedPath );
  
  // Make sure we have an absolute path
  if ( pathInfo.isRelative() )
  {
    normalizedPath = QDir::current().absoluteFilePath( normalizedPath );
    normalizedPath = QDir::cleanPath( normalizedPath );
  }

  QStringList targetVariants = generatePathVariants( normalizedPath );
  for ( const QString &variant : targetVariants )
  {
    QModelIndex index = mModel->findPath( variant );
    if ( index.isValid() )
      return true;
  }

  QStringList pathHierarchy;
  QString currentPath = normalizedPath;
  
  // Build the hierarchy from target to root
  while ( !currentPath.isEmpty() )
  {
    pathHierarchy.prepend( currentPath );
    
    QDir dir( currentPath );
    
    // Check if we've reached a root
#ifdef Q_OS_WIN
    // Windows drive root check (e.g., "C:", "C:\")
    if ( currentPath.length() <= 3 && currentPath.contains( ':' ) )
    {
      // Normalize drive letter format
      QString driveLetter = currentPath.left( 2 ).toUpper();
      if ( currentPath.length() == 2 )
        pathHierarchy[0] = driveLetter + '/';
      else
        pathHierarchy[0] = driveLetter + '/';
      break;
    }
#else
    if ( currentPath == "/" )
      break;
#endif
    
    if ( !dir.cdUp() || dir.absolutePath() == currentPath )
      break;
      
    currentPath = dir.absolutePath();
  }

  
#ifdef Q_OS_WIN
  if ( pathHierarchy.size() > 0 )
  {
    QString driveRoot = pathHierarchy[0];
    if ( driveRoot.contains( ':' ) )
    {
      mModel->refreshDrives();
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 200 );
      
      QModelIndex driveIndex;
      const auto driveItems = mModel->driveItems();
      for ( auto it = driveItems.constBegin(); it != driveItems.constEnd(); ++it )
      {
        QgsDirectoryItem *driveItem = it.value();
        if ( driveItem )
        {
          QString drivePath = driveItem->path();
          QString driveDir = driveItem->dirPath();
          
          
          if ( drivePath.startsWith( driveRoot.left( 2 ), Qt::CaseInsensitive ) ||
               driveDir.startsWith( driveRoot.left( 2 ), Qt::CaseInsensitive ) )
          {
            driveIndex = mModel->findItem( driveItem );
            break;
          }
        }
      }
      
      if ( !driveIndex.isValid() )
      {
        // Try to find the drive using findPath
        QStringList driveVariants = generatePathVariants( driveRoot );
        for ( const QString &variant : driveVariants )
        {
          driveIndex = mModel->findPath( variant );
          if ( driveIndex.isValid() )
            break;
        }
      }
      
      if ( driveIndex.isValid() )
      {
        QgsDataItem *driveItem = mModel->dataItem( driveIndex );
        if ( driveItem )
        {
          if ( driveItem->state() == Qgis::BrowserItemState::NotPopulated )
          {
            try
            {
              driveItem->populate();
              QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 200 );
            }
            catch ( ... )
            {
              return false;
            }
          }
          
          QModelIndex proxyIndex = mProxyModel->mapFromSource( driveIndex );
          if ( proxyIndex.isValid() && !mBrowserView->isExpanded( proxyIndex ) )
          {
            mBrowserView->expand( proxyIndex );
            QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 200 );
          }
          
          QModelIndex currentParent = driveIndex;
          for ( int i = 1; i < pathHierarchy.size(); ++i )
          {
            const QString &targetComponent = pathHierarchy[i];
            
            // Ensure parent is populated and refreshed
            QgsDataItem *parentItem = mModel->dataItem( currentParent );
            if ( parentItem )
            {
              if ( parentItem->state() == Qgis::BrowserItemState::NotPopulated )
              {
                parentItem->populate();
                QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 300 );
              }
              
              QModelIndex parentProxy = mProxyModel->mapFromSource( currentParent );
              if ( parentProxy.isValid() )
              {
                mBrowserView->expand( parentProxy );
                QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 300 );
              }
            }
            
            // Try to find the child
            QStringList variants = generatePathVariants( targetComponent );
            bool found = false;
            
            for ( const QString &variant : variants )
            {
              QModelIndex childIndex = mModel->findPath( variant );
              if ( childIndex.isValid() )
              {
                currentParent = childIndex;
                found = true;
                break;
              }
            }
            
            if ( !found && parentItem )
            {
              try
              {
                // Only refresh if it's safe to do so
                if ( parentItem->state() == Qgis::BrowserItemState::Populated )
                {
                  parentItem->refresh();
                  QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 200 );
                  
                  for ( const QString &variant : variants )
                  {
                    QModelIndex childIndex = mModel->findPath( variant );
                    if ( childIndex.isValid() )
                    {
                      currentParent = childIndex;
                      found = true;
                      break;
                    }
                  }
                }
              }
              catch ( ... )
              {
                break;
              }
            }
            
            if ( !found && i == pathHierarchy.size() - 1 )
            {
              return true;
            }
            else if ( !found )
            {
              return false;
            }
          }
          
          return true;
        }
      }
    }
  }
#endif

  QModelIndex deepestParent;
  int startIndex = 0;
  
  for ( int i = 0; i < pathHierarchy.size(); ++i )
  {
    const QString &pathComponent = pathHierarchy[i];
    QStringList variants = generatePathVariants( pathComponent );
    
    bool found = false;
    for ( const QString &variant : variants )
    {
      QModelIndex index = mModel->findPath( variant );
      if ( index.isValid() )
      {
        deepestParent = index;
        startIndex = i + 1;
        found = true;
        break;
      }
    }
    
    if ( !found )
      break;
  }

  // Expand from the deepest parent to the target
  for ( int i = startIndex; i < pathHierarchy.size(); ++i )
  {
    if ( deepestParent.isValid() )
    {
      QgsDataItem *parentItem = mModel->dataItem( deepestParent );
      if ( parentItem )
      {
        // Populate if needed
        if ( parentItem->state() == Qgis::BrowserItemState::NotPopulated )
        {
          try
          {
            parentItem->populate();
            QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 150 );
          }
          catch ( ... )
          {
            continue;
          }
        }
        // Expand in view
        QModelIndex proxyIndex = mProxyModel->mapFromSource( deepestParent );
        if ( proxyIndex.isValid() && !mBrowserView->isExpanded( proxyIndex ) )
        {
          mBrowserView->expand( proxyIndex );
          QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 150 );
        }
      }
    }
    
    // Try to find the next level
    const QString &targetComponent = pathHierarchy[i];
    QStringList variants = generatePathVariants( targetComponent );
    
    bool found = false;
    for ( const QString &variant : variants )
    {
      QModelIndex index = mModel->findPath( variant );
      if ( index.isValid() )
      {
        deepestParent = index;
        found = true;
        break;
      }
    }
    
    if ( !found && deepestParent.isValid() )
    {
      QgsDataItem *parentItem = mModel->dataItem( deepestParent );
      if ( parentItem )
      {
        try
        {
          if ( parentItem->state() == Qgis::BrowserItemState::Populated )
          {
            parentItem->refresh();
            QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 200 );
            
            for ( const QString &variant : variants )
            {
              QModelIndex index = mModel->findPath( variant );
              if ( index.isValid() )
              {
                deepestParent = index;
                found = true;
                break;
              }
            }
          }
        }
        catch ( ... )
        {
          break;
        }
      }
    }
    
    if ( !found && i == pathHierarchy.size() - 1 )
    {
      //if  Last component? might be a file
      return deepestParent.isValid();
    }
  }
  
  return deepestParent.isValid();
}

QStringList QgsBrowserWidget::generatePathVariants( const QString &path )
{
  QStringList pathVariants;
  pathVariants << path;

#ifdef Q_OS_WIN
  if ( path.contains( '/' ) )
    pathVariants << QString( path ).replace( '/', '\\' );
  if ( path.contains( '\\' ) )
    pathVariants << QString( path ).replace( '\\', '/' );
#else
  
  if ( path.contains( '\\' ) )
    pathVariants << QString( path ).replace( '\\', '/' );
#endif
  
  QString nativePath = QDir::toNativeSeparators( path );
  if ( !pathVariants.contains( nativePath ) )
    pathVariants << nativePath;
  
  QFileInfo fileInfo( path );
  if ( fileInfo.exists() )
  {
    QString canonicalPath = fileInfo.canonicalFilePath();
    if ( !canonicalPath.isEmpty() && !pathVariants.contains( canonicalPath ) )
      pathVariants << canonicalPath;
  }
  
  return pathVariants;
}

void QgsBrowserWidget::updateLocationBar()
{
  if ( !mBrowserView || !mBrowserView->selectionModel() || !mLeLocationBar || !mModel )
    return;

  const QModelIndexList selection = mBrowserView->selectionModel()->selectedIndexes();
  if ( selection.isEmpty() )
  {
    mLeLocationBar->clear();
    return;
  }

  const QModelIndex index = selection.first();
  if ( !index.isValid() )
  {
    mLeLocationBar->clear();
    return;
  }

  QgsDataItem *item = mModel->dataItem( mProxyModel->mapToSource( index ) );
  if ( !item )
  {
    mLeLocationBar->clear();
    return;
  }

  QString path;
  
  if ( QgsDirectoryItem *dirItem = qobject_cast<QgsDirectoryItem *>( item ) )
  {
    path = dirItem->dirPath();
  }
  else if ( QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item ) )
  {
    const QString uri = layerItem->uri();
    QFileInfo fileInfo( uri );
    if ( fileInfo.exists() )
    {
      path = fileInfo.absoluteFilePath();
    }
    else
    {
      path = uri;
    }
  }
  else
  {
    path = item->path();
  }

  if ( !path.isEmpty() )
  {
    // Convert to native separators for display (backslashes on Windows)
    QString nativePath = QDir::toNativeSeparators( path );
    mLeLocationBar->setText( nativePath );
  }
  else
  {
    mLeLocationBar->clear();
  }
}
