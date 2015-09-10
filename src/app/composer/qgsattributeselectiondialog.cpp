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
#include "qgscomposerattributetablemodel.h"
#include "qgscomposerattributetablemodelv2.h"
#include "qgsvectorlayer.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsdoublespinbox.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QSortFilterProxyModel>


// QgsComposerColumnAlignmentDelegate

QgsComposerColumnAlignmentDelegate::QgsComposerColumnAlignmentDelegate( QObject* parent ) : QItemDelegate( parent )
{

}

QWidget* QgsComposerColumnAlignmentDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
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

void QgsComposerColumnAlignmentDelegate::setEditorData( QWidget* editor, const QModelIndex& index ) const
{
  Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag )index.model()->data( index, Qt::EditRole ).toInt();

  //set the value for the combobox
  QComboBox *comboBox = static_cast<QComboBox*>( editor );
  comboBox->setCurrentIndex( comboBox->findData( alignment ) );
}

void QgsComposerColumnAlignmentDelegate::setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
  QComboBox *comboBox = static_cast<QComboBox*>( editor );
  Qt::AlignmentFlag alignment = ( Qt::AlignmentFlag ) comboBox->itemData( comboBox->currentIndex() ).toInt();
  model->setData( index, alignment, Qt::EditRole );
}

void QgsComposerColumnAlignmentDelegate::updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}


// QgsComposerColumnSourceDelegate

QgsComposerColumnSourceDelegate::QgsComposerColumnSourceDelegate( QgsVectorLayer* vlayer, QObject* parent, const QgsComposerObject* composerObject )
    : QItemDelegate( parent )
    , mVectorLayer( vlayer )
    , mComposerObject( composerObject )
{

}

static QgsExpressionContext _getExpressionContext( const void* context )
{
  const QgsComposerObject* object = ( const QgsComposerObject* ) context;
  if ( !object )
  {
    return QgsExpressionContext();
  }

  QScopedPointer< QgsExpressionContext > expContext( object->createExpressionContext() );
  expContext->lastScope()->setVariable( "row_number", 1 );
  expContext->setHighlightedVariables( QStringList() << "row_number" );
  return QgsExpressionContext( *expContext );
}

QWidget* QgsComposerColumnSourceDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  QgsFieldExpressionWidget *fieldExpression = new QgsFieldExpressionWidget( parent );
  fieldExpression->setLayer( mVectorLayer );
  fieldExpression->registerGetExpressionContextCallback( &_getExpressionContext, mComposerObject );

  //listen out for field changes
  connect( fieldExpression, SIGNAL( fieldChanged( QString ) ), this, SLOT( commitAndCloseEditor() ) );
  return fieldExpression;
}

void QgsComposerColumnSourceDelegate::setEditorData( QWidget* editor, const QModelIndex& index ) const
{
  QString field = index.model()->data( index, Qt::EditRole ).toString();

  //set the value for the field combobox
  QgsFieldExpressionWidget *fieldExpression = static_cast<QgsFieldExpressionWidget*>( editor );
  fieldExpression->setField( field );
}

void QgsComposerColumnSourceDelegate::setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
  QgsFieldExpressionWidget *fieldExpression = static_cast<QgsFieldExpressionWidget*>( editor );
  QString field = fieldExpression->currentField();

  model->setData( index, field, Qt::EditRole );
}

void QgsComposerColumnSourceDelegate::updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}

void QgsComposerColumnSourceDelegate::commitAndCloseEditor()
{
  QgsFieldExpressionWidget *fieldExpression = qobject_cast<QgsFieldExpressionWidget*>( sender() );
  emit commitData( fieldExpression );
}


// QgsComposerColumnSortOrderDelegate

QgsComposerColumnSortOrderDelegate::QgsComposerColumnSortOrderDelegate( QObject* parent ) : QItemDelegate( parent )
{

}

QWidget* QgsComposerColumnSortOrderDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  QComboBox *comboBox = new QComboBox( parent );
  QStringList sortOrders;
  sortOrders << tr( "Ascending" ) << tr( "Descending" );
  comboBox->addItems( sortOrders );
  return comboBox;
}

