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
#include <QFutureSynchronizer>
#include <QtConcurrent>

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
    if ( role == Qt::ItemDataRole::DisplayRole )
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
          return mResult.metadata().at( index.row() ).dataProviderName();
        case Sections::GeometryType:
          return QgsWkbTypes::geometryDisplayString( mResult.metadata().at( index.row() ).geometryType() );
        default:
          return QVariant();
      }
    }
    else if ( role == Qt::ItemDataRole::ToolTipRole )
    {
      const QgsLayerMetadataFormatter formatter { mResult.metadata().at( index.row() ) };

      /**
       * Long version (too long?)
      *      return tr( R"HTML(
      *        <html>
      *        <body>
      *          <!-- metadata headers --->
      *          <h3>Identification</h3>%1
      *          <h3>Contacts</h3>%2
      *          <h3>Extent</h3>%3
      *          <h3>History</h3>%4
      *          <h3>Access</h3>%5
      *          <h3>Links</h3>%6
      *        </body>
      *        </html>
      *      )HTML" )
      *          .arg(
      *            formatter.identificationSectionHtml(),
      *            formatter.contactsSectionHtml(),
      *            formatter.extentSectionHtml(),
      *            formatter.historySectionHtml(),
      *            formatter.accessSectionHtml(),
      *            formatter.linksSectionHtml() );
            */
      // Shorter version
      return tr( R"HTML(
        <html>
        <body>
          <!-- metadata headers --->
          <h3>Identification</h3>%1
        </body>
        </html>
)HTML" )
          .arg(
            formatter.identificationSectionHtml());
    }
    else if ( role == Roles::Metadata )
    {
      return QVariant::fromValue( mResult.metadata().at( index.row() ) );
    }
  }
  return QVariant();
}

QVariant QgsLayerMetadataResultsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ( orientation == Qt::Orientation::Horizontal && section < columnCount( createIndex( -1, -1 ) ) )
  {
    if ( role == Qt::ItemDataRole::DisplayRole )
    {
      switch ( section )
      {
        case Sections::Identifier:
          return tr( "Identifier");
        case Sections::Title:
          return tr( "Title");
        case Sections::Abstract:
          return tr( "Abstract");
        case Sections::DataProviderName:
          return tr( "Provider");
        case Sections::GeometryType:
          return tr( "Geometry Type");
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
    connect( fetcher.get(), &QgsMetadataResultsFetcher::resultsReady, this, [=] (const QgsLayerMetadataSearchResults& results)
    {
      resultsReady( results );
    } );
    connect(thread.get(), &QThread::started, fetcher.get(), &QgsMetadataResultsFetcher::fetchMetadata );
    mWorkerThreads.push_back( std::move( thread ));
    mWorkers.push_back( std::move( fetcher ) );
    mWorkerThreads.back()->start();
  }
}

void QgsLayerMetadataResultsModel::resultsReady(const QgsLayerMetadataSearchResults& results)
{
  mFeedback->setProgress( mFeedback->progress() + 100 / QgsApplication::instance()->layerMetadataProviderRegistry()->layerMetadataProviders().count() );
  beginInsertRows( QModelIndex(), mResult.metadata().count(), mResult.metadata().count() + results.metadata().count() - 1 );
  const QList<QgsLayerMetadataProviderResult> metadata { results.metadata() };
  for(const QgsLayerMetadataProviderResult &result: std::as_const( metadata ) )
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

  for(const auto &workerThread: std::as_const( mWorkerThreads ) )
  {
    workerThread->quit();
    workerThread->wait();
  }

  mWorkers.clear();
  mWorkerThreads.clear();

  mFeedback = std::make_unique<QgsFeedback>();
  connect(mFeedback.get(), &QgsFeedback::progressChanged, this, &QgsLayerMetadataResultsModel::progressChanged );
}


///@cond private

QgsMetadataResultsFetcher::QgsMetadataResultsFetcher(const QgsAbstractLayerMetadataProvider *metadataProvider, const QgsMetadataSearchContext &searchContext, QgsFeedback *feedback)
  : mLayerMetadataProvider ( metadataProvider)
  , mSearchContext( searchContext )
  , mFeedback( feedback )
{  
}

void QgsMetadataResultsFetcher::fetchMetadata()
{
  emit resultsReady( mLayerMetadataProvider->search( mSearchContext, QString(), QgsRectangle(), mFeedback ));
}

///@endcond private
