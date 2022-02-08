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



// QgsLayoutAttributeTableColumnModelBase

QgsLayoutAttributeTableColumnModelBase::QgsLayoutAttributeTableColumnModelBase( QgsLayoutItemAttributeTable *table, QObject *parent )
  : QAbstractTableModel( parent )
  , mTable( table )
{
}

QModelIndex QgsLayoutAttributeTableColumnModelBase::index( int row, int column, const QModelIndex &parent ) const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  if ( !parent.isValid() )
  {
    return createIndex( row, column );
  }

  return QModelIndex();
}

QModelIndex QgsLayoutAttributeTableColumnModelBase::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

int QgsLayoutAttributeTableColumnModelBase::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return columns().length();
}

int QgsLayoutAttributeTableColumnModelBase::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return displayedColumns().count();
}

QVariant QgsLayoutAttributeTableColumnModelBase::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ||
       ( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole ) )
  {
    return QVariant();
  }

  if ( index.row() >= columns().length() )
  {
    return QVariant();
  }

  // get layout column for index
  const QgsLayoutTableColumn column = columns().value( index.row() );

  const Column col = displayedColumns()[index.column()];
  switch ( col )
  {
    case Attribute:
      return column.attribute();
    case Heading:
      return column.heading();
    case Alignment:
    {
      if ( role == Qt::DisplayRole )
      {
        switch ( column.hAlignment() )
        {
          case Qt::AlignHCenter:
            switch ( column.vAlignment() )
            {
              case Qt::AlignTop:
                return tr( "Top center" );
              case Qt::AlignBottom:
                return tr( "Bottom center" );
              default:
                return tr( "Middle center" );
            }
          case Qt::AlignRight:
            switch ( column.vAlignment() )
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
            switch ( column.vAlignment() )
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
        return int( column.hAlignment() | column.vAlignment() );
      }
    }
    case Width:
    {
      if ( role == Qt::DisplayRole )
      {
        return column.width() <= 0 ? tr( "Automatic" ) : tr( "%1 mm" ).arg( column.width(), 0, 'f', 2 );
      }
      else
      {
        //edit role
        return column.width();
      }
    }
    case SortOrder:
      if ( role == Qt::DisplayRole )
      {
        switch ( column.sortOrder() )
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
        return column.sortOrder();
      }
  }

  return QVariant();
}

QVariant QgsLayoutAttributeTableColumnModelBase::headerData( int section, Qt::Orientation orientation, int role ) const
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
      const Column col = displayedColumns()[section];
      switch ( col )
      {
        case Attribute:
          return QVariant( tr( "Attribute" ) );

        case Heading:
          return QVariant( tr( "Heading" ) );

        case Alignment:
          return QVariant( tr( "Alignment" ) );

        case Width:
          return QVariant( tr( "Width" ) );

        case SortOrder:
          return QVariant( tr( "Sort Order" ) );
      }
    }
  }

  return QVariant();
}

bool QgsLayoutAttributeTableColumnModelBase::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole || !mTable )
    return false;

  if ( index.row() >= columns().length() )
    return false;

  if ( index.column() > displayedColumns().count() )
    return false;

  QgsLayoutTableColumn &colToUpdate = columns()[index.row()];

  const Column col = displayedColumns()[index.column()];
  switch ( col )
  {
    case Attribute:
      // also update column's heading, if it hasn't been customized
      if ( colToUpdate.heading().isEmpty() || ( colToUpdate.heading() == colToUpdate.attribute() ) )
      {
        colToUpdate.setHeading( value.toString() );
        emit dataChanged( createIndex( index.row(), 1 ), createIndex( index.row(), 1 ) );
      }
      colToUpdate.setAttribute( value.toString() );
      emit dataChanged( index, index );
      return true;

    case Heading:
      colToUpdate.setHeading( value.toString() );
      emit dataChanged( index, index );
      return true;

    case Alignment:
      colToUpdate.setHAlignment( Qt::AlignmentFlag( value.toInt() & Qt::AlignHorizontal_Mask ) );
      colToUpdate.setVAlignment( Qt::AlignmentFlag( value.toInt() & Qt::AlignVertical_Mask ) );
      emit dataChanged( index, index );
      return true;

    case Width:
      colToUpdate.setWidth( value.toDouble() );
      emit dataChanged( index, index );
      return true;

    case SortOrder:
      colToUpdate.setSortOrder( static_cast< Qt::SortOrder >( value.toInt() ) );
      emit dataChanged( index, index );
      return true;
  }

  return false;
}

