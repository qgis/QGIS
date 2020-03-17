/***************************************************************************
  qgsfieldmappingmodel.cpp - QgsFieldMappingModel

 ---------------------
 begin                : 17.3.2020
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

#include "qgsfieldmappingmodel.h"
#include "qgsexpressioncontextutils.h"


QgsFieldMappingModel::QgsFieldMappingModel( const QgsFields &sourceFields,
    const QgsFields &destinationFields,
    const QMap<QString, QgsExpression> &expressions,
    QObject *parent )
  : QAbstractTableModel( parent )
  , mSourceFields( sourceFields )
  , mExpressionContextGenerator( new ExpressionContextGenerator( &mSourceFields ) )
{
  setDestinationFields( destinationFields, expressions );
}

QVariant QgsFieldMappingModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Horizontal )
    {
      switch ( section )
      {
        case ColumnDataIndex::SourceExpression:
        {
          return tr( "Source expression" );
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

int QgsFieldMappingModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mMapping.count();
}

int QgsFieldMappingModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 5;
}

QVariant QgsFieldMappingModel::data( const QModelIndex &index, int role ) const
{
  if ( index.isValid() )
  {
    const int col { index.column() };
    const Field &f { mMapping.at( index.row() ) };

    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {
      switch ( col )
      {
          {
          case ColumnDataIndex::SourceExpression:
          {
            return f.expression.expression();
          }
          case ColumnDataIndex::DestinationName:
          {
            return f.field.displayName();
          }
          case ColumnDataIndex::DestinationType:
          {
            return static_cast<int>( f.field.type() );
          }
          case ColumnDataIndex::DestinationLength:
          {
            return f.field.length();
          }
          case ColumnDataIndex::DestinationPrecision:
          {
            return f.field.precision();
          }
        }
      }
    }
  }
  return QVariant();
}

QgsExpressionContextGenerator *QgsFieldMappingModel::contextGenerator() const
{
  return mExpressionContextGenerator.get();
}


QgsFieldMappingModel::ExpressionContextGenerator::ExpressionContextGenerator( const QgsFields *sourceFields )
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

Qt::ItemFlags QgsFieldMappingModel::flags( const QModelIndex &index ) const
{
  if ( index.isValid() &&
       ( index.column() == ColumnDataIndex::SourceExpression || destinationEditable() ) )
  {
    return Qt::ItemFlags( Qt::ItemIsSelectable |
                          Qt::ItemIsEditable |
                          Qt::ItemIsEnabled );
  }
  return Qt::ItemFlags();
}

bool QgsFieldMappingModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.isValid() )
  {
    if ( role == Qt::EditRole )
    {
      Field &f { const_cast<Field &>( mMapping.at( index.row() ) ) };
      switch ( index.column() )
      {
          {
          case ColumnDataIndex::SourceExpression:
          {
            const QgsExpression exp { value.toString() };
            f.expression = exp;
            break;
          }
          case ColumnDataIndex::DestinationName:
          {
            f.field.setName( value.toString() );
            break;
          }
          case ColumnDataIndex::DestinationType:
          {
            f.field.setType( static_cast<QVariant::Type>( value.toInt( ) ) );
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
      }
      emit dataChanged( index, index );
    }
  }
  return true;
}

bool QgsFieldMappingModel::moveUpOrDown( const QModelIndex &index, bool up )
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
  mMapping.swapItemsAt( row, row + 1 );
  endMoveRows();
  return true;
}

void QgsFieldMappingModel::setSourceFields( const QgsFields &sourceFields )
{
  mSourceFields = sourceFields;
}

void QgsFieldMappingModel::setDestinationFields( const QgsFields &destinationFields,
    const QMap<QString, QgsExpression> &expressions )
{
  beginResetModel();
  mMapping.clear();
  // Prepare the model data
  QStringList usedFields;
  for ( const auto &df : qgis::as_const( destinationFields ) )
  {
    Field f;
    f.field = df;
    f.originalName = df.name();
    if ( expressions.contains( f.field.name() ) )
    {
      f.expression = expressions.value( f.field.name() );
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
      for ( const auto &sf : qgis::as_const( mSourceFields ) )
      {
        if ( sf.name() == f.field.name() )
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
        for ( const auto &sf : qgis::as_const( mSourceFields ) )
        {
          if ( usedFields.contains( sf.name() ) || sf.type() != f.field.type() )
            continue;
          f.expression = QgsExpression::quotedColumnRef( sf.name() );
          usedFields.push_back( sf.name() );
          found = true;
        }
      }
    }
    mMapping.push_back( f );
  }
  endResetModel();
}

bool QgsFieldMappingModel::destinationEditable() const
{
  return mDestinationEditable;
}

void QgsFieldMappingModel::setDestinationEditable( bool destinationEditable )
{
  mDestinationEditable = destinationEditable;
}

const QMap<QVariant::Type, QString> QgsFieldMappingModel::dataTypes() const
{
  static const QMap<QVariant::Type, QString> sDataTypes
  {
    { QVariant::Type::Int, tr( "Whole number (integer - 32bit)" ) },
    { QVariant::Type::LongLong, tr( "Whole number (integer - 64bit)" ) },
    { QVariant::Type::Double, tr( "Decimal number (double)" ) },
    { QVariant::Type::String, tr( "Text (string)" ) },
    { QVariant::Type::Date, tr( "Date" ) },
    { QVariant::Type::Time, tr( "Time" ) },
    { QVariant::Type::DateTime, tr( "Date & Time" ) },
    { QVariant::Type::Bool, tr( "Boolean" ) },
    { QVariant::Type::ByteArray, tr( "Binary object (BLOB)" ) },
  };
  return sDataTypes;
}

QList<QgsFieldMappingModel::Field> QgsFieldMappingModel::mapping() const
{
  return mMapping;
}

void QgsFieldMappingModel::appendField( const QgsField &field, const QgsExpression &expression )
{
  const int lastRow { rowCount( QModelIndex( ) ) };
  beginInsertRows( QModelIndex(), lastRow, lastRow );
  Field f;
  f.field = field;
  f.expression = expression;
  f.originalName = field.name();
  mMapping.push_back( f );
  endInsertRows( );
}

bool QgsFieldMappingModel::removeField( const QModelIndex &index )
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

bool QgsFieldMappingModel::moveUp( const QModelIndex &index )
{
  return moveUpOrDown( index );
}

bool QgsFieldMappingModel::moveDown( const QModelIndex &index )
{
  return moveUpOrDown( index, false );
}

