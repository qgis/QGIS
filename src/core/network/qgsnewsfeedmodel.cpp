/***************************************************************************
    qgsnewsfeedmodel.cpp
    -------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnewsfeedmodel.h"
#include "moc_qgsnewsfeedmodel.cpp"
#include "qgsnetworkcontentfetcher.h"
#include <QPainter>

//
// QgsNewsFeedModel
//

QgsNewsFeedModel::QgsNewsFeedModel( QgsNewsFeedParser *parser, QObject *parent )
  : QAbstractItemModel( parent )
  , mParser( parser )
{
  Q_ASSERT( mParser );
  const QList< QgsNewsFeedParser::Entry > initialEntries = mParser->entries();
  for ( const QgsNewsFeedParser::Entry &e : initialEntries )
    onEntryAdded( e );

  connect( mParser, &QgsNewsFeedParser::entryAdded, this, &QgsNewsFeedModel::onEntryAdded );
  connect( mParser, &QgsNewsFeedParser::entryDismissed, this, &QgsNewsFeedModel::onEntryRemoved );
  connect( mParser, &QgsNewsFeedParser::entryUpdated, this, &QgsNewsFeedModel::onEntryUpdated );
  connect( mParser, &QgsNewsFeedParser::imageFetched, this, &QgsNewsFeedModel::onImageFetched );
}

QVariant QgsNewsFeedModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const QgsNewsFeedParser::Entry &entry = mEntries.at( index.row() );

  switch ( role )
  {
    case Qt::DisplayRole:
    case static_cast< int >( CustomRole::Content ):
      return entry.content;

    case Qt::ToolTipRole:
    case static_cast< int >( CustomRole::Title ):
      return entry.title;

    case static_cast< int >( CustomRole::Key ):
      return entry.key;

    case static_cast< int >( CustomRole::ImageUrl ):
      return entry.imageUrl;

    case static_cast< int >( CustomRole::Image ):
      return entry.image;

    case static_cast< int >( CustomRole::Link ):
      return entry.link;

    case static_cast< int >( CustomRole::Sticky ):
      return entry.sticky;

    case Qt::DecorationRole:
      if ( entry.image.isNull() )
        return QVariant();
      return entry.image;
  }
  return QVariant();
}

Qt::ItemFlags QgsNewsFeedModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );
  return flags;
}

QModelIndex QgsNewsFeedModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  if ( !parent.isValid() )
  {
    return createIndex( row, column );
  }

  return QModelIndex();
}

QModelIndex QgsNewsFeedModel::parent( const QModelIndex & ) const
{
  //all items are top level for now
  return QModelIndex();
}

int QgsNewsFeedModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mEntries.count();
  }
  return 0;
}

int QgsNewsFeedModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

void QgsNewsFeedModel::onEntryAdded( const QgsNewsFeedParser::Entry &entry )
{
  beginInsertRows( QModelIndex(), mEntries.count(), mEntries.count() );
  mEntries.append( entry );
  endInsertRows();
}

void QgsNewsFeedModel::onEntryUpdated( const QgsNewsFeedParser::Entry &entry )
{
  for ( int idx = 0; idx < mEntries.count(); idx++ )
  {
    if ( mEntries.at( idx ).key == entry.key )
    {
      mEntries[ idx ] = entry;
      emit dataChanged( index( idx, 0 ), index( idx, 0 ) );
      break;
    }
  }
}

void QgsNewsFeedModel::onEntryRemoved( const QgsNewsFeedParser::Entry &entry )
{
  // find index of entry
  const auto findIter = std::find_if( mEntries.begin(), mEntries.end(), [entry]( const QgsNewsFeedParser::Entry & candidate )
  {
    return candidate.key == entry.key;
  } );
  if ( findIter == mEntries.end() )
    return;

  const int entryIndex = static_cast< int >( std::distance( mEntries.begin(), findIter ) );
  beginRemoveRows( QModelIndex(), entryIndex, entryIndex );
  mEntries.removeAt( entryIndex );
  endRemoveRows();
}

void QgsNewsFeedModel::onImageFetched( const int key, const QPixmap &pixmap )
{
  // find index of entry
  const auto findIter = std::find_if( mEntries.begin(), mEntries.end(), [key]( const QgsNewsFeedParser::Entry & candidate )
  {
    return candidate.key == key;
  } );
  if ( findIter == mEntries.end() )
    return;

  const int entryIndex = static_cast< int >( std::distance( mEntries.begin(), findIter ) );
  mEntries[ entryIndex ].image = pixmap;
  emit dataChanged( index( entryIndex, 0, QModelIndex() ), index( entryIndex, 0, QModelIndex() ) );
}


//
// QgsNewsFeedProxyModel
//

QgsNewsFeedProxyModel::QgsNewsFeedProxyModel( QgsNewsFeedParser *parser, QObject *parent )
  : QSortFilterProxyModel( parent )
{
  mModel = new QgsNewsFeedModel( parser, this );
  setSortCaseSensitivity( Qt::CaseInsensitive );
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  sort( 0 );
}

bool QgsNewsFeedProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const bool leftSticky = sourceModel()->data( left, static_cast< int >( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool();
  const bool rightSticky = sourceModel()->data( right, static_cast< int >( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool();

  // sticky items come first
  if ( leftSticky && !rightSticky )
    return true;
  if ( rightSticky && !leftSticky )
    return false;

  // else sort by descending key
  const int leftKey = sourceModel()->data( left, static_cast< int >( QgsNewsFeedModel::CustomRole::Key ) ).toInt();
  const int rightKey = sourceModel()->data( right, static_cast< int >( QgsNewsFeedModel::CustomRole::Key ) ).toInt();
  return rightKey < leftKey;
}
