// This file is part of CppSheets.
//
// Copyright 2018 Patrick Flynn <patrick_dev2000@outlook.com>
//
// CppSheets is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CppSheets is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CppSheets. If not, see <https://www.gnu.org/licenses/>.

#include "qgstableeditorwidget.h"
#include "qgsnumericformat.h"
#include <QStringList>
#include <QKeyEvent>
#include <QHeaderView>
#include <QMenu>

QgsTableEditorWidget::QgsTableEditorWidget( QWidget *parent )
  : QTableWidget( parent )
{
  mHeaderMenu = new QMenu( this );
  setColumnCount( 0 );
  setRowCount( 0 );
  connect( this, &QgsTableEditorWidget::cellChanged, this, [ = ]
  {
    if ( !mBlockSignals )
      emit tableChanged();
  } );

  horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( horizontalHeader(), &QWidget::customContextMenuRequested, this, [ = ]( const QPoint & point )
  {
    const int column = horizontalHeader()->logicalIndexAt( point.x() );

    QSet< int > selectedColumns;
    for ( const QModelIndex &index : selectedIndexes() )
    {
      selectedColumns.insert( index.column() );
    }
    int minCol = 0;
    int maxCol = 0;
    bool isConsecutive = collectConsecutiveColumnRange( selectedIndexes(), minCol, maxCol );

    // this is modeled off Libreoffice calc!
    if ( selectedIndexes().count() == 1 )
    {
      // select whole column
      selectColumn( column );
      isConsecutive = true;
    }
    else if ( !selectedColumns.contains( column ) )
    {
      // select whole column
      selectColumn( column );
      isConsecutive = true;
    }

    mHeaderMenu->clear();
    if ( isConsecutive )
    {
      QAction *insertBefore = mHeaderMenu->addAction( tr( "Insert Columns Before" ) );
      connect( insertBefore, &QAction::triggered, this, &QgsTableEditorWidget::insertColumnsBefore );
      QAction *insertAfter = mHeaderMenu->addAction( tr( "Insert Columns After" ) );
      connect( insertAfter, &QAction::triggered, this, &QgsTableEditorWidget::insertColumnsAfter );
    }
    QAction *deleteSelected = mHeaderMenu->addAction( tr( "Delete Columns" ) );
    connect( deleteSelected, &QAction::triggered, this, &QgsTableEditorWidget::deleteColumns );

    mHeaderMenu->popup( horizontalHeader()->mapToGlobal( point ) );
  } );

  verticalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( verticalHeader(), &QWidget::customContextMenuRequested, this, [ = ]( const QPoint & point )
  {
    const int row = verticalHeader()->logicalIndexAt( point.y() );

    QSet< int > selectedRows;
    for ( const QModelIndex &index : selectedIndexes() )
    {
      selectedRows.insert( index.row() );
    }
    int minRow = 0;
    int maxRow = 0;
    bool isConsecutive = collectConsecutiveRowRange( selectedIndexes(), minRow, maxRow );

    // this is modeled off Libreoffice calc!
    if ( selectedIndexes().count() == 1 )
    {
      // select whole row
      selectRow( row );
      isConsecutive = true;
    }
    else if ( !selectedRows.contains( row ) )
    {
      // select whole row
      selectRow( row );
      isConsecutive = true;
    }

    mHeaderMenu->clear();
    if ( isConsecutive )
    {
      QAction *insertBefore = mHeaderMenu->addAction( tr( "Insert Rows Above" ) );
      connect( insertBefore, &QAction::triggered, this, &QgsTableEditorWidget::insertRowsAbove );
      QAction *insertAfter = mHeaderMenu->addAction( tr( "Insert Rows Below" ) );
      connect( insertAfter, &QAction::triggered, this, &QgsTableEditorWidget::insertRowsBelow );
    }
    QAction *deleteSelected = mHeaderMenu->addAction( tr( "Delete Rows" ) );
    connect( deleteSelected, &QAction::triggered, this, &QgsTableEditorWidget::deleteRows );

    mHeaderMenu->popup( verticalHeader()->mapToGlobal( point ) );
  } );

  connect( selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsTableEditorWidget::activeCellChanged );
}

