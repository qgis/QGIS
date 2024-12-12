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
#include "moc_qgsrecentcoordinatereferencesystemsmodel.cpp"
#include "qgscoordinatereferencesystemregistry.h"
#include "qgsapplication.h"

#include <QFont>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

QgsRecentCoordinateReferenceSystemsModel::QgsRecentCoordinateReferenceSystemsModel( QObject *parent, int subclassColumnCount )
  : QAbstractItemModel( parent )
  , mColumnCount( subclassColumnCount )
{
#ifdef ENABLE_MODELTEST
  new ModelTest( this, this );
#endif

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

  if ( index.column() == 0 )
  {
    switch ( role )
    {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
        return crs.userFriendlyIdentifier();

      case static_cast<int>( CustomRole::Crs ):
        return crs;

      case static_cast<int>( CustomRole::AuthId ):
        return crs.authid();

      default:
        break;
    }
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
  return mColumnCount;
}

QModelIndex QgsRecentCoordinateReferenceSystemsModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( row < 0 || row >= mCrs.size() || column < 0 || column >= columnCount( parent ) || parent.isValid() )
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
  if ( currentRow > 0 )
  {
    // move operation
    beginMoveRows( QModelIndex(), currentRow, currentRow, QModelIndex(), 0 );
    mCrs.removeAt( currentRow );
    mCrs.insert( 0, crs );
    endMoveRows();
  }
  else if ( currentRow < 0 )
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


//
// QgsRecentCoordinateReferenceSystemsProxyModel
//

QgsRecentCoordinateReferenceSystemsProxyModel::QgsRecentCoordinateReferenceSystemsProxyModel( QObject *parent, int subclassColumnCount )
  : QSortFilterProxyModel( parent )
  , mModel( new QgsRecentCoordinateReferenceSystemsModel( this, subclassColumnCount ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
}

QgsRecentCoordinateReferenceSystemsModel *QgsRecentCoordinateReferenceSystemsProxyModel::recentCoordinateReferenceSystemsModel()
{
  return mModel;
}

const QgsRecentCoordinateReferenceSystemsModel *QgsRecentCoordinateReferenceSystemsProxyModel::recentCoordinateReferenceSystemsModel() const
{
  return mModel;
}

void QgsRecentCoordinateReferenceSystemsProxyModel::setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters )
{
  if ( mFilters == filters )
    return;

  mFilters = filters;
  invalidateFilter();
}

void QgsRecentCoordinateReferenceSystemsProxyModel::setFilterDeprecated( bool filter )
{
  if ( mFilterDeprecated == filter )
    return;

  mFilterDeprecated = filter;
  invalidateFilter();
}

void QgsRecentCoordinateReferenceSystemsProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}

bool QgsRecentCoordinateReferenceSystemsProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( !mFilters )
    return true;

  const QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );

  const QgsCoordinateReferenceSystem crs = mModel->crs( sourceIndex );
  if ( mFilterDeprecated && crs.isDeprecated() )
    return false;

  const Qgis::CrsType type = crs.type();
  switch ( type )
  {
    case Qgis::CrsType::Unknown:
    case Qgis::CrsType::Other:
      break;

    case Qgis::CrsType::Geodetic:
    case Qgis::CrsType::Geocentric:
    case Qgis::CrsType::Geographic2d:
    case Qgis::CrsType::Geographic3d:
    case Qgis::CrsType::Projected:
    case Qgis::CrsType::Temporal:
    case Qgis::CrsType::Engineering:
    case Qgis::CrsType::Bound:
    case Qgis::CrsType::DerivedProjected:
      if ( !mFilters.testFlag( QgsCoordinateReferenceSystemProxyModel::Filter::FilterHorizontal ) )
        return false;
      break;

    case Qgis::CrsType::Vertical:
      if ( !mFilters.testFlag( QgsCoordinateReferenceSystemProxyModel::Filter::FilterVertical ) )
        return false;
      break;

    case Qgis::CrsType::Compound:
      if ( !mFilters.testFlag( QgsCoordinateReferenceSystemProxyModel::Filter::FilterCompound ) )
        return false;
      break;
  }

  if ( !mFilterString.trimmed().isEmpty() )
  {
    if ( !( crs.description().contains( mFilterString, Qt::CaseInsensitive )
            || crs.authid().contains( mFilterString, Qt::CaseInsensitive ) ) )
      return false;
  }

  return true;
}

QgsCoordinateReferenceSystem QgsRecentCoordinateReferenceSystemsProxyModel::crs( const QModelIndex &index ) const
{
  const QModelIndex sourceIndex = mapToSource( index );
  return mModel->crs( sourceIndex );
}
