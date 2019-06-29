/***************************************************************************
                         qgslayoutattributeselectiondialog.cpp
                         -------------------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutattributeselectiondialog.h"
#include "qgslayoutitemattributetable.h"
#include "qgsvectorlayer.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsdoublespinbox.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgslayouttablecolumn.h"
#include "qgshelp.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QSortFilterProxyModel>



//QgsLayoutAttributeTableColumnModel

QgsLayoutAttributeTableColumnModel::QgsLayoutAttributeTableColumnModel( QgsLayoutItemAttributeTable *table, QObject *parent )
  : QAbstractTableModel( parent )
  , mTable( table )
{
}

QModelIndex QgsLayoutAttributeTableColumnModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    if ( ( mTable->columns() )[row] )
    {
      return createIndex( row, column, ( mTable->columns() )[row] );
    }
  }
  return QModelIndex();
}

QModelIndex QgsLayoutAttributeTableColumnModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsLayoutAttributeTableColumnModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mTable->columns().length();
}

int QgsLayoutAttributeTableColumnModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 4;
}

QVariant QgsLayoutAttributeTableColumnModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ||
       ( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole ) )
  {
    return QVariant();
  }

  if ( index.row() >= mTable->columns().length() )
  {
    return QVariant();
  }

  //get column for index
  QgsLayoutTableColumn *column = columnFromIndex( index );
  if ( !column )
  {
    return QVariant();
  }

  if ( role == Qt::UserRole )
  {
    //user role stores reference in column object
    return qVariantFromValue( column );
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
            switch ( column->vAlignment() )
            {
              case Qt::AlignTop:
                return tr( "Top center" );
              case Qt::AlignBottom:
                return tr( "Bottom center" );
              default:
                return tr( "Middle center" );
            }
          case Qt::AlignRight:
            switch ( column->vAlignment() )
            {
              case Qt::AlignTop:
                return tr( "Top right" );
              case Qt::AlignBottom:
                return tr( "Bottom right" );
              default:
                return tr( "Middle right" );
            }
          case Qt::AlignLeft:
          default:
            switch ( column->vAlignment() )
            {
              case Qt::AlignTop:
                return tr( "Top left" );
              case Qt::AlignBottom:
                return tr( "Bottom left" );
              default:
                return tr( "Middle left" );
            }
        }
      }
      else
      {
        //edit role
        return int( column->hAlignment() | column->vAlignment() );
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

QVariant QgsLayoutAttributeTableColumnModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( !mTable )
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

bool QgsLayoutAttributeTableColumnModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole || !mTable )
  {
    return false;
  }
  if ( index.row() >= mTable->columns().length() )
  {
    return false;
  }

  //get column for index
  QgsLayoutTableColumn *column = columnFromIndex( index );
  if ( !column )
  {
    return false;
  }

  switch ( index.column() )
  {
    case 0:
      // also update column's heading, if it hasn't been customized
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
      column->setHAlignment( Qt::AlignmentFlag( value.toInt() & Qt::AlignHorizontal_Mask ) );
      column->setVAlignment( Qt::AlignmentFlag( value.toInt() & Qt::AlignVertical_Mask ) );
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

Qt::ItemFlags QgsLayoutAttributeTableColumnModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractTableModel::flags( index );

  if ( index.isValid() )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
}

bool QgsLayoutAttributeTableColumnModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent )

  int maxRow = std::min( row + count - 1, mTable->columns().length() - 1 );
  beginRemoveRows( QModelIndex(), row, maxRow );
  //move backwards through rows, removing each corresponding QgsComposerTableColumn
  for ( int i = maxRow; i >= row; --i )
  {
    delete ( mTable->columns() )[i];
    mTable->columns().removeAt( i );
  }
  endRemoveRows();
  return true;
}

bool QgsLayoutAttributeTableColumnModel::insertRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent )
  beginInsertRows( QModelIndex(), row, row + count - 1 );
  //create new QgsComposerTableColumns for each inserted row
  for ( int i = row; i < row + count; ++i )
  {
    QgsLayoutTableColumn *col = new QgsLayoutTableColumn;
    mTable->columns().insert( i, col );
  }
  endInsertRows();
  return true;
}

bool QgsLayoutAttributeTableColumnModel::moveRow( int row, ShiftDirection direction )
{
  if ( ( direction == ShiftUp && row <= 0 ) ||
       ( direction == ShiftDown &&  row >= rowCount() - 1 ) )
  {
    //row is already at top/bottom
    return false;
  }

  //we shift a row by removing the next row up/down, then reinserting it before/after the target row
  int swapWithRow = direction == ShiftUp ? row - 1 : row + 1;

  //remove row
  beginRemoveRows( QModelIndex(), swapWithRow, swapWithRow );
  QgsLayoutTableColumn *temp = mTable->columns().takeAt( swapWithRow );
  endRemoveRows();

  //insert row
  beginInsertRows( QModelIndex(), row, row );
  mTable->columns().insert( row, temp );
  endInsertRows();

  return true;
}

void QgsLayoutAttributeTableColumnModel::resetToLayer()
{
  beginResetModel();
  mTable->resetColumns();
  endResetModel();
}

QgsLayoutTableColumn *QgsLayoutAttributeTableColumnModel::columnFromIndex( const QModelIndex &index ) const
{
  QgsLayoutTableColumn *column = static_cast<QgsLayoutTableColumn *>( index.internalPointer() );
  return column;
}

QModelIndex QgsLayoutAttributeTableColumnModel::indexFromColumn( QgsLayoutTableColumn *column )
{
  if ( !mTable )
  {
    return QModelIndex();
  }

  int r = mTable->columns().indexOf( column );

  QModelIndex idx = index( r, 0, QModelIndex() );
  if ( idx.isValid() )
  {
    return idx;
  }

  return QModelIndex();
}

void QgsLayoutAttributeTableColumnModel::setColumnAsSorted( QgsLayoutTableColumn *column, Qt::SortOrder order )
{
  if ( !column || !mTable )
  {
    return;
  }

  //find current highest sort by rank
  int highestRank = 0;
  for ( auto columnIt = mTable->columns().constBegin(); columnIt != mTable->columns().constEnd(); ++columnIt )
  {
    highestRank = std::max( highestRank, ( *columnIt )->sortByRank() );
  }

  column->setSortByRank( highestRank + 1 );
  column->setSortOrder( order );

  QModelIndex idx = indexFromColumn( column );
  emit dataChanged( idx, idx );
}

void QgsLayoutAttributeTableColumnModel::setColumnAsUnsorted( QgsLayoutTableColumn *column )
{
  if ( !mTable || !column )
  {
    return;
  }

  column->setSortByRank( 0 );
  QModelIndex idx = indexFromColumn( column );
  emit dataChanged( idx, idx );
}

static bool columnsBySortRank( QgsLayoutTableColumn *a, QgsLayoutTableColumn *b )
{
  return a->sortByRank() < b->sortByRank();
}

bool QgsLayoutAttributeTableColumnModel::moveColumnInSortRank( QgsLayoutTableColumn *column, ShiftDirection direction )
{
  if ( !mTable || !column )
  {
    return false;
  }
  if ( ( direction == ShiftUp && column->sortByRank() <= 1 )
       || ( direction == ShiftDown && column->sortByRank() <= 0 ) )
  {
    //already at start/end of list or not being used for sort
    return false;
  }

  //find column before this one in sort order
  QVector<QgsLayoutTableColumn *> sortedColumns;
  const QgsLayoutTableColumns &columns = mTable->columns();
  for ( QgsLayoutTableColumn *currentColumn : columns )
  {
    if ( currentColumn->sortByRank() > 0 )
    {
      sortedColumns.append( currentColumn );
    }
  }
  std::stable_sort( sortedColumns.begin(), sortedColumns.end(), columnsBySortRank );
  int columnPos = sortedColumns.indexOf( column );

  if ( ( columnPos == 0 && direction == ShiftUp )
       || ( ( columnPos == sortedColumns.length() - 1 ) && direction == ShiftDown ) )
  {
    //column already at start/end
    return false;
  }

  QgsLayoutTableColumn *swapColumn = direction == ShiftUp ?
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



//QgsLayoutTableSortColumnsProxyModel

QgsLayoutTableSortColumnsProxyModel::QgsLayoutTableSortColumnsProxyModel( QgsLayoutItemAttributeTable *table, ColumnFilterType filterType, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mTable( table )
  , mFilterType( filterType )
{
  setDynamicSortFilter( true );
}

bool QgsLayoutTableSortColumnsProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  //get QgsComposerTableColumn corresponding to row
  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  QgsLayoutTableColumn *column = columnFromSourceIndex( index );

  if ( !column )
  {
    return false;
  }

  if ( ( column->sortByRank() > 0 && mFilterType == ShowSortedColumns )
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

QgsLayoutTableColumn *QgsLayoutTableSortColumnsProxyModel::columnFromIndex( const QModelIndex &index ) const
{
  //get column corresponding to an index from the proxy
  QModelIndex sourceIndex = mapToSource( index );
  return columnFromSourceIndex( sourceIndex );
}

QgsLayoutTableColumn *QgsLayoutTableSortColumnsProxyModel::columnFromSourceIndex( const QModelIndex &sourceIndex ) const
{
  //get column corresponding to an index from the source model
  QVariant columnAsVariant = sourceModel()->data( sourceIndex, Qt::UserRole );
  QgsLayoutTableColumn *column = qobject_cast<QgsLayoutTableColumn *>( columnAsVariant.value<QObject *>() );
  return column;
}

bool QgsLayoutTableSortColumnsProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  QgsLayoutTableColumn *column1 = columnFromSourceIndex( left );
  QgsLayoutTableColumn *column2 = columnFromSourceIndex( right );
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

int QgsLayoutTableSortColumnsProxyModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 2;
}

QVariant QgsLayoutTableSortColumnsProxyModel::data( const QModelIndex &index, int role ) const
{
  if ( ( role != Qt::DisplayRole && role != Qt::EditRole ) || !index.isValid() )
  {
    return QVariant();
  }

  QgsLayoutTableColumn *column = columnFromIndex( index );
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

QVariant QgsLayoutTableSortColumnsProxyModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( !mTable )
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

Qt::ItemFlags QgsLayoutTableSortColumnsProxyModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QSortFilterProxyModel::flags( index );

  if ( index.column() == 1 )
  {
    //only sort order is editable
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

bool QgsLayoutTableSortColumnsProxyModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  if ( !mTable )
  {
    return false;
  }

  QgsLayoutTableColumn *column = columnFromIndex( index );
  if ( !column )
  {
    return false;
  }

  if ( index.column() == 1 )
  {
    column->setSortOrder( static_cast< Qt::SortOrder >( value.toInt() ) );
    emit dataChanged( index, index );
    return true;
  }

  return false;
}

QgsLayoutTableColumn *QgsLayoutTableSortColumnsProxyModel::columnFromRow( int row )
{
  QModelIndex proxyIndex = index( row, 0 );
  return columnFromIndex( proxyIndex );
}

void QgsLayoutTableSortColumnsProxyModel::resetFilter()
{
  invalidate();
}


// QgsLayoutColumnAlignmentDelegate

QgsLayoutColumnAlignmentDelegate::QgsLayoutColumnAlignmentDelegate( QObject *parent ) : QItemDelegate( parent )
{

}

QWidget *QgsLayoutColumnAlignmentDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  Q_UNUSED( index )

  //create a combo box showing alignment options
  QComboBox *comboBox = new QComboBox( parent );

  comboBox->addItem( tr( "Top left" ), int( Qt::AlignTop | Qt::AlignLeft ) );
  comboBox->addItem( tr( "Top center" ), int( Qt::AlignTop | Qt::AlignHCenter ) );
  comboBox->addItem( tr( "Top right" ), int( Qt::AlignTop | Qt::AlignRight ) );
  comboBox->addItem( tr( "Middle left" ), int( Qt::AlignVCenter | Qt::AlignLeft ) );
  comboBox->addItem( tr( "Middle center" ), int( Qt::AlignVCenter | Qt::AlignHCenter ) );
  comboBox->addItem( tr( "Middle right" ), int( Qt::AlignVCenter | Qt::AlignRight ) );
  comboBox->addItem( tr( "Bottom left" ), int( Qt::AlignBottom | Qt::AlignLeft ) );
  comboBox->addItem( tr( "Bottom center" ), int( Qt::AlignBottom | Qt::AlignHCenter ) );
  comboBox->addItem( tr( "Bottom right" ), int( Qt::AlignBottom | Qt::AlignRight ) );

  Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag )index.model()->data( index, Qt::EditRole ).toInt();
  comboBox->setCurrentIndex( comboBox->findData( alignment ) );

  return comboBox;
}

void QgsLayoutColumnAlignmentDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag )index.model()->data( index, Qt::EditRole ).toInt();

  //set the value for the combobox
  QComboBox *comboBox = static_cast<QComboBox *>( editor );
  comboBox->setCurrentIndex( comboBox->findData( alignment ) );
}

void QgsLayoutColumnAlignmentDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *comboBox = static_cast<QComboBox *>( editor );
  Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag ) comboBox->currentData().toInt();
  model->setData( index, alignment, Qt::EditRole );
}

void QgsLayoutColumnAlignmentDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )
  editor->setGeometry( option.rect );
}


// QgsLayoutColumnSourceDelegate

QgsLayoutColumnSourceDelegate::QgsLayoutColumnSourceDelegate( QgsVectorLayer *vlayer, QObject *parent, const QgsLayoutObject *layoutObject )
  : QItemDelegate( parent )
  , mVectorLayer( vlayer )
  , mLayoutObject( layoutObject )
{

}

QgsExpressionContext QgsLayoutColumnSourceDelegate::createExpressionContext() const
{
  if ( !mLayoutObject )
  {
    return QgsExpressionContext();
  }

  QgsExpressionContext expContext = mLayoutObject->createExpressionContext();
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), 1, true ) );
  expContext.setHighlightedVariables( QStringList() << QStringLiteral( "row_number" ) );
  return expContext;
}

QWidget *QgsLayoutColumnSourceDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  Q_UNUSED( index )

  QgsFieldExpressionWidget *fieldExpression = new QgsFieldExpressionWidget( parent );
  fieldExpression->setLayer( mVectorLayer );
  fieldExpression->registerExpressionContextGenerator( this );

  //listen out for field changes
  connect( fieldExpression, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), this, [ = ] { const_cast< QgsLayoutColumnSourceDelegate * >( this )->commitAndCloseEditor(); } );
  return fieldExpression;
}

void QgsLayoutColumnSourceDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString field = index.model()->data( index, Qt::EditRole ).toString();

  //set the value for the field combobox
  QgsFieldExpressionWidget *fieldExpression = static_cast<QgsFieldExpressionWidget *>( editor );
  fieldExpression->setField( field );
}

void QgsLayoutColumnSourceDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsFieldExpressionWidget *fieldExpression = static_cast<QgsFieldExpressionWidget *>( editor );
  QString field = fieldExpression->currentField();

  model->setData( index, field, Qt::EditRole );
}

void QgsLayoutColumnSourceDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )
  editor->setGeometry( option.rect );
}

void QgsLayoutColumnSourceDelegate::commitAndCloseEditor()
{
  QgsFieldExpressionWidget *fieldExpression = qobject_cast<QgsFieldExpressionWidget *>( sender() );
  emit commitData( fieldExpression );
}


// QgsLayoutColumnSortOrderDelegate

QgsLayoutColumnSortOrderDelegate::QgsLayoutColumnSortOrderDelegate( QObject *parent ) : QItemDelegate( parent )
{

}

QWidget *QgsLayoutColumnSortOrderDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  Q_UNUSED( index )

  QComboBox *comboBox = new QComboBox( parent );
  QStringList sortOrders;
  sortOrders << tr( "Ascending" ) << tr( "Descending" );
  comboBox->addItems( sortOrders );
  return comboBox;
}

void QgsLayoutColumnSortOrderDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  Qt::SortOrder order = ( Qt::SortOrder )index.model()->data( index, Qt::EditRole ).toInt();

  //set the value for the combobox
  QComboBox *comboBox = static_cast<QComboBox *>( editor );
  switch ( order )
  {
    case Qt::DescendingOrder:
      comboBox->setCurrentIndex( 1 );
      break;
    case Qt::AscendingOrder:
    default:
      comboBox->setCurrentIndex( 0 );
      break;
  }
}

void QgsLayoutColumnSortOrderDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *comboBox = static_cast<QComboBox *>( editor );
  int value = comboBox->currentIndex();
  Qt::SortOrder order;
  switch ( value )
  {
    case 1:
      order = Qt::DescendingOrder;
      break;
    case 0:
    default:
      order = Qt::AscendingOrder;
      break;
  }

  model->setData( index, order, Qt::EditRole );
}

void QgsLayoutColumnSortOrderDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )
  editor->setGeometry( option.rect );
}


//
// QgsLayoutColumnWidthDelegate
//

QgsLayoutColumnWidthDelegate::QgsLayoutColumnWidthDelegate( QObject *parent )
  : QItemDelegate( parent )
{

}

QWidget *QgsLayoutColumnWidthDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )
  Q_UNUSED( option )
  QgsDoubleSpinBox *editor = new QgsDoubleSpinBox( parent );
  editor->setMinimum( 0 );
  editor->setMaximum( 1000 );
  editor->setDecimals( 2 );
  editor->setSuffix( tr( " mm" ) );
  editor->setSpecialValueText( tr( "Automatic" ) );
  editor->setShowClearButton( true );
  return editor;
}

void QgsLayoutColumnWidthDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  int value = index.model()->data( index, Qt::EditRole ).toInt();

  QgsDoubleSpinBox *spinBox = static_cast<QgsDoubleSpinBox *>( editor );
  spinBox->setValue( value );
}

void QgsLayoutColumnWidthDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsDoubleSpinBox *spinBox = static_cast<QgsDoubleSpinBox *>( editor );
  spinBox->interpretText();
  int value = spinBox->value();

  model->setData( index, value, Qt::EditRole );
}

void QgsLayoutColumnWidthDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )
  editor->setGeometry( option.rect );
}


// QgsLayoutAttributeSelectionDialog

QgsLayoutAttributeSelectionDialog::QgsLayoutAttributeSelectionDialog( QgsLayoutItemAttributeTable *table, QgsVectorLayer *vLayer,
    QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mTable( table )
  , mVectorLayer( vLayer )

{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mRemoveColumnPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mRemoveColumnPushButton_clicked );
  connect( mAddColumnPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mAddColumnPushButton_clicked );
  connect( mColumnUpPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mColumnUpPushButton_clicked );
  connect( mColumnDownPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mColumnDownPushButton_clicked );
  connect( mResetColumnsPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mResetColumnsPushButton_clicked );
  connect( mClearColumnsPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mClearColumnsPushButton_clicked );
  connect( mAddSortColumnPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mAddSortColumnPushButton_clicked );
  connect( mRemoveSortColumnPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mRemoveSortColumnPushButton_clicked );
  connect( mSortColumnUpPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mSortColumnUpPushButton_clicked );
  connect( mSortColumnDownPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeSelectionDialog::mSortColumnDownPushButton_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsLayoutAttributeSelectionDialog::showHelp );

  if ( mTable )
  {
    //set up models, views and delegates
    mColumnModel = new QgsLayoutAttributeTableColumnModel( mTable, mColumnsTableView );
    mColumnsTableView->setModel( mColumnModel );
    mColumnsTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

    mColumnSourceDelegate = new QgsLayoutColumnSourceDelegate( vLayer, mColumnsTableView, mTable );
    mColumnsTableView->setItemDelegateForColumn( 0, mColumnSourceDelegate );
    mColumnAlignmentDelegate = new QgsLayoutColumnAlignmentDelegate( mColumnsTableView );
    mColumnsTableView->setItemDelegateForColumn( 2, mColumnAlignmentDelegate );
    mColumnWidthDelegate = new QgsLayoutColumnWidthDelegate( mColumnsTableView );
    mColumnsTableView->setItemDelegateForColumn( 3, mColumnWidthDelegate );

    mAvailableSortProxyModel = new QgsLayoutTableSortColumnsProxyModel( mTable, QgsLayoutTableSortColumnsProxyModel::ShowUnsortedColumns, mSortColumnComboBox );
    mAvailableSortProxyModel->setSourceModel( mColumnModel );
    mSortColumnComboBox->setModel( mAvailableSortProxyModel );
    mSortColumnComboBox->setModelColumn( 0 );

    mColumnSortOrderDelegate = new QgsLayoutColumnSortOrderDelegate( mSortColumnTableView );
    mSortColumnTableView->setItemDelegateForColumn( 1, mColumnSortOrderDelegate );

    mSortedProxyModel = new QgsLayoutTableSortColumnsProxyModel( mTable, QgsLayoutTableSortColumnsProxyModel::ShowSortedColumns, mSortColumnTableView );
    mSortedProxyModel->setSourceModel( mColumnModel );
    mSortedProxyModel->sort( 0, Qt::AscendingOrder );
    mSortColumnTableView->setSortingEnabled( false );
    mSortColumnTableView->setModel( mSortedProxyModel );
    mSortColumnTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
  }

  mOrderComboBox->insertItem( 0, tr( "Ascending" ) );
  mOrderComboBox->insertItem( 1, tr( "Descending" ) );
}

void QgsLayoutAttributeSelectionDialog::mRemoveColumnPushButton_clicked()
{
  //remove selected rows from model
  QModelIndexList indexes =  mColumnsTableView->selectionModel()->selectedRows();
  int count = indexes.count();

  for ( int i = count; i > 0; --i )
    mColumnModel->removeRow( indexes.at( i - 1 ).row(), QModelIndex() );
}

void QgsLayoutAttributeSelectionDialog::mAddColumnPushButton_clicked()
{
  //add a new row to the model
  mColumnModel->insertRow( mColumnModel->rowCount() );
}

void QgsLayoutAttributeSelectionDialog::mColumnUpPushButton_clicked()
{
  //move selected row up

  QModelIndexList indexes =  mColumnsTableView->selectionModel()->selectedRows();
  int count = indexes.count();

  std::reverse( indexes.begin(), indexes.end() );
  for ( int i = count; i > 0; --i )
    mColumnModel->moveRow( indexes.at( i - 1 ).row(), QgsLayoutAttributeTableColumnModel::ShiftUp );
}

void QgsLayoutAttributeSelectionDialog::mColumnDownPushButton_clicked()
{
  //move selected row down
  QModelIndexList indexes =  mColumnsTableView->selectionModel()->selectedRows();
  int count = indexes.count();

  for ( int i = count; i > 0; --i )
    mColumnModel->moveRow( indexes.at( i - 1 ).row(), QgsLayoutAttributeTableColumnModel::ShiftDown );
}

void QgsLayoutAttributeSelectionDialog::mResetColumnsPushButton_clicked()
{
  //reset columns to match vector layer's fields
  mColumnModel->resetToLayer();
  mSortColumnComboBox->setCurrentIndex( 0 );
}

void QgsLayoutAttributeSelectionDialog::mClearColumnsPushButton_clicked()
{
  //remove all columns
  mColumnModel->removeRows( 0, mColumnModel->rowCount() );
  mSortColumnComboBox->setCurrentIndex( 0 );
}

void QgsLayoutAttributeSelectionDialog::mAddSortColumnPushButton_clicked()
{
  //add column to sort order widget
  QgsLayoutTableColumn *column = mAvailableSortProxyModel->columnFromRow( mSortColumnComboBox->currentIndex() );
  if ( ! column )
  {
    return;
  }

  mColumnModel->setColumnAsSorted( column, mOrderComboBox->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder );

  //required so that rows can be reordered if initially no rows were shown in the table view
  mSortedProxyModel->resetFilter();
}

void QgsLayoutAttributeSelectionDialog::mRemoveSortColumnPushButton_clicked()
{
  //remove selected rows from sort order widget
  QItemSelection sortSelection( mSortColumnTableView->selectionModel()->selection() );
  if ( sortSelection.length() < 1 )
  {
    return;
  }
  QModelIndex selectedIndex = sortSelection.indexes().at( 0 );
  int rowToRemove = selectedIndex.row();

  //find corresponding column
  QgsLayoutTableColumn *column = nullptr;
  column = mSortedProxyModel->columnFromIndex( selectedIndex );

  if ( !column )
  {
    return;
  }

  //set column as unsorted
  mColumnModel->setColumnAsUnsorted( column );
  //set next row as selected
  mSortColumnTableView->selectRow( rowToRemove );
}

void QgsLayoutAttributeSelectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "print_composer/composer_items/composer_attribute_table.html" ) );
}

void QgsLayoutAttributeSelectionDialog::mSortColumnUpPushButton_clicked()
{
  //find selected row
  QItemSelection sortSelection( mSortColumnTableView->selectionModel()->selection() );
  if ( sortSelection.length() < 1 )
  {
    return;
  }
  QModelIndex selectedIndex = sortSelection.indexes().at( 0 );

  QgsLayoutTableColumn *column = mSortedProxyModel->columnFromIndex( selectedIndex );

  if ( !column )
  {
    return;
  }
  mColumnModel->moveColumnInSortRank( column, QgsLayoutAttributeTableColumnModel::ShiftUp );
}

void QgsLayoutAttributeSelectionDialog::mSortColumnDownPushButton_clicked()
{
  //find selected row
  QItemSelection sortSelection( mSortColumnTableView->selectionModel()->selection() );
  if ( sortSelection.length() < 1 )
  {
    return;
  }

  QModelIndex selectedIndex = sortSelection.indexes().at( 0 );

  QgsLayoutTableColumn *column = mSortedProxyModel->columnFromIndex( selectedIndex );

  if ( !column )
  {
    return;
  }
  mColumnModel->moveColumnInSortRank( column, QgsLayoutAttributeTableColumnModel::ShiftDown );
}

