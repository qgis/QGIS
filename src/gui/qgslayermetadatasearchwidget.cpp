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


QgsLayerMetadataSearchWidget::QgsLayerMetadataSearchWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{

  setupUi( this );
  setupButtons( mButtonBox );

  QgsMetadataSearchContext searchContext;
  searchContext.transformContext = QgsProject::instance()->transformContext();

  mSourceModel = new QgsLayerMetadataResultsModel( searchContext, this );
  mProxyModel = new QgsLayerMetadataResultsProxyModel( this );
  mProxyModel->setSourceModel( mSourceModel );
  mMetadataTableView->setModel( mProxyModel );
  mMetadataTableView->setSortingEnabled( true );
  mMetadataTableView->sortByColumn( 0, Qt::SortOrder::AscendingOrder );
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

  connect( mSourceModel, &QgsLayerMetadataResultsModel::progressChanged, mProgressBar, &QProgressBar::setValue );
  connect( mSourceModel, &QgsLayerMetadataResultsModel::progressChanged, this,  [ = ]( int progress )
  {
    if ( progress == 100 )
    {
      mIsLoading = false;
      mProgressBar->hide();
      updateLoadBtn();
    }
  } );

  connect( mAbortPushButton, &QPushButton::clicked, mSourceModel, [ = ]( bool )
  {
    if ( ! mIsLoading )
    {
      mIsLoading = true;
      mProgressBar->show();
      mSourceModel->reloadAsync( );
    }
    else
    {
      mProgressBar->hide();
      mSourceModel->cancel();
      mIsLoading = false;
    }
    updateLoadBtn();
  } );

  connect( mMetadataTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [ = ]( const QItemSelection &, const QItemSelection & )
  {
    emit enableButtons( mMetadataTableView->selectionModel()->hasSelection() );
  } );

  connect( mSearchFilterLineEdit, &QLineEdit::textEdited, mProxyModel, &QgsLayerMetadataResultsProxyModel::setFilterString );
  connect( mExtentFilterComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsLayerMetadataSearchWidget::updateExtentFilter );

  connect( QgsProject::instance(), &QgsProject::layersAdded, this, [ = ]( const QList<QgsMapLayer *> & )
  {
    updateExtentFilter( mExtentFilterComboBox->currentIndex() );
  } );

  connect( QgsProject::instance(), &QgsProject::layersRemoved, this, [ = ]( const QStringList & )
  {
    updateExtentFilter( mExtentFilterComboBox->currentIndex() );
  } );

  // Start loading metadata in the model
  mSourceModel->reloadAsync();
  mIsLoading = true;

}

void QgsLayerMetadataSearchWidget::setMapCanvas( QgsMapCanvas *newMapCanvas )
{
  if ( newMapCanvas && mapCanvas() != newMapCanvas )
  {
    connect( newMapCanvas, &QgsMapCanvas::extentsChanged, this, [ = ]
    {
      updateExtentFilter( mExtentFilterComboBox->currentIndex() );
    } );
  }
  QgsAbstractDataSourceWidget::setMapCanvas( newMapCanvas );
}

void QgsLayerMetadataSearchWidget::updateExtentFilter( int index )
{
  if ( index == 1 && mapCanvas() )
  {
    QgsCoordinateTransform ct( mapCanvas()->mapSettings().destinationCrs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsProject::instance()->transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    mProxyModel->setFilterExtent( ct.transformBoundingBox( mapCanvas()->extent() ) );
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

void QgsLayerMetadataSearchWidget::refresh()
{
  mSourceModel->reloadAsync();
}

void QgsLayerMetadataSearchWidget::addButtonClicked()
{
  const QModelIndexList &selectedIndexes { mMetadataTableView->selectionModel()->selectedRows() };
  if ( ! selectedIndexes.isEmpty() )
  {
    for ( const auto &selectedIndex : std::as_const( selectedIndexes ) )
    {
      const QgsLayerMetadataProviderResult metadataResult { mSourceModel->data( mProxyModel->mapToSource( selectedIndex ), QgsLayerMetadataResultsModel::Roles::Metadata ).value<QgsLayerMetadataProviderResult>() };
      switch ( metadataResult.layerType() )
      {
        case QgsMapLayerType::RasterLayer:
        {
          emit addRasterLayer( metadataResult.uri(), metadataResult.identifier(), metadataResult.dataProviderName() );
          break;
        }
        case QgsMapLayerType::VectorLayer:
        {
          emit addVectorLayer( metadataResult.uri(), metadataResult.identifier(), metadataResult.dataProviderName() );
          break;
        }
        case QgsMapLayerType::MeshLayer:
        {
          emit addMeshLayer( metadataResult.uri(), metadataResult.identifier(), metadataResult.dataProviderName() );
          break;
        }
        default:  // unsupported
        {
          // Ignore
          break;
        }
      }
    }
  }
}

void QgsLayerMetadataSearchWidget::reset()
{
  mSearchFilterLineEdit->clear();
  mExtentFilterComboBox->setCurrentIndex( 0 );
}
