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
        case static_cast<int>( ColumnDataIndex::SourceExpression ):
        {
          return tr( "Source expression" );
        }
        case static_cast<int>( ColumnDataIndex::DestinationName ):
        {
          return tr( "Name" );
        }
        case static_cast<int>( ColumnDataIndex::DestinationType ):
        {
          return tr( "Type" );
        }
        case static_cast<int>( ColumnDataIndex::DestinationLength ):
        {
          return tr( "Length" );
        }
        case static_cast<int>( ColumnDataIndex::DestinationPrecision ):
        {
          return tr( "Precision" );
        }
        case static_cast<int>( ColumnDataIndex::DestinationConstraints ):
        {
          return tr( "Constraints" );
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
  return 6;
}

QVariant QgsFieldMappingModel::data( const QModelIndex &index, int role ) const
{
  if ( index.isValid() )
  {
    const int col { index.column() };
    const Field &f { mMapping.at( index.row() ) };

    const QgsFieldConstraints::Constraints constraints { fieldConstraints( f.field ) };

    if ( role == Qt::DisplayRole || role == Qt::EditRole )
    {
      switch ( col )
      {
          {
          case static_cast<int>( ColumnDataIndex::SourceExpression ):
          {
            return f.expression.expression();
          }
          case static_cast<int>( ColumnDataIndex::DestinationName ):
          {
            return f.field.displayName();
          }
          case static_cast<int>( ColumnDataIndex::DestinationType ):
          {
            return static_cast<int>( f.field.type() );
          }
          case static_cast<int>( ColumnDataIndex::DestinationLength ):
          {
            return f.field.length();
          }
          case static_cast<int>( ColumnDataIndex::DestinationPrecision ):
          {
            return f.field.precision();
          }
          case static_cast<int>( ColumnDataIndex::DestinationConstraints ):
          {
            return constraints != 0 ? tr( "Constraints active" ) : QString();
          }
        }
      }
    }
    else if ( role == Qt::ToolTipRole &&
              col == static_cast<int>( ColumnDataIndex::DestinationConstraints ) &&
              constraints != 0 )
    {
      QStringList constraintDescription;
      if ( constraints.testFlag( QgsFieldConstraints::Constraint::ConstraintUnique ) )
      {
        constraintDescription.push_back( tr( "Unique" ) );
      }
      if ( constraints.testFlag( QgsFieldConstraints::Constraint::ConstraintNotNull ) )
      {
        constraintDescription.push_back( tr( "Not null" ) );
      }
      if ( constraints.testFlag( QgsFieldConstraints::Constraint::ConstraintNotNull ) )
      {
        constraintDescription.push_back( tr( "Expression" ) );
      }
      return constraintDescription.join( QStringLiteral( "<br>" ) );
    }
    else if ( role == Qt::BackgroundRole &&
              constraints != 0 )
    {
      return QBrush( QColor( 255, 224, 178 ) );
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
       index.column() != static_cast<int>( ColumnDataIndex::DestinationConstraints ) &&
       ( index.column() == static_cast<int>( ColumnDataIndex::SourceExpression ) || destinationEditable() ) )
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
      Field &f = mMapping[index.row()];
      switch ( index.column() )
      {
        case static_cast<int>( ColumnDataIndex::SourceExpression ):
        {
          const QgsExpression exp { value.toString() };
          f.expression = exp;
          break;
        }
        case static_cast<int>( ColumnDataIndex::DestinationName ):
        {
          f.field.setName( value.toString() );
          break;
        }
        case static_cast<int>( ColumnDataIndex::DestinationType ):
        {
          f.field.setType( static_cast<QVariant::Type>( value.toInt( ) ) );
          break;
        }
        case static_cast<int>( ColumnDataIndex::DestinationLength ):
        {
          bool ok;
          const int length { value.toInt( &ok ) };
          if ( ok )
            f.field.setLength( length );
          break;
        }
        case static_cast<int>( ColumnDataIndex::DestinationPrecision ):
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
  }
  return true;
}

QgsFieldConstraints::Constraints QgsFieldMappingModel::fieldConstraints( const QgsField &field ) const
{
  QgsFieldConstraints::Constraints constraints;

  const QgsFieldConstraints fieldConstraints { field.constraints() };

  if ( fieldConstraints.constraints() & QgsFieldConstraints::ConstraintNotNull &&
       fieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) & QgsFieldConstraints::ConstraintStrengthHard )
    constraints.setFlag( QgsFieldConstraints::ConstraintNotNull );

  if ( fieldConstraints.constraints() & QgsFieldConstraints::ConstraintUnique &&
       fieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) & QgsFieldConstraints::ConstraintStrengthHard )
    constraints.setFlag( QgsFieldConstraints::ConstraintUnique );

  if ( fieldConstraints.constraints() & QgsFieldConstraints::ConstraintExpression &&
       fieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) & QgsFieldConstraints::ConstraintStrengthHard )
    constraints.setFlag( QgsFieldConstraints::ConstraintExpression );

  return constraints;
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
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
  mMapping.swap( row, row + 1 );
#else
  mMapping.swapItemsAt( row, row + 1 );
#endif
  endMoveRows();
  return true;
}

QString QgsFieldMappingModel::bestMatchforField( const QgsFieldMappingModel::Field &f, QStringList &excludedFieldNames )
{
  // Search for fields in the source
  // 1. match by name
  for ( const auto &sf : qgis::as_const( mSourceFields ) )
  {
    if ( sf.name() == f.field.name() )
    {
      return QgsExpression::quotedColumnRef( sf.name() );
    }
  }
  // 2. match by type
  for ( const auto &sf : qgis::as_const( mSourceFields ) )
  {
    if ( excludedFieldNames.contains( sf.name() ) || sf.type() != f.field.type() )
      continue;
    excludedFieldNames.push_back( sf.name() );
    return QgsExpression::quotedColumnRef( sf.name() );
  }
  return QString();
}

void QgsFieldMappingModel::setSourceFields( const QgsFields &sourceFields )
{
  mSourceFields = sourceFields;
  QStringList usedFields;
  beginResetModel();
  for ( const Field &f : qgis::as_const( mMapping ) )
  {
    if ( f.expression.isField() )
    {
      usedFields.push_back( f.expression.expression().mid( 1, f.expression.expression().length() - 2 ) );
    }
  }
  // not const on purpose
  for ( Field &f : mMapping )
  {
    if ( f.expression.expression().isEmpty() )
    {
      const QString expression { bestMatchforField( f, usedFields ) };
      if ( ! expression.isEmpty() )
        f.expression = expression;
    }
  }
  endResetModel();
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
      const QString expression { bestMatchforField( f, usedFields ) };
      if ( ! expression.isEmpty() )
        f.expression = expression;
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