void QgsComposerColumnSortOrderDelegate::setEditorData( QWidget* editor, const QModelIndex& index ) const
{
  Qt::SortOrder order = ( Qt::SortOrder )index.model()->data( index, Qt::EditRole ).toInt();

  //set the value for the combobox
  QComboBox *comboBox = static_cast<QComboBox*>( editor );
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

void QgsComposerColumnSortOrderDelegate::setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
  QComboBox *comboBox = static_cast<QComboBox*>( editor );
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

void QgsComposerColumnSortOrderDelegate::updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const
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

  QgsDoubleSpinBox *spinBox = static_cast<QgsDoubleSpinBox*>( editor );
  spinBox->setValue( value );
}

void QgsComposerColumnWidthDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsDoubleSpinBox *spinBox = static_cast<QgsDoubleSpinBox*>( editor );
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

QgsAttributeSelectionDialog::QgsAttributeSelectionDialog( QgsComposerAttributeTableV2* table, QgsVectorLayer* vLayer,
    QWidget* parent, Qt::WindowFlags f )
    : QDialog( parent, f )
    , mComposerTable( table )
    , mComposerTableV1( NULL )
    , mVectorLayer( vLayer )
    , mColumnModel( NULL )
    , mColumnModelV1( NULL )
    , mSortedProxyModel( NULL )
    , mSortedProxyModelV1( NULL )
    , mAvailableSortProxyModel( NULL )
    , mAvailableSortProxyModelV1( NULL )
    , mColumnAlignmentDelegate( NULL )
    , mColumnSourceDelegate( NULL )
    , mColumnSortOrderDelegate( NULL )
    , mColumnWidthDelegate( NULL )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/AttributeSelectionDialog/geometry" ).toByteArray() );

  if ( mComposerTable )
  {
    //set up models, views and delegates
    mColumnModel = new QgsComposerAttributeTableColumnModelV2( mComposerTable, mColumnsTableView );
    mColumnsTableView->setModel( mColumnModel );
    mColumnsTableView->horizontalHeader()->setResizeMode( QHeaderView::Stretch );

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
    mSortColumnTableView->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  }

  mOrderComboBox->insertItem( 0, tr( "Ascending" ) );
  mOrderComboBox->insertItem( 1, tr( "Descending" ) );
}

QgsAttributeSelectionDialog::QgsAttributeSelectionDialog( QgsComposerAttributeTable *table, QgsVectorLayer *vLayer, QWidget *parent, Qt::WindowFlags f )
    : QDialog( parent, f )
    , mComposerTable( NULL )
    , mComposerTableV1( table )
    , mVectorLayer( vLayer )
    , mColumnModel( NULL )
    , mColumnModelV1( NULL )
    , mSortedProxyModel( NULL )
    , mSortedProxyModelV1( NULL )
    , mAvailableSortProxyModel( NULL )
    , mAvailableSortProxyModelV1( NULL )
    , mColumnAlignmentDelegate( NULL )
    , mColumnSourceDelegate( NULL )
    , mColumnSortOrderDelegate( NULL )
    , mColumnWidthDelegate( NULL )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/AttributeSelectionDialog/geometry" ).toByteArray() );

  if ( mComposerTableV1 )
  {
    //set up models, views and delegates
    mColumnModelV1 = new QgsComposerAttributeTableColumnModel( mComposerTableV1, mColumnsTableView );
    mColumnsTableView->setModel( mColumnModelV1 );
    mColumnsTableView->horizontalHeader()->setResizeMode( QHeaderView::Stretch );

    mColumnSourceDelegate = new QgsComposerColumnSourceDelegate( vLayer, mColumnsTableView, mComposerTableV1 );
    mColumnsTableView->setItemDelegateForColumn( 0, mColumnSourceDelegate );
    mColumnAlignmentDelegate = new QgsComposerColumnAlignmentDelegate( mColumnsTableView );
    mColumnsTableView->setItemDelegateForColumn( 2, mColumnAlignmentDelegate );

    mAvailableSortProxyModelV1 = new QgsComposerTableSortColumnsProxyModel( mComposerTableV1, QgsComposerTableSortColumnsProxyModel::ShowUnsortedColumns, mSortColumnComboBox );
    mAvailableSortProxyModelV1->setSourceModel( mColumnModelV1 );
    mSortColumnComboBox->setModel( mAvailableSortProxyModelV1 );
    mSortColumnComboBox->setModelColumn( 0 );

    mColumnSortOrderDelegate = new QgsComposerColumnSortOrderDelegate( mSortColumnTableView );
    mSortColumnTableView->setItemDelegateForColumn( 1, mColumnSortOrderDelegate );

    mSortedProxyModelV1 = new QgsComposerTableSortColumnsProxyModel( mComposerTableV1, QgsComposerTableSortColumnsProxyModel::ShowSortedColumns, mSortColumnTableView );
    mSortedProxyModelV1->setSourceModel( mColumnModelV1 );
    mSortedProxyModelV1->sort( 0, Qt::AscendingOrder );
    mSortColumnTableView->setSortingEnabled( false );
    mSortColumnTableView->setModel( mSortedProxyModelV1 );
    mSortColumnTableView->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  }

  mOrderComboBox->insertItem( 0, tr( "Ascending" ) );
  mOrderComboBox->insertItem( 1, tr( "Descending" ) );
}

