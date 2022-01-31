/***************************************************************************
  qgsfieldmappingwidget.cpp - QgsFieldMappingWidget

 ---------------------
 begin                : 16.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldmappingwidget.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsexpression.h"
#include "qgsprocessingaggregatewidgets.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include <QTableView>
#include <QVBoxLayout>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

QgsFieldMappingWidget::QgsFieldMappingWidget( QWidget *parent,
    const QgsFields &sourceFields,
    const QgsFields &destinationFields,
    const QMap<QString, QString> &expressions )
  : QgsPanelWidget( parent )
{
  QVBoxLayout *verticalLayout = new QVBoxLayout();
  verticalLayout->setContentsMargins( 0, 0, 0, 0 );
  mTableView = new QTableView();
  verticalLayout->addWidget( mTableView );
  setLayout( verticalLayout );

  mModel = new QgsFieldMappingModel( sourceFields, destinationFields, expressions, this );

#ifdef ENABLE_MODELTEST
  new ModelTest( mModel, this );
#endif

  mTableView->setModel( mModel );
  mTableView->setItemDelegateForColumn( static_cast<int>( QgsFieldMappingModel::ColumnDataIndex::SourceExpression ), new ExpressionDelegate( this ) );
  mTableView->setItemDelegateForColumn( static_cast<int>( QgsFieldMappingModel::ColumnDataIndex::DestinationType ), new TypeDelegate( mTableView ) );
  updateColumns();
  // Make sure columns are updated when rows are added
  connect( mModel, &QgsFieldMappingModel::rowsInserted, this, [ = ] { updateColumns(); } );
  connect( mModel, &QgsFieldMappingModel::modelReset, this, [ = ] { updateColumns(); } );
  connect( mModel, &QgsFieldMappingModel::dataChanged, this, &QgsFieldMappingWidget::changed );
  connect( mModel, &QgsFieldMappingModel::rowsInserted, this, &QgsFieldMappingWidget::changed );
  connect( mModel, &QgsFieldMappingModel::rowsRemoved, this, &QgsFieldMappingWidget::changed );
  connect( mModel, &QgsFieldMappingModel::modelReset, this, &QgsFieldMappingWidget::changed );
}

void QgsFieldMappingWidget::setDestinationEditable( bool editable )
{
  qobject_cast<QgsFieldMappingModel *>( mModel )->setDestinationEditable( editable );
  updateColumns();
}

bool QgsFieldMappingWidget::destinationEditable() const
{
  return qobject_cast<QgsFieldMappingModel *>( mModel )->destinationEditable();
}

QgsFieldMappingModel *QgsFieldMappingWidget::model() const
{
  return qobject_cast<QgsFieldMappingModel *>( mModel );
}

QList<QgsFieldMappingModel::Field> QgsFieldMappingWidget::mapping() const
{
  return model()->mapping();
}

QMap<QString, QgsProperty> QgsFieldMappingWidget::fieldPropertyMap() const
{
  return model()->fieldPropertyMap();
}

void QgsFieldMappingWidget::setFieldPropertyMap( const QMap<QString, QgsProperty> &map )
{
  model()->setFieldPropertyMap( map );
}

QItemSelectionModel *QgsFieldMappingWidget::selectionModel()
{
  return mTableView->selectionModel();
}

void QgsFieldMappingWidget::setSourceFields( const QgsFields &sourceFields )
{
  model()->setSourceFields( sourceFields );
}

void QgsFieldMappingWidget::setSourceLayer( QgsVectorLayer *layer )
{
  mSourceLayer = layer;
}

QgsVectorLayer *QgsFieldMappingWidget::sourceLayer()
{
  return mSourceLayer;
}

void QgsFieldMappingWidget::setDestinationFields( const QgsFields &destinationFields, const QMap<QString, QString> &expressions )
{
  model()->setDestinationFields( destinationFields, expressions );
}

void QgsFieldMappingWidget::scrollTo( const QModelIndex &index ) const
{
  mTableView->scrollTo( index );
}

void QgsFieldMappingWidget::registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  model()->setBaseExpressionContextGenerator( generator );
}

void QgsFieldMappingWidget::appendField( const QgsField &field, const QString &expression )
{
  model()->appendField( field, expression );
}

bool QgsFieldMappingWidget::removeSelectedFields()
{
  if ( ! mTableView->selectionModel()->hasSelection() )
    return false;

  std::list<int> rowsToRemove { selectedRows() };
  rowsToRemove.reverse();
  for ( const int row : rowsToRemove )
  {
    if ( ! model()->removeField( model()->index( row, 0, QModelIndex() ) ) )
    {
      return false;
    }
  }
  return true;
}

bool QgsFieldMappingWidget::moveSelectedFieldsUp()
{
  if ( ! mTableView->selectionModel()->hasSelection() )
    return false;

  const std::list<int> rowsToMoveUp { selectedRows() };
  for ( const int row : rowsToMoveUp )
  {
    if ( ! model()->moveUp( model()->index( row, 0, QModelIndex() ) ) )
    {
      return false;
    }
  }
  return true;
}

bool QgsFieldMappingWidget::moveSelectedFieldsDown()
{
  if ( ! mTableView->selectionModel()->hasSelection() )
    return false;

  std::list<int> rowsToMoveDown { selectedRows() };
  rowsToMoveDown.reverse();
  for ( const int row : rowsToMoveDown )
  {
    if ( ! model()->moveDown( model()->index( row, 0, QModelIndex() ) ) )
    {
      return false;
    }
  }
  return true;
}

void QgsFieldMappingWidget::updateColumns()
{
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    mTableView->openPersistentEditor( mModel->index( i, static_cast<int>( QgsFieldMappingModel::ColumnDataIndex::SourceExpression ) ) );
    mTableView->openPersistentEditor( mModel->index( i, static_cast<int>( QgsFieldMappingModel::ColumnDataIndex::DestinationType ) ) );
  }

  for ( int i = 0; i < mModel->columnCount(); ++i )
  {
    mTableView->resizeColumnToContents( i );
  }
}

std::list<int> QgsFieldMappingWidget::selectedRows()
{
  std::list<int> rows;
  if ( mTableView->selectionModel()->hasSelection() )
  {
    const QModelIndexList constSelection { mTableView->selectionModel()->selectedIndexes() };
    for ( const QModelIndex &index : constSelection )
    {
      rows.push_back( index.row() );
    }
    rows.sort();
    rows.unique();
  }
  return rows;
}

//
// ExpressionDelegate
//

QgsFieldMappingWidget::ExpressionDelegate::ExpressionDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

void QgsFieldMappingWidget::ExpressionDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsFieldExpressionWidget *editorWidget { qobject_cast<QgsFieldExpressionWidget *>( editor ) };
  if ( ! editorWidget )
    return;

  bool isExpression;
  bool isValid;
  const QString currentValue { editorWidget->currentField( &isExpression, &isValid ) };
  if ( isExpression )
  {
    model->setData( index, currentValue, Qt::EditRole );
  }
  else
  {
    model->setData( index, QgsExpression::quotedColumnRef( currentValue ), Qt::EditRole );
  }
}

void QgsFieldMappingWidget::ExpressionDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsFieldExpressionWidget *editorWidget { qobject_cast<QgsFieldExpressionWidget *>( editor ) };
  if ( ! editorWidget )
    return;

  const QVariant value = index.model()->data( index, Qt::EditRole );
  editorWidget->setField( value.toString() );
}

QWidget *QgsFieldMappingWidget::ExpressionDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  QgsFieldExpressionWidget *editor = new QgsFieldExpressionWidget( parent );
  editor->setAutoFillBackground( true );
  editor->setAllowEvalErrors( false );
  if ( const QgsFieldMappingModel *model = qobject_cast<const QgsFieldMappingModel *>( index.model() ) )
  {
    editor->registerExpressionContextGenerator( model->contextGenerator() );
    editor->setFields( model->sourceFields() );
  }
  else if ( const QgsAggregateMappingModel *model = qobject_cast<const QgsAggregateMappingModel *>( index.model() ) )
  {
    editor->registerExpressionContextGenerator( model->contextGenerator() );
    editor->setFields( model->sourceFields() );
  }
  else
  {
    Q_ASSERT( false );
  }

  if ( QgsFieldMappingWidget *mappingWidget = qobject_cast< QgsFieldMappingWidget *>( ExpressionDelegate::parent() ) )
  {
    if ( mappingWidget->sourceLayer() )
      editor->setLayer( mappingWidget->sourceLayer() );
  }
  else if ( QgsAggregateMappingWidget *aggregateWidget = qobject_cast< QgsAggregateMappingWidget *>( ExpressionDelegate::parent() ) )
  {
    if ( aggregateWidget->sourceLayer() )
      editor->setLayer( aggregateWidget->sourceLayer() );
  }

  editor->setField( index.model()->data( index, Qt::DisplayRole ).toString() );
  connect( editor,
           qOverload<const  QString &, bool >( &QgsFieldExpressionWidget::fieldChanged ),
           this,
           [ = ]( const QString & fieldName, bool isValid )
  {
    Q_UNUSED( fieldName )
    Q_UNUSED( isValid )
    const_cast< QgsFieldMappingWidget::ExpressionDelegate *>( this )->emit commitData( editor );
  } );
  return editor;
}


//
// TypeDelegate
//

QgsFieldMappingWidget::TypeDelegate::TypeDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

QWidget *QgsFieldMappingWidget::TypeDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  QComboBox *editor = new QComboBox( parent );

  const QList<QgsVectorDataProvider::NativeType> typeList = QgsFieldMappingModel::supportedDataTypes();
  for ( int i = 0; i < typeList.size(); i++ )
  {
    editor->addItem( QgsFields::iconForFieldType( typeList[i].mType, typeList[i].mSubType ), typeList[i].mTypeDesc );
    editor->setItemData( i, typeList[i].mTypeName, Qt::UserRole );
  }

  const QgsFieldMappingModel *model { qobject_cast<const QgsFieldMappingModel *>( index.model() ) };

  if ( model && !model->destinationEditable() )
  {
    editor->setEnabled( false );
  }
  else
  {
    connect( editor,
             qOverload<int >( &QComboBox::currentIndexChanged ),
             this,
             [ = ]( int currentIndex )
    {
      Q_UNUSED( currentIndex )
      const_cast< QgsFieldMappingWidget::TypeDelegate *>( this )->emit commitData( editor );
    } );
  }
  return editor;
}

void QgsFieldMappingWidget::TypeDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QComboBox *editorWidget { qobject_cast<QComboBox *>( editor ) };
  if ( ! editorWidget )
    return;

  const QVariant value = index.model()->data( index, Qt::EditRole );
  editorWidget->setCurrentIndex( editorWidget->findData( value ) );
}

void QgsFieldMappingWidget::TypeDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *editorWidget { qobject_cast<QComboBox *>( editor ) };
  if ( ! editorWidget )
    return;

  const QVariant currentValue = editorWidget->currentData( );
  model->setData( index, currentValue, Qt::EditRole );
}
