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
#include <QPlainTextEdit>

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
      QAction *insertBefore = mHeaderMenu->addAction( selectedColumns.size() > 1 ? tr( "Insert %n Column(s) Before", nullptr, selectedColumns.size() ) : tr( "Insert Column Before" ) );
      connect( insertBefore, &QAction::triggered, this, &QgsTableEditorWidget::insertColumnsBefore );
      QAction *insertAfter = mHeaderMenu->addAction( selectedColumns.size() > 1 ? tr( "Insert %n Column(s) After", nullptr, selectedColumns.size() ) : tr( "Insert Column After" ) );
      connect( insertAfter, &QAction::triggered, this, &QgsTableEditorWidget::insertColumnsAfter );
    }
    QAction *deleteSelected = mHeaderMenu->addAction( selectedColumns.size() > 1 ? tr( "Delete %n Column(s)", nullptr, selectedColumns.size() ) : tr( "Delete Column" ) );
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
      QAction *insertBefore = mHeaderMenu->addAction( selectedRows.size() > 1 ? tr( "Insert %n Row(s) Above", nullptr, selectedRows.size() ) : tr( "Insert Row Above" ) );
      connect( insertBefore, &QAction::triggered, this, &QgsTableEditorWidget::insertRowsAbove );
      QAction *insertAfter = mHeaderMenu->addAction( selectedRows.size() > 1 ? tr( "Insert %n Row(s) Below", nullptr, selectedRows.size() ) : tr( "Insert Row Below" ) );
      connect( insertAfter, &QAction::triggered, this, &QgsTableEditorWidget::insertRowsBelow );
    }
    QAction *deleteSelected = mHeaderMenu->addAction( selectedRows.size() > 1 ? tr( "Delete %n Row(s)", nullptr, selectedRows.size() ) : tr( "Delete Row" ) );
    connect( deleteSelected, &QAction::triggered, this, &QgsTableEditorWidget::deleteRows );

    mHeaderMenu->popup( verticalHeader()->mapToGlobal( point ) );
  } );


  QgsTableEditorDelegate *delegate = new QgsTableEditorDelegate( this );
  connect( delegate, &QgsTableEditorDelegate::updateNumericFormatForIndex, this, &QgsTableEditorWidget::updateNumericFormatForIndex );
  setItemDelegate( delegate );


  connect( this, &QTableWidget::cellDoubleClicked, this, [ = ]
  {
    if ( QgsTableEditorDelegate *d = qobject_cast< QgsTableEditorDelegate *>( itemDelegate( ) ) )
    {
      d->setWeakEditorMode( false );
    }
  } );

  connect( selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsTableEditorWidget::activeCellChanged );
}

QgsTableEditorWidget::~QgsTableEditorWidget()
{
  qDeleteAll( mNumericFormats );
}

void QgsTableEditorWidget::updateNumericFormatForIndex( const QModelIndex &index )
{
  if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
  {
    if ( QgsNumericFormat *format = mNumericFormats.value( i ) )
    {
      i->setData( Qt::DisplayRole, format->formatDouble( index.data( CellContent ).toDouble(), QgsNumericFormatContext() ) );
    }
  }
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

  headers.clear();
  if ( mIncludeHeader )
    headers << tr( "Header" );
  for ( int i = 1; i <= 1000; i++ )
  {
    headers << QString::number( i );
  }

  setVerticalHeaderLabels( headers );
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
  if ( QgsTableEditorDelegate *d = qobject_cast< QgsTableEditorDelegate *>( itemDelegate( ) ) )
  {
    d->setWeakEditorMode( true );
  }
}

