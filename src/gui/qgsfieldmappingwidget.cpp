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
#include "qgsexpressioncontextutils.h"

QgsFieldMappingWidget::QgsFieldMappingWidget(const QgsFields &sourceFields,
                                             const QgsFields &destinationFields,
                                             const QMap<QString, QgsExpression> &expressions,
                                             QWidget *parent)
  : QWidget(parent)
  , mSourceFields( sourceFields )
  , mDestinationFields( destinationFields )
  , mExpressions( expressions )
{

  setupUi( this );

  mModel = new QgsFieldMappingModel( sourceFields, destinationFields, expressions, this );
  mTableView->setModel( mModel );
  mTableView->setItemDelegateForColumn( 0, new ExpressionDelegate( mTableView ) );

  for (int i=0; i<mModel->rowCount(); ++i)
  {
    mTableView->openPersistentEditor( mModel->index(i, 0));
  }

  for (int i=0; i<mModel->columnCount(); ++i)
  {
    mTableView->resizeColumnToContents( i );
  }
}

QMap<QString, QgsExpression> QgsFieldMappingWidget::expressions() const
{
  QMap<QString, QgsExpression> results;
  for ( const auto &f: qgis::as_const( mDestinationFields ) )
  {
    results[ f.name() ] = mExpressions.value( f.name(), QgsExpression() );
  }
  return results;
}

void QgsFieldMappingWidget::ExpressionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QgsFieldExpressionWidget *editorWidget { qobject_cast<QgsFieldExpressionWidget *>( editor ) };
  if ( ! editorWidget )
      return;

  bool isExpression;
  bool isValid;
  const QString currentValue { editorWidget->currentField( &isExpression, &isValid ) };
  if ( isExpression )
  {
    model->setData( index, currentValue , Qt::EditRole );
  }
  else
  {
    model->setData(index, QgsExpression::quotedColumnRef( currentValue ), Qt::EditRole );
  }
}

void QgsFieldMappingWidget::ExpressionDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QgsFieldExpressionWidget *editorWidget { qobject_cast<QgsFieldExpressionWidget *>( editor ) };
  if ( ! editorWidget )
      return;

  const auto value = index.model()->data(index, Qt::EditRole);
  editorWidget->setField( value.toString() );

}

QWidget* QgsFieldMappingWidget::ExpressionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED( option )   QgsFieldExpressionWidget *editor = new QgsFieldExpressionWidget( parent );
   editor->setAutoFillBackground( true );
   editor->setAllowEvalErrors( false );
   editor->setAllowEmptyFieldName( true );
   const QgsFieldMappingModel *model { qobject_cast<const QgsFieldMappingModel*>( index.model() ) };
   Q_ASSERT( model );
   editor->registerExpressionContextGenerator( model->contextGenerator() );
   editor->setFields( model->sourceFields() );
   editor->setField( index.model()->data(index, Qt::DisplayRole ).toString() );
   connect (editor,
            qgis::overload<const  QString&, bool >::of( &QgsFieldExpressionWidget::fieldChanged ),
            this,
            [ = ] (const QString &fieldName, bool isValid )
   {
     Q_UNUSED( fieldName )
     Q_UNUSED( isValid )
     const_cast< QgsFieldMappingWidget::ExpressionDelegate *>( this )->emit commitData( editor );
   });
   return editor;
}

QgsFieldMappingWidget::ExpressionDelegate::ExpressionDelegate(QObject* parent)
  : QStyledItemDelegate( parent )
{
}


QVariant QgsFieldMappingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Horizontal )
    {
      if ( section == 0)
        return tr( "Source expression" );
      else if ( section == 1 )
        return tr( "Field name" );
      else if ( section == 2 )
        return tr( "Type" );
      else if ( section == 3 )
        return tr( "Length" );
      else if ( section == 4 )
        return tr( "Precision" );
      else
        return QString();
    }
    else if ( orientation == Qt::Vertical )
    {
      return section;
    }
  }
  return QVariant();
}

QgsFields QgsFieldMappingModel::sourceFields() const
{
  return mSourceFields;
}

