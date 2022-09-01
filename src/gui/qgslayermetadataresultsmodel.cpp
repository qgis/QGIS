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

QgsLayerMetadataResultsModel::QgsLayerMetadataResultsModel( const QgsMetadataSearchContext &searchContext, QgsFeedback *feedback, QObject *parent )
  : QAbstractTableModel( parent )
  , mFeedback( feedback )
  , mSearchContext( searchContext )
{
  reload();
}

int QgsLayerMetadataResultsModel::rowCount( const QModelIndex &parent ) const
{
  return parent.isValid() ? 0 : mResult.metadata().count();
}

int QgsLayerMetadataResultsModel::columnCount( const QModelIndex &parent ) const
{
  return parent.isValid() ? 0 : 3;
}

QVariant QgsLayerMetadataResultsModel::data( const QModelIndex &index, int role ) const
{
  if ( index.isValid() )
  {
    if ( role == Qt::ItemDataRole::DisplayRole && index.row() < rowCount( createIndex( -1, -1 ) ) )
    {
      switch ( index.column() )
      {
        case 0:
          return mResult.metadata().at( index.row() ).identifier( );
        case 1:
          return mResult.metadata().at( index.row() ).title();
        case 2:
          return mResult.metadata().at( index.row() ).abstract();
        default:
          return QVariant();
      }
    }
  }
  return QVariant();
}

void QgsLayerMetadataResultsModel::reload()
{
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