void QgsTableEditorWidget::setTableContents( const QgsTableContents &contents )
{
  mBlockSignals++;
  qDeleteAll( mNumericFormats );
  mNumericFormats.clear();

  QgsNumericFormatContext numericContext;
  int rowNumber = mIncludeHeader ? 1 : 0;
  bool first = true;
  setRowCount( contents.size() + rowNumber );
  for ( const QgsTableRow &row : contents )
  {
    if ( first )
    {
      setColumnCount( row.size() );
      first = false;
    }

    int colNumber = 0;
    for ( const QgsTableCell &col : row )
    {
      QTableWidgetItem *item = new QTableWidgetItem( col.content().value< QgsProperty >().isActive() ? col.content().value< QgsProperty >().asExpression() : col.content().toString() );
      item->setData( CellContent, col.content() ); // can't use EditRole, because Qt. (https://bugreports.qt.io/browse/QTBUG-11549)
      item->setData( Qt::BackgroundRole, col.backgroundColor().isValid() ? col.backgroundColor() : QColor( 255, 255, 255 ) );
      item->setData( PresetBackgroundColorRole, col.backgroundColor().isValid() ? col.backgroundColor() : QVariant() );
      item->setData( Qt::ForegroundRole, col.textFormat().isValid() ? col.textFormat().color() : QVariant() );
      item->setData( TextFormat, QVariant::fromValue( col.textFormat() ) );
      item->setData( HorizontalAlignment, static_cast< int >( col.horizontalAlignment() ) );
      item->setData( VerticalAlignment, static_cast< int >( col.verticalAlignment() ) );
      item->setData( CellProperty, QVariant::fromValue( col.content().value< QgsProperty >() ) );

      if ( col.content().value< QgsProperty >().isActive() )
        item->setFlags( item->flags() & ( ~Qt::ItemIsEditable ) );

      if ( auto *lNumericFormat = col.numericFormat() )
      {
        mNumericFormats.insert( item, lNumericFormat->clone() );
        item->setData( Qt::DisplayRole, mNumericFormats.value( item )->formatDouble( col.content().toDouble(), numericContext ) );
      }
      setItem( rowNumber, colNumber, item );
      colNumber++;
    }
    rowNumber++;
  }

  mBlockSignals--;
  updateHeaders();

  if ( mFirstSet )
  {
    resizeColumnsToContents();
    resizeRowsToContents();
    mFirstSet = false;
  }
  emit tableChanged();
}

