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
#include "qgsnetworkcontentfetcher.h"
#include <QPixmap>

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
}

QVariant QgsNewsFeedModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const QgsNewsFeedParser::Entry &entry = mEntries.at( index.row() );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Content:
      return entry.content;

    case Qt::ToolTipRole:
    case Title:
      return entry.title;

    case Key:
      return entry.key;

    case ImageUrl:
      return entry.imageUrl;

    case Image:
      return entry.image;

    case Link:
      return entry.link;

    case Sticky:
      return entry.sticky;

    case Qt::DecorationRole:
      if ( entry.image.isNull() )
        return QVariant();
      return QPixmap::fromImage( entry.image );
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

  if ( !entry.imageUrl.isEmpty() )
  {
    // start fetching image
    QgsNetworkContentFetcher *fetcher = new QgsNetworkContentFetcher();
    connect( fetcher, &QgsNetworkContentFetcher::finished, this, [ = ]
    {
      auto findIter = std::find_if( mEntries.begin(), mEntries.end(), [entry]( const QgsNewsFeedParser::Entry & candidate )
      {
        return candidate.key == entry.key;
      } );
      if ( findIter != mEntries.end() )
      {
        const int entryIndex = static_cast< int >( std::distance( mEntries.begin(), findIter ) );
        mEntries[ entryIndex ].image = QImage::fromData( fetcher->reply()->readAll() );
        this->emit dataChanged( index( entryIndex, 0, QModelIndex() ), index( entryIndex, 0, QModelIndex() ) );
      }
      fetcher->deleteLater();
    } );
    fetcher->fetchContent( entry.imageUrl, mParser->authcfg() );
  }

  endInsertRows();
}

void QgsNewsFeedModel::onEntryRemoved( const QgsNewsFeedParser::Entry &entry )
{
  // find index of entry
  auto findIter = std::find_if( mEntries.begin(), mEntries.end(), [entry]( const QgsNewsFeedParser::Entry & candidate )
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
  const bool leftSticky = sourceModel()->data( left, QgsNewsFeedModel::Sticky ).toBool();
  const bool rightSticky = sourceModel()->data( right, QgsNewsFeedModel::Sticky ).toBool();

  // sticky items come first
  if ( leftSticky && !rightSticky )
    return true;
  if ( rightSticky && !leftSticky )
    return false;

  // else sort by descending key
  const int leftKey = sourceModel()->data( left, QgsNewsFeedModel::Key ).toInt();
  const int rightKey = sourceModel()->data( right, QgsNewsFeedModel::Key ).toInt();
  return rightKey < leftKey;
}
