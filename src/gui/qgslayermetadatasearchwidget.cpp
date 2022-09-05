/***************************************************************************
  qgslayermetadatasearchwidget.cpp - QgsLayerMetadataSearchWidget

 ---------------------
 begin                : 1.9.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayermetadatasearchwidget.h"
#include "qgslayermetadataresultsmodel.h"
#include "qgslayermetadataresultsproxymodel.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsprojectviewsettings.h"

QgsLayerMetadataSearchWidget::QgsLayerMetadataSearchWidget( const QgsMapCanvas *mapCanvas, QWidget *parent )
  : QWidget( parent )
  , mMapCanvas( mapCanvas )
{
  setupUi( this );

  QgsMetadataSearchContext searchContext;
  searchContext.transformContext = QgsProject::instance()->transformContext();

  QgsLayerMetadataResultsModel *sourceModel = new QgsLayerMetadataResultsModel( searchContext, this );
  mProxyModel = new QgsLayerMetadataResultsProxyModel( this );
  mProxyModel->setSourceModel( sourceModel );
  mMetadataTableView->setModel( mProxyModel );
  mMetadataTableView->setSortingEnabled( true );
  mMetadataTableView->horizontalHeader()->setSectionResizeMode( QgsLayerMetadataResultsModel::Sections::Identifier, QHeaderView::ResizeMode::Stretch );
  mMetadataTableView->horizontalHeader()->setSectionResizeMode( QgsLayerMetadataResultsModel::Sections::Title, QHeaderView::ResizeMode::Stretch );
  mMetadataTableView->horizontalHeader()->setSectionResizeMode( QgsLayerMetadataResultsModel::Sections::Abstract, QHeaderView::ResizeMode::Stretch );
  mMetadataTableView->horizontalHeader()->setSectionResizeMode( QgsLayerMetadataResultsModel::Sections::DataProviderName, QHeaderView::ResizeMode::ResizeToContents );
  mMetadataTableView->horizontalHeader()->setSectionResizeMode( QgsLayerMetadataResultsModel::Sections::GeometryType, QHeaderView::ResizeMode::ResizeToContents );
  mMetadataTableView->setSelectionBehavior( QAbstractItemView::SelectRows );

  mExtentFilterComboBox->addItem( QString( ) );
  mExtentFilterComboBox->addItem( QStringLiteral( "Map Canvas Extent" ) );
  mExtentFilterComboBox->addItem( QStringLiteral( "Current Project Extent" ) );
  mExtentFilterComboBox->setCurrentIndex( 0 );

  auto updateLoadBtn = [ = ]
  {
    if ( mIsLoading )
    {
      mAbortPushButton->setText( tr( "Abort" ) );
      mAbortPushButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mTaskCancel.svg" ) ) );
    }
    else
    {
      mAbortPushButton->setText( tr( "Refresh" ) );
      mAbortPushButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionRefresh.svg" ) ) );
    }
  };

  connect( sourceModel, &QgsLayerMetadataResultsModel::progressChanged, mProgressBar, &QProgressBar::setValue );
  connect( sourceModel, &QgsLayerMetadataResultsModel::progressChanged, this,  [ = ]( int progress )
  {
    if ( progress == 100 )
    {
      mIsLoading = false;
      updateLoadBtn();
    }
  } );

  connect( mAbortPushButton, &QPushButton::clicked, sourceModel, [ = ]( bool )
  {
    if ( ! mIsLoading )
    {
      mIsLoading = true;
      sourceModel->reloadAsync( );
    }
    else
    {
      sourceModel->cancel();
      mIsLoading = false;
    }
    updateLoadBtn();
  } );


  // Setup buttons
  mButtonBox->setStandardButtons( QDialogButtonBox::Apply | QDialogButtonBox::Close | QDialogButtonBox::Help );
#ifdef Q_OS_MACX
  mButtonBox->setStyleSheet( "* { button-layout: 2 }" );
#endif
  mAddButton = mButtonBox->button( QDialogButtonBox::Apply );
  mAddButton->setText( tr( "&Add" ) );
  mAddButton->setToolTip( tr( "Add selected layers to map" ) );
  mAddButton->setEnabled( false );
  connect( mAddButton, &QPushButton::clicked, this, [ = ]( bool )
  {
    const QModelIndexList &selectedIndexes { mMetadataTableView->selectionModel()->selectedRows() };
    if ( ! selectedIndexes.isEmpty() )
    {
      QList< QgsLayerMetadataProviderResult > layersToAdd;
      for ( const auto &selectedIndex : std::as_const( selectedIndexes ) )
      {
        layersToAdd.push_back( sourceModel->data( mProxyModel->mapToSource( selectedIndex ), QgsLayerMetadataResultsModel::Roles::Metadata ).value<QgsLayerMetadataProviderResult>() );
      }
      emit addLayers( layersToAdd );
    }

  } );

  connect( mMetadataTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [ = ]( const QItemSelection &, const QItemSelection & )
  {
    mAddButton->setEnabled( mMetadataTableView->selectionModel()->hasSelection() );
  } );

  QPushButton *closeButton = mButtonBox->button( QDialogButtonBox::Close );
  closeButton->setToolTip( tr( "Close this dialog without adding any layer" ) );
  connect( closeButton, &QPushButton::clicked, this, &QgsLayerMetadataSearchWidget::rejected );

  connect( mSearchFilterLineEdit, &QLineEdit::textEdited, mProxyModel, &QgsLayerMetadataResultsProxyModel::setFilterString );
  connect( mExtentFilterComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsLayerMetadataSearchWidget::updateExtentFilter );

  if ( mMapCanvas )
  {
    connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, [ = ]
    {
      updateExtentFilter( mExtentFilterComboBox->currentIndex() );
    } );
  }

  connect( QgsProject::instance(), &QgsProject::layersAdded, this, [ = ]( const QList<QgsMapLayer *> & )
  {
    updateExtentFilter( mExtentFilterComboBox->currentIndex() );
  } );

  connect( QgsProject::instance(), &QgsProject::layersRemoved, this, [ = ]( const QStringList & )
  {
    updateExtentFilter( mExtentFilterComboBox->currentIndex() );
  } );

  // Start loading metadata in the model
  sourceModel->reloadAsync();
  mIsLoading = true;

}

void QgsLayerMetadataSearchWidget::updateExtentFilter( int index )
{
  if ( index == 1 && mMapCanvas )
  {
    QgsCoordinateTransform ct( mMapCanvas->mapSettings().destinationCrs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsProject::instance()->transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    mProxyModel->setFilterExtent( ct.transformBoundingBox( mMapCanvas->extent() ) );
  }
  else if ( index == 2 )
  {
    const QgsReferencedRectangle extent = QgsProject::instance()->viewSettings()->fullExtent();
    QgsCoordinateTransform ct( extent.crs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsProject::instance()->transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    mProxyModel->setFilterExtent( ct.transformBoundingBox( extent ) );
  }
  else
  {
    mProxyModel->setFilterExtent( QgsRectangle( ) );
  }
}