QgsTableEditorWidget::~QgsTableEditorWidget()
{
  qDeleteAll( mNumericFormats );
}

void QgsTableEditorWidget::updateHeaders()
{
  QStringList headers;
  QStringList letters;

  QString first;
  QString current;

  for ( char c = 'A'; c <= 'Z'; c++ )
  {
    letters.push_back( QString( c ) );
  }

  int len = letters.length();
  int index = 0;
  int fIndex = 0;

  for ( int i = 0; i < 1000; i++ )
  {
    if ( index == len )
    {
      index = 0;

      first = letters.at( fIndex );
      fIndex++;

      if ( fIndex == len )
      {
        fIndex = 0;
      }
    }

    current = first;
    current += letters.at( index );
    headers.push_back( current );
    current.clear();

    index++;
  }

  setHorizontalHeaderLabels( headers );
}

bool QgsTableEditorWidget::collectConsecutiveRowRange( const QModelIndexList &list, int &minRow, int &maxRow ) const
{
  QSet< int > includedRows;
  minRow = std::numeric_limits< int >::max();
  maxRow = -1;
  for ( const QModelIndex &index : list )
  {
    includedRows.insert( index.row() );
    minRow = std::min( minRow, index.row() );
    maxRow = std::max( maxRow, index.row() );
  }

  // test that selection is consecutive rows
  for ( int r = minRow + 1; r < maxRow; r++ )
  {
    if ( !includedRows.contains( r ) )
      return false;
  }
  return true;
}

bool QgsTableEditorWidget::collectConsecutiveColumnRange( const QModelIndexList &list, int &minColumn, int &maxColumn ) const
{
  QSet< int > includedColumns;
  minColumn = std::numeric_limits< int >::max();
  maxColumn = -1;
  for ( const QModelIndex &index : list )
  {
    includedColumns.insert( index.column() );
    minColumn = std::min( minColumn, index.column() );
    maxColumn = std::max( maxColumn, index.column() );
  }

  // test that selection is consecutive columns
  for ( int r = minColumn + 1; r < maxColumn; r++ )
  {
    if ( !includedColumns.contains( r ) )
      return false;
  }
  return true;
}

QList<int> QgsTableEditorWidget::collectUniqueRows( const QModelIndexList &list ) const
{
  QList<int > res;
  for ( const QModelIndex &index : list )
  {
    if ( !res.contains( index.row() ) )
      res << index.row();
  }
  std::sort( res.begin(), res.end() );
  return res;
}

QList<int> QgsTableEditorWidget::collectUniqueColumns( const QModelIndexList &list ) const
{
  QList<int > res;
  for ( const QModelIndex &index : list )
  {
    if ( !res.contains( index.column() ) )
      res << index.column();
  }
  std::sort( res.begin(), res.end() );
  return res;
}

void QgsTableEditorWidget::keyPressEvent( QKeyEvent *event )
{
  switch ( event->key() )
  {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
      //Enter or return keys moves to next row
      QTableWidget::keyPressEvent( event );
      setCurrentCell( currentRow() + 1, currentColumn() );
      break;
    }

    case Qt::Key_Delete:
    {
      clearSelectedCells();
      break;
    }

    default:
      QTableWidget::keyPressEvent( event );
  }
}

void QgsTableEditorWidget::setTableContents( const QgsTableContents &contents )
{
  mBlockSignals++;
  qDeleteAll( mNumericFormats );
  mNumericFormats.clear();

  int rowNumber = 0;
  setRowCount( contents.size() );
  for ( const QgsTableRow &row : contents )
  {
    if ( rowNumber == 0 )
      setColumnCount( row.size() );

    int colNumber = 0;
    for ( const QgsTableCell &col : row )
    {
      QTableWidgetItem *item = new QTableWidgetItem( col.content().toString() );
      item->setData( Qt::BackgroundRole, col.backgroundColor().isValid() ? col.backgroundColor() : QColor( 255, 255, 255 ) );
      item->setData( PresetBackgroundColorRole, col.backgroundColor().isValid() ? col.backgroundColor() : QVariant() );
      item->setData( Qt::ForegroundRole, col.foregroundColor().isValid() ? col.foregroundColor() : QVariant() );
      if ( col.numericFormat() )
        mNumericFormats.insert( item, col.numericFormat()->clone() );
      setItem( rowNumber, colNumber, item );
      colNumber++;
    }
    rowNumber++;
  }

  mBlockSignals--;
  updateHeaders();
  emit tableChanged();
}

