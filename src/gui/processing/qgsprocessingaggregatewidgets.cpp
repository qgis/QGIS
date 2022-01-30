/***************************************************************************
  qgsprocessingaggregatewidgets.cpp
  ---------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingaggregatewidgets.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsfieldmappingwidget.h"

#include <QBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolButton>
#include <QTableView>
#include <mutex>

//
// QgsAggregateMappingModel
//

QgsAggregateMappingModel::QgsAggregateMappingModel( const QgsFields &sourceFields,
    QObject *parent )
  : QAbstractTableModel( parent )
  , mExpressionContextGenerator( new QgsFieldMappingModel::ExpressionContextGenerator( sourceFields ) )
{
  setSourceFields( sourceFields );
}

QVariant QgsAggregateMappingModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    switch ( orientation )
    {
      case Qt::Horizontal:
      {
        switch ( static_cast<ColumnDataIndex>( section ) )
        {
          case ColumnDataIndex::SourceExpression:
          {
            return tr( "Source Expression" );
          }
          case ColumnDataIndex::Aggregate:
          {
            return tr( "Aggregate Function" );
          }
          case ColumnDataIndex::Delimiter:
          {
            return tr( "Delimiter" );
          }
          case ColumnDataIndex::DestinationName:
          {
            return tr( "Name" );
          }
          case ColumnDataIndex::DestinationType:
          {
            return tr( "Type" );
          }
          case ColumnDataIndex::DestinationLength:
          {
            return tr( "Length" );
          }
          case ColumnDataIndex::DestinationPrecision:
          {
            return tr( "Precision" );
          }
        }
        break;
      }
      case Qt::Vertical:
      {
        return section;
      }
    }
  }
  return QVariant();
}

QgsFields QgsAggregateMappingModel::sourceFields() const
{
  return mSourceFields;
}

int QgsAggregateMappingModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mMapping.count();
}

int QgsAggregateMappingModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return 7;
}

QVariant QgsAggregateMappingModel::data( const QModelIndex &index, int role ) const
{
  if ( index.isValid() )
  {
    const ColumnDataIndex col { static_cast<ColumnDataIndex>( index.column() ) };
    const Aggregate &agg { mMapping.at( index.row() ) };

    switch ( role )
    {
      case Qt::DisplayRole:
      case Qt::EditRole:
      {
        switch ( col )
        {
          case ColumnDataIndex::SourceExpression:
          {
            return agg.source;
          }
          case ColumnDataIndex::Aggregate:
          {
            return agg.aggregate;
          }
          case ColumnDataIndex::Delimiter:
          {
            return agg.delimiter;
          }
          case ColumnDataIndex::DestinationName:
          {
            return agg.field.displayName();
          }
          case ColumnDataIndex::DestinationType:
          {
            return agg.field.typeName();
          }
          case ColumnDataIndex::DestinationLength:
          {
            return agg.field.length();
          }
          case ColumnDataIndex::DestinationPrecision:
          {
            return agg.field.precision();
          }
        }
        break;
      }
    }
  }
  return QVariant();
}

Qt::ItemFlags QgsAggregateMappingModel::flags( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    return Qt::ItemFlags( Qt::ItemIsSelectable |
                          Qt::ItemIsEditable |
                          Qt::ItemIsEnabled );
  }
  return Qt::ItemFlags();
}

bool QgsAggregateMappingModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.isValid() )
  {
    if ( role == Qt::EditRole )
    {
      Aggregate &f = mMapping[index.row()];
      switch ( static_cast<ColumnDataIndex>( index.column() ) )
      {
        case ColumnDataIndex::SourceExpression:
        {
          const QgsExpression exp { value.toString() };
          f.source = exp;
          break;
        }
        case ColumnDataIndex::Aggregate:
        {
          f.aggregate = value.toString();
          break;
        }
        case ColumnDataIndex::Delimiter:
        {
          f.delimiter = value.toString();
          break;
        }
        case ColumnDataIndex::DestinationName:
        {
          f.field.setName( value.toString() );
          break;
        }
        case ColumnDataIndex::DestinationType:
        {
          QgsFieldMappingModel::setFieldTypeFromName( f.field, value.toString() );
          break;
        }
        case ColumnDataIndex::DestinationLength:
        {
          bool ok;
          const int length { value.toInt( &ok ) };
          if ( ok )
            f.field.setLength( length );
          break;
        }
        case ColumnDataIndex::DestinationPrecision:
        {
          bool ok;
          const int precision { value.toInt( &ok ) };
          if ( ok )
            f.field.setPrecision( precision );
          break;
        }
      }
      emit dataChanged( index, index );
    }
    return true;
  }
  else
  {
    return false;
  }
}


bool QgsAggregateMappingModel::moveUpOrDown( const QModelIndex &index, bool up )
{
  if ( ! index.isValid() && index.model() == this )
    return false;

  // Always swap down
  const int row { up ? index.row() - 1 : index.row() };
  // Range checking
  if ( row < 0 || row + 1 >= rowCount( QModelIndex() ) )
  {
    return false;
  }
  beginMoveRows( QModelIndex( ), row, row, QModelIndex(), row + 2 );
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
  mMapping.swap( row, row + 1 );
#else
  mMapping.swapItemsAt( row, row + 1 );
#endif
  endMoveRows();
  return true;
}

void QgsAggregateMappingModel::setSourceFields( const QgsFields &sourceFields )
{
  mSourceFields = sourceFields;
  if ( mExpressionContextGenerator )
    mExpressionContextGenerator->setSourceFields( mSourceFields );

  const QStringList usedFields;
  beginResetModel();
  mMapping.clear();

  for ( const QgsField &f : sourceFields )
  {
    Aggregate aggregate;
    aggregate.field = f;
    aggregate.field.setTypeName( QgsFieldMappingModel::qgsFieldToTypeName( f ) );
    aggregate.source = QgsExpression::quotedColumnRef( f.name() );

    if ( f.isNumeric() )
      aggregate.aggregate = QStringLiteral( "sum" );
    else if ( f.type() == QVariant::String || ( f.type() == QVariant::List && f.subType() == QVariant::String ) )
      aggregate.aggregate = QStringLiteral( "concatenate" );

    aggregate.delimiter = ',';

    mMapping.push_back( aggregate );
  }
  endResetModel();
}

QgsExpressionContextGenerator *QgsAggregateMappingModel::contextGenerator() const
{
  return mExpressionContextGenerator.get();
}

void QgsAggregateMappingModel::setBaseExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator->setBaseExpressionContextGenerator( generator );
}

QList<QgsAggregateMappingModel::Aggregate> QgsAggregateMappingModel::mapping() const
{
  return mMapping;
}

void QgsAggregateMappingModel::setMapping( const QList<QgsAggregateMappingModel::Aggregate> &mapping )
{
  beginResetModel();
  mMapping = mapping;
  for ( auto &agg : mMapping )
  {
    agg.field.setTypeName( QgsFieldMappingModel::qgsFieldToTypeName( agg.field ) );
  }
  endResetModel();
}

void QgsAggregateMappingModel::appendField( const QgsField &field, const QString &source, const QString &aggregate )
{
  const int lastRow { rowCount( QModelIndex( ) ) };
  beginInsertRows( QModelIndex(), lastRow, lastRow );
  Aggregate agg;
  agg.field = field;
  agg.field.setTypeName( QgsFieldMappingModel::qgsFieldToTypeName( field ) );
  agg.source = source;
  agg.aggregate = aggregate;
  agg.delimiter = ',';
  mMapping.push_back( agg );
  endInsertRows( );
}

bool QgsAggregateMappingModel::removeField( const QModelIndex &index )
{
  if ( index.isValid() && index.model() == this && index.row() < rowCount( QModelIndex() ) )
  {
    beginRemoveRows( QModelIndex(), index.row(), index.row() );
    mMapping.removeAt( index.row() );
    endRemoveRows();
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsAggregateMappingModel::moveUp( const QModelIndex &index )
{
  return moveUpOrDown( index );
}

bool QgsAggregateMappingModel::moveDown( const QModelIndex &index )
{
  return moveUpOrDown( index, false );
}


//
// QgsAggregateMappingWidget
//

QgsAggregateMappingWidget::QgsAggregateMappingWidget( QWidget *parent,
    const QgsFields &sourceFields )
  : QgsPanelWidget( parent )
{
  QVBoxLayout *verticalLayout = new QVBoxLayout();
  verticalLayout->setContentsMargins( 0, 0, 0, 0 );
  mTableView = new QTableView();
  verticalLayout->addWidget( mTableView );
  setLayout( verticalLayout );

  mModel = new QgsAggregateMappingModel( sourceFields, this );
  mTableView->setModel( mModel );
  mTableView->setItemDelegateForColumn( static_cast<int>( QgsAggregateMappingModel::ColumnDataIndex::SourceExpression ), new QgsFieldMappingWidget::ExpressionDelegate( this ) );
  mTableView->setItemDelegateForColumn( static_cast<int>( QgsAggregateMappingModel::ColumnDataIndex::Aggregate ), new QgsAggregateMappingWidget::AggregateDelegate( mTableView ) );
  mTableView->setItemDelegateForColumn( static_cast<int>( QgsAggregateMappingModel::ColumnDataIndex::DestinationType ), new QgsFieldMappingWidget::TypeDelegate( mTableView ) );
  updateColumns();
  // Make sure columns are updated when rows are added
  connect( mModel, &QgsAggregateMappingModel::rowsInserted, this, [ = ] { updateColumns(); } );
  connect( mModel, &QgsAggregateMappingModel::modelReset, this, [ = ] { updateColumns(); } );
  connect( mModel, &QgsAggregateMappingModel::dataChanged, this, &QgsAggregateMappingWidget::changed );
  connect( mModel, &QgsAggregateMappingModel::rowsInserted, this, &QgsAggregateMappingWidget::changed );
  connect( mModel, &QgsAggregateMappingModel::rowsRemoved, this, &QgsAggregateMappingWidget::changed );
  connect( mModel, &QgsAggregateMappingModel::modelReset, this, &QgsAggregateMappingWidget::changed );
}

QgsAggregateMappingModel *QgsAggregateMappingWidget::model() const
{
  return qobject_cast<QgsAggregateMappingModel *>( mModel );
}

QList<QgsAggregateMappingModel::Aggregate> QgsAggregateMappingWidget::mapping() const
{
  return model()->mapping();
}

void QgsAggregateMappingWidget::setMapping( const QList<QgsAggregateMappingModel::Aggregate> &mapping )
{
  model()->setMapping( mapping );
}

QItemSelectionModel *QgsAggregateMappingWidget::selectionModel()
{
  return mTableView->selectionModel();
}

void QgsAggregateMappingWidget::setSourceFields( const QgsFields &sourceFields )
{
  model()->setSourceFields( sourceFields );
}

void QgsAggregateMappingWidget::setSourceLayer( QgsVectorLayer *layer )
{
  mSourceLayer = layer;
}

QgsVectorLayer *QgsAggregateMappingWidget::sourceLayer()
{
  return mSourceLayer;
}

void QgsAggregateMappingWidget::scrollTo( const QModelIndex &index ) const
{
  mTableView->scrollTo( index );
}

void QgsAggregateMappingWidget::registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  model()->setBaseExpressionContextGenerator( generator );
}

void QgsAggregateMappingWidget::appendField( const QgsField &field, const QString &source, const QString &aggregate )
{
  model()->appendField( field, source, aggregate );
}

bool QgsAggregateMappingWidget::removeSelectedFields()
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

bool QgsAggregateMappingWidget::moveSelectedFieldsUp()
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

bool QgsAggregateMappingWidget::moveSelectedFieldsDown()
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

void QgsAggregateMappingWidget::updateColumns()
{
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    mTableView->openPersistentEditor( mModel->index( i, static_cast<int>( QgsAggregateMappingModel::ColumnDataIndex::SourceExpression ) ) );
    mTableView->openPersistentEditor( mModel->index( i, static_cast<int>( QgsAggregateMappingModel::ColumnDataIndex::DestinationType ) ) );
    mTableView->openPersistentEditor( mModel->index( i, static_cast<int>( QgsAggregateMappingModel::ColumnDataIndex::Aggregate ) ) );
  }

  for ( int i = 0; i < mModel->columnCount(); ++i )
  {
    mTableView->resizeColumnToContents( i );
  }
}

std::list<int> QgsAggregateMappingWidget::selectedRows()
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
// AggregateDelegate
//

QgsAggregateMappingWidget::AggregateDelegate::AggregateDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

QWidget *QgsAggregateMappingWidget::AggregateDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex & ) const
{
  Q_UNUSED( option )
  QComboBox *editor = new QComboBox( parent );

  const QStringList aggregateList { QgsAggregateMappingWidget::AggregateDelegate::aggregates() };
  int i = 0;
  for ( const QString &aggregate : aggregateList )
  {
    editor->addItem( aggregate );
    editor->setItemData( i, aggregate, Qt::UserRole );
    ++i;
  }

  connect( editor,
           qOverload<int >( &QComboBox::currentIndexChanged ),
           this,
           [ = ]( int currentIndex )
  {
    Q_UNUSED( currentIndex )
    const_cast< QgsAggregateMappingWidget::AggregateDelegate *>( this )->emit commitData( editor );
  } );

  return editor;
}

void QgsAggregateMappingWidget::AggregateDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QComboBox *editorWidget { qobject_cast<QComboBox *>( editor ) };
  if ( ! editorWidget )
    return;

  const QVariant value = index.model()->data( index, Qt::EditRole );
  editorWidget->setCurrentIndex( editorWidget->findData( value ) );
}

void QgsAggregateMappingWidget::AggregateDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *editorWidget { qobject_cast<QComboBox *>( editor ) };
  if ( ! editorWidget )
    return;

  const QVariant currentValue = editorWidget->currentData( );
  model->setData( index, currentValue, Qt::EditRole );
}

const QStringList QgsAggregateMappingWidget::AggregateDelegate::aggregates()
{
  static QStringList sAggregates;
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    sAggregates << QStringLiteral( "first_value" )
                << QStringLiteral( "last_value" );

    const QList<QgsExpressionFunction *> functions = QgsExpression::Functions();
    for ( const QgsExpressionFunction *function : functions )
    {
      if ( !function || function->isDeprecated() || function->name().isEmpty() || function->name().at( 0 ) == '_' )
        continue;

      if ( function->groups().contains( QLatin1String( "Aggregates" ) ) )
      {
        if ( function->name() == QLatin1String( "aggregate" )
             || function->name() == QLatin1String( "relation_aggregate" ) )
          continue;

        sAggregates.append( function->name() );
      }

      std::sort( sAggregates.begin(), sAggregates.end() );
    }
  } );

  return sAggregates;
}