QgsFieldMappingModel::QgsFieldMappingModel(const QgsFields& sourceFields,
                                           const QgsFields& destinationFields,
                                           const QMap<QString, QgsExpression>& expressions,
                                           QObject* parent)
  : QAbstractTableModel( parent )
  , mSourceFields( sourceFields )
  , mDestinationFields( destinationFields )
  , mExpressionContextGenerator( new ExpressionContextGenerator( &mSourceFields ) )
{
  // Prepare the model data
  QStringList usedFields;
  for (const auto &df: qgis::as_const( destinationFields ) )
  {
    Field f;
    f.name = df.name();
    f.type = df.type();
    f.length = df.length();
    f.precision = df.precision();
    if ( expressions.contains( f.name ) )
    {
      f.expression = expressions.value( f.name );
      // if it's source field
      if ( f.expression.isField() &&
           mSourceFields.names().contains( f.expression.referencedColumns().toList().first() ) )
      {
        usedFields.push_back( f.expression.referencedColumns().toList().first() );
      }
    }
    else
    {
      bool found { false };
      // Search for fields in the source
      // 1. match by name
      for ( const auto &sf: qgis::as_const( mSourceFields ) )
      {
        if ( sf.name() == f.name )
        {
          f.expression = QgsExpression::quotedColumnRef( sf.name() );
          found = true;
          usedFields.push_back( sf.name() );
          break;
        }
      }
      // 2. match by type
      if ( ! found )
      {
        for ( const auto &sf: qgis::as_const( mSourceFields ) )
        {
          if ( usedFields.contains( sf.name() ) || sf.type() != f.type )
            continue;
          f.expression = QgsExpression::quotedColumnRef( sf.name() );
          usedFields.push_back( sf.name() );
          found = true;
        }
      }
    }

    mMapping.push_back( f );
  }
}

int QgsFieldMappingModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED( parent );
  return mDestinationFields.count();
}

int QgsFieldMappingModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED( parent );
  return 4;
}

QVariant QgsFieldMappingModel::data(const QModelIndex& index, int role) const
{
  if ( index.isValid() )
  {

    const int col { index.column() };
    const Field &f { mMapping.at( index.row() ) };

    if ( role == Qt::DisplayRole )
    {
      // First column is the expression widget
      if ( col == 0 )
      {
        return f.expression.expression();
      }
      else if ( col == 1)
      {
        return f.name;
      }
      else if ( col == 2 )
      {
        return f.length;
      }
      else if ( col == 3 ) {
        return f.precision;
      }
    }

    if ( role == Qt::EditRole )
    {
      if ( col == 0 )
      {
        return f.expression.expression();
      }
      else
      {
        return QVariant();
      }
    }
  }
  return QVariant();
}

QgsExpressionContextGenerator* QgsFieldMappingModel::contextGenerator() const
{
  return mExpressionContextGenerator.get();
}


QgsFieldMappingModel::ExpressionContextGenerator::ExpressionContextGenerator(const QgsFields* sourceFields)
{
  mSourceFields = sourceFields;
}

QgsExpressionContext QgsFieldMappingModel::ExpressionContextGenerator::createExpressionContext() const
{
  QgsExpressionContext ctx;
  ctx.appendScope( QgsExpressionContextUtils::globalScope() );
  ctx.setFields( *mSourceFields );
  QgsFeature feature { *mSourceFields };
  feature.setValid( true );
  ctx.setFeature( feature );
  return ctx;
}

Qt::ItemFlags QgsFieldMappingModel::flags(const QModelIndex& index) const
{
  if ( index.isValid() && index.column() == 0 )
    return Qt::ItemFlags(Qt::ItemIsSelectable |
                          Qt::ItemIsEditable |
                          Qt::ItemIsEnabled);
  return Qt::ItemFlags();

}

bool QgsFieldMappingModel::setData(const QModelIndex& index, const QVariant& value, int role)
{

  if ( index.isValid() )
  {
    if ( role == Qt::EditRole )
    {
      Field &f { const_cast<Field&>( mMapping.at( index.row() ) ) };
      const QgsExpression exp { value.toString() };
      f.expression = exp;
      emit dataChanged(index, index);
    }
  }
  return true;
}