QgsTableContents QgsTableEditorWidget::tableContents() const
{
  QgsTableContents items;
  items.reserve( rowCount() );

  for ( int r = 0; r < rowCount(); r++ )
  {
    QgsTableRow row;
    row.reserve( columnCount() );
    for ( int c = 0; c < columnCount(); c++ )
    {
      QgsTableCell cell;
      if ( QTableWidgetItem *i = item( r, c ) )
      {
        cell.setContent( i->text() );
        cell.setBackgroundColor( i->data( PresetBackgroundColorRole ).value< QColor >() );
        cell.setForegroundColor( i->data( Qt::ForegroundRole ).value< QColor >() );

        if ( mNumericFormats.value( i ) )
        {
          cell.setNumericFormat( mNumericFormats.value( i )->clone() );
        }
      }
      row.push_back( cell );
    }
    items.push_back( row );
  }

  return items;
}

void QgsTableEditorWidget::setSelectionNumericFormat( QgsNumericFormat *format )
{
  bool changed = false;
  mBlockSignals++;
  std::unique_ptr< QgsNumericFormat > newFormat( format );
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    QTableWidgetItem *i = item( index.row(), index.column() );
    if ( !i )
    {
      i = new QTableWidgetItem();
      setItem( index.row(), index.column(), i );
    }
    if ( !mNumericFormats.value( i ) && newFormat )
    {
      changed = true;
      mNumericFormats.insert( i, newFormat->clone() );
    }
    else if ( mNumericFormats.value( i ) && !newFormat )
    {
      changed = true;
      delete mNumericFormats.value( i );
      mNumericFormats.remove( i );
    }
    else if ( newFormat && *newFormat != *mNumericFormats.value( i ) )
    {
      changed = true;
      delete mNumericFormats.value( i );
      mNumericFormats.insert( i, newFormat->clone() );
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

QgsNumericFormat *QgsTableEditorWidget::selectionNumericFormat()
{
  QgsNumericFormat *f = nullptr;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      if ( first )
      {
        f = mNumericFormats.value( i );
        first = false;
      }
      else if ( ( !f && !mNumericFormats.value( i ) )
                || ( f && mNumericFormats.value( i ) && *f == *mNumericFormats.value( i ) ) )
        continue;
      else
      {
        return nullptr;
      }
    }
    else
    {
      return nullptr;
    }
  }
  return f;
}

bool QgsTableEditorWidget::hasMixedSelectionNumericFormat()
{
  QgsNumericFormat *f = nullptr;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      if ( first )
      {
        f = mNumericFormats.value( i );
        first = false;
      }
      else if ( ( !f && !mNumericFormats.value( i ) )
                || ( f && mNumericFormats.value( i ) && *f == *mNumericFormats.value( i ) ) )
        continue;
      else
      {
        return true;
      }
    }
    else if ( f )
    {
      return true;
    }
  }
  return false;
}

QColor QgsTableEditorWidget::selectionForegroundColor()
{
  QColor c;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    QColor indexColor = model()->data( index, Qt::ForegroundRole ).isValid() ? model()->data( index, Qt::ForegroundRole ).value< QColor >() : QColor();
    if ( first )
    {
      c = indexColor;
      first = false;
    }
    else if ( indexColor == c )
      continue;
    else
    {
      return QColor();
    }
  }
  return c;
}

QColor QgsTableEditorWidget::selectionBackgroundColor()
{
  QColor c;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    QColor indexColor = model()->data( index, PresetBackgroundColorRole ).isValid() ? model()->data( index, PresetBackgroundColorRole ).value< QColor >() : QColor();
    if ( first )
    {
      c = indexColor;
      first = false;
    }
    else if ( indexColor == c )
      continue;
    else
    {
      return QColor();
    }
  }
  return c;
}

