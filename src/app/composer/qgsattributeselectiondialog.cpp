/***************************************************************************
                         qgsattributeselectiondialog.cpp
                         -------------------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeselectiondialog.h"
#include "qgscomposerattributetablev2.h"
#include "qgscomposerattributetablemodelv2.h"
#include "qgsvectorlayer.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsdoublespinbox.h"
#include "qgssettings.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QSortFilterProxyModel>


// QgsComposerColumnAlignmentDelegate

QgsComposerColumnAlignmentDelegate::QgsComposerColumnAlignmentDelegate( QObject *parent ) : QItemDelegate( parent )
{

}

QWidget *QgsComposerColumnAlignmentDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

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

void QgsComposerColumnAlignmentDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag )index.model()->data( index, Qt::EditRole ).toInt();

  //set the value for the combobox
  QComboBox *comboBox = static_cast<QComboBox *>( editor );
  comboBox->setCurrentIndex( comboBox->findData( alignment ) );
}

void QgsComposerColumnAlignmentDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *comboBox = static_cast<QComboBox *>( editor );
  Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag ) comboBox->currentData().toInt();
  model->setData( index, alignment, Qt::EditRole );
}

void QgsComposerColumnAlignmentDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}


// QgsComposerColumnSourceDelegate

QgsComposerColumnSourceDelegate::QgsComposerColumnSourceDelegate( QgsVectorLayer *vlayer, QObject *parent, const QgsComposerObject *composerObject )
  : QItemDelegate( parent )
  , mVectorLayer( vlayer )
  , mComposerObject( composerObject )
{

}

QgsExpressionContext QgsComposerColumnSourceDelegate::createExpressionContext() const
{
  if ( !mComposerObject )
  {
    return QgsExpressionContext();
  }

  QgsExpressionContext expContext = mComposerObject->createExpressionContext();
  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), 1, true ) );
  expContext.setHighlightedVariables( QStringList() << QStringLiteral( "row_number" ) );
  return expContext;
}

QWidget *QgsComposerColumnSourceDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  QgsFieldExpressionWidget *fieldExpression = new QgsFieldExpressionWidget( parent );
  fieldExpression->setLayer( mVectorLayer );
  fieldExpression->registerExpressionContextGenerator( this );

  //listen out for field changes
  connect( fieldExpression, static_cast < void ( QgsFieldExpressionWidget::* )( const QString & ) >( &QgsFieldExpressionWidget::fieldChanged ), this, [ = ] { const_cast< QgsComposerColumnSourceDelegate * >( this )->commitAndCloseEditor(); } );
  return fieldExpression;
}

void QgsComposerColumnSourceDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString field = index.model()->data( index, Qt::EditRole ).toString();

  //set the value for the field combobox
  QgsFieldExpressionWidget *fieldExpression = static_cast<QgsFieldExpressionWidget *>( editor );
  fieldExpression->setField( field );
}

void QgsComposerColumnSourceDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsFieldExpressionWidget *fieldExpression = static_cast<QgsFieldExpressionWidget *>( editor );
  QString field = fieldExpression->currentField();

  model->setData( index, field, Qt::EditRole );
}

void QgsComposerColumnSourceDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}

void QgsComposerColumnSourceDelegate::commitAndCloseEditor()
{
  QgsFieldExpressionWidget *fieldExpression = qobject_cast<QgsFieldExpressionWidget *>( sender() );
  emit commitData( fieldExpression );
}


// QgsComposerColumnSortOrderDelegate

QgsComposerColumnSortOrderDelegate::QgsComposerColumnSortOrderDelegate( QObject *parent ) : QItemDelegate( parent )
{

}

QWidget *QgsComposerColumnSortOrderDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  QComboBox *comboBox = new QComboBox( parent );
  QStringList sortOrders;
  sortOrders << tr( "Ascending" ) << tr( "Descending" );
  comboBox->addItems( sortOrders );
  return comboBox;
}

void QgsComposerColumnSortOrderDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
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

void QgsComposerColumnSortOrderDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
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

void QgsComposerColumnSortOrderDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}


//
// QgsComposerColumnWidthDelegate
//

QgsComposerColumnWidthDelegate::QgsComposerColumnWidthDelegate( QObject *parent )
  : QItemDelegate( parent )
{

}

QWidget *QgsComposerColumnWidthDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index );
  Q_UNUSED( option );
  QgsDoubleSpinBox *editor = new QgsDoubleSpinBox( parent );
  editor->setMinimum( 0 );
  editor->setMaximum( 1000 );
  editor->setDecimals( 2 );
  editor->setSuffix( tr( " mm" ) );
  editor->setSpecialValueText( tr( "Automatic" ) );
  editor->setShowClearButton( true );
  return editor;
}

void QgsComposerColumnWidthDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  int value = index.model()->data( index, Qt::EditRole ).toInt();

  QgsDoubleSpinBox *spinBox = static_cast<QgsDoubleSpinBox *>( editor );
  spinBox->setValue( value );
}

void QgsComposerColumnWidthDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsDoubleSpinBox *spinBox = static_cast<QgsDoubleSpinBox *>( editor );
  spinBox->interpretText();
  int value = spinBox->value();

  model->setData( index, value, Qt::EditRole );
}

void QgsComposerColumnWidthDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}


// QgsAttributeSelectionDialog

QgsAttributeSelectionDialog::QgsAttributeSelectionDialog( QgsComposerAttributeTableV2 *table, QgsVectorLayer *vLayer,
    QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mComposerTable( table )
  , mVectorLayer( vLayer )

{
  setupUi( this );
  connect( mRemoveColumnPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mRemoveColumnPushButton_clicked );
  connect( mAddColumnPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mAddColumnPushButton_clicked );
  connect( mColumnUpPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mColumnUpPushButton_clicked );
  connect( mColumnDownPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mColumnDownPushButton_clicked );
  connect( mResetColumnsPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mResetColumnsPushButton_clicked );
  connect( mClearColumnsPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mClearColumnsPushButton_clicked );
  connect( mAddSortColumnPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mAddSortColumnPushButton_clicked );
  connect( mRemoveSortColumnPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mRemoveSortColumnPushButton_clicked );
  connect( mSortColumnUpPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mSortColumnUpPushButton_clicked );
  connect( mSortColumnDownPushButton, &QPushButton::clicked, this, &QgsAttributeSelectionDialog::mSortColumnDownPushButton_clicked );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/AttributeSelectionDialog/geometry" ) ).toByteArray() );

  if ( mComposerTable )
  {
    //set up models, views and delegates
    mColumnModel = new QgsComposerAttributeTableColumnModelV2( mComposerTable, mColumnsTableView );
    mColumnsTableView->setModel( mColumnModel );
    mColumnsTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

    mColumnSourceDelegate = new QgsComposerColumnSourceDelegate( vLayer, mColumnsTableView, mComposerTable );
    mColumnsTableView->setItemDelegateForColumn( 0, mColumnSourceDelegate );
    mColumnAlignmentDelegate = new QgsComposerColumnAlignmentDelegate( mColumnsTableView );
    mColumnsTableView->setItemDelegateForColumn( 2, mColumnAlignmentDelegate );
    mColumnWidthDelegate = new QgsComposerColumnWidthDelegate( mColumnsTableView );
    mColumnsTableView->setItemDelegateForColumn( 3, mColumnWidthDelegate );

    mAvailableSortProxyModel = new QgsComposerTableSortColumnsProxyModelV2( mComposerTable, QgsComposerTableSortColumnsProxyModelV2::ShowUnsortedColumns, mSortColumnComboBox );
    mAvailableSortProxyModel->setSourceModel( mColumnModel );
    mSortColumnComboBox->setModel( mAvailableSortProxyModel );
    mSortColumnComboBox->setModelColumn( 0 );

    mColumnSortOrderDelegate = new QgsComposerColumnSortOrderDelegate( mSortColumnTableView );
    mSortColumnTableView->setItemDelegateForColumn( 1, mColumnSortOrderDelegate );

    mSortedProxyModel = new QgsComposerTableSortColumnsProxyModelV2( mComposerTable, QgsComposerTableSortColumnsProxyModelV2::ShowSortedColumns, mSortColumnTableView );
    mSortedProxyModel->setSourceModel( mColumnModel );
    mSortedProxyModel->sort( 0, Qt::AscendingOrder );
    mSortColumnTableView->setSortingEnabled( false );
    mSortColumnTableView->setModel( mSortedProxyModel );
    mSortColumnTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
  }

  mOrderComboBox->insertItem( 0, tr( "Ascending" ) );
  mOrderComboBox->insertItem( 1, tr( "Descending" ) );
}

QgsAttributeSelectionDialog::~QgsAttributeSelectionDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/AttributeSelectionDialog/geometry" ), saveGeometry() );
}

void QgsAttributeSelectionDialog::mRemoveColumnPushButton_clicked()
{
  //remove selected rows from model
  QModelIndexList indexes =  mColumnsTableView->selectionModel()->selectedRows();
  int count = indexes.count();

  for ( int i = count; i > 0; --i )
    mColumnModel->removeRow( indexes.at( i - 1 ).row(), QModelIndex() );
}

void QgsAttributeSelectionDialog::mAddColumnPushButton_clicked()
{
  //add a new row to the model
  mColumnModel->insertRow( mColumnModel->rowCount() );
}

void QgsAttributeSelectionDialog::mColumnUpPushButton_clicked()
{
  //move selected row up

  QModelIndexList indexes =  mColumnsTableView->selectionModel()->selectedRows();
  int count = indexes.count();

  std::reverse( indexes.begin(), indexes.end() );
  for ( int i = count; i > 0; --i )
    mColumnModel->moveRow( indexes.at( i - 1 ).row(), QgsComposerAttributeTableColumnModelV2::ShiftUp );
}

void QgsAttributeSelectionDialog::mColumnDownPushButton_clicked()
{
  //move selected row down
  QModelIndexList indexes =  mColumnsTableView->selectionModel()->selectedRows();
  int count = indexes.count();

  for ( int i = count; i > 0; --i )
    mColumnModel->moveRow( indexes.at( i - 1 ).row(), QgsComposerAttributeTableColumnModelV2::ShiftDown );
}

void QgsAttributeSelectionDialog::mResetColumnsPushButton_clicked()
{
  //reset columns to match vector layer's fields
  mColumnModel->resetToLayer();
  mSortColumnComboBox->setCurrentIndex( 0 );
}

void QgsAttributeSelectionDialog::mClearColumnsPushButton_clicked()
{
  //remove all columns
  mColumnModel->removeRows( 0, mColumnModel->rowCount() );
  mSortColumnComboBox->setCurrentIndex( 0 );
}

void QgsAttributeSelectionDialog::mAddSortColumnPushButton_clicked()
{
  //add column to sort order widget
  QgsComposerTableColumn *column = mAvailableSortProxyModel->columnFromRow( mSortColumnComboBox->currentIndex() );
  if ( ! column )
  {
    return;
  }

  mColumnModel->setColumnAsSorted( column, mOrderComboBox->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder );

  //required so that rows can be reordered if initially no rows were shown in the table view
  mSortedProxyModel->resetFilter();
}

void QgsAttributeSelectionDialog::mRemoveSortColumnPushButton_clicked()
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
  QgsComposerTableColumn *column = nullptr;
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

void QgsAttributeSelectionDialog::mSortColumnUpPushButton_clicked()
{
  //find selected row
  QItemSelection sortSelection( mSortColumnTableView->selectionModel()->selection() );
  if ( sortSelection.length() < 1 )
  {
    return;
  }
  QModelIndex selectedIndex = sortSelection.indexes().at( 0 );

  QgsComposerTableColumn *column = mSortedProxyModel->columnFromIndex( selectedIndex );

  if ( !column )
  {
    return;
  }
  mColumnModel->moveColumnInSortRank( column, QgsComposerAttributeTableColumnModelV2::ShiftUp );
}

void QgsAttributeSelectionDialog::mSortColumnDownPushButton_clicked()
{
  //find selected row
  QItemSelection sortSelection( mSortColumnTableView->selectionModel()->selection() );
  if ( sortSelection.length() < 1 )
  {
    return;
  }

  QModelIndex selectedIndex = sortSelection.indexes().at( 0 );

  QgsComposerTableColumn *column = mSortedProxyModel->columnFromIndex( selectedIndex );

  if ( !column )
  {
    return;
  }
  mColumnModel->moveColumnInSortRank( column, QgsComposerAttributeTableColumnModelV2::ShiftDown );
}