QgsAttributeSelectionDialog::~QgsAttributeSelectionDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/AttributeSelectionDialog/geometry", saveGeometry() );
}

void QgsAttributeSelectionDialog::on_mRemoveColumnPushButton_clicked()
{
  if ( mComposerTable )
  {
    //remove selected row from model
    QItemSelection viewSelection( mColumnsTableView->selectionModel()->selection() );
    if ( viewSelection.length() > 0 )
    {
      int selectedRow = viewSelection.indexes().at( 0 ).row();
      mColumnModel->removeRow( selectedRow );
    }
  }
  if ( mComposerTableV1 )
  {
    //remove selected row from model
    QItemSelection viewSelection( mColumnsTableView->selectionModel()->selection() );
    if ( viewSelection.length() > 0 )
    {
      int selectedRow = viewSelection.indexes().at( 0 ).row();
      mColumnModelV1->removeRow( selectedRow );
    }
  }

}

void QgsAttributeSelectionDialog::on_mAddColumnPushButton_clicked()
{
  if ( mComposerTable )
  {
    //add a new row to the model
    mColumnModel->insertRow( mColumnModel->rowCount() );
  }
  else if ( mComposerTableV1 )
  {
    //add a new row to the model
    mColumnModelV1->insertRow( mColumnModelV1->rowCount() );
  }
}

void QgsAttributeSelectionDialog::on_mColumnUpPushButton_clicked()
{
  if ( mComposerTable )
  {
    //move selected row up
    QItemSelection viewSelection( mColumnsTableView->selectionModel()->selection() );
    if ( viewSelection.size() > 0 )
    {
      int selectedRow = viewSelection.indexes().at( 0 ).row();
      mColumnModel->moveRow( selectedRow, QgsComposerAttributeTableColumnModelV2::ShiftUp );
    }
  }
  else if ( mComposerTableV1 )
  {
    //move selected row up
    QItemSelection viewSelection( mColumnsTableView->selectionModel()->selection() );
    if ( viewSelection.size() > 0 )
    {
      int selectedRow = viewSelection.indexes().at( 0 ).row();
      mColumnModelV1->moveRow( selectedRow, QgsComposerAttributeTableColumnModel::ShiftUp );
    }
  }
}

void QgsAttributeSelectionDialog::on_mColumnDownPushButton_clicked()
{
  if ( mComposerTable )
  {
    //move selected row down
    QItemSelection viewSelection( mColumnsTableView->selectionModel()->selection() );
    if ( viewSelection.size() > 0 )
    {
      int selectedRow = viewSelection.indexes().at( 0 ).row();
      mColumnModel->moveRow( selectedRow, QgsComposerAttributeTableColumnModelV2::ShiftDown );
    }
  }
  else if ( mComposerTableV1 )
  {
    //move selected row down
    QItemSelection viewSelection( mColumnsTableView->selectionModel()->selection() );
    if ( viewSelection.size() > 0 )
    {
      int selectedRow = viewSelection.indexes().at( 0 ).row();
      mColumnModelV1->moveRow( selectedRow, QgsComposerAttributeTableColumnModel::ShiftDown );
    }
  }

}