QgsTableContents QgsTableEditorWidget::tableContents() const
{
  QgsTableContents items;
  items.reserve( rowCount() );

  for ( int r = mIncludeHeader ? 1 : 0; r < rowCount(); r++ )
  {
    QgsTableRow row;
    row.reserve( columnCount() );
    for ( int c = 0; c < columnCount(); c++ )
    {
      QgsTableCell cell;
      if ( QTableWidgetItem *i = item( r, c ) )
      {
        cell.setContent( i->data( CellProperty ).value< QgsProperty >().isActive() ? i->data( CellProperty ) : i->data( CellContent ) );
        cell.setBackgroundColor( i->data( PresetBackgroundColorRole ).value< QColor >() );
        cell.setTextFormat( i->data( TextFormat ).value< QgsTextFormat >() );
        cell.setHorizontalAlignment( static_cast< Qt::Alignment >( i->data( HorizontalAlignment ).toInt() ) );
        cell.setVerticalAlignment( static_cast< Qt::Alignment >( i->data( VerticalAlignment ).toInt() ) );

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
  QgsNumericFormatContext numericContext;
  for ( const QModelIndex &index : selection )
  {
    if ( index.row() == 0 && mIncludeHeader )
      continue;

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
    i->setData( Qt::DisplayRole, newFormat ? mNumericFormats.value( i )->formatDouble( i->data( CellContent ).toDouble(), numericContext ) : i->data( CellContent ) );
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
  const QgsTextFormat f = selectionTextFormat();
  return f.isValid() ? f.color() : QColor();
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

Qt::Alignment QgsTableEditorWidget::selectionHorizontalAlignment()
{
  Qt::Alignment alignment = Qt::AlignLeft;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    Qt::Alignment cellAlign = static_cast< Qt::Alignment >( model()->data( index, HorizontalAlignment ).toInt() );
    if ( first )
    {
      alignment = cellAlign;
      first = false;
    }
    else if ( cellAlign == alignment )
      continue;
    else
    {
      return Qt::AlignLeft | Qt::AlignTop;
    }
  }
  return alignment;
}

Qt::Alignment QgsTableEditorWidget::selectionVerticalAlignment()
{
  Qt::Alignment alignment = Qt::AlignVCenter;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    Qt::Alignment cellAlign = static_cast< Qt::Alignment >( model()->data( index, VerticalAlignment ).toInt() );
    if ( first )
    {
      alignment = cellAlign;
      first = false;
    }
    else if ( cellAlign == alignment )
      continue;
    else
    {
      return Qt::AlignLeft | Qt::AlignTop;
    }
  }
  return alignment;
}

QgsProperty QgsTableEditorWidget::selectionCellProperty()
{
  QgsProperty property;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    const QgsProperty cellProperty = model()->data( index, CellProperty ).value< QgsProperty >();
    if ( first )
    {
      property = cellProperty;
      first = false;
    }
    else if ( cellProperty == property )
      continue;
    else
    {
      return QgsProperty();
    }
  }
  return property;
}

QgsTextFormat QgsTableEditorWidget::selectionTextFormat()
{
  QgsTextFormat format;
  bool first = true;
  const QModelIndexList selection = selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    if ( !model()->data( index, TextFormat ).isValid() )
      return QgsTextFormat();

    QgsTextFormat cellFormat = model()->data( index, TextFormat ).value< QgsTextFormat >();
    if ( first )
    {
      format = cellFormat;
      first = false;
    }
    else if ( cellFormat == format )
      continue;
    else
      return QgsTextFormat();
  }
  return format;
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
    double thisHeight = model()->data( model()->index( row + ( mIncludeHeader ? 1 : 0 ), col ), RowHeight ).toDouble();
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
  if ( row == 0 && mIncludeHeader )
    return;

  bool changed = false;
  mBlockSignals++;

  for ( int col = 0; col < columnCount(); ++col )
  {
    if ( QTableWidgetItem *i = item( row + ( mIncludeHeader ? 1 : 0 ), col ) )
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
      setItem( row + ( mIncludeHeader ? 1 : 0 ), col, newItem );
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

QList<int> QgsTableEditorWidget::rowsAssociatedWithSelection()
{
  return collectUniqueRows( selectedIndexes() );
}

QList<int> QgsTableEditorWidget::columnsAssociatedWithSelection()
{
  return collectUniqueColumns( selectedIndexes() );
}

QVariantList QgsTableEditorWidget::tableHeaders() const
{
  if ( !mIncludeHeader )
    return QVariantList();

  QVariantList res;
  res.reserve( columnCount() );
  for ( int col = 0; col < columnCount(); ++col )
  {
    if ( QTableWidgetItem *i = item( 0, col ) )
    {
      res << i->data( CellContent );
    }
    else
    {
      res << QVariant();
    }
  }
  return res;
}

bool QgsTableEditorWidget::isHeaderCellSelected()
{
  if ( !mIncludeHeader )
    return false;

  return collectUniqueRows( selectedIndexes() ).contains( 0 );
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
  const QList< int > rows = rowsAssociatedWithSelection();
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
  const QList< int > columns = columnsAssociatedWithSelection();
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
      i->setData( CellContent, QVariant() );
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
    if ( index.row() == 0 && mIncludeHeader )
      continue;

    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      if ( i->data( Qt::ForegroundRole ).value< QColor >() != color )
      {
        i->setData( Qt::ForegroundRole, color.isValid() ? color : QVariant() );
        QgsTextFormat f = i->data( TextFormat ).value< QgsTextFormat >();
        f.setColor( color );
        i->setData( TextFormat, QVariant::fromValue( f ) );
        changed = true;
      }
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem();
      newItem->setData( Qt::ForegroundRole, color.isValid() ? color : QVariant() );
      QgsTextFormat f;
      f.setColor( color );
      newItem->setData( TextFormat, QVariant::fromValue( f ) );
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
    if ( index.row() == 0 && mIncludeHeader )
      continue;

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

void QgsTableEditorWidget::setSelectionHorizontalAlignment( Qt::Alignment alignment )
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  for ( const QModelIndex &index : selection )
  {
    if ( index.row() == 0 && mIncludeHeader )
      continue;

    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      if ( static_cast< Qt::Alignment >( i->data( HorizontalAlignment ).toInt() ) != alignment )
      {
        i->setData( HorizontalAlignment, static_cast< int >( alignment ) );
        changed = true;
      }
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem();
      newItem->setData( HorizontalAlignment, static_cast< int >( alignment ) );
      setItem( index.row(), index.column(), newItem );
      changed = true;
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::setSelectionVerticalAlignment( Qt::Alignment alignment )
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  for ( const QModelIndex &index : selection )
  {
    if ( index.row() == 0 && mIncludeHeader )
      continue;

    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      if ( static_cast< Qt::Alignment >( i->data( HorizontalAlignment ).toInt() ) != alignment )
      {
        i->setData( VerticalAlignment, static_cast< int >( alignment ) );
        changed = true;
      }
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem();
      newItem->setData( VerticalAlignment, static_cast< int >( alignment ) );
      setItem( index.row(), index.column(), newItem );
      changed = true;
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::setSelectionCellProperty( const QgsProperty &property )
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  for ( const QModelIndex &index : selection )
  {
    if ( index.row() == 0 && mIncludeHeader )
      continue;

    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      if ( i->data( CellProperty ).value< QgsProperty >() != property )
      {
        if ( property.isActive() )
        {
          i->setData( CellProperty, QVariant::fromValue( property ) );
          i->setText( property.asExpression() );
          i->setFlags( i->flags() & ( ~Qt::ItemIsEditable ) );
        }
        else
        {
          i->setData( CellProperty, QVariant() );
          i->setText( QString() );
          i->setFlags( i->flags() | Qt::ItemIsEditable );
        }
        changed = true;
      }
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem( property.asExpression() );
      if ( property.isActive() )
      {
        newItem->setData( CellProperty,  QVariant::fromValue( property ) );
        newItem->setFlags( newItem->flags() & ( ~Qt::ItemIsEditable ) );
      }
      else
      {
        newItem->setData( CellProperty, QVariant() );
        newItem->setFlags( newItem->flags() | Qt::ItemIsEditable );
      }
      setItem( index.row(), index.column(), newItem );
      changed = true;
    }
  }
  mBlockSignals--;
  if ( changed && !mBlockSignals )
    emit tableChanged();
}

void QgsTableEditorWidget::setSelectionTextFormat( const QgsTextFormat &format )
{
  const QModelIndexList selection = selectedIndexes();
  bool changed = false;
  mBlockSignals++;
  for ( const QModelIndex &index : selection )
  {
    if ( index.row() == 0 && mIncludeHeader )
      continue;

    if ( QTableWidgetItem *i = item( index.row(), index.column() ) )
    {
      i->setData( TextFormat, QVariant::fromValue( format ) );
      i->setData( Qt::ForegroundRole, format.color() );
      changed = true;
    }
    else
    {
      QTableWidgetItem *newItem = new QTableWidgetItem();
      newItem->setData( TextFormat, QVariant::fromValue( format ) );
      newItem->setData( Qt::ForegroundRole, format.color() );
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
  bool changed = false;
  mBlockSignals++;
  const QList< int > rows = rowsAssociatedWithSelection();
  for ( int row : rows )
  {
    if ( row == 0 && mIncludeHeader )
      continue;

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
  bool changed = false;
  mBlockSignals++;
  const QList< int > cols = columnsAssociatedWithSelection();
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

void QgsTableEditorWidget::setIncludeTableHeader( bool included )
{
  if ( included == mIncludeHeader )
    return;

  mIncludeHeader = included;

  if ( mIncludeHeader )
    insertRow( 0 );
  else
    removeRow( 0 );
  updateHeaders();
}

void QgsTableEditorWidget::setTableHeaders( const QVariantList &headers )
{
  if ( !mIncludeHeader )
    return;

  mBlockSignals++;

  for ( int col = 0; col < columnCount(); ++col )
  {
    if ( QTableWidgetItem *i = item( 0, col ) )
    {
      i->setText( headers.value( col ).toString() );
      i->setData( CellContent, headers.value( col ) ); // can't use EditRole, because Qt. (https://bugreports.qt.io/browse/QTBUG-11549)
    }
    else
    {
      QTableWidgetItem *item = new QTableWidgetItem( headers.value( col ).toString() );
      item->setData( CellContent, headers.value( col ) ); // can't use EditRole, because Qt. (https://bugreports.qt.io/browse/QTBUG-11549)
      setItem( 0, col, item );
    }
  }
  mBlockSignals--;
}

/// @cond PRIVATE

QgsTableEditorTextEdit::QgsTableEditorTextEdit( QWidget *parent )
  : QPlainTextEdit( parent )
{
  // narrower default margins
  document()->setDocumentMargin( document()->documentMargin() / 2 );

  connect( this, &QPlainTextEdit::textChanged, this, &QgsTableEditorTextEdit::resizeToContents );
  updateMinimumSize();
}

void QgsTableEditorTextEdit::keyPressEvent( QKeyEvent *event )
{
  switch ( event->key() )
  {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    {
      if ( event->modifiers() & Qt::ControlModifier )
      {
        // ctrl+enter inserts a line break
        insertPlainText( QString( '\n' ) );
        resizeToContents();
      }
      else
      {
        // closes editor
        event->ignore();
      }
      break;
    }

    case Qt::Key_Right:
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Down:
    {
      if ( mWeakEditorMode )
      {
        // close editor and defer to table
        event->ignore();
      }
      else
      {
        QPlainTextEdit::keyPressEvent( event );
      }
      break;
    }

    case Qt::Key_Tab:
    {
      if ( event->modifiers() & Qt::ControlModifier )
      {
        // if tab is pressed then defer to table, unless ctrl modifier is also held
        // (emulate spreadsheet behavior)
        insertPlainText( QString( '\t' ) );
        resizeToContents();
      }
      else
      {
        event->ignore();
      }
      break;
    }

    default:
      QPlainTextEdit::keyPressEvent( event );
  }
}

void QgsTableEditorTextEdit::updateMinimumSize()
{
  const double tm = document()->documentMargin();
  const QMargins cm = contentsMargins();
  const int width = tm * 2 + cm.left() + cm.right() + 30;
  const int height = tm * 2 + cm.top() + cm.bottom() + 4;
  QStyleOptionFrame opt;
  initStyleOption( &opt );
  const QSize sizeFromContent = style()->sizeFromContents( QStyle::CT_LineEdit, &opt, QSize( width, height ), this );
  setMinimumWidth( sizeFromContent.width() );
  setMinimumHeight( sizeFromContent.height() );
}

void QgsTableEditorTextEdit::setWeakEditorMode( bool weakEditorMode )
{
  mWeakEditorMode = weakEditorMode;
}

void QgsTableEditorTextEdit::resizeToContents()
{
  int oldWidth = width();
  int oldHeight = height();
  if ( mOriginalWidth == -1 )
    mOriginalWidth = oldWidth;
  if ( mOriginalHeight == -1 )
    mOriginalHeight = oldHeight;

  if ( QWidget *parent = parentWidget() )
  {
    QPoint position = pos();
    QFontMetrics fm( font() );

    const QStringList lines = toPlainText().split( '\n' );
    int maxTextLineWidth = 0;
    int totalTextHeight = 0;
    for ( const QString &line : lines )
    {
      const QRect bounds = fontMetrics().boundingRect( line );
      maxTextLineWidth = std::max( maxTextLineWidth, bounds.width() );
      totalTextHeight += fm.height();
    }

    int hintWidth = minimumWidth() + maxTextLineWidth;
    int hintHeight = minimumHeight() + totalTextHeight;
    int parentWidth = parent->width();
    int maxWidth = isRightToLeft() ? position.x() + oldWidth : parentWidth - position.x();
    int maxHeight = parent->height() - position.y();
    int newWidth = std::clamp( hintWidth, mOriginalWidth, maxWidth );
    int newHeight = std::clamp( hintHeight, mOriginalHeight, maxHeight );

    if ( mWidgetOwnsGeometry )
    {
      setMaximumWidth( newWidth );
      setMaximumHeight( newHeight );
    }
    if ( isRightToLeft() )
      move( position.x() - newWidth + oldWidth, position.y() );
    resize( newWidth, newHeight );
  }
}

void QgsTableEditorTextEdit::changeEvent( QEvent *e )
{
  switch ( e->type() )
  {
    case QEvent::FontChange:
    case QEvent::StyleChange:
    case QEvent::ContentsRectChange:
      updateMinimumSize();
      break;
    default:
      break;
  }
  QPlainTextEdit::changeEvent( e );
}

QgsTableEditorDelegate::QgsTableEditorDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{

}

void QgsTableEditorDelegate::setWeakEditorMode( bool weakEditorMode )
{
  mWeakEditorMode = weakEditorMode;
}

QWidget *QgsTableEditorDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  QgsTableEditorTextEdit *w = new QgsTableEditorTextEdit( parent );
  w->setWeakEditorMode( mWeakEditorMode );

  if ( !w->style()->styleHint( QStyle::SH_ItemView_DrawDelegateFrame, 0, w ) )
    w->setFrameShape( QFrame::NoFrame );
  if ( !w->style()->styleHint( QStyle::SH_ItemView_ShowDecorationSelected, 0, w ) )
    w->setWidgetOwnsGeometry( true );

  return w;
}

void QgsTableEditorDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QVariant value = index.model()->data( index, QgsTableEditorWidget::CellContent );
  if ( QgsTableEditorTextEdit *lineEdit = qobject_cast<QgsTableEditorTextEdit * >( editor ) )
  {
    if ( index != mLastIndex || lineEdit->toPlainText() != value.toString() )
    {
      lineEdit->setPlainText( value.toString() );
      lineEdit->selectAll();
    }
  }
  mLastIndex = index;
}

void QgsTableEditorDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( QgsTableEditorTextEdit *lineEdit = qobject_cast<QgsTableEditorTextEdit * >( editor ) )
  {
    const QString text = lineEdit->toPlainText();
    if ( text != model->data( index, QgsTableEditorWidget::CellContent ).toString() && !model->data( index, QgsTableEditorWidget::CellProperty ).value< QgsProperty >().isActive() )
    {
      model->setData( index, text, QgsTableEditorWidget::CellContent );
      model->setData( index, text, Qt::DisplayRole );
      emit updateNumericFormatForIndex( index );
    }
  }
}


///@endcond

