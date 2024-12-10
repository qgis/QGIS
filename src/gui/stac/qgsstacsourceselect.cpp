/***************************************************************************
    qgsstacsourceselect.cpp
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacsourceselect.h"
#include "moc_qgsstacsourceselect.cpp"
#include "qgsdatasourcemanagerdialog.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsstaccontroller.h"
#include "qgsstaccatalog.h"
#include "qgsstaccollection.h"
#include "qgsstaccollections.h"
#include "qgsstacconnection.h"
#include "qgsstacconnectiondialog.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgshelp.h"
#include "qgsstacdownloadassetsdialog.h"
#include "qgsstacitem.h"
#include "qgsstacitemcollection.h"
#include "qgsstacsearchparametersdialog.h"
#include "qgsstacobjectdetailsdialog.h"
#include "qgsstacitemlistmodel.h"

#include <QScrollBar>
#include <QUrlQuery>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>

///@cond PRIVATE

QgsStacSourceSelect::QgsStacSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
  , mStac( new QgsStacController )
  , mItemsModel( new QgsStacItemListModel( this ) )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Add Layer from STAC Catalog" ) );

  connect( btnConnect, &QToolButton::clicked, this, &QgsStacSourceSelect::btnConnect_clicked );
  connect( btnNew, &QToolButton::clicked, this, &QgsStacSourceSelect::btnNew_clicked );
  connect( btnEdit, &QToolButton::clicked, this, &QgsStacSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QToolButton::clicked, this, &QgsStacSourceSelect::btnDelete_clicked );
  connect( btnSave, &QToolButton::clicked, this, &QgsStacSourceSelect::btnSave_clicked );
  connect( btnLoad, &QToolButton::clicked, this, &QgsStacSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsStacSourceSelect::cmbConnections_currentTextChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsStacSourceSelect::showHelp );
  setupButtons( buttonBox );

  populateConnectionList();

  connect( mStac, &QgsStacController::finishedStacObjectRequest, this, &QgsStacSourceSelect::onStacObjectRequestFinished );
  connect( mStac, &QgsStacController::finishedItemCollectionRequest, this, &QgsStacSourceSelect::onItemCollectionRequestFinished );
  connect( mStac, &QgsStacController::finishedCollectionsRequest, this, &QgsStacSourceSelect::onCollectionsRequestFinished );

  connect( mFiltersButton, &QToolButton::clicked, this, &QgsStacSourceSelect::openSearchParametersDialog );

  mItemsView->setModel( mItemsModel );
  mItemsView->setItemDelegate( new QgsStacItemDelegate );
  mItemsView->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  mItemsView->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( mItemsView, &QListView::customContextMenuRequested, this, &QgsStacSourceSelect::showItemsContextMenu );
  connect( mItemsView, &QListView::doubleClicked, this, &QgsStacSourceSelect::onItemDoubleClicked );
  connect( mItemsView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsStacSourceSelect::onCurrentItemChanged );
  connect( mItemsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &QgsStacSourceSelect::onItemsViewScroll );

  connect( mFootprintsCheckBox, &QCheckBox::clicked, this, &QgsStacSourceSelect::showFootprints );

  mParametersDialog = new QgsStacSearchParametersDialog( mapCanvas(), this );
  mFiltersLabel->clear();
}

QgsStacSourceSelect::~QgsStacSourceSelect()
{
  delete mStac;
}

void QgsStacSourceSelect::hideEvent( QHideEvent *e )
{
  if ( !e->spontaneous() )
  {
    showFootprints( false );
  }
  QgsAbstractDataSourceWidget::hideEvent( e );
}

void QgsStacSourceSelect::showEvent( QShowEvent *e )
{
  if ( !e->spontaneous() && mFootprintsCheckBox->isChecked() )
  {
    showFootprints( true );
  }
  QgsAbstractDataSourceWidget::showEvent( e );
}

void QgsStacSourceSelect::addButtonClicked()
{
  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );
  const QItemSelection selection = mItemsView->selectionModel()->selection();
  const QModelIndexList selectedIndices = selection.indexes();

  QgsMimeDataUtils::UriList allUris;
  for ( QModelIndexList::const_iterator it = selectedIndices.constBegin(); it != selectedIndices.constEnd(); ++it )
  {
    QgsMimeDataUtils::UriList uris = it->data( QgsStacItemListModel::Role::Uris ).value<QgsMimeDataUtils::UriList>();
    allUris.append( uris );
  }

  for ( auto &uri : std::as_const( allUris ) )
  {
    loadUri( uri );
  }
}

void QgsStacSourceSelect::onItemsViewScroll( int value )
{
  if ( !mNextPageUrl.isEmpty() && value == mItemsView->verticalScrollBar()->maximum() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Scrolled to end, fetching next page" ), 3 );
    fetchNextResultPage();
  }
}

void QgsStacSourceSelect::onItemDoubleClicked( const QModelIndex &index )
{
  QgsStacObjectDetailsDialog details( this );
  details.setStacObject( index.data( QgsStacItemListModel::Role::StacObject ).value<QgsStacObject *>() );
  details.exec();
}

void QgsStacSourceSelect::onCurrentItemChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )

  if ( mFootprintsCheckBox->isChecked() )
    highlightFootprint( current );

  const QVariant mediaTypes = current.data( QgsStacItemListModel::Role::MediaTypes );
  emit enableButtons( !mediaTypes.toStringList().isEmpty() );
}

void QgsStacSourceSelect::btnConnect_clicked()
{
  // mapCanvas() was nullptr during construction
  mParametersDialog->setMapCanvas( mapCanvas() );

  const QgsStacConnection::Data connection = QgsStacConnection::connection( cmbConnections->currentText() );

  mStac->setAuthCfg( connection.authCfg );
  mCollectionsUrl.clear();
  mSearchUrl.clear();
  mNextPageUrl.clear();
  mItemsModel->clear();
  qDeleteAll( mRubberBands );
  mRubberBands.clear();
  mStatusLabel->setText( tr( "Connecting…" ) );
  mStac->cancelPendingAsyncRequests();
  mStac->fetchStacObjectAsync( connection.url );
  mFiltersLabel->clear();
  mFiltersButton->setEnabled( false );
}

void QgsStacSourceSelect::btnNew_clicked()
{
  QgsStacConnectionDialog nc( this );
  if ( nc.exec() )
  {
    QgsStacConnection::addConnection( nc.connectionName(), QgsStacConnection::decodedUri( nc.connectionUri() ) );
    populateConnectionList();
    QgsStacConnection::setSelectedConnection( nc.connectionName() );
    setConnectionListPosition();
    emit connectionsChanged();
  }
}


void QgsStacSourceSelect::btnEdit_clicked()
{
  const QgsStacConnection::Data connection = QgsStacConnection::connection( cmbConnections->currentText() );
  const QString uri = QgsStacConnection::encodedUri( connection );

  QgsStacConnectionDialog nc( this );
  nc.setConnection( cmbConnections->currentText(), uri );
  if ( nc.exec() )
  {
    const QgsStacConnection::Data connectionData = QgsStacConnection::decodedUri( nc.connectionUri() );
    QgsStacConnection::addConnection( nc.connectionName(), connectionData );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsStacSourceSelect::btnDelete_clicked()
{
  const QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                        .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsStacConnection( QString() ).remove( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsStacSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::STAC );
  dlg.exec();
}

void QgsStacSourceSelect::btnLoad_clicked()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::STAC, fileName );
  dlg.exec();
  populateConnectionList();
}

void QgsStacSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsStacConnection::connectionList() );
  cmbConnections->blockSignals( false );

  btnConnect->setDisabled( cmbConnections->count() == 0 );
  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnSave->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  setConnectionListPosition();
}

void QgsStacSourceSelect::setConnectionListPosition()
{
  const QString toSelect = QgsStacConnection::selectedConnection();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsStacSourceSelect::cmbConnections_currentTextChanged( const QString &text )
{
  QgsStacConnection::setSelectedConnection( text );
}

void QgsStacSourceSelect::onStacObjectRequestFinished( int requestId, QString error )
{
  QgsDebugMsgLevel( QStringLiteral( "Finished object request %1" ).arg( requestId ), 2 );
  QgsStacObject *obj = mStac->takeStacObject( requestId );
  QgsStacCatalog *cat = dynamic_cast<QgsStacCatalog *>( obj );

  if ( !cat )
  {
    mStatusLabel->setText( error );
    return;
  }
  QgsDebugMsgLevel( QStringLiteral( "STAC catalog supports API: %1" ).arg( cat->supportsStacApi() ), 2 );

  for ( auto &l : cat->links() )
  {
    // collections endpoint should have a "data" relation according to spec but some servers don't
    // so let's be less strict and only check the href
    if ( l.href().endsWith( "/collections" ) )
      mCollectionsUrl = l.href();
    else if ( l.relation() == "search" )
      mSearchUrl = l.href();
  }

  if ( mCollectionsUrl.isEmpty() || mSearchUrl.isEmpty() )
  {
    mStatusLabel->setText( tr( "Server does not support STAC search API" ) );
  }
  else
  {
    mStatusLabel->setText( tr( "Fetching Collections…" ) );
    mStac->cancelPendingAsyncRequests();
    mStac->fetchCollectionsAsync( mCollectionsUrl );
  }
}

void QgsStacSourceSelect::onCollectionsRequestFinished( int requestId, QString error )
{
  QgsDebugMsgLevel( QStringLiteral( "Finished collections request %1" ).arg( requestId ), 2 );
  QgsStacCollections *cols = mStac->takeCollections( requestId );

  if ( !cols )
  {
    mStatusLabel->setText( error );
    mParametersDialog->setCollections( {} );
    updateFilterPreview();
    return;
  }

  const QVector<QgsStacCollection *> vcols = cols->takeCollections();
  mParametersDialog->setCollections( vcols );
  mItemsModel->setCollections( vcols );
  mStatusLabel->setText( tr( "Searching…" ) );
  mFiltersButton->setEnabled( true );
  updateFilterPreview();
  search();
}

void QgsStacSourceSelect::onItemCollectionRequestFinished( int requestId, QString error )
{
  QgsDebugMsgLevel( QStringLiteral( "Finished item collection request %1" ).arg( requestId ), 2 );
  QgsStacItemCollection *col = mStac->takeItemCollection( requestId );

  if ( !col )
  {
    mStatusLabel->setText( error );
    return;
  }

  mNextPageUrl = col->nextUrl();

  const QVector<QgsStacItem *> items = col->takeItems();
  mItemsModel->addItems( items );

  for ( QgsStacItem *i : items )
  {
    QgsRubberBand *band = new QgsRubberBand( mapCanvas(), Qgis::GeometryType::Polygon );
    band->setToGeometry( i->geometry(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
    mRubberBands.append( band );
  }

  const int count = mItemsModel->rowCount();
  if ( mNextPageUrl.isEmpty() )
  {
    mStatusLabel->setText( tr( "Showing: %1/%1 total" ).arg( count ) );
  }
  else
  {
    // Suppress warning: Potential leak of memory in qtimer.h [clang-analyzer-cplusplus.NewDeleteLeaks]
#ifndef __clang_analyzer__
    // Let the results appear, then fetch more if there's no scrollbar
    QTimer::singleShot( 100, this, [=] {
      if ( !mItemsView->verticalScrollBar()->isVisible() )
      {
        fetchNextResultPage();
      }
    } );
#endif
    if ( col->numberMatched() > 0 )
    {
      mStatusLabel->setText( tr( "Showing: %1/%2 total" ).arg( count ).arg( col->numberMatched() ) );
    }
    else
    {
      mStatusLabel->setText( tr( "Showing: %1/%2 total" ).arg( count ).arg( tr( "unknown" ) ) );
    }
  }
}

void QgsStacSourceSelect::search()
{
  QStringList collections;
  if ( mParametersDialog->hasCollectionsFilter() )
  {
    const QSet<QString> collectionsSet = mParametersDialog->selectedCollections();
    collections = QStringList( collectionsSet.constBegin(), collectionsSet.constEnd() );
  }
  else
  {
    const QVector<QgsStacCollection *> allCollections = mParametersDialog->collections();
    for ( QgsStacCollection *col : allCollections )
    {
      collections.append( col->id() );
    }
  }

  QUrlQuery q;

  QList<QPair<QString, QString>> collectionsParameters = { qMakePair( QStringLiteral( "collections" ), collections.join( "," ) ) };
  q.setQueryItems( collectionsParameters );

  if ( mParametersDialog->hasSpatialFilter() )
  {
    const QgsGeometry geom = mParametersDialog->spatialExtent();
    const QgsRectangle bbox = geom.boundingBox();
    if ( bbox == QgsGeometry::fromRect( bbox ).boundingBox() )
    {
      q.addQueryItem( QStringLiteral( "bbox" ), QStringLiteral( "%1,%2,%3,%4" ).arg( bbox.xMinimum() ).arg( bbox.yMinimum() ).arg( bbox.xMaximum() ).arg( bbox.yMaximum() ) );
    }
    else
    {
      q.addQueryItem( QStringLiteral( "intersects" ), geom.asJson() );
    }
  }

  if ( mParametersDialog->hasTemporalFilter() )
  {
    const QgsDateTimeRange dateRange = mParametersDialog->temporalRange();
    const QDateTime fromDate = dateRange.begin();
    const QDateTime toDate = dateRange.end();
    QString dateString;

    if ( fromDate == toDate )
    {
      dateString = fromDate.toString( Qt::ISODateWithMs );
    }
    else
    {
      dateString = QStringLiteral( "%1/%2" ).arg(
        fromDate.isNull() ? QStringLiteral( ".." ) : fromDate.toString( Qt::ISODateWithMs ),
        toDate.isNull() ? QStringLiteral( ".." ) : toDate.toString( Qt::ISODateWithMs )
      );
    }
    q.addQueryItem( QStringLiteral( "datetime" ), dateString );
  }

  QUrl searchUrl( mSearchUrl );
  searchUrl.setQuery( q );
  mStac->cancelPendingAsyncRequests();
  mStac->fetchItemCollectionAsync( searchUrl );
}

void QgsStacSourceSelect::openSearchParametersDialog()
{
  if ( mParametersDialog->exec() == QDialog::Rejected )
    return;

  mItemsModel->clear();
  qDeleteAll( mRubberBands );
  mRubberBands.clear();
  mItemsModel->setCollections( mParametersDialog->collections() );
  mNextPageUrl.clear();
  mStatusLabel->setText( tr( "Searching…" ) );
  updateFilterPreview();
  search();
}

void QgsStacSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html" ) );
}

void QgsStacSourceSelect::updateFilterPreview()
{
  mFiltersLabel->setText( mParametersDialog->activeFiltersPreview() );
}

void QgsStacSourceSelect::fetchNextResultPage()
{
  if ( mNextPageUrl.isEmpty() )
    return;

  mStatusLabel->setText( tr( "Searching…" ) );
  mStac->cancelPendingAsyncRequests();
  mStac->fetchItemCollectionAsync( mNextPageUrl );
  mNextPageUrl.clear();
}

void QgsStacSourceSelect::showItemsContextMenu( QPoint point )
{
  if ( mItemsModel->rowCount() == 0 )
    return;

  const QModelIndex index = mItemsView->indexAt( point );

  QMenu *menu = new QMenu( this );

  QgsMessageBar *bar = nullptr;
  if ( QgsDataSourceManagerDialog *dsm = qobject_cast<QgsDataSourceManagerDialog *>( window() ) )
    bar = dsm->messageBar();

  QMenu *assetsMenu = menu->addMenu( tr( "Add Layer" ) );
  if ( const QgsStacItem *item = dynamic_cast<QgsStacItem *>( index.data( QgsStacItemListModel::Role::StacObject ).value<QgsStacObject *>() ) )
  {
    const QMap<QString, QgsStacAsset> assets = item->assets();
    for ( const QgsStacAsset &asset : assets )
    {
      if ( asset.isCloudOptimized() )
      {
        QAction *loadAssetAction = new QAction( asset.title(), assetsMenu );
        connect( loadAssetAction, &QAction::triggered, this, [this, uri = asset.uri()] {
          QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );
          loadUri( uri );
        } );
        assetsMenu->addAction( loadAssetAction );
      }
    }
  }

  QAction *zoomToAction = new QAction( tr( "Zoom to Item" ), menu );
  connect( zoomToAction, &QAction::triggered, this, [index, this] {
    QgsGeometry geom = index.data( QgsStacItemListModel::Role::Geometry ).value<QgsGeometry>();
    if ( QgsMapCanvas *map = mapCanvas() )
    {
      const QgsRectangle bbox = geom.boundingBox();
      const QgsCoordinateTransform ct( QgsCoordinateReferenceSystem::fromEpsgId( 4324 ), map->mapSettings().destinationCrs(), QgsProject::instance() );
      QgsRectangle extent = ct.transformBoundingBox( bbox );
      map->zoomToFeatureExtent( extent );
    }
  } );

  QAction *panToAction = new QAction( tr( "Pan to Item" ), menu );
  connect( panToAction, &QAction::triggered, this, [index, this] {
    QgsGeometry geom = index.data( QgsStacItemListModel::Role::Geometry ).value<QgsGeometry>();
    if ( QgsMapCanvas *map = mapCanvas() )
    {
      const QgsRectangle bbox = geom.boundingBox();
      const QgsCoordinateTransform ct( QgsCoordinateReferenceSystem::fromEpsgId( 4324 ), map->mapSettings().destinationCrs(), QgsProject::instance() );
      const QgsRectangle extent = ct.transformBoundingBox( bbox );
      map->setCenter( extent.center() );
    }
  } );

  QAction *downloadAction = new QAction( tr( "Download Assets…" ), menu );
  connect( downloadAction, &QAction::triggered, this, [index, bar, authCfg = mStac->authCfg()] {
    QgsStacDownloadAssetsDialog dialog;
    QgsStacItem *item = dynamic_cast<QgsStacItem *>( index.data( QgsStacItemListModel::Role::StacObject ).value<QgsStacObject *>() );
    dialog.setStacItem( item );
    dialog.setMessageBar( bar );
    dialog.setAuthCfg( authCfg );
    dialog.exec();
  } );

  QAction *detailsAction = new QAction( tr( "Details…" ), menu );
  connect( detailsAction, &QAction::triggered, this, [this, index] {
    QgsStacObjectDetailsDialog details( this );
    details.setStacObject( index.data( QgsStacItemListModel::Role::StacObject ).value<QgsStacObject *>() );
    details.exec();
  } );


  menu->addAction( zoomToAction );
  menu->addAction( panToAction );
  if ( !assetsMenu->isEmpty() )
    menu->addMenu( assetsMenu );
  menu->addAction( downloadAction );
  menu->addAction( detailsAction );

  menu->popup( mItemsView->mapToGlobal( point ) );
  connect( menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater );
}

void QgsStacSourceSelect::highlightFootprint( const QModelIndex &index )
{
  QgsGeometry geom = index.data( QgsStacItemListModel::Role::Geometry ).value<QgsGeometry>();
  if ( QgsMapCanvas *map = mapCanvas() )
  {
    mCurrentItemBand.reset( new QgsRubberBand( map, Qgis::GeometryType::Polygon ) );
    mCurrentItemBand->setFillColor( QColor::fromRgb( 255, 0, 0, 128 ) );
    mCurrentItemBand->setToGeometry( geom, QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
  }
}

void QgsStacSourceSelect::showFootprints( bool enable )
{
  if ( enable )
  {
    const QVector<QgsStacItem *> items = mItemsModel->items();
    for ( QgsStacItem *i : items )
    {
      QgsRubberBand *band = new QgsRubberBand( mapCanvas(), Qgis::GeometryType::Polygon );
      band->setToGeometry( i->geometry(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
      mRubberBands.append( band );
    }
    const QModelIndex index = mItemsView->selectionModel()->currentIndex();
    if ( index.isValid() )
    {
      highlightFootprint( index );
    }
  }
  else
  {
    qDeleteAll( mRubberBands );
    mRubberBands.clear();
    mCurrentItemBand.reset();
  }
}

void QgsStacSourceSelect::loadUri( const QgsMimeDataUtils::Uri &uri )
{
  if ( uri.layerType == QLatin1String( "raster" ) )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit addRasterLayer( uri.uri, uri.name, uri.providerKey );
    Q_NOWARN_DEPRECATED_POP
    emit addLayer( Qgis::LayerType::Raster, uri.uri, uri.name, uri.providerKey );
  }
  else if ( uri.layerType == QLatin1String( "pointcloud" ) )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit addPointCloudLayer( uri.uri, uri.name, uri.providerKey );
    Q_NOWARN_DEPRECATED_POP
    emit addLayer( Qgis::LayerType::PointCloud, uri.uri, uri.name, uri.providerKey );
  }
}
///@endcond
