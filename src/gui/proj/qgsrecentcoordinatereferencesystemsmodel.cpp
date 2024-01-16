/***************************************************************************
                    qgsrecentcoordinatereferencesystemsmodel.cpp
                    -------------------
    begin                : January 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrecentcoordinatereferencesystemsmodel.h"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgsapplication.h"

#include <QFont>

QgsRecentCoordinateReferenceSystemsModel::QgsRecentCoordinateReferenceSystemsModel( QObject *parent )
  : QAbstractItemModel( parent )
{
  mCrs = QgsApplication::coordinateReferenceSystemRegistry()->recentCrs();
  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::recentCrsPushed, this, &QgsRecentCoordinateReferenceSystemsModel::recentCrsPushed );
  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::recentCrsRemoved, this, &QgsRecentCoordinateReferenceSystemsModel::recentCrsRemoved );
  connect( QgsApplication::coordinateReferenceSystemRegistry(), &QgsCoordinateReferenceSystemRegistry::recentCrsCleared, this, &QgsRecentCoordinateReferenceSystemsModel::recentCrsCleared );
}

Qt::ItemFlags QgsRecentCoordinateReferenceSystemsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    return Qt::ItemFlags();
  }

  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant QgsRecentCoordinateReferenceSystemsModel::data( const QModelIndex &index, int role ) const
{
  const QgsCoordinateReferenceSystem crs = QgsRecentCoordinateReferenceSystemsModel::crs( index );
  if ( !crs.isValid() )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
      return crs.userFriendlyIdentifier();

    default:
      break;
  }

  return QVariant();
}

int QgsRecentCoordinateReferenceSystemsModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mCrs.size();
}

int QgsRecentCoordinateReferenceSystemsModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QModelIndex QgsRecentCoordinateReferenceSystemsModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( row < 0 || row >= mCrs.size() || column != 0 || parent.isValid() )
    return QModelIndex();

  return createIndex( row, column );
}

QModelIndex QgsRecentCoordinateReferenceSystemsModel::parent( const QModelIndex & ) const
{
  return QModelIndex();
}

QgsCoordinateReferenceSystem QgsRecentCoordinateReferenceSystemsModel::crs( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QgsCoordinateReferenceSystem();

  return mCrs.value( index.row() );
}

void QgsRecentCoordinateReferenceSystemsModel::recentCrsPushed( const QgsCoordinateReferenceSystem &crs )
{
  const int currentRow = mCrs.indexOf( crs );
  if ( currentRow >= 0 )
  {
    // move operation
    beginMoveRows( QModelIndex(), currentRow, currentRow, QModelIndex(), 0 );
    mCrs.removeAt( currentRow );
    mCrs.insert( 0, crs );
    endMoveRows();
  }
  else
  {
    // add operation
    beginInsertRows( QModelIndex(), 0, 0 );
    mCrs.insert( 0, crs );
    endInsertRows();
  }
}

void QgsRecentCoordinateReferenceSystemsModel::recentCrsRemoved( const QgsCoordinateReferenceSystem &crs )
{
  const int currentRow = mCrs.indexOf( crs );
  if ( currentRow >= 0 )
  {
    beginRemoveRows( QModelIndex(), currentRow, currentRow );
    mCrs.removeAt( currentRow );
    endRemoveRows();
  }
}

void QgsRecentCoordinateReferenceSystemsModel::recentCrsCleared()
{
  beginResetModel();
  mCrs.clear();
  endResetModel();
}