Qt::ItemFlags QgsLayoutAttributeTableColumnModelBase::flags( const QModelIndex &index ) const
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

bool QgsLayoutAttributeTableColumnModelBase::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent )

  const int maxRow = std::min< int >( row + count - 1, columns().length() - 1 );
  beginRemoveRows( QModelIndex(), row, maxRow );
  //move backwards through rows, removing each corresponding QgsComposerTableColumn
  for ( int i = maxRow; i >= row; --i )
  {
    columns().removeAt( i );
  }
  endRemoveRows();
  return true;
}

bool QgsLayoutAttributeTableColumnModelBase::insertRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent )
  beginInsertRows( QModelIndex(), row, row + count - 1 );
  //create new QgsComposerTableColumns for each inserted row
  for ( int i = row; i < row + count; ++i )
  {
    columns().insert( i, QgsLayoutTableColumn() );
  }
  endInsertRows();
  return true;
}

bool QgsLayoutAttributeTableColumnModelBase::moveRow( int row, ShiftDirection direction )
{
  if ( ( direction == ShiftUp && row <= 0 ) ||
       ( direction == ShiftDown &&  row >= rowCount() - 1 ) )
  {
    //row is already at top/bottom
    return false;
  }

  //we shift a row by removing the next row up/down, then reinserting it before/after the target row
  const int swapWithRow = direction == ShiftUp ? row - 1 : row + 1;

  //remove row
  beginRemoveRows( QModelIndex(), swapWithRow, swapWithRow );
  const QgsLayoutTableColumn temp = columns().takeAt( swapWithRow );
  endRemoveRows();

  //insert row
  beginInsertRows( QModelIndex(), row, row );
  columns().insert( row, temp );
  endInsertRows();

  return true;
}

// QgsLayoutAttributeTableColumnModel

QVector<QgsLayoutTableColumn> &QgsLayoutAttributeTableColumnModel::columns() const
{
  return mTable->columns();
}

void QgsLayoutAttributeTableColumnModel::resetToLayer()
{
  beginResetModel();
  mTable->resetColumns();
  endResetModel();
}


// QgsLayoutTableSortModel

QVector<QgsLayoutTableColumn> &QgsLayoutTableSortModel::columns() const
{
  return mTable->sortColumns();
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

  comboBox->addItem( tr( "Top Left" ), int( Qt::AlignTop | Qt::AlignLeft ) );
  comboBox->addItem( tr( "Top Center" ), int( Qt::AlignTop | Qt::AlignHCenter ) );
  comboBox->addItem( tr( "Top Right" ), int( Qt::AlignTop | Qt::AlignRight ) );
  comboBox->addItem( tr( "Middle Left" ), int( Qt::AlignVCenter | Qt::AlignLeft ) );
  comboBox->addItem( tr( "Middle Center" ), int( Qt::AlignVCenter | Qt::AlignHCenter ) );
  comboBox->addItem( tr( "Middle Right" ), int( Qt::AlignVCenter | Qt::AlignRight ) );
  comboBox->addItem( tr( "Bottom Left" ), int( Qt::AlignBottom | Qt::AlignLeft ) );
  comboBox->addItem( tr( "Bottom Center" ), int( Qt::AlignBottom | Qt::AlignHCenter ) );
  comboBox->addItem( tr( "Bottom Right" ), int( Qt::AlignBottom | Qt::AlignRight ) );

  const Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag )index.model()->data( index, Qt::EditRole ).toInt();
  comboBox->setCurrentIndex( comboBox->findData( alignment ) );

  return comboBox;
}

void QgsLayoutColumnAlignmentDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  const Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag )index.model()->data( index, Qt::EditRole ).toInt();

  //set the value for the combobox
  QComboBox *comboBox = static_cast<QComboBox *>( editor );
  comboBox->setCurrentIndex( comboBox->findData( alignment ) );
}

void QgsLayoutColumnAlignmentDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *comboBox = static_cast<QComboBox *>( editor );
  const Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag ) comboBox->currentData().toInt();
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
  const QString field = index.model()->data( index, Qt::EditRole ).toString();

  //set the value for the field combobox
  QgsFieldExpressionWidget *fieldExpression = static_cast<QgsFieldExpressionWidget *>( editor );
  fieldExpression->setField( field );
}

void QgsLayoutColumnSourceDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsFieldExpressionWidget *fieldExpression = static_cast<QgsFieldExpressionWidget *>( editor );
  const QString field = fieldExpression->expression();
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
  const Qt::SortOrder order = ( Qt::SortOrder )index.model()->data( index, Qt::EditRole ).toInt();

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
  const int value = comboBox->currentIndex();
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
  const int value = index.model()->data( index, Qt::EditRole ).toInt();

  QgsDoubleSpinBox *spinBox = static_cast<QgsDoubleSpinBox *>( editor );
  spinBox->setValue( value );
}

void QgsLayoutColumnWidthDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsDoubleSpinBox *spinBox = static_cast<QgsDoubleSpinBox *>( editor );
  spinBox->interpretText();
  const int value = spinBox->value();

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

    mSortColumnModel = new QgsLayoutTableSortModel( mTable, mSortColumnTableView );
    mSortColumnTableView->setModel( mSortColumnModel );
    mSortColumnTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    mSortColumnSourceDelegate = new QgsLayoutColumnSourceDelegate( vLayer, mSortColumnTableView, mTable );
    mSortColumnTableView->setItemDelegateForColumn( 0, mSortColumnSourceDelegate );
    mSortColumnOrderDelegate = new QgsLayoutColumnSortOrderDelegate( mSortColumnTableView );
    mSortColumnTableView->setItemDelegateForColumn( 1, mSortColumnOrderDelegate );
  }
}

void QgsLayoutAttributeSelectionDialog::mRemoveColumnPushButton_clicked()
{
  //remove selected rows from model
  const QModelIndexList indexes =  mColumnsTableView->selectionModel()->selectedRows();
  const int count = indexes.count();

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
  const int count = indexes.count();

  std::reverse( indexes.begin(), indexes.end() );
  for ( int i = count; i > 0; --i )
    mColumnModel->moveRow( indexes.at( i - 1 ).row(), QgsLayoutAttributeTableColumnModelBase::ShiftUp );
}

void QgsLayoutAttributeSelectionDialog::mColumnDownPushButton_clicked()
{
  //move selected row down
  const QModelIndexList indexes =  mColumnsTableView->selectionModel()->selectedRows();
  const int count = indexes.count();

  for ( int i = count; i > 0; --i )
    mColumnModel->moveRow( indexes.at( i - 1 ).row(), QgsLayoutAttributeTableColumnModelBase::ShiftDown );
}

void QgsLayoutAttributeSelectionDialog::mResetColumnsPushButton_clicked()
{
  //reset columns to match vector layer's fields
  mColumnModel->resetToLayer();
}

void QgsLayoutAttributeSelectionDialog::mClearColumnsPushButton_clicked()
{
  //remove all columns
  mColumnModel->removeRows( 0, mColumnModel->rowCount() );
}

void QgsLayoutAttributeSelectionDialog::mAddSortColumnPushButton_clicked()
{
  //add a new row to the model
  mSortColumnModel->insertRow( mSortColumnModel->rowCount() );
}

void QgsLayoutAttributeSelectionDialog::mRemoveSortColumnPushButton_clicked()
{
  //remove selected rows from model
  const QModelIndexList indexes =  mSortColumnTableView->selectionModel()->selectedRows();
  const int count = indexes.count();

  for ( int i = count; i > 0; --i )
    mSortColumnModel->removeRow( indexes.at( i - 1 ).row(), QModelIndex() );
}

void QgsLayoutAttributeSelectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "print_composer/composer_items/composer_attribute_table.html" ) );
}

void QgsLayoutAttributeSelectionDialog::mSortColumnDownPushButton_clicked()
{
  //move selected row down
  const QModelIndexList indexes =  mSortColumnTableView->selectionModel()->selectedRows();
  const int count = indexes.count();

  for ( int i = count; i > 0; --i )
    mSortColumnModel->moveRow( indexes.at( i - 1 ).row(), QgsLayoutTableSortModel::ShiftDown );
}

void QgsLayoutAttributeSelectionDialog::mSortColumnUpPushButton_clicked()
{
  //move selected row up
  const QModelIndexList indexes =  mSortColumnTableView->selectionModel()->selectedRows();
  const int count = indexes.count();

  for ( int i = count; i > 0; --i )
    mSortColumnModel->moveRow( indexes.at( i - 1 ).row(), QgsLayoutTableSortModel::ShiftUp );
}


