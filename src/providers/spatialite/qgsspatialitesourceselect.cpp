/***************************************************************************
                             qgsspatialitesourceselect.cpp
       Dialog to select SpatiaLite layer(s) and add it to the map canvas
                              -------------------
begin                : Dec 2008
copyright            : (C) 2008 by Sandro Furieri
email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialitesourceselect.h"

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "qgsmapcanvas.h"

#include <QMenu>
#include <QClipboard>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStringList>
#include <QPushButton>

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

#ifdef HAVE_GUI
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect *selectWidget++
//-----------------------------------------------------------------
QGISEXTERN QgsSpatiaLiteSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsSpatiaLiteSourceSelect( parent, fl, widgetMode );
}
#endif
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect
//-----------------------------------------------------------------
QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );
  mTabWidget->setTabEnabled( getTabLayerOrderIndex(), false );
#if 0
  mTabWidget->setTabEnabled( getTabLayerTilesetsIndex(), false );
  mTabWidget->setTabEnabled( getTabServerSearchIndex(), false );
#else
  mTabWidget->removeTab( getTabLayerTilesetsIndex() );
  mTabWidget->removeTab( getTabServerSearchIndex() );
#endif
  mConnectionsRootItem = mConnectionsSpatialiteDbInfoModel.setModelType( QgsSpatialiteDbInfoModel::ModelTypeConnections );
  mLayerOrderRootItem = mLayerOrderSpatialiteDbInfoModel.setModelType( QgsSpatialiteDbInfoModel::ModelTypeLayerOrder );
  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/SpatiaLiteSourceSelect/geometry" ) ).toByteArray() );
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/SpatiaLiteSourceSelect/HoldDialogOpen" ), false ).toBool() );
  setWindowTitle( tr( "Add SpatiaLite / GeoPackage / MBTiles Layer(s)" ) );
//-----------------------------------------------------------------
// Not used
//-----------------------------------------------------------------
  mEditButton->hide();  // hide the edit button
  mLoadButton->hide();
//-----------------------------------------------------------------
// Tab 'Connections'
//-----------------------------------------------------------------
  mConnectionsCreateEmptySpatialiteDbButton->setText( tr( "Create empty Database" ) );
  // mDbCreateOptionComboBox = new QComboBox( connectionsGroupBox );
  mDbCreateOptionComboBox->setObjectName( QStringLiteral( "mDbCreateOptionComboBox" ) );
  //horizontalLayout->addWidget( mDbCreateOptionComboBox );
  mDbCreateOptionComboBox->addItem( tr( "Spatialite (basic)" ), QgsSpatialiteDbInfo::Spatialite40 );
  mDbCreateOptionComboBox->addItem( tr( "Spatialite (with Styles, Raster and VectorCoverages Table's)" ), QgsSpatialiteDbInfo::Spatialite50 );
  mDbCreateOptionComboBox->addItem( "GeoPackage", QgsSpatialiteDbInfo::SpatialiteGpkg );
  mDbCreateOptionComboBox->addItem( "MBTiles", QgsSpatialiteDbInfo::SpatialiteMBTiles );
//-----------------------------------------------------------------
// TODO: build these into the ui file
//-----------------------------------------------------------------
  mSourceSelectUpdateLayerStatiticsButton = new QPushButton( tr( "&Update Statistics" ) );
  mSourceSelectUpdateLayerStatiticsButton->setEnabled( false );
  mSourceSelectAddLayersButton = new QPushButton( tr( "&Add" ) );
  mSourceSelectAddLayersButton->setEnabled( false );
  mSourceSelectBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  mSourceSelectBuildQueryButton->setEnabled( false );
  if ( mWidgetMode != QgsProviderRegistry::WidgetMode::None )
  {
    mDialogButtonBox->removeButton( mDialogButtonBox->button( QDialogButtonBox::Close ) );
    mHoldDialogOpen->hide();
  }
  mDialogButtonBox->addButton( mSourceSelectAddLayersButton, QDialogButtonBox::ActionRole );
  mDialogButtonBox->addButton( mSourceSelectBuildQueryButton, QDialogButtonBox::ActionRole );
  mDialogButtonBox->addButton( mSourceSelectUpdateLayerStatiticsButton, QDialogButtonBox::ActionRole );
//----------------------------------------------------------------
  populateConnectionList();
  mSearchModeComboBox->addItem( tr( "Wildcard" ) );
  mSearchModeComboBox->addItem( tr( "RegExp" ) );
  mSearchColumnComboBox->addItem( tr( "All" ) );
//----------------------------------------------------------------
// Tab 'Connections'
//-----------------------------------------------------------------
  mConnectionsProxyModel.setParent( this );
  mConnectionsProxyModel.setFilterKeyColumn( -1 );
  mConnectionsProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mConnectionsProxyModel.setDynamicSortFilter( true );
  mConnectionsProxyModel.setSourceModel( &mConnectionsSpatialiteDbInfoModel );
  mConnectionsTreeView->setModel( &mConnectionsProxyModel );
  mConnectionsTreeView->setSortingEnabled( true );
  mConnectionsTreeView->setColumnHidden( mConnectionsSpatialiteDbInfoModel.getColumnSortHidden(), true );
  mConnectionsTreeView->header()->setResizeMode( QHeaderView::Stretch ); // QHeaderView::ResizeToContents
  // Insures that text is onSync with the QgsSpatialiteDbInfoModel Header-Labels
  mSearchColumnComboBox->addItem( mConnectionsSpatialiteDbInfoModel.getTableNameText() );
  mSearchColumnComboBox->addItem( mConnectionsSpatialiteDbInfoModel.getGeometryNameText() );
  mSearchColumnComboBox->addItem( mConnectionsSpatialiteDbInfoModel.getGeometryTypeText() );
  mSearchColumnComboBox->addItem( mConnectionsSpatialiteDbInfoModel.getSqlQueryText() );
  // mConnectionsTreeView defined in 'ui_qgsdbsourceselectbase.h'
  // Default values:
  // - editTriggers : QAbstractItemView::NoEditTriggers
  // - alternatingRowColors : true
  // - selectionMode : QAbstractItemView::ExtendedSelection
  // enum QAbstractItemView::EditTrigger
  // Set Editable - DoubleClick of F2
  // Item-Column must be enabled and Editable
  mConnectionsTreeView->setEditTriggers( QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed );
  // During the Model creation a default Help-Text will be created, expand all levels
  mConnectionsTreeView->setColumnHidden( mConnectionsSpatialiteDbInfoModel.getColumnSortHidden(), true );
  mConnectionsTreeView->sortByColumn( mConnectionsSpatialiteDbInfoModel.getColumnSortHidden(), Qt::AscendingOrder );
  mConnectionsTreeView->expandToDepth( mConnectionsSpatialiteDbInfoModel.getExpandToDepth() );
  mConnectionsTreeView->setContextMenuPolicy( Qt::CustomContextMenu );

  // This must be keep onSync with the QgsSpatiaLiteTableModel Header-Labels
  mSearchColumnComboBox->addItem( tr( "Table" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry-Type" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry column" ) );
  mSearchColumnComboBox->addItem( tr( "Sql" ) );
  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 1 );

  //hide the search options by default
  //they will be shown when the user ticks
  //the search options group box
  mSearchLabel->setVisible( false );
  mSearchColumnComboBox->setVisible( false );
  mSearchColumnsLabel->setVisible( false );
  mSearchModeComboBox->setVisible( false );
  mSearchModeLabel->setVisible( false );
  mSearchLineEdit->setVisible( false );
  // cbxAllowGeometrylessTables->setVisible( false );
  connect( this, &QgsAbstractDataSourceWidget::addDatabaseLayers, this, &QgsSpatiaLiteSourceSelect::addConnectionsDatabaseSource );
  connect( mConnectionsConnectButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
  connect( mConnectionsCreateButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
  connect( mConnectionsDeleteButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
  connect( mConnectionsTreeView, &QWidget::customContextMenuRequested, this, &QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequested );
  connect( mConnectionsTreeView, &QTreeView::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_clicked );
  connect( mConnectionsTreeView, &QTreeView::doubleClicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_doubleClicked );
  connect( mConnectionsTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_SelectionChanged );
  connect( mConnectionsCreateEmptySpatialiteDbButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
//-----------------------------------------------------------------
// Tab 'Layer Order'
// - no sorting
//-----------------------------------------------------------------
  mLayerOrderProxyModel.setParent( this );
  mLayerOrderProxyModel.setFilterKeyColumn( -1 );
  mLayerOrderProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mLayerOrderProxyModel.setDynamicSortFilter( false );
  mLayerOrderProxyModel.setSourceModel( &mLayerOrderSpatialiteDbInfoModel );

  mLayerOrderTreeView->setModel( &mLayerOrderProxyModel );
  mLayerOrderTreeView->setSortingEnabled( false );
  mLayerOrderTreeView->setColumnHidden( mLayerOrderSpatialiteDbInfoModel.getColumnSortHidden(), true );
  mLayerOrderTreeView->header()->setResizeMode( QHeaderView::ResizeToContents ); // QHeaderView::ResizeToContents
  mLayerOrderTreeView->setEditTriggers( QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed );
  mLayerOrderTreeView->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( mLayerOrderTreeView, &QWidget::customContextMenuRequested, this, &QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequested );
  mLayerOrderTreeView->setColumnHidden( mLayerOrderSpatialiteDbInfoModel.getColumnSortHidden(), true );
  connect( mLayerOrderDownButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
  connect( mLayerOrderTreeView, &QTreeView::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_clicked );
  connect( mLayerOrderTreeView, &QTreeView::doubleClicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_doubleClicked );
  connect( mLayerOrderTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_SelectionChanged );
  connect( mLayerOrderUpButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
//-----------------------------------------------------------------
// Outside of Tabs
//-----------------------------------------------------------------
  connect( mSourceSelectUpdateLayerStatiticsButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
  connect( mSourceSelectAddLayersButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
  connect( mSourceSelectBuildQueryButton, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked );
//-----------------------------------------------------------------
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_clicked
//-----------------------------------------------------------------
// - Tab 'Connections'
//-----------------------------------------------------------------
// -> mConnectionsTreeView
//-----------------------------------------------------------------
// - Tab 'Layer Order'
//-----------------------------------------------------------------
// -> mLayerOrderTreeView
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_clicked( const QModelIndex &index )
{
  QTreeView *treeviewSender = qobject_cast<QTreeView *>( sender() );
  QModelIndex item_index;
  bool bValidSelection = false;
  bool bIsSpatialite = false;
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( treeviewSender == mConnectionsTreeView )
    {
      item_index = mConnectionsProxyModel.mapToSource( index );
      if ( !mConnectionsSpatialiteDbInfoModel.getLayerName( item_index ).isEmpty() )
      {
        bValidSelection = index.parent().isValid();
      }
      if ( mConnectionsSpatialiteDbInfoModel.isSpatialite() )
      {
        bIsSpatialite = true;
      }
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
    if ( treeviewSender == mLayerOrderTreeView )
    {
      item_index = mLayerOrderProxyModel.mapToSource( index );
      if ( !mLayerOrderSpatialiteDbInfoModel.getLayerName( item_index ).isEmpty() )
      {
        bValidSelection = index.parent().isValid();
      }
      if ( mLayerOrderSpatialiteDbInfoModel.isSpatialite() )
      {
        bIsSpatialite = true;
      }
    }
  }
  // Only known and valid layers will be enabled [directories, Non-Spatial will not]
  mSourceSelectAddLayersButton->setEnabled( bValidSelection );
  mSourceSelectBuildQueryButton->setEnabled( bValidSelection );
  mSourceSelectUpdateLayerStatiticsButton->setEnabled( bIsSpatialite );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_doubleClicked
//-----------------------------------------------------------------
// - Tab 'Connections'
//-----------------------------------------------------------------
// -> mConnectionsTreeView
//-----------------------------------------------------------------
// - Tab 'Layer Order'
//-----------------------------------------------------------------
// -> mLayerOrderTreeView
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_doubleClicked( const QModelIndex &index )
{
  QTreeView *treeviewSender = qobject_cast<QTreeView *>( sender() );
  QModelIndex item_index;
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( treeviewSender == mConnectionsTreeView )
    {
      // QgsDebugMsgLevel( QString( "QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_doubleClicked [ConnectionsTreeView]" ), 7 );
      item_index = mConnectionsProxyModel.mapToSource( index );
      if ( !mConnectionsSpatialiteDbInfoModel.getLayerName( item_index ).isEmpty() )
      {
        // Only known and valid layers will be enabled [directories, Non-Spatial will not]
        setLayerSqlQuery( index );
      }
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
    if ( treeviewSender == mLayerOrderTreeView )
    {
      item_index = mLayerOrderProxyModel.mapToSource( index );
      QgsSpatialiteDbInfoItem *currentItem = mLayerOrderSpatialiteDbInfoModel.getLayerItem( item_index );
      if ( currentItem )
      {
        switch ( currentItem->getItemType() )
        {
          case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderVectorLayer:
          case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderRasterLayer:
            showStatusMessage( QString( "doubleClicked -Layer- ItemText[%1 ] LayerName[%2 ] ItemType[%3]" ).arg( currentItem->text() ).arg( currentItem->getLayerName() ).arg( currentItem->getItemTypeString() ), 0 );
            break;
          case QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata:
            // Set the selected Style in the Layer-Style column
            if ( mLayerOrderSpatialiteDbInfoModel.setSelectedStyle( currentItem ) )
            {
              showStatusMessage( QString( "'%1' has been set to use the Style: '%2'" ) .arg( currentItem->getLayerName() ).arg( currentItem->text() ), 0 );
            }
            break;
          default:
            showStatusMessage( QString( "doubleClicked -default- ItemText[%1 ] LayerName[%2 ] ItemType[%3]" ).arg( currentItem->text() ).arg( currentItem->getLayerName() ).arg( currentItem->getItemTypeString() ), 0 );
            break;
        }
      }
    }
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_SelectionChanged
// - Tab 'Connections'
//-----------------------------------------------------------------
// -> mConnectionsTreeView
//-----------------------------------------------------------------
// - Tab 'Layer Order'
//-----------------------------------------------------------------
// -> mLayerOrderTreeView
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::onSourceSelectTreeView_SelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  // Note: sender() is a QItemSelectionModel
  QTreeView *treeviewSender = qobject_cast<QTreeView *>( sender()->parent() );
  bool bValidSelection = false;
  bool bRemoveItems = false;
  QString sSelectedType = QStringLiteral( "added" );
  int iCountItems = 0;
  if ( selected.indexes().size() < deselected.indexes().size() )
  {
    bRemoveItems = true;
    sSelectedType = QStringLiteral( "removed" );
  }
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( treeviewSender == mConnectionsTreeView )
    {
      // Retrieve the list of what is still selected.
      // When 'bRemoveItems = true', the Items NOT in the list will be removed
      QItemSelection selection = mConnectionsTreeView->selectionModel()->selection();
      QModelIndexList selectedIndices = selection.indexes();
      QgsSpatialiteDbInfoItem *currentItem = nullptr;
      QModelIndexList::const_iterator selected_it = selectedIndices.constBegin();
      for ( ; selected_it != selectedIndices.constEnd(); ++selected_it )
      {
        if ( !selected_it->parent().isValid() )
        {
          //top level items only contain the schema names
          continue;
        }
        currentItem = mConnectionsSpatialiteDbInfoModel.getItem( mConnectionsProxyModel.mapToSource( *selected_it ) );
        // Only valid, Layers are to be selected, everything will be ignored
        if ( ( !currentItem ) || ( !currentItem->isValid() ) || ( !currentItem->isLayerSelectable() ) )
        {
          continue;
        }
        if ( mLayerOrderRootItem )
        {
          // Only unique LayerNames of selected Items will be added
          mLayerOrderRootItem->addPrepairedChildItem( currentItem );
        }
        bValidSelection = true;
      }
      // When 'bRemoveItems = true', the Items NOT in the LayerOrder list will be removed
      iCountItems = setLayerOrderData( bRemoveItems );
      showStatusMessage( QString( "Layer(s) have been  %2: %1." ).arg( iCountItems ).arg( sSelectedType ), 0 );
      mSourceSelectAddLayersButton->setEnabled( bValidSelection );
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
    if ( treeviewSender == mLayerOrderTreeView )
    {
      // QgsDebugMsgLevel( QString( "onSourceSelectTreeView_SelectionChanged [LayerOrderTreeView] RemoveItems[%1]" ).arg( bRemoveItems ), 7 );
    }
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked
//-----------------------------------------------------------------
// - Tab 'Connections'
//-----------------------------------------------------------------
// -> mConnectionsConnectButton
// --> calls addConnectionDatabaseSource
// -> mConnectionsCreateButton
// --> calls populateConnectionList
// -> mConnectionsDeleteButton
// --> calls connectionDelete
// -> mConnectionsCreateEmptySpatialiteDbButton
// --> calls createEmptySpatialiteDb
//-----------------------------------------------------------------
// - Tab 'Layer Order'
//-----------------------------------------------------------------
// -> mLayerOrderUpButton
// -> mLayerOrderDownButton
// --> moveLayerOrderUpDown [both]
//-----------------------------------------------------------------
// - outside of Tabs
//-----------------------------------------------------------------
// -> mSourceSelectAddLayersButton
// --> addSelectedDbLayers
// -> mSourceSelectUpdateLayerStatiticsButton
// --> onUpdateLayerStatitics
// -> mSourceSelectBuildQueryButton
// --> calls setLayerSqlQuery
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::onSourceSelectButtons_clicked()
{
  QPushButton *buttonSender = qobject_cast<QPushButton *>( sender() );
  if ( buttonSender == mSourceSelectAddLayersButton )
  {
    addSelectedDbLayers();
  }
  else if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( buttonSender == mConnectionsConnectButton )
    {
      // trying to connect to SpatiaLite DB
      addConnectionDatabaseSource( mConnectionsComboBox->currentText() );
    }
    else if ( buttonSender == mConnectionsCreateButton )
    {
      if ( ! newConnection( this ) )
      {
        return;
      }
      populateConnectionList();
      emit connectionsChanged();
    }
    else if ( buttonSender == mConnectionsDeleteButton )
    {
      connectionDelete( mConnectionsComboBox->currentText() );
    }
    else if ( buttonSender == mConnectionsCreateEmptySpatialiteDbButton )
    {
      createEmptySpatialiteDb( ( QgsSpatialiteDbInfo::SpatialMetadata )mDbCreateOptionComboBox->currentData().toInt(), mDbCreateOptionComboBox->currentText() );
    }
    else if ( buttonSender == mSourceSelectUpdateLayerStatiticsButton )
    {
      onUpdateLayerStatitics();
    }
    else if ( buttonSender == mSourceSelectBuildQueryButton )
    {
      setLayerSqlQuery( mConnectionsTreeView->currentIndex() );
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
    if ( ( buttonSender == mLayerOrderUpButton ) || ( buttonSender == mLayerOrderDownButton ) )
    {
      int iMoveCount = 0;
      if ( buttonSender == mLayerOrderUpButton )
      {
        iMoveCount = -1;
      }
      else if ( buttonSender == mLayerOrderDownButton )
      {
        iMoveCount = 1;
      }
      if ( iMoveCount != 0 )
      {
        moveLayerOrderUpDown( iMoveCount );
      }
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::createEmptySpatialiteDb
// - called from onSourceSelectButtons_clicked
//-----------------------------------------------------------------
bool QgsSpatiaLiteSourceSelect::createEmptySpatialiteDb( QgsSpatialiteDbInfo::SpatialMetadata dbCreateOption, QString sDbCreateOption )
{
  bool bRc = false;
  QString sFileName = QStringLiteral( "empty_Spatialite40.db" );
  QString sDbType = QStringLiteral( "Spatialite" );
  QString sFileTypes = QStringLiteral( "%1 (*.sqlite *.db *.sqlite3 *.db3 *.s3db *.atlas *.mbtiles *.gpkg);;%2 (*)" ).arg( sDbType ).arg( tr( "All files" ) );
  switch ( dbCreateOption )
  {
    case QgsSpatialiteDbInfo::Spatialite50:
      if ( sDbCreateOption.isEmpty() )
      {
        sDbCreateOption = QStringLiteral( "Spatialite (with Styles, Raster and VectorCoverages Table's)" );
      }
      sFileName = QStringLiteral( "empty_Spatialite50.db" );
      break;
    case QgsSpatialiteDbInfo::SpatialiteGpkg:
      if ( sDbCreateOption.isEmpty() )
      {
        sDbCreateOption = QStringLiteral( "GeoPackage" );
      }
      sFileName = QStringLiteral( "empty_GeoPackage.gpkg" );
      sDbType = QStringLiteral( "GeoPackage" );
      break;
    case QgsSpatialiteDbInfo::SpatialiteMBTiles:
      if ( sDbCreateOption.isEmpty() )
      {
        sDbCreateOption = QStringLiteral( "MBTiles" );
      }
      sFileName = QStringLiteral( "empty_MBTiles.mbtiles" );
      sDbType = QStringLiteral( "MBTiles" );
      break;
    case QgsSpatialiteDbInfo::Spatialite40:
    default:
      if ( sDbCreateOption.isEmpty() )
      {
        sDbCreateOption = QStringLiteral( "Spatialite (basic)" );
      }
      sFileName = QStringLiteral( "empty_Spatialite40.db" );
      break;
  }
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::currentPath() ).toString();
  lastUsedDir = QStringLiteral( "%1/%2" ).arg( lastUsedDir ).arg( sFileName );
  QString sTitle = QString( tr( "Choose a Database to create [%1]" ) ).arg( sDbCreateOption );
  QString fileSelect = QFileDialog::getSaveFileName( this, sTitle, lastUsedDir, sFileTypes );
  if ( fileSelect.isEmpty() )
  {
    return bRc;
  }
  if ( mConnectionsSpatialiteDbInfoModel.createDatabase( fileSelect, dbCreateOption ) )
  {
    bRc = true;
    mConnectionsTreeView->setColumnHidden( mConnectionsSpatialiteDbInfoModel.getColumnSortHidden(), true );
    mConnectionsTreeView->sortByColumn( mConnectionsSpatialiteDbInfoModel.getColumnSortHidden(), Qt::AscendingOrder );
    mConnectionsTreeView->expandToDepth( mConnectionsSpatialiteDbInfoModel.getExpandToDepth() );
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::showStatusMessage
// - Outside of Tab area
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::showStatusMessage( QString const &message, int iDebug )
{
  if ( iDebug < 1 )
  {
    mStatusLabel->setText( message );
    // update the display of this widget
    update();
  }
  else
  {
    QgsDebugMsgLevel( message, iDebug );
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::moveLayerOrderUpDown
// - Tab 'Layer Order'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::moveLayerOrderUpDown( int iMoveCount )
{
  QgsSpatialiteDbInfoItem *currentItem = nullptr;
  if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
    QItemSelection selection = mLayerOrderTreeView->selectionModel()->selection();
    QModelIndexList selectedIndices = selection.indexes();
    QModelIndexList::const_iterator selected_it = selectedIndices.constBegin();
    for ( ; selected_it != selectedIndices.constEnd(); ++selected_it )
    {
      if ( !selected_it->parent().isValid() )
      {
        //top level items only contain the schema names
        continue;
      }
      QModelIndex currentIndex = mLayerOrderProxyModel.mapToSource( *selected_it );
      currentItem = mLayerOrderSpatialiteDbInfoModel.getItem( currentIndex );
      // Only valid, Layers are to be selected, everything will be ignored
      if ( ( !currentItem ) || ( !currentItem->isValid() ) || ( !currentItem->isLayerSelectable() ) )
      {
        currentItem = nullptr;
        continue;
      }
      // Only when the LayerOrder is active and there is more than 1 Child
      //  [rather senseless to move 1 child Up or Down]
      if ( ( mLayerOrderRootItem ) && ( currentItem->parent()->childCount() > 1 ) )
      {
        QString sLayerName = currentItem->getLayerName();
        QString sMoveType = QStringLiteral( "Down" );
        if ( iMoveCount < 0 )
        {
          sMoveType = QStringLiteral( "Up" );
        }
        if ( ( currentItem->childNumber() + iMoveCount ) < 0 )
        {
          sMoveType = QStringLiteral( "to the Bottom" );
        }
        else if ( ( currentItem->childNumber() + iMoveCount ) >=  currentItem->parent()->childCount() )
        {
          sMoveType = QStringLiteral( "to the Top" );
        }
        // This will move the Item to the desired position in the Parent [Up/Down/Top/Bottom]
        currentIndex = mLayerOrderSpatialiteDbInfoModel.moveLayerOrderChild( currentItem, iMoveCount );
        if ( currentIndex.isValid() )
        {
          // Reselect the moved Item
          mLayerOrderTreeView->setCurrentIndex( currentIndex );
#if 0
          sMoveType += QStringLiteral( " MoveCount[%1]" ).arg( iMoveCount );
#endif
          showStatusMessage( QString( "'%1' has been moved %2" ).arg( sLayerName ).arg( sMoveType ), 0 );
        }
#if 0
        else
        {
          showStatusMessage( QString( "-E-> LayerName[%1] has been moved %2." ).arg( sLayerName ).arg( sMoveType ), 0 );
        }
#endif
      }
      break;
    }
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequested
// - Tab 'Connections'
// - Tab 'Layer Order'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequested( QPoint pointPosition )
{
  QModelIndex index;
  int itemColumn = -1;
  QgsSpatialiteDbInfoItem *currentItem = nullptr;
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    index = mConnectionsTreeView->indexAt( pointPosition );
    itemColumn = mConnectionsTreeView->header()->logicalIndexAt( pointPosition );
    currentItem = mConnectionsSpatialiteDbInfoModel.getItem( mConnectionsProxyModel.mapToSource( index ) );
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
    index = mLayerOrderTreeView->indexAt( pointPosition );
    itemColumn = mLayerOrderTreeView->header()->logicalIndexAt( pointPosition );
    currentItem = mLayerOrderSpatialiteDbInfoModel.getItem( mLayerOrderProxyModel.mapToSource( index ) );
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
  if ( ( currentItem ) && ( currentItem->isValid() ) )
  {
    QString sMenuText;
    QString sToolTip;
    int iCommandRole = -1; // if getCopyCellText=true : iCommandRole= 0=copy ; 1=exit ; 2=remove
    if ( currentItem->getCopyCellText( itemColumn, iCommandRole, sMenuText, sToolTip ) )
    {
      // Select the right-clicked item
      QMenu *menu = new QMenu( this );
      menu->setToolTipsVisible( true );
      if ( ( iCommandRole == 0 ) || ( iCommandRole == 10 )  || ( iCommandRole == 20 ) )
      {
        QString sMenuTextCopy;
        QString sToolTipCopy;
        switch ( iCommandRole )
        {
          case 10:
          case 20:
            sMenuTextCopy = QStringLiteral( "Copy Cell text" );
            sToolTipCopy = QStringLiteral( "Copy Cell text for ItemType[%1] ModelType[%2]" ).arg( currentItem->getItemTypeString() ).arg( currentItem->getModelTypeString() );
            break;
          default:
            sMenuTextCopy = sMenuText;
            sToolTipCopy = sToolTip;
            break;
        }
        QAction *copyCellText = new QAction( sMenuTextCopy, this );
        copyCellText->setToolTip( sToolTipCopy );
        copyCellText->setData( pointPosition );
        copyCellText->setObjectName( QStringLiteral( "copyCellText" ) );
        connect( copyCellText, &QAction::triggered, this, &QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequest );
        menu->addAction( copyCellText );
      }
      // The combination Edit and Remove does not exist (yet)
      if ( ( iCommandRole == 2 ) || ( iCommandRole == 20 ) )
      {
        // Select the right-clicked item
        menu->setToolTipsVisible( true );
        QAction *removeLayerOrderChild = new QAction( sMenuText, this );
        removeLayerOrderChild->setToolTip( sToolTip );
        removeLayerOrderChild->setData( pointPosition );
        removeLayerOrderChild->setObjectName( QStringLiteral( "removeLayerOrderChild" ) );
        connect( removeLayerOrderChild, &QAction::triggered, this, &QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequest );
        menu->addAction( removeLayerOrderChild );
      }
      if ( ( currentItem->getDbLayer() ) && ( mapCanvas() ) )
      {
        if ( ( iCommandRole == 1 ) || ( iCommandRole == 10 ) )
        {
          bool bEditable = currentItem->isEditable( itemColumn );
          sMenuText = QStringLiteral( "Set Cell to Editable" );
          if ( !bEditable )
          {
            sMenuText = QStringLiteral( "Set Cell to Browsable" );
          }
          sToolTip = sMenuText;
          QAction *setEditRole = new QAction( sMenuText, this );
          setEditRole->setToolTip( sToolTip );
          setEditRole->setData( pointPosition );
          setEditRole->setObjectName( QStringLiteral( "setEditRole" ) );
          connect( setEditRole, &QAction::triggered, this, &QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequest );
          menu->addAction( setEditRole );
        }
        // mapCanvas() can be null [but is not]
#if 0
        // Note: mapCanvas() is read only [mapCanvas()->setExtent cannot be called in spatialiteDbInfoContextMenuRequest]
        switch ( currentItem->getItemType() )
        {
          case QgsSpatialiteDbInfoItem::ItemTypePointMetadata:
          {
            sMenuText = QStringLiteral( "Move to Center of Extent" );
            QAction *gotoPoint = new QAction( sMenuText, this );
            gotoPoint->setToolTip( sToolTip );
            gotoPoint->setData( pointPosition );
            gotoPoint->setObjectName( QStringLiteral( "gotoPoint" ) );
            connect( gotoPoint, &QAction::triggered, this, &QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequest );
            menu->addAction( gotoPoint );
          }
          break;
          case QgsSpatialiteDbInfoItem::ItemTypeRectangleMetadata:
          {
            sMenuText = QStringLiteral( "Move to  Extent" );
            QAction *gotoExtent = new QAction( sMenuText, this );
            gotoExtent->setToolTip( sToolTip );
            gotoExtent->setData( pointPosition );
            gotoExtent->setObjectName( QStringLiteral( "gotoExtent" ) );
            connect( gotoExtent, &QAction::triggered, this, &QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequest );
            menu->addAction( gotoExtent );
          }
          break;
          default:
            break;
        }
#endif
      }
      menu->popup( mConnectionsTreeView->viewport()->mapToGlobal( pointPosition ) );
#if 1
    }
#else
      // This is only needed during development of new Item-Type settings
      QString sMessage = QStringLiteral( "CopyCellText: Column[%1] EditRole[%2] MenuText[%3] ToolTip[%4] LayerName[%5] ItemType[%6]" ).arg( itemColumn ).arg( iCommandRole ).arg( sMenuText ).arg( sToolTip ).arg( currentItem->getLayerName() ).arg( currentItem->getItemTypeString() );
      showStatusMessage( sMessage, 0 );
    }
    else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
    {
      QString sMessage = QStringLiteral( "CopyCellText: Column[%1] EditRole[%2] MenuText[%3] ToolTip[%4] LayerName[%5] ItemType[%6]" ).arg( itemColumn ).arg( iCommandRole ).arg( sMenuText ).arg( sToolTip ).arg( currentItem->getLayerName() ).arg( currentItem->getItemTypeString() );
      showStatusMessage( sMessage, 0 );
    }
#endif
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequest
// - Tab 'Connections'
// - Tab 'Layer Order'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::spatialiteDbInfoContextMenuRequest()
{
  QAction *pAction = qobject_cast<QAction *>( sender() );
  QPoint pointPosition = pAction->data().toPoint();
  QString sActionName = pAction->objectName();
  if ( pointPosition.isNull() )
  {
    pointPosition = QCursor::pos(); ///< pos will be current cursor's position
  }
  QModelIndex index;
  int itemColumn = -1;
  QgsSpatialiteDbInfoItem *currentItem = nullptr;
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    index = mConnectionsTreeView->indexAt( pointPosition );
    itemColumn = mConnectionsTreeView->header()->logicalIndexAt( pointPosition );
    currentItem = mConnectionsSpatialiteDbInfoModel.getItem( mConnectionsProxyModel.mapToSource( index ) );
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
    index = mLayerOrderTreeView->indexAt( pointPosition );
    itemColumn = mLayerOrderTreeView->header()->logicalIndexAt( pointPosition );
    currentItem = mLayerOrderSpatialiteDbInfoModel.getItem( mLayerOrderProxyModel.mapToSource( index ) );
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
  if ( ( currentItem ) && ( currentItem->isValid() ) )
  {
    QString sLayerName = currentItem->getLayerName();
    if ( sActionName == QStringLiteral( "copyCellText" ) )
    {
      QApplication::clipboard()->setText( currentItem->text( itemColumn ) );
      showStatusMessage( QStringLiteral( "Text from Column %1 has been copied" ).arg( itemColumn ), 0 );
    }
    else if ( sActionName == QStringLiteral( "setEditRole" ) )
    {
      currentItem->setEditable( itemColumn, currentItem->isEditable( itemColumn ) );
      showStatusMessage( QStringLiteral( "'Column %1 has been set to Editable" ).arg( itemColumn ), 0 );
    }
    else if ( sActionName == QStringLiteral( "removeLayerOrderChild" ) )
    {
      if ( getCurrentTabIndex() == getTabConnectionsIndex() )
      {
      }
      else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
      {
        QModelIndex currentIndex = mLayerOrderSpatialiteDbInfoModel.removeLayerOrderChild( currentItem );
        if ( currentIndex.isValid() )
        {
          mLayerOrderTreeView->setCurrentIndex( currentIndex );
        }
        showStatusMessage( QStringLiteral( "'%1' has been removed" ).arg( sLayerName ), 0 );
      }
    }
    else if ( ( sActionName == QStringLiteral( "gotoPoint" ) ) || ( sActionName == QStringLiteral( "gotoExtent" ) ) )
    {
      if ( ( currentItem->getDbLayer() ) && ( mapCanvas() ) )
      {
        // mapCanvas() can be null
        QgsRectangle layerExtent = currentItem->getDbLayer()->getLayerExtent();
        if ( QgsProject::instance()->crs().authid() != currentItem->getDbLayer()->getAuthId() )
        {
          QgsCoordinateReferenceSystem layerCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( currentItem->getDbLayer()->getAuthId() );
          QgsCoordinateTransform t( layerCrs, QgsProject::instance()->crs() );
          layerExtent = t.transformBoundingBox( layerExtent );
        }
        // layerExtent is now in the Crs of the MapCanvas
        if ( sActionName == QStringLiteral( "gotoPoint" ) )
        {
          QgsPointXY layerCenter = layerExtent.center();
          QgsRectangle canvasExtent = mapCanvas()->extent();
          QgsPointXY canvasCenter = canvasExtent.center();
          QgsPointXY canvasDiff( layerCenter.x() - canvasCenter.x(), layerCenter.y() - canvasCenter.y() );
          layerExtent = QgsRectangle( canvasExtent.xMinimum() + canvasDiff.x(), canvasExtent.yMinimum() + canvasDiff.y(),
                                      canvasExtent.xMaximum() + canvasDiff.x(), canvasExtent.yMaximum() + canvasDiff.y() );
        }
        // For both 'gotoPoint' and 'gotoExtent'
        // ‘const QgsMapCanvas’ as ‘this’ argument of ‘void QgsMapCanvas::setExtent(const QgsRectangle&, bool)’ discards qualifiers [-fpermissive]
        // mapCanvas()->setExtent( layerExtent );
      }
    }
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::~QgsSpatiaLiteSourceSelect
//-----------------------------------------------------------------
QgsSpatiaLiteSourceSelect::~QgsSpatiaLiteSourceSelect()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/SpatiaLiteSourceSelect/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/SpatiaLiteSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::on_mConnectionsComboBox_activated
// - Tab 'Connections'
//-----------------------------------------------------------------
// Remember which database is selected
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::on_mConnectionsComboBox_activated( int )
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    dbChanged();
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::onUpdateLayerStatitics
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::onUpdateLayerStatitics()
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( !mConnectionsSpatialiteDbInfoModel.isSpatialite() )
    {
      return;
    }
    QString subKey = mConnectionsSpatialiteDbInfoModel.getDbName( false ); // without path
    if ( collectConnectionsSelectedTables() > 0 )
    {
      if ( m_selectedLayers.size() == 1 )
      {
        subKey = QStringLiteral( "%1 of %2" ).arg( m_selectedLayers.at( 0 ) ).arg( subKey );
      }
      else
      {
        subKey = QStringLiteral( "%1 selected layers of %2" ).arg( m_selectedLayers.size() ).arg( subKey );
      }
    }
    QString msg = tr( "Are you sure you want to update the internal statistics for DB: %1?\n\n"
                      "This could take a long time (depending on the DB size),\n"
                      "but implies better performance thereafter." ).arg( subKey );
    QMessageBox::StandardButton result =  QMessageBox::information( this, tr( "Confirm Update Statistics" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
    if ( result != QMessageBox::Ok )
    {
      return;
    }
    if ( mConnectionsSpatialiteDbInfoModel.UpdateLayerStatistics( m_selectedLayers ) )
    {
      QMessageBox::information( this, tr( "Update Statistics" ), tr( "Internal statistics successfully updated for: %1" ).arg( subKey ) );
    }
    else
    {
      QMessageBox::critical( this, tr( "Update Statistics" ), tr( "Error while updating internal statistics for: %1" ).arg( subKey ) );
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::on_mSearchGroupBox_toggled
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::on_mSearchGroupBox_toggled( bool checked )
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( mSearchLineEdit->text().isEmpty() )
    {
      return;
    }
    on_mSearchLineEdit_textChanged( checked ? mSearchLineEdit->text() : QLatin1String( "" ) );
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::on_mSearchLineEdit_textChanged
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::on_mSearchLineEdit_textChanged( const QString &text )
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( mSearchModeComboBox->currentText() == tr( "Wildcard" ) )
    {
      mConnectionsProxyModel._setFilterWildcard( text );
    }
    else if ( mSearchModeComboBox->currentText() == tr( "RegExp" ) )
    {
      mConnectionsProxyModel._setFilterRegExp( text );
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString &text )
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( text == tr( "All" ) )
    {
      mConnectionsProxyModel.setFilterKeyColumn( -1 );
    }
    else if ( text == tr( "Table" ) )
    {
      mConnectionsProxyModel.setFilterKeyColumn( mConnectionsSpatialiteDbInfoModel.getTableNameIndex() );
    }
    else if ( text == tr( "Geometry column" ) )
    {
      mConnectionsProxyModel.setFilterKeyColumn( mConnectionsSpatialiteDbInfoModel.getGeometryNameIndex() );
    }
    else if ( text == tr( "Geometry-Type" ) )
    {
      mConnectionsProxyModel.setFilterKeyColumn( mConnectionsSpatialiteDbInfoModel.getGeometryTypeIndex() );
    }
    else if ( text == tr( "Sql" ) )
    {
      mConnectionsProxyModel.setFilterKeyColumn( mConnectionsSpatialiteDbInfoModel.getSqlQueryIndex() );
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::on_mSearchModeComboBox_currentIndexChanged
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text );
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    on_mSearchLineEdit_textChanged( mSearchLineEdit->text() );
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::populateConnectionList
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::populateConnectionList()
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    mConnectionsComboBox->clear();
    Q_FOREACH ( const QString &name, QgsSpatiaLiteConnection::connectionList() )
    {
      // retrieving the SQLite DB name and full path
      QString text = name + tr( "@" ) + QgsSpatiaLiteConnection::connectionPath( name );
      mConnectionsComboBox->addItem( text );
    }
    setConnectionListPosition();
    mConnectionsConnectButton->setDisabled( mConnectionsComboBox->count() == 0 );
    mConnectionsDeleteButton->setDisabled( mConnectionsComboBox->count() == 0 );
    mConnectionsComboBox->setDisabled( mConnectionsComboBox->count() == 0 );
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::newConnection
// - static
//-----------------------------------------------------------------
bool QgsSpatiaLiteSourceSelect::newConnection( QWidget *parent )
{
  // Retrieve last used project dir from persistent settings
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::currentPath() ).toString();
  // RasterLite1 : sometimes used the '.atlas' extension for [Vector and Rasters]
  QString fileSelect = QFileDialog::getOpenFileName( parent, tr( "Choose a SpatiaLite/SQLite DB to open" ),
                       lastUsedDir, tr( "SpatiaLite DB" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db *.atlas *.mbtiles *.gpkg);;" + tr( "All files" ) + " (*)" );
  if ( fileSelect.isEmpty() )
  {
    return false;
  }
  QFileInfo fileInfo( fileSelect );
  QString sPath = fileInfo.path();
  QString sName = fileInfo.fileName();
  QString savedName = fileInfo.fileName();
  QString baseKey = QStringLiteral( "/SpatiaLite/connections/" );
  // if there is already a connection with this name, ask for a new name
  while ( ! settings.value( baseKey + savedName + "/sqlitepath", "" ).toString().isEmpty() )
  {
    bool ok;
    savedName = QInputDialog::getText( nullptr, tr( "Cannot add connection '%1'" ).arg( sName ),
                                       tr( "A connection with the same name already exists,\nplease provide a new name:" ), QLineEdit::Normal,  QLatin1String( "" ), &ok );
    if ( !ok || savedName.isEmpty() )
    {
      return false;
    }
  }
  // Persist last used SpatiaLite dir
  settings.setValue( QStringLiteral( "UI/lastSpatiaLiteDir" ), sPath );
  // inserting this SQLite DB path
  settings.setValue( baseKey + "selected", savedName );
  settings.setValue( baseKey + savedName + "/sqlitepath", fileInfo.canonicalFilePath() );
  return true;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::layerUriSql
// - Tab 'Connections'
//-----------------------------------------------------------------
QString QgsSpatiaLiteSourceSelect::layerUriSql( const QModelIndex &index )
{
  QString sql = QString();
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    QString tableName = mConnectionsSpatialiteDbInfoModel.getTableName( index );
    QString geomColumnName = mConnectionsSpatialiteDbInfoModel.getGeometryName( index );
    QString sql = mConnectionsSpatialiteDbInfoModel.getSqlQuery( index );

    if ( geomColumnName.contains( QLatin1String( " AS " ) ) )
    {
      int a = geomColumnName.indexOf( QLatin1String( " AS " ) );
      QString typeName = geomColumnName.mid( a + 4 ); //only the type name
      geomColumnName = geomColumnName.left( a ); //only the geom column name
      QString geomFilter;

      if ( typeName == QLatin1String( "POINT" ) )
      {
        geomFilter = QStringLiteral( "geometrytype(\"%1\") IN ('POINT','MULTIPOINT')" ).arg( geomColumnName );
      }
      else if ( typeName == QLatin1String( "LINESTRING" ) )
      {
        geomFilter = QStringLiteral( "geometrytype(\"%1\") IN ('LINESTRING','MULTILINESTRING')" ).arg( geomColumnName );
      }
      else if ( typeName == QLatin1String( "POLYGON" ) )
      {
        geomFilter = QStringLiteral( "geometrytype(\"%1\") IN ('POLYGON','MULTIPOLYGON')" ).arg( geomColumnName );
      }
      if ( !geomFilter.isEmpty() && !sql.contains( geomFilter ) )
      {
        if ( !sql.isEmpty() )
        {
          sql += QLatin1String( " AND " );
        }
        sql += geomFilter;
      }
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
  return sql;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::connectionDelete
// - Tab 'Connections'
//-----------------------------------------------------------------
// Slot for deleting an existing connection
//-----------------------------------------------------------------

void QgsSpatiaLiteSourceSelect::connectionDelete( QString connectionString )
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    QString subKey = connectionString;
    int idx = subKey.indexOf( '@' );
    if ( idx > 0 )
    {
      subKey.truncate( idx );
    }
    QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" ).arg( subKey );
    QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
    if ( result != QMessageBox::Ok )
    {
      return;
    }
    QgsSpatiaLiteConnection::deleteConnection( subKey );
    populateConnectionList();
    emit connectionsChanged();
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::collectConnectionsSelectedTables
// - Tab 'Connections'
//-----------------------------------------------------------------
// Note: this may be removed in the future, since 'Layer Order'
// - is always filled upon a single section
// - and this fuction has no 'select Style' option
// Still used in 'onUpdateLayerStatitics'
//-----------------------------------------------------------------
int QgsSpatiaLiteSourceSelect::collectConnectionsSelectedTables()
{
  int iSelectedCount = 0;
#if 0
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
#endif
    m_selectedLayers.clear();
    m_selectedLayersStyles.clear();
    m_selectedLayersSql.clear();
    typedef QMap < int, bool >schemaInfo;
    QMap < QString, schemaInfo > dbInfo;
    QItemSelection selection = mConnectionsTreeView->selectionModel()->selection();
    QModelIndexList selectedIndices = selection.indexes();
    QgsSpatialiteDbInfoItem *currentItem = nullptr;
    QModelIndexList::const_iterator selected_it = selectedIndices.constBegin();
    for ( ; selected_it != selectedIndices.constEnd(); ++selected_it )
    {
      if ( !selected_it->parent().isValid() )
      {
        //top level items only contain the schema names
        continue;
      }
      currentItem = mConnectionsSpatialiteDbInfoModel.getItem( mConnectionsProxyModel.mapToSource( *selected_it ) );
      if ( ( !currentItem ) || ( !currentItem->isValid() ) || ( !currentItem->isLayerSelectable() ) )
      {
        continue;
      }

      QString currentSchemaName;
      currentSchemaName = currentItem->getLayerTypeString(); // ItemTypeLayer [isLayerSelectable]
      int currentRow = currentItem->row();
      if ( !dbInfo[currentSchemaName].contains( currentRow ) )
      {
        dbInfo[currentSchemaName][currentRow] = true;
        m_selectedLayers.append( mConnectionsSpatialiteDbInfoModel.getLayerName( mConnectionsProxyModel.mapToSource( *selected_it ) ) );
        QgsDebugMsgLevel( QStringLiteral( "QgsSpatiaLiteSourceSelect::collectConnectionsSelectedTables row[%4] getItem[%1] m_selectedLayers[%2] m_selectedLayersSql[%3]" ).arg( currentItem->getItemTypeString() ).arg( mConnectionsSpatialiteDbInfoModel.getLayerName( mConnectionsProxyModel.mapToSource( *selected_it ) ) ).arg( layerUriSql( mConnectionsProxyModel.mapToSource( *selected_it ) ) ).arg( currentRow ), 7 );
        m_selectedLayersSql  << layerUriSql( mConnectionsProxyModel.mapToSource( *selected_it ) );
      }
    }
    iSelectedCount = m_selectedLayers.count();
#if 0
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
#endif
  return iSelectedCount;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::addSelectedDbLayers
// - Tab 'Connections'
// - Tab 'LayersOrder'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::addSelectedDbLayers()
{
  if ( ( mLayerOrderRootItem ) && ( mLayerOrderRootItem->childCount() > 0 ) )
  {
    mLayerOrderRootItem->addSelectedDbLayers( m_selectedLayers, m_selectedLayersStyles, m_selectedLayersSql );
  }
  else
  {
    // This should never happen
    collectConnectionsSelectedTables();
  }
  if ( m_selectedLayers.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a Layer." ) );
    showStatusMessage( QStringLiteral( "You must select a table in order to add a Layer." ), 0 );
  }
  else
  {
    showStatusMessage( QStringLiteral( "%1 Layers will be added to the QGis-MapCanvas" ).arg( m_selectedLayers.count() ), 0 );
    addConnectionsDbMapLayers();
    if ( mWidgetMode == QgsProviderRegistry::WidgetMode::None && ! mHoldDialogOpen->isChecked() )
    {
      accept();
    }
  }

}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::addConnectionDatabaseSource
// - called from onSourceSelectButtons_clicked
// - sets mSqlitePath
// - calls addConnectionsDatabaseSource
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::addConnectionDatabaseSource( QString const &path, QString const &providerKey )
{
  Q_UNUSED( providerKey );
  QgsSpatiaLiteConnection connectionInfo( path );
  mSqlitePath = connectionInfo.dbPath();
  addConnectionsDatabaseSource( QStringList( mSqlitePath ) );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::addConnectionsDatabaseSource
// - called from addConnectionDatabaseSource
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::addConnectionsDatabaseSource( QStringList const &paths, QString const &providerKey )
{
  Q_UNUSED( providerKey );
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    QString fileName = paths.at( 0 );
    bool bLoadLayers = true;
    bool bShared = true;
    // Since all layer details will be needed, 'bLoadLayers = true' should be used [noticeably faster].
    QgsSpatialiteDbInfo *spatialiteDbInfo = QgsSpatiaLiteUtils::CreateQgsSpatialiteDbInfo( fileName, bLoadLayers, bShared );
    QApplication::restoreOverrideCursor();
    if ( !spatialiteDbInfo )
    {
      if ( !QFile::exists( mSqlitePath ) )
      {
        QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ), tr( "Database does not exist: %1" ).arg( mSqlitePath ) );
      }
      return;
    }
    else
    {
      if ( !spatialiteDbInfo->isDbValid() )
      {
        if ( spatialiteDbInfo->isDbLocked() )
        {
          QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ), tr( "The read Sqlite3 Container [%2] is locked: [%1]" ).arg( mSqlitePath ).arg( spatialiteDbInfo->dbSpatialMetadataString() ) );
        }
        else
        {
          QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ), tr( "The read Sqlite3 Container [%2] is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider: [%1]" ).arg( mSqlitePath ).arg( spatialiteDbInfo->dbSpatialMetadataString() ) );
        }
        return;
      }
      if ( fileName != mSqlitePath )
      {
        mSqlitePath = fileName;
      }
      // populate the table list
      // get the list of suitable tables and columns and populate the UI
      setSpatialiteDbInfo( spatialiteDbInfo );
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::setSpatialiteDbInfo
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::setSpatialiteDbInfo( QgsSpatialiteDbInfo *spatialiteDbInfo, bool setConnectionInfo )
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbValid() ) )
    {
      mConnectionsSpatialiteDbInfoModel.setSpatialiteDbInfo( spatialiteDbInfo );
      mConnectionsTreeView->setColumnHidden( mConnectionsSpatialiteDbInfoModel.getColumnSortHidden(), true );
      mConnectionsTreeView->sortByColumn( mConnectionsSpatialiteDbInfoModel.getColumnSortHidden(), Qt::AscendingOrder );
      mConnectionsTreeView->expandToDepth( mConnectionsSpatialiteDbInfoModel.getExpandToDepth() );
      if ( setConnectionInfo )
      {
        mSqlitePath = spatialiteDbInfo->getDatabaseFileName();
        // TODO: add to comboBox, if not there already
      }
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::setLayerOrderData
// - Tab 'Connections'
// - Tab 'Layer Order'
//-----------------------------------------------------------------
int QgsSpatiaLiteSourceSelect::setLayerOrderData( bool bRemoveItems )
{
  int iCountItems = 0;
  if ( ( getCurrentTabIndex() == getTabConnectionsIndex() ) || ( getCurrentTabIndex() == getTabLayerOrderIndex() ) )
  {
    if ( mLayerOrderRootItem )
    {
      iCountItems = mLayerOrderSpatialiteDbInfoModel.setLayerOrderData( bRemoveItems );
      mTabWidget->setTabEnabled( getTabLayerOrderIndex(), mLayerOrderRootItem->childCount() > 0 );
      // Even if nothing has changed (iRc== 0), expandToDepth still must be called
      // mLayerOrderTreeView->setColumnHidden( mLayerOrderSpatialiteDbInfoModel.getColumnSortHidden(), true );
      // mLayerOrderTreeView->sortByColumn( mLayerOrderSpatialiteDbInfoModel.getColumnSortHidden(), Qt::AscendingOrder );
      mLayerOrderTreeView->expandToDepth( mLayerOrderSpatialiteDbInfoModel.getExpandToDepth() );
      if ( iCountItems  > 0 )
      {
      }
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
  return iCountItems;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::setLayerSqlQuery
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::setLayerSqlQuery( const QModelIndex &index )
{
  QModelIndex item_index;
  QString tableName;
  QString sLayerNameUris;
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    item_index = mConnectionsProxyModel.mapToSource( index );
    tableName = mConnectionsSpatialiteDbInfoModel.getTableName( item_index );
    sLayerNameUris = mConnectionsSpatialiteDbInfoModel.getLayerNameUris( item_index );
    if ( sLayerNameUris.isEmpty() )
    {
      return;
    }
    QgsVectorLayer *vlayer = new QgsVectorLayer( sLayerNameUris, tableName, mSpatialiteProviderKey );
    if ( !vlayer->isValid() )
    {
      delete vlayer;
      return;
    }
    // create a query builder object
    QgsQueryBuilder *gb = new QgsQueryBuilder( vlayer, this );
    if ( gb->exec() )
    {
      mConnectionsSpatialiteDbInfoModel.setLayerSqlQuery( mConnectionsProxyModel.mapToSource( index ), gb->sql() );
    }
    delete gb;
    delete vlayer;
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::fullDescription
//-----------------------------------------------------------------
QString QgsSpatiaLiteSourceSelect::fullDescription( const QString &table, const QString &column, const QString &type )
{
  QString full_desc = QLatin1String( "" );
  full_desc += table + "\" (" + column + ") " + type;
  return full_desc;
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::dbChanged
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::dbChanged()
{
  // Remember which database was selected.
  QgsSettings settings;
  settings.setValue( QStringLiteral( "SpatiaLite/connections/selected" ), mConnectionsComboBox->currentText() );
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::setConnectionListPosition
// - Tab 'Connections'
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::setConnectionListPosition()
{
  if ( getCurrentTabIndex() == getTabConnectionsIndex() )
  {
    QgsSettings settings;
    // If possible, set the item currently displayed database
    QString toSelect = settings.value( QStringLiteral( "SpatiaLite/connections/selected" ) ).toString();

    toSelect += '@' + settings.value( "/SpatiaLite/connections/" + toSelect + "/sqlitepath" ).toString();

    mConnectionsComboBox->setCurrentIndex( mConnectionsComboBox->findText( toSelect ) );

    if ( mConnectionsComboBox->currentIndex() < 0 )
    {
      if ( toSelect.isNull() )
        mConnectionsComboBox->setCurrentIndex( 0 );
      else
        mConnectionsComboBox->setCurrentIndex( mConnectionsComboBox->count() - 1 );
    }
  }
  else if ( getCurrentTabIndex() == getTabLayerOrderIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabLayerTilesetsIndex() )
  {
  }
  else if ( getCurrentTabIndex() == getTabServerSearchIndex() )
  {
  }
}
//-----------------------------------------------------------------
// QgsSpatiaLiteSourceSelect::setSearchExpression
//-----------------------------------------------------------------
void QgsSpatiaLiteSourceSelect::setSearchExpression( const QString &regexp )
{
  Q_UNUSED( regexp );
}

