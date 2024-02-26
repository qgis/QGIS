/***************************************************************************
    qgsbookmarkmodel.cpp
    --------------------
    Date                 : September 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsbookmarkmodel.h"
#include "qgsbookmarkmanager.h"

#include <QIcon>

QgsBookmarkManagerModel::QgsBookmarkManagerModel( QgsBookmarkManager *manager, QgsBookmarkManager *projectManager, QObject *parent )
  : QAbstractTableModel( parent )
  , mManager( manager )
  , mProjectManager( projectManager )
{
  for ( QgsBookmarkManager *obj : { manager, projectManager } )
  {
    connect( obj, &QgsBookmarkManager::bookmarkAdded, this, &QgsBookmarkManagerModel::bookmarkAdded );
    connect( obj, &QgsBookmarkManager::bookmarkAboutToBeAdded, this, &QgsBookmarkManagerModel::bookmarkAboutToBeAdded );
    connect( obj, &QgsBookmarkManager::bookmarkRemoved, this, &QgsBookmarkManagerModel::bookmarkRemoved );
    connect( obj, &QgsBookmarkManager::bookmarkAboutToBeRemoved, this, &QgsBookmarkManagerModel::bookmarkAboutToBeRemoved );
    connect( obj, &QgsBookmarkManager::bookmarkChanged, this, &QgsBookmarkManagerModel::bookmarkChanged );
  }
}

int QgsBookmarkManagerModel::rowCount( const QModelIndex & ) const
{
  return mManager->bookmarks().count() + mProjectManager->bookmarks().count();
}

int QgsBookmarkManagerModel::columnCount( const QModelIndex & ) const
{
  return ColumnStore + 1;
}

QVariant QgsBookmarkManagerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const QgsBookmark b = bookmarkForIndex( index );
  const int managerCount = mManager->bookmarks().count();

  switch ( role )
  {
    case static_cast< int >( CustomRole::Extent ):
      return b.extent();

    case static_cast< int >( CustomRole::Rotation ):
      return b.rotation();

    case static_cast< int >( CustomRole::Name ):
      return b.name();

    case static_cast< int >( CustomRole::Id ):
      return b.id();

    case static_cast< int >( CustomRole::Group ):
      return b.group();

    case Qt::DecorationRole:
      return index.column() == ColumnName ? QgsApplication::getThemeIcon( QStringLiteral( "/mItemBookmark.svg" ) ) : QIcon();

    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case ColumnName:
          return b.name();
        case ColumnGroup:
          return b.group();
        case ColumnXMin:
          return b.extent().xMinimum();
        case ColumnYMin:
          return b.extent().yMinimum();
        case ColumnXMax:
          return b.extent().xMaximum();
        case ColumnYMax:
          return b.extent().yMaximum();
        case ColumnRotation:
          return b.rotation();
        case ColumnCrs:
          return b.extent().crs().authid();
        case ColumnStore:
          return QVariant();
      }
      break;
    }

    case Qt::CheckStateRole:
    {
      if ( index.column() == ColumnStore )
        return index.row() < managerCount ? Qt::Unchecked : Qt::Checked;
      break;
    }
  }
  return QVariant();
}

Qt::ItemFlags QgsBookmarkManagerModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || index.row() < 0 || index.row() >= rowCount() )
    return Qt::ItemFlags();

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if ( index.column() == ColumnStore )
  {
    flags |= Qt::ItemIsUserCheckable;
  }
  else
  {
    // projection column is not editable!
    if ( index.column() != ColumnCrs )
      flags |= Qt::ItemIsEditable;
  }
  return flags;
}

bool QgsBookmarkManagerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsBookmark b = bookmarkForIndex( index );
  const int managerCount = mManager->bookmarks().count();

  switch ( role )
  {
    case Qt::EditRole:
    {
      QgsReferencedRectangle extent = b.extent();
      switch ( index.column() )
      {
        case ColumnName:
          b.setName( value.toString() );
          break;
        case ColumnGroup:
          b.setGroup( value.toString() );
          break;
        case ColumnXMin:
        {
          bool ok = false;
          extent.setXMinimum( value.toDouble( &ok ) );
          if ( !ok )
            return false;
          break;
        }
        case ColumnYMin:
        {
          bool ok = false;
          extent.setYMinimum( value.toDouble( &ok ) );
          if ( !ok )
            return false;
          break;
        }
        case ColumnXMax:
        {
          bool ok = false;
          extent.setXMaximum( value.toDouble( &ok ) );
          if ( !ok )
            return false;
          break;
        }
        case ColumnYMax:
        {
          bool ok = false;
          extent.setYMaximum( value.toDouble( &ok ) );
          if ( !ok )
            return false;
          break;
        }
        case ColumnRotation:
          b.setRotation( value.toDouble() );
          break;
        case ColumnCrs:
        {
          QgsCoordinateReferenceSystem crs;
          if ( !crs.createFromString( value.toString() ) )
            return false;
          extent.setCrs( crs );
          break;
        }
        default:
          return false;
      }
      b.setExtent( extent );
      return index.row() < managerCount ? mManager->updateBookmark( b ) : mProjectManager->updateBookmark( b );
    }

    case Qt::CheckStateRole:
    {
      if ( index.column() != ColumnStore )
        return false;

      if ( index.row() < managerCount )
      {
        if ( value.toInt() != Qt::Checked )
          return false;
        return mManager->moveBookmark( b.id(), mProjectManager );
      }
      else
      {
        if ( value.toInt() != Qt::Unchecked )
          return false;
        return mProjectManager->moveBookmark( b.id(), mManager );
      }
    }
  }

  return false;
}

bool QgsBookmarkManagerModel::insertRows( int, int count, const QModelIndex & )
{
  // append
  bool result = true;
  for ( int i = 0; i < count; ++i )
  {
    bool res = false;
    mBlocked = true;
    mManager->addBookmark( QgsBookmark(), &res );
    mBlocked = false;
    result &= res;
  }
  return result;
}

bool QgsBookmarkManagerModel::removeRows( int row, int count, const QModelIndex & )
{
  const QList< QgsBookmark > appBookmarks = mManager->bookmarks();
  const QList< QgsBookmark > projectBookmarks = mProjectManager->bookmarks();
  for ( int r = row + count - 1; r >= row; --r )
  {
    if ( r >= appBookmarks.count() )
      mProjectManager->removeBookmark( projectBookmarks.at( r - appBookmarks.size() ).id() );
    else
      mManager->removeBookmark( appBookmarks.at( r ).id() );
  }
  return true;
}

QVariant QgsBookmarkManagerModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    switch ( section )
    {
      case ColumnName:
        return tr( "Name" );
      case ColumnGroup:
        return tr( "Group" );
      case ColumnXMin:
        return tr( "xMin" );
      case ColumnYMin:
        return tr( "yMin" );
      case ColumnXMax:
        return tr( "xMax" );
      case ColumnYMax:
        return tr( "yMax" );
      case ColumnRotation:
        return tr( "Rotation" );
      case ColumnCrs:
        return tr( "CRS" );
      case ColumnStore:
        return tr( "In Project" );
    }
  }
  return QAbstractTableModel::headerData( section, orientation, role );
}

void QgsBookmarkManagerModel::bookmarkAboutToBeAdded( const QString & )
{
  if ( mBlocked )
    return;

  if ( qobject_cast< QgsBookmarkManager * >( sender() ) == mManager )
    beginInsertRows( QModelIndex(), mManager->bookmarks().count(), mManager->bookmarks().count() );
  else
    beginInsertRows( QModelIndex(), mManager->bookmarks().count() + mProjectManager->bookmarks().count(),
                     mManager->bookmarks().count() + mProjectManager->bookmarks().count() );
}

void QgsBookmarkManagerModel::bookmarkAdded( const QString & )
{
  if ( mBlocked )
    return;

  endInsertRows();
}

void QgsBookmarkManagerModel::bookmarkAboutToBeRemoved( const QString &id )
{
  if ( mBlocked )
    return;

  QgsBookmarkManager *manager = qobject_cast< QgsBookmarkManager * >( sender() );

  const QList< QgsBookmark > bookmarks = manager->bookmarks();
  bool found = false;
  int i = 0;
  for ( i = 0; i < bookmarks.count(); ++i )
  {
    if ( bookmarks.at( i ).id() == id )
    {
      found = true;
      break;
    }
  }
  if ( !found )
    return;

  if ( manager == mManager )
    beginRemoveRows( QModelIndex(), i, i );
  else
    beginRemoveRows( QModelIndex(), mManager->bookmarks().count() + i,
                     mManager->bookmarks().count() + i );
}

void QgsBookmarkManagerModel::bookmarkRemoved( const QString & )
{
  if ( mBlocked )
    return;

  endRemoveRows();
}

void QgsBookmarkManagerModel::bookmarkChanged( const QString &id )
{
  if ( mBlocked )
    return;

  QgsBookmarkManager *manager = qobject_cast< QgsBookmarkManager * >( sender() );
  const QList< QgsBookmark > bookmarks = manager->bookmarks();
  bool found = false;
  int i = 0;
  for ( i = 0; i < bookmarks.count(); ++i )
  {
    if ( bookmarks.at( i ).id() == id )
    {
      found = true;
      break;
    }
  }
  if ( !found )
    return;

  if ( manager == mManager )
    emit dataChanged( index( i, 0 ), index( i, columnCount() - 1 ) );
  else
    emit dataChanged( index( mManager->bookmarks().count() + i, 0 ), index( mManager->bookmarks().count() + i, columnCount() - 1 ) );
}

QgsBookmark QgsBookmarkManagerModel::bookmarkForIndex( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QgsBookmark();

  const int managerCount = mManager->bookmarks().count();
  const int projectCount = mProjectManager->bookmarks().count();
  if ( index.row() < managerCount )
    return mManager->bookmarks().at( index.row() );
  else if ( index.row() < managerCount + projectCount )
    return mProjectManager->bookmarks().at( index.row() - managerCount );
  return QgsBookmark();
}

//
// QgsBookmarkManagerProxyModel
//

QgsBookmarkManagerProxyModel::QgsBookmarkManagerProxyModel( QgsBookmarkManager *manager, QgsBookmarkManager *projectManager, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mModel( new QgsBookmarkManagerModel( manager, projectManager, this ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );
}
