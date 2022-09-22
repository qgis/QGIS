/***************************************************************************
  qgslayermetadataresultsmodel.cpp - QgsLayerMetadataResultsModel

 ---------------------
 begin                : 1.9.2022
 copyright            : (C) 2022 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayermetadataresultsmodel.h"
#include "qgsfeedback.h"
#include "qgsapplication.h"
#include "qgslayermetadataproviderregistry.h"
#include "qgslayermetadataformatter.h"
#include "qgsiconutils.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include <QIcon>

QgsLayerMetadataResultsModel::QgsLayerMetadataResultsModel( const QgsMetadataSearchContext &searchContext, QObject *parent )
  : QAbstractTableModel( parent )
  , mSearchContext( searchContext )
{
  qRegisterMetaType< QgsLayerMetadataSearchResults>( "QgsLayerMetadataSearchResults" );
  qRegisterMetaType< QgsLayerMetadataProviderResult>( "QgsLayerMetadataProviderResult" );
}

QgsLayerMetadataResultsModel::~QgsLayerMetadataResultsModel()
{
  cancel();
}

int QgsLayerMetadataResultsModel::rowCount( const QModelIndex &parent ) const
{
  return parent.isValid() ? 0 : mResult.metadata().count();
}

int QgsLayerMetadataResultsModel::columnCount( const QModelIndex &parent ) const
{
  return parent.isValid() ? 0 : 5;
}

QVariant QgsLayerMetadataResultsModel::data( const QModelIndex &index, int role ) const
{
  if ( index.isValid() && index.row() < mResult.metadata().count( ) )
  {
    switch ( role )
    {
      case Qt::ItemDataRole::DisplayRole:
      {
        switch ( index.column() )
        {
          case Sections::Identifier:
            return mResult.metadata().at( index.row() ).identifier( );
          case Sections::Title:
            return mResult.metadata().at( index.row() ).title();
          case Sections::Abstract:
            return mResult.metadata().at( index.row() ).abstract();
          case Sections::DataProviderName:
          {
            const QString providerName { mResult.metadata().at( index.row() ).dataProviderName() };
            QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerName ) };
            return md ? md->description() : providerName;
          }
          case Sections::GeometryType:
          {
            const QgsLayerMetadataProviderResult &md { mResult.metadata().at( index.row() ) };
            if ( md.layerType() == QgsMapLayerType::RasterLayer )
              return tr( "Raster" );
            return md.geometryType() == QgsWkbTypes::GeometryType::UnknownGeometry ? QgsWkbTypes::geometryDisplayString( QgsWkbTypes::GeometryType::NullGeometry ) : QgsWkbTypes::geometryDisplayString( md.geometryType() );
          }
          default:
            return QVariant();
        }
        break;
      }
      case Qt::ItemDataRole::ToolTipRole:
      {
        const QgsLayerMetadataFormatter formatter { mResult.metadata().at( index.row() ) };
        return tr( R"HTML(<html><body><!-- metadata headers ---><h3>Identification</h3>%1</body></html>)HTML" )
               .arg(
                 formatter.identificationSectionHtml() );
        break;
      }
      case Qt::ItemDataRole::DecorationRole:
      {
        if ( index.column() == 0 )
        {
          const QgsLayerMetadataProviderResult &md { mResult.metadata().at( index.row() ) };
          if ( md.layerType() == QgsMapLayerType::RasterLayer )
            return QgsApplication::getThemeIcon( QStringLiteral( "mIconRaster.svg" ) );
          return QgsIconUtils::iconForGeometryType( md.geometryType() == QgsWkbTypes::GeometryType::UnknownGeometry ? QgsWkbTypes::GeometryType::NullGeometry : md.geometryType() );
        }
        break;
      }
      case Roles::Metadata:
      {
        return QVariant::fromValue( mResult.metadata().at( index.row() ) );
      }
      default:
        // Ignore
        break;

    }
  }
  return QVariant();
}

QVariant QgsLayerMetadataResultsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Orientation::Horizontal && section < columnCount( createIndex( -1, -1 ) ) )
  {
    if ( role == Qt::ItemDataRole::DisplayRole )
    {
      switch ( section )
      {
        case Sections::Identifier:
          return tr( "Identifier" );
        case Sections::Title:
          return tr( "Title" );
        case Sections::Abstract:
          return tr( "Abstract" );
        case Sections::DataProviderName:
          return tr( "Provider" );
        case Sections::GeometryType:
          return tr( "Layer Type" );
      }
    }
    // other roles here ...
  }
  return QAbstractTableModel::headerData( section, orientation, role );
}

void QgsLayerMetadataResultsModel::reload()
{
  cancel();
  beginResetModel();
  // Load results from layer metadata providers
  mResult = QgsLayerMetadataSearchResults();
  const QList<QgsAbstractLayerMetadataProvider *> providers { QgsApplication::instance()->layerMetadataProviderRegistry()->layerMetadataProviders() };
  for ( QgsAbstractLayerMetadataProvider *mdProvider : std::as_const( providers ) )
  {
    const QList<QgsLayerMetadataProviderResult> results { mdProvider->search( mSearchContext ).metadata() };
    for ( const QgsLayerMetadataProviderResult &metadata : std::as_const( results ) )
    {
      mResult.addMetadata( metadata );
    }
  }
  endResetModel();
}

void QgsLayerMetadataResultsModel::reloadAsync()
{
  cancel();
  beginResetModel();
  // Load results from layer metadata providers
  mResult = QgsLayerMetadataSearchResults();
  endResetModel();
  mFeedback->setProgress( 0 );
  const QList<QgsAbstractLayerMetadataProvider *> providers { QgsApplication::instance()->layerMetadataProviderRegistry()->layerMetadataProviders() };
  for ( QgsAbstractLayerMetadataProvider *mdProvider : std::as_const( providers ) )
  {
    std::unique_ptr<QgsMetadataResultsFetcher> fetcher = std::make_unique<QgsMetadataResultsFetcher>( mdProvider, mSearchContext, mFeedback.get() );
    std::unique_ptr<QThread> thread = std::make_unique<QThread>();
    fetcher->moveToThread( thread.get() );
    // Forward signals to the model
    connect( fetcher.get(), &QgsMetadataResultsFetcher::resultsReady, this, [ = ]( const QgsLayerMetadataSearchResults & results )
    {
      resultsReady( results );
    } );
    connect( thread.get(), &QThread::started, fetcher.get(), &QgsMetadataResultsFetcher::fetchMetadata );
    mWorkerThreads.push_back( std::move( thread ) );
    mWorkers.push_back( std::move( fetcher ) );
    mWorkerThreads.back()->start();
  }
}

void QgsLayerMetadataResultsModel::resultsReady( const QgsLayerMetadataSearchResults &results )
{
  mFeedback->setProgress( mFeedback->progress() + 100 / QgsApplication::instance()->layerMetadataProviderRegistry()->layerMetadataProviders().count() );
  beginInsertRows( QModelIndex(), mResult.metadata().count(), mResult.metadata().count() + results.metadata().count() - 1 );
  const QList<QgsLayerMetadataProviderResult> metadata { results.metadata() };
  for ( const QgsLayerMetadataProviderResult &result : std::as_const( metadata ) )
  {
    mResult.addMetadata( result );
  }
  endInsertRows();
}

void QgsLayerMetadataResultsModel::cancel()
{
  if ( mFeedback )
  {
    mFeedback->cancel();
  }

  for ( const auto &workerThread : std::as_const( mWorkerThreads ) )
  {
    workerThread->quit();
    workerThread->wait();
  }

  mWorkers.clear();
  mWorkerThreads.clear();

  mFeedback = std::make_unique<QgsFeedback>();
  connect( mFeedback.get(), &QgsFeedback::progressChanged, this, &QgsLayerMetadataResultsModel::progressChanged );
}


///@cond private

QgsMetadataResultsFetcher::QgsMetadataResultsFetcher( const QgsAbstractLayerMetadataProvider *metadataProvider, const QgsMetadataSearchContext &searchContext, QgsFeedback *feedback )
  : mLayerMetadataProvider( metadataProvider )
  , mSearchContext( searchContext )
  , mFeedback( feedback )
{
}

void QgsMetadataResultsFetcher::fetchMetadata()
{
  emit resultsReady( mLayerMetadataProvider->search( mSearchContext, QString(), QgsRectangle(), mFeedback ) );
}

///@endcond private