void QgsAttributeSelectionDialog::on_mResetColumnsPushButton_clicked()
{
  if ( mComposerTable )
  {
    //reset columns to match vector layer's fields
    mColumnModel->resetToLayer();
  }
  else if ( mComposerTableV1 )
  {
    //reset columns to match vector layer's fields
    mColumnModelV1->resetToLayer();
  }

  mSortColumnComboBox->setCurrentIndex( 0 );
}

void QgsAttributeSelectionDialog::on_mAddSortColumnPushButton_clicked()
{
  //add column to sort order widget
  if ( mComposerTable )
  {
    QgsComposerTableColumn* column = mAvailableSortProxyModel->columnFromRow( mSortColumnComboBox->currentIndex() );
    if ( ! column )
    {
      return;
    }

    mColumnModel->setColumnAsSorted( column, mOrderComboBox->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder );

    //required so that rows can be reordered if initially no rows were shown in the table view
    mSortedProxyModel->resetFilter();
  }
  else if ( mComposerTableV1 )
  {
    QgsComposerTableColumn* column = mAvailableSortProxyModelV1->columnFromRow( mSortColumnComboBox->currentIndex() );
    if ( ! column )
    {
      return;
    }

    mColumnModelV1->setColumnAsSorted( column, mOrderComboBox->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder );

    //required so that rows can be reordered if initially no rows were shown in the table view
    mSortedProxyModelV1->resetFilter();
  }

}

void QgsAttributeSelectionDialog::on_mRemoveSortColumnPushButton_clicked()
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
  QgsComposerTableColumn * column = 0;
  if ( mComposerTable )
  {
    column = mSortedProxyModel->columnFromIndex( selectedIndex );
  }
  else if ( mComposerTableV1 )
  {
    column = mSortedProxyModelV1->columnFromIndex( selectedIndex );
  }

  if ( !column )
  {
    return;
  }

  //set column as unsorted
  if ( mComposerTable )
  {
    mColumnModel->setColumnAsUnsorted( column );
  }
  else if ( mComposerTableV1 )
  {
    mColumnModelV1->setColumnAsUnsorted( column );
  }
  //set next row as selected
  mSortColumnTableView->selectRow( rowToRemove );
}

void QgsAttributeSelectionDialog::on_mSortColumnUpPushButton_clicked()
{
  //find selected row
  QItemSelection sortSelection( mSortColumnTableView->selectionModel()->selection() );
  if ( sortSelection.length() < 1 )
  {
    return;
  }
  QModelIndex selectedIndex = sortSelection.indexes().at( 0 );

  if ( mComposerTable )
  {
    QgsComposerTableColumn * column = mSortedProxyModel->columnFromIndex( selectedIndex );

    if ( !column )
    {
      return;
    }
    mColumnModel->moveColumnInSortRank( column, QgsComposerAttributeTableColumnModelV2::ShiftUp );
  }
  else if ( mComposerTableV1 )
  {
    QgsComposerTableColumn * column = mSortedProxyModelV1->columnFromIndex( selectedIndex );

    if ( !column )
    {
      return;
    }
    mColumnModelV1->moveColumnInSortRank( column, QgsComposerAttributeTableColumnModel::ShiftUp );
  }
}

void QgsAttributeSelectionDialog::on_mSortColumnDownPushButton_clicked()
{
  //find selected row
  QItemSelection sortSelection( mSortColumnTableView->selectionModel()->selection() );
  if ( sortSelection.length() < 1 )
  {
    return;
  }

  QModelIndex selectedIndex = sortSelection.indexes().at( 0 );

  if ( mComposerTable )
  {
    QgsComposerTableColumn * column = mSortedProxyModel->columnFromIndex( selectedIndex );

    if ( !column )
    {
      return;
    }
    mColumnModel->moveColumnInSortRank( column, QgsComposerAttributeTableColumnModelV2::ShiftDown );
  }
  else if ( mComposerTableV1 )
  {
    QgsComposerTableColumn * column = mSortedProxyModelV1->columnFromIndex( selectedIndex );

    if ( !column )
    {
      return;
    }
    mColumnModelV1->moveColumnInSortRank( column, QgsComposerAttributeTableColumnModel::ShiftDown );
  }
}