double QgsTableEditorWidget::selectionRowHeight()
{
  double height = 0;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    double thisHeight = tableRowHeight( index.row() );
    if ( first )
      height = thisHeight;
    else if ( thisHeight != height )
    {
      return -1;
    }
    first = false;
  }
  return height;
}

double QgsTableEditorWidget::selectionColumnWidth()
{
  double width = 0;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    double thisWidth = tableColumnWidth( index.column() );
    if ( first )
      width = thisWidth;
    else if ( thisWidth != width )
    {
      return -1;
    }
    first = false;
  }
  return width;
}

double QgsTableEditorWidget::tableRowHeight( int row )
{
  double height = 0;
  for ( int col = 0; col < columnCount(); ++col )
  {
    double thisHeight = model()->data( model()->index( row, col ), RowHeight ).toDouble();
    height = std::max( thisHeight, height );
  }
  return height;
}

double QgsTableEditorWidget::tableColumnWidth( int column )
{
  double width = 0;
  for ( int row = 0; row < rowCount(); ++row )
  {
    double thisWidth = model()->data( model()->index( row, column ), ColumnWidth ).toDouble();
    width = std::max( thisWidth, width );
  }
  return width;
}

void QgsTableEditorWidget::setTableRowHeight( int row, double height )
{
  bool changed = false;
  mBlockSignals++;

  for ( int col = 0; col < columnCount(); ++col )
  {
    if ( QTableWidgetItem *i = item( row, col ) )
    {
      if ( i->data( RowHeight ).toDouble() != height )
      {
        i->setData( RowHeight, height );
        changed = true;
      }
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem();
      newItem->setData( RowHeight, height );
      setItem( row, col, newItem );
      changed = true;
    }
  }

  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::setTableColumnWidth( int col, double width )
{
  bool changed = false;
  mBlockSignals++;
  for ( int row = 0; row < rowCount(); ++row )
  {
    if ( QTableWidgetItem *i = item( row, col ) )
    {
      if ( i->data( ColumnWidth ).toDouble() != width )
      {
        i->setData( ColumnWidth, width );
        changed = true;
      }
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem();
      newItem->setData( ColumnWidth, width );
      setItem( row, col, newItem );
      changed = true;
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::insertRowsBelow()
{
  if ( rowCount() == 0 )
  {
    insertRow( 0 );
    return;
  }

  int minRow = 0;
  int maxRow = 0;
  if ( !collectConsecutiveRowRange( selectedIndexes(), minRow, maxRow ) )
    return;

  const int rowsToInsert = maxRow - minRow + 1;
  for ( int i = 0; i < rowsToInsert; ++i )
    insertRow( maxRow + 1 );

  updateHeaders();
  if ( !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::insertRowsAbove()
{
  if ( rowCount() == 0 )
  {
    insertRow( 0 );
    return;
  }

  int minRow = 0;
  int maxRow = 0;
  if ( !collectConsecutiveRowRange( selectedIndexes(), minRow, maxRow ) )
    return;

  const int rowsToInsert = maxRow - minRow + 1;
  for ( int i = 0; i < rowsToInsert; ++i )
    insertRow( minRow );

  updateHeaders();
  if ( !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::insertColumnsBefore()
{
  if ( columnCount() == 0 )
  {
    insertColumn( 0 );
    return;
  }

  int minColumn = 0;
  int maxColumn = 0;
  if ( !collectConsecutiveColumnRange( selectedIndexes(), minColumn, maxColumn ) )
    return;

  const int columnsToInsert = maxColumn - minColumn + 1;
  for ( int i = 0; i < columnsToInsert; ++i )
    insertColumn( minColumn );

  updateHeaders();
  if ( !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::insertColumnsAfter()
{
  if ( columnCount() == 0 )
  {
    insertColumn( 0 );
    return;
  }

  int minColumn = 0;
  int maxColumn = 0;
  if ( !collectConsecutiveColumnRange( selectedIndexes(), minColumn, maxColumn ) )
    return;

  const int columnsToInsert = maxColumn - minColumn + 1;
  for ( int i = 0; i < columnsToInsert; ++i )
    insertColumn( maxColumn + 1 );

  updateHeaders();
  if ( !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::deleteRows()
{
  const QList< int > rows = collectUniqueRows( selectedIndexes() );
  if ( rows.empty() )
    return;

  bool changed = false;
  for ( int i = rows.size() - 1; i >= 0 && rowCount() > 1; i-- )
  {
    removeRow( rows.at( i ) );
    changed = true;
  }
  updateHeaders();
  if ( changed &&  !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::deleteColumns()
{
  const QList< int > columns = collectUniqueColumns( selectedIndexes() );
  if ( columns.empty() )
    return;

  bool changed = false;
  for ( int i = columns.size() - 1; i >= 0 && columnCount() > 1; i-- )
  {
    removeColumn( columns.at( i ) );
    changed = true;
  }
  updateHeaders();
  if ( !mBlockSignals && changed )
    emit tableChanged();
}

void QgsTableEditorWidget::expandRowSelection()
{
  const QModelIndexList s = selectedIndexes();
  for ( const QModelIndex &index : s )
  {
    selectionModel()->select( index, QItemSelectionModel::Rows | QItemSelectionModel::Select );
  }
}

void QgsTableEditorWidget::expandColumnSelection()
{
  const QModelIndexList s = selectedIndexes();
  for ( const QModelIndex &index : s )
  {
    selectionModel()->select( index, QItemSelectionModel::Columns | QItemSelectionModel::Select );
  }
}

void QgsTableEditorWidget::clearSelectedCells()
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  for ( const QModelIndex &index : selection )
  {
    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      i->setText( QString() );
      changed = true;
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::setSelectionForegroundColor( const QColor &color )
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  for ( const QModelIndex &index : selection )
  {
    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      if ( i->data( Qt::ForegroundRole ).value< QColor >() != color )
      {
        i->setData( Qt::ForegroundRole, color.isValid() ? color : QVariant() );
        changed = true;
      }
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem();
      newItem->setData( Qt::ForegroundRole, color.isValid() ? color : QVariant() );
      setItem( index.row(), index.column(), newItem );
      changed = true;
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::setSelectionBackgroundColor( const QColor &color )
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  for ( const QModelIndex &index : selection )
  {
    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      if ( i->data( PresetBackgroundColorRole ).value< QColor >() != color )
      {
        i->setData( Qt::BackgroundRole, color.isValid() ? color : QVariant() );
        i->setData( PresetBackgroundColorRole, color.isValid() ? color : QVariant() );
        changed = true;
      }
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem();
      newItem->setData( Qt::BackgroundRole, color.isValid() ? color : QVariant() );
      newItem->setData( PresetBackgroundColorRole, color.isValid() ? color : QVariant() );
      setItem( index.row(), index.column(), newItem );
      changed = true;
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::setSelectionRowHeight( double height )
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  const QList< int > rows = collectUniqueRows( selection );
  for ( int row : rows )
  {
    for ( int col = 0; col < columnCount(); ++col )
    {
      if ( QTableWidgetItem *i = item( row, col ) )
      {
        if ( i->data( RowHeight ).toDouble() != height )
        {
          i->setData( RowHeight, height );
          changed = true;
        }
      }
      else
      {
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setData( RowHeight, height );
        setItem( row, col, newItem );
        changed = true;
      }
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::setSelectionColumnWidth( double width )
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  const QList< int > cols = collectUniqueColumns( selection );
  for ( int col : cols )
  {
    for ( int row = 0; row < rowCount(); ++row )
    {
      if ( QTableWidgetItem *i = item( row, col ) )
      {
        if ( i->data( ColumnWidth ).toDouble() != width )
        {
          i->setData( ColumnWidth, width );
          changed = true;
        }
      }
      else
      {
        QTableWidgetItem *newItem = new QTableWidgetItem();
        newItem->setData( ColumnWidth, width );
        setItem( row, col, newItem );
        changed = true;
      }
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

