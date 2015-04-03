/***************************************************************************
                      qgscomposerattributetablemodel.cpp
                         --------------------
    begin                : April 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgscomposerattributetable.h"
#include "qgscomposerattributetablemodel.h"
#include "qgscomposertable.h"
#include "qgscomposertablecolumn.h"


//QgsComposerAttributeTableColumnModel

QgsComposerAttributeTableColumnModel::QgsComposerAttributeTableColumnModel( QgsComposerAttributeTable *composerTable, QObject *parent ) : QAbstractTableModel( parent )
    , mComposerTable( composerTable )
{

}

QgsComposerAttributeTableColumnModel::~QgsComposerAttributeTableColumnModel()
{

}

QModelIndex QgsComposerAttributeTableColumnModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    if (( *mComposerTable->columns() )[row] )
    {
      return createIndex( row, column, ( *mComposerTable->columns() )[row] );
    }
  }
  return QModelIndex();
}

QModelIndex QgsComposerAttributeTableColumnModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child );
  return QModelIndex();
}

int QgsComposerAttributeTableColumnModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mComposerTable->columns()->length();
}

int QgsComposerAttributeTableColumnModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 3;
}

QVariant QgsComposerAttributeTableColumnModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ||
       ( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole ) )
  {
    return QVariant();
  }

  if ( index.row() >= mComposerTable->columns()->length() )
  {
    return QVariant();
  }

  //get column for index
  QgsComposerTableColumn* column = columnFromIndex( index );
  if ( !column )
  {
    return QVariant();
  }

  if ( role == Qt::UserRole )
  {
    //user role stores reference in column object
    return qVariantFromValue( qobject_cast<QObject *>( column ) );
  }

  switch ( index.column() )
  {
    case 0:
      return column->attribute();
    case 1:
      return column->heading();
    case 2:
    {
      if ( role == Qt::DisplayRole )
      {
        switch ( column->hAlignment() )
        {
          case Qt::AlignHCenter:
            return tr( "Center" );
          case Qt::AlignRight:
            return tr( "Right" );
          case Qt::AlignLeft:
          default:
            return tr( "Left" );
        }
      }
      else
      {
        //edit role
        return column->hAlignment();
      }
    }

    default:
      return QVariant();
  }

}

QVariant QgsComposerAttributeTableColumnModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( !mComposerTable )
  {
    return QVariant();
  }

  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section );
    }
    else
    {
      switch ( section )
      {
        case 0:
          return QVariant( tr( "Attribute" ) );

        case 1:
          return QVariant( tr( "Heading" ) );

        case 2:
          return QVariant( tr( "Alignment" ) );

        default:
          return QVariant();
      }
    }
  }
  else
  {
    return QVariant();
  }
}

bool QgsComposerAttributeTableColumnModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole || !mComposerTable )
  {
    return false;
  }
  if ( index.row() >= mComposerTable->columns()->length() )
  {
    return false;
  }

  //get column for index
  QgsComposerTableColumn* column = columnFromIndex( index );
  if ( !column )
  {
    return false;
  }

  switch ( index.column() )
  {
    case 0:
      // also update column's heading, if it hasn't been customised
      if ( column->heading().isEmpty() || ( column->heading() == column->attribute() ) )
      {
        column->setHeading( value.toString() );
        emit dataChanged( createIndex( index.row(), 1 ), createIndex( index.row(), 1 ) );
      }
      column->setAttribute( value.toString() );
      emit dataChanged( index, index );
      return true;
    case 1:
      column->setHeading( value.toString() );
      emit dataChanged( index, index );
      return true;
    case 2:
      column->setHAlignment(( Qt::AlignmentFlag )value.toInt() );
      emit dataChanged( index, index );
      return true;
    default:
      break;
  }

  return false;
}

Qt::ItemFlags QgsComposerAttributeTableColumnModel::flags( const QModelIndex& index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( index.isValid() )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
}

bool QgsComposerAttributeTableColumnModel::removeRows( int row, int count, const QModelIndex& parent )
{
  Q_UNUSED( parent );

  int maxRow = qMin( row + count - 1, mComposerTable->columns()->length() - 1 );
  beginRemoveRows( QModelIndex(), row, maxRow );
  //move backwards through rows, removing each corresponding QgsComposerTableColumn
  for ( int i = maxRow; i >= row; --i )
  {
    delete( *mComposerTable->columns() )[i];
    mComposerTable->columns()->removeAt( i );
  }
  endRemoveRows();
  return true;
}

bool QgsComposerAttributeTableColumnModel::insertRows( int row, int count, const QModelIndex& parent )
{
  Q_UNUSED( parent );
  beginInsertRows( QModelIndex(), row, row + count - 1 );
  //create new QgsComposerTableColumns for each inserted row
  for ( int i = row; i < row + count; ++i )
  {
    QgsComposerTableColumn* col = new QgsComposerTableColumn;
    mComposerTable->columns()->insert( i, col );
  }
  endInsertRows();
  return true;
}

bool QgsComposerAttributeTableColumnModel::moveRow( int row, ShiftDirection direction )
{
  if (( direction == ShiftUp && row <= 0 ) ||
      ( direction == ShiftDown &&  row >= rowCount() - 1 ) )
  {
    //row is already at top/bottom
    return false;
  }

  //we shift a row by removing the next row up/down, then reinserting it before/after the target row
  int swapWithRow = direction == ShiftUp ? row - 1 : row + 1;

  //remove row
  beginRemoveRows( QModelIndex(), swapWithRow, swapWithRow );
  QgsComposerTableColumn* temp = mComposerTable->columns()->takeAt( swapWithRow );
  endRemoveRows();

  //insert row
  beginInsertRows( QModelIndex(), row, row );
  mComposerTable->columns()->insert( row, temp );
  endInsertRows();

  return true;
}

void QgsComposerAttributeTableColumnModel::resetToLayer()
{
  beginResetModel();
  mComposerTable->resetColumns();
  endResetModel();
}

QgsComposerTableColumn* QgsComposerAttributeTableColumnModel::columnFromIndex( const QModelIndex &index ) const
{
  QgsComposerTableColumn* column = static_cast<QgsComposerTableColumn*>( index.internalPointer() );
  return column;
}

QModelIndex QgsComposerAttributeTableColumnModel::indexFromColumn( QgsComposerTableColumn* column )
{
  if ( !mComposerTable )
  {
    return QModelIndex();
  }

  int r = mComposerTable->columns()->indexOf( column );

  QModelIndex idx = index( r, 0, QModelIndex() );
  if ( idx.isValid() )
  {
    return idx;
  }

  return QModelIndex();
}

void QgsComposerAttributeTableColumnModel::setColumnAsSorted( QgsComposerTableColumn* column, Qt::SortOrder order )
{
  if ( !column || !mComposerTable )
  {
    return;
  }

  //find current highest sort by rank
  int highestRank = 0;
  QList<QgsComposerTableColumn*>::const_iterator columnIt = mComposerTable->columns()->constBegin();
  for ( ; columnIt != mComposerTable->columns()->constEnd(); ++columnIt )
  {
    highestRank = qMax( highestRank, ( *columnIt )->sortByRank() );
  }

  column->setSortByRank( highestRank + 1 );
  column->setSortOrder( order );

  QModelIndex idx = indexFromColumn( column );
  emit dataChanged( idx, idx );
}

void QgsComposerAttributeTableColumnModel::setColumnAsUnsorted( QgsComposerTableColumn * column )
{
  if ( !mComposerTable || !column )
  {
    return;
  }

  column->setSortByRank( 0 );
  QModelIndex idx = indexFromColumn( column );
  emit dataChanged( idx, idx );
}

static bool columnsBySortRank( QgsComposerTableColumn * a, QgsComposerTableColumn * b )
{
  return a->sortByRank() < b->sortByRank();
}

bool QgsComposerAttributeTableColumnModel::moveColumnInSortRank( QgsComposerTableColumn * column, ShiftDirection direction )
{
  if ( !mComposerTable || !column )
  {
    return false;
  }
  if (( direction == ShiftUp && column->sortByRank() <= 1 )
      || ( direction == ShiftDown && column->sortByRank() <= 0 ) )
  {
    //already at start/end of list or not being used for sort
    return false;
  }

  //find column before this one in sort order
  QList<QgsComposerTableColumn*> sortedColumns;
  QList<QgsComposerTableColumn*>::iterator columnIt = mComposerTable->columns()->begin();
  for ( ; columnIt != mComposerTable->columns()->end(); ++columnIt )
  {
    if (( *columnIt )->sortByRank() > 0 )
    {
      sortedColumns.append( *columnIt );
    }
  }
  qStableSort( sortedColumns.begin(), sortedColumns.end(), columnsBySortRank );
  int columnPos = sortedColumns.indexOf( column );

  if (( columnPos == 0 && direction == ShiftUp )
      || (( columnPos == sortedColumns.length() - 1 ) && direction == ShiftDown ) )
  {
    //column already at start/end
    return false;
  }

  QgsComposerTableColumn* swapColumn = direction == ShiftUp ?
                                       sortedColumns[ columnPos - 1]
                                       : sortedColumns[ columnPos + 1];
  QModelIndex idx = indexFromColumn( column );
  QModelIndex idxSwap = indexFromColumn( swapColumn );

  //now swap sort ranks
  int oldSortRank = column->sortByRank();
  column->setSortByRank( swapColumn->sortByRank() );
  emit dataChanged( idx, idx );

  swapColumn->setSortByRank( oldSortRank );
  emit dataChanged( idxSwap, idxSwap );

  return true;
}



//QgsComposerTableSortColumnsProxyModel

QgsComposerTableSortColumnsProxyModel::QgsComposerTableSortColumnsProxyModel( QgsComposerAttributeTable *composerTable, ColumnFilterType filterType, QObject *parent )
    : QSortFilterProxyModel( parent )
    , mComposerTable( composerTable )
    , mFilterType( filterType )
{
  setDynamicSortFilter( true );
}

QgsComposerTableSortColumnsProxyModel::~QgsComposerTableSortColumnsProxyModel()
{

}

bool QgsComposerTableSortColumnsProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  //get QgsComposerTableColumn corresponding to row
  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  QgsComposerTableColumn* column = columnFromSourceIndex( index );

  if ( !column )
  {
    return false;
  }

  if (( column->sortByRank() > 0 && mFilterType == ShowSortedColumns )
      || ( column->sortByRank() <= 0 && mFilterType == ShowUnsortedColumns ) )
  {
    //column matches filter type
    return true;
  }
  else
  {
    return false;
  }
}

QgsComposerTableColumn *QgsComposerTableSortColumnsProxyModel::columnFromIndex( const QModelIndex &index ) const
{
  //get column corresponding to an index from the proxy
  QModelIndex sourceIndex = mapToSource( index );
  return columnFromSourceIndex( sourceIndex );
}

QgsComposerTableColumn* QgsComposerTableSortColumnsProxyModel::columnFromSourceIndex( const QModelIndex &sourceIndex ) const
{
  //get column corresponding to an index from the source model
  QVariant columnAsVariant = sourceModel()->data( sourceIndex, Qt::UserRole );
  QgsComposerTableColumn* column = qobject_cast<QgsComposerTableColumn *>( columnAsVariant.value<QObject *>() );
  return column;
}

bool QgsComposerTableSortColumnsProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  QgsComposerTableColumn* column1 = columnFromSourceIndex( left );
  QgsComposerTableColumn* column2 = columnFromSourceIndex( right );
  if ( !column1 )
  {
    return false;
  }
  if ( !column2 )
  {
    return true;
  }
  return column1->sortByRank() < column2->sortByRank();
}

int QgsComposerTableSortColumnsProxyModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 2;
}

QVariant QgsComposerTableSortColumnsProxyModel::data( const QModelIndex &index, int role ) const
{
  if (( role != Qt::DisplayRole && role != Qt::EditRole ) || !index.isValid() )
  {
    return QVariant();
  }

  QgsComposerTableColumn* column = columnFromIndex( index );
  if ( !column )
  {
    return QVariant();
  }

  switch ( index.column() )
  {
    case 0:
      return column->attribute();
    case 1:
      if ( role == Qt::DisplayRole )
      {
        switch ( column->sortOrder() )
        {
          case Qt::DescendingOrder:
            return tr( "Descending" );
          case Qt::AscendingOrder:
          default:
            return tr( "Ascending" );
        }
      }
      else
      {
        //edit role
        return column->sortOrder();
      }

    default:
      return QVariant();
  }
}

QVariant QgsComposerTableSortColumnsProxyModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( !mComposerTable )
  {
    return QVariant();
  }

  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section );
    }
    else
    {
      switch ( section )
      {
        case 0:
          return QVariant( tr( "Attribute" ) );

        case 1:
          return QVariant( tr( "Sort Order" ) );

        default:
          return QVariant();
      }
    }
  }
  else
  {
    return QVariant();
  }
}

Qt::ItemFlags QgsComposerTableSortColumnsProxyModel::flags( const QModelIndex& index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( index.column() == 1 )
  {
    //only sort order is editable
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

bool QgsComposerTableSortColumnsProxyModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  if ( !mComposerTable )
  {
    return false;
  }

  QgsComposerTableColumn* column = columnFromIndex( index );
  if ( !column )
  {
    return false;
  }

  if ( index.column() == 1 )
  {
    column->setSortOrder(( Qt::SortOrder )value.toInt() );
    emit dataChanged( index, index );
    return true;
  }

  return false;
}

QgsComposerTableColumn *QgsComposerTableSortColumnsProxyModel::columnFromRow( int row )
{
  QModelIndex proxyIndex = index( row, 0 );
  return columnFromIndex( proxyIndex );
}

void QgsComposerTableSortColumnsProxyModel::resetFilter()
{
  invalidate();
}
