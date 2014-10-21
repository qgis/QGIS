/***************************************************************************
                      qgscomposerattributetablemodelv2.cpp
                         --------------------
    begin                : September 2014
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

#include "qgscomposerattributetablev2.h"
#include "qgscomposerattributetablemodelv2.h"
#include "qgscomposertablev2.h"
#include "qgscomposertablecolumn.h"


//QgsComposerAttributeTableColumnModelV2V2

QgsComposerAttributeTableColumnModelV2::QgsComposerAttributeTableColumnModelV2( QgsComposerAttributeTableV2 *composerTable, QObject *parent ) : QAbstractTableModel( parent )
    , mComposerTable( composerTable )
{

}

QgsComposerAttributeTableColumnModelV2::~QgsComposerAttributeTableColumnModelV2()
{

}

QModelIndex QgsComposerAttributeTableColumnModelV2::index( int row, int column, const QModelIndex &parent ) const
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

QModelIndex QgsComposerAttributeTableColumnModelV2::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child );
  return QModelIndex();
}

int QgsComposerAttributeTableColumnModelV2::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mComposerTable->columns()->length();
}

int QgsComposerAttributeTableColumnModelV2::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 4;
}

QVariant QgsComposerAttributeTableColumnModelV2::data( const QModelIndex &index, int role ) const
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
    case 3:
    {
      if ( role == Qt::DisplayRole )
      {
        return column->width() <= 0 ? tr( "Automatic" ) : QString( tr( "%1 mm" ) ).arg( column->width(), 0, 'f', 2 );
      }
      else
      {
        //edit role
        return column->width();
      }
    }
    default:
      return QVariant();
  }

}

QVariant QgsComposerAttributeTableColumnModelV2::headerData( int section, Qt::Orientation orientation, int role ) const
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

        case 3:
          return QVariant( tr( "Width" ) );

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

bool QgsComposerAttributeTableColumnModelV2::setData( const QModelIndex& index, const QVariant& value, int role )
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
        emit dataChanged( createIndex( index.row(), 1, 0 ), createIndex( index.row(), 1, 0 ) );
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
    case 3:
      column->setWidth( value.toDouble() );
      emit dataChanged( index, index );
      return true;
    default:
      break;
  }

  return false;
}

Qt::ItemFlags QgsComposerAttributeTableColumnModelV2::flags( const QModelIndex& index ) const
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

bool QgsComposerAttributeTableColumnModelV2::removeRows( int row, int count, const QModelIndex& parent )
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

bool QgsComposerAttributeTableColumnModelV2::insertRows( int row, int count, const QModelIndex& parent )
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

bool QgsComposerAttributeTableColumnModelV2::moveRow( int row, ShiftDirection direction )
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

void QgsComposerAttributeTableColumnModelV2::resetToLayer()
{
  beginResetModel();
  mComposerTable->resetColumns();
  endResetModel();
}

QgsComposerTableColumn* QgsComposerAttributeTableColumnModelV2::columnFromIndex( const QModelIndex &index ) const
{
  QgsComposerTableColumn* column = static_cast<QgsComposerTableColumn*>( index.internalPointer() );
  return column;
}

QModelIndex QgsComposerAttributeTableColumnModelV2::indexFromColumn( QgsComposerTableColumn* column )
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

void QgsComposerAttributeTableColumnModelV2::setColumnAsSorted( QgsComposerTableColumn* column, Qt::SortOrder order )
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

void QgsComposerAttributeTableColumnModelV2::setColumnAsUnsorted( QgsComposerTableColumn * column )
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

bool QgsComposerAttributeTableColumnModelV2::moveColumnInSortRank( QgsComposerTableColumn * column, ShiftDirection direction )
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



//QgsComposerTableSortColumnsProxyModelV2V2

QgsComposerTableSortColumnsProxyModelV2::QgsComposerTableSortColumnsProxyModelV2( QgsComposerAttributeTableV2 *composerTable, ColumnFilterType filterType, QObject *parent )
    : QSortFilterProxyModel( parent )
    , mComposerTable( composerTable )
    , mFilterType( filterType )
{
  setDynamicSortFilter( true );
}

QgsComposerTableSortColumnsProxyModelV2::~QgsComposerTableSortColumnsProxyModelV2()
{

}

bool QgsComposerTableSortColumnsProxyModelV2::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
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

QgsComposerTableColumn *QgsComposerTableSortColumnsProxyModelV2::columnFromIndex( const QModelIndex &index ) const
{
  //get column corresponding to an index from the proxy
  QModelIndex sourceIndex = mapToSource( index );
  return columnFromSourceIndex( sourceIndex );
}

QgsComposerTableColumn* QgsComposerTableSortColumnsProxyModelV2::columnFromSourceIndex( const QModelIndex &sourceIndex ) const
{
  //get column corresponding to an index from the source model
  QVariant columnAsVariant = sourceModel()->data( sourceIndex, Qt::UserRole );
  QgsComposerTableColumn* column = qobject_cast<QgsComposerTableColumn *>( columnAsVariant.value<QObject *>() );
  return column;
}

bool QgsComposerTableSortColumnsProxyModelV2::lessThan( const QModelIndex &left, const QModelIndex &right ) const
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

int QgsComposerTableSortColumnsProxyModelV2::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 2;
}

QVariant QgsComposerTableSortColumnsProxyModelV2::data( const QModelIndex &index, int role ) const
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

QVariant QgsComposerTableSortColumnsProxyModelV2::headerData( int section, Qt::Orientation orientation, int role ) const
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

Qt::ItemFlags QgsComposerTableSortColumnsProxyModelV2::flags( const QModelIndex& index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( index.column() == 1 )
  {
    //only sort order is editable
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

bool QgsComposerTableSortColumnsProxyModelV2::setData( const QModelIndex& index, const QVariant& value, int role )
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

QgsComposerTableColumn *QgsComposerTableSortColumnsProxyModelV2::columnFromRow( int row )
{
  QModelIndex proxyIndex = index( row, 0 );
  return columnFromIndex( proxyIndex );
}

void QgsComposerTableSortColumnsProxyModelV2::resetFilter()
{
  invalidate();
}
