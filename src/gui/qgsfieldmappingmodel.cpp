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
#include "qgsexpressionnodeimpl.h"
#include "qgsvariantutils.h"

#include "moc_qgsfieldmappingmodel.cpp"

QgsFieldMappingModel::QgsFieldMappingModel( const QgsFields &sourceFields, const QgsFields &destinationFields, const QMap<QString, QString> &expressions, QObject *parent )
  : QAbstractTableModel( parent )
  , mNativeTypes( supportedDataTypes() )
  , mSourceFields( sourceFields )
  , mExpressionContextGenerator( new ExpressionContextGenerator( mSourceFields ) )
{
  setDestinationFields( destinationFields, expressions );
}

void QgsFieldMappingModel::setNativeTypes( const QList<QgsVectorDataProvider::NativeType> &nativeTypes )
{
  mNativeTypes = nativeTypes.isEmpty() ? supportedDataTypes() : nativeTypes;
}

QVariant QgsFieldMappingModel::headerData( int section, Qt::Orientation orientation, int role ) const
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
          case ColumnDataIndex::DestinationConstraints:
          {
            return tr( "Constraints" );
          }
          case ColumnDataIndex::DestinationAlias:
          {
            return tr( "Alias" );
          }
          case ColumnDataIndex::DestinationComment:
          {
            return tr( "Comment" );
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

QgsFields QgsFieldMappingModel::sourceFields() const
{
  return mSourceFields;
}

int QgsFieldMappingModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mMapping.count();
}

int QgsFieldMappingModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return 8;
}

QVariant QgsFieldMappingModel::data( const QModelIndex &index, int role ) const
{
  if ( index.isValid() )
  {
    const ColumnDataIndex col { static_cast<ColumnDataIndex>( index.column() ) };
    const Field &f { mMapping.at( index.row() ) };

    const QgsFieldConstraints::Constraints constraints { fieldConstraints( f.field ) };

    switch ( role )
    {
      case Qt::DisplayRole:
      case Qt::EditRole:
      {
        switch ( col )
        {
          case ColumnDataIndex::SourceExpression:
          {
            return f.expression;
          }
          case ColumnDataIndex::DestinationName:
          {
            return f.field.name();
          }
          case ColumnDataIndex::DestinationType:
          {
            return f.field.typeName();
          }
          case ColumnDataIndex::DestinationLength:
          {
            return f.field.length();
          }
          case ColumnDataIndex::DestinationPrecision:
          {
            return f.field.precision();
          }
          case ColumnDataIndex::DestinationConstraints:
          {
            return constraints != 0 ? tr( "Constraints active" ) : QString();
          }
          case ColumnDataIndex::DestinationAlias:
          {
            return f.field.alias();
          }
          case ColumnDataIndex::DestinationComment:
          {
            return f.field.comment();
          }
        }
        break;
      }
      case Qt::ToolTipRole:
      {
        if ( col == ColumnDataIndex::DestinationConstraints && constraints != 0 )
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
          if ( constraints.testFlag( QgsFieldConstraints::Constraint::ConstraintExpression ) )
          {
            constraintDescription.push_back( tr( "Expression" ) );
          }
          return constraintDescription.join( "<br>"_L1 );
        }
        break;
      }
      case Qt::BackgroundRole:
      {
        if ( constraints != 0 )
        {
          return QBrush( QColor( 255, 224, 178 ) );
        }
        break;
      }

      default:
        break;
    }
  }
  return QVariant();
}

Qt::ItemFlags QgsFieldMappingModel::flags( const QModelIndex &index ) const
{
  if ( index.isValid() && index.column() != static_cast<int>( ColumnDataIndex::DestinationConstraints ) && ( index.column() == static_cast<int>( ColumnDataIndex::SourceExpression ) || destinationEditable() ) )
  {
    return Qt::ItemFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
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
      switch ( static_cast<ColumnDataIndex>( index.column() ) )
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
          setFieldTypeFromName( f.field, value.toString() );
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
        case ColumnDataIndex::DestinationConstraints:
        {
          // Not editable: do nothing
          break;
        }
        case ColumnDataIndex::DestinationAlias:
        {
          f.field.setAlias( value.toString() );
          break;
        }
        case ColumnDataIndex::DestinationComment:
        {
          f.field.setComment( value.toString() );
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

QgsFieldConstraints::Constraints QgsFieldMappingModel::fieldConstraints( const QgsField &field ) const
{
  QgsFieldConstraints::Constraints constraints;

  const QgsFieldConstraints fieldConstraints { field.constraints() };

  if ( fieldConstraints.constraints() & QgsFieldConstraints::ConstraintNotNull && fieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) & QgsFieldConstraints::ConstraintStrengthHard )
    constraints.setFlag( QgsFieldConstraints::ConstraintNotNull );

  if ( fieldConstraints.constraints() & QgsFieldConstraints::ConstraintUnique && fieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) & QgsFieldConstraints::ConstraintStrengthHard )
    constraints.setFlag( QgsFieldConstraints::ConstraintUnique );

  if ( fieldConstraints.constraints() & QgsFieldConstraints::ConstraintExpression && fieldConstraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) & QgsFieldConstraints::ConstraintStrengthHard )
    constraints.setFlag( QgsFieldConstraints::ConstraintExpression );

  return constraints;
}

bool QgsFieldMappingModel::moveUpOrDown( const QModelIndex &index, bool up )
{
  if ( !index.isValid() && index.model() == this )
    return false;

  // Always swap down
  const int row { up ? index.row() - 1 : index.row() };
  // Range checking
  if ( row < 0 || row + 1 >= rowCount( QModelIndex() ) )
  {
    return false;
  }
  beginMoveRows( QModelIndex(), row, row, QModelIndex(), row + 2 );
  mMapping.swapItemsAt( row, row + 1 );
  endMoveRows();
  return true;
}

QString QgsFieldMappingModel::findExpressionForDestinationField( const QgsFieldMappingModel::Field &f, QStringList &excludedFieldNames )
{
  // Search for fields in the source
  // 1. match by name
  for ( const QgsField &sf : std::as_const( mSourceFields ) )
  {
    if ( sf.name() == f.field.name() )
    {
      excludedFieldNames.push_back( sf.name() );
      return QgsExpression::quotedColumnRef( sf.name() );
    }
  }
  // 2. match by type
  for ( const QgsField &sf : std::as_const( mSourceFields ) )
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
  if ( mExpressionContextGenerator )
    mExpressionContextGenerator->setSourceFields( mSourceFields );
  QStringList usedFields;
  beginResetModel();
  for ( const Field &f : std::as_const( mMapping ) )
  {
    if ( QgsExpression( f.expression ).isField() )
    {
      usedFields.push_back( f.expression.mid( 1, f.expression.length() - 2 ) );
    }
  }
  for ( auto it = mMapping.begin(); it != mMapping.end(); ++it )
  {
    if ( it->expression.isEmpty() )
    {
      const QString expression { findExpressionForDestinationField( *it, usedFields ) };
      if ( !expression.isEmpty() )
        it->expression = expression;
    }
  }
  endResetModel();
}

QgsExpressionContextGenerator *QgsFieldMappingModel::contextGenerator() const
{
  return mExpressionContextGenerator.get();
}

void QgsFieldMappingModel::setBaseExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator->setBaseExpressionContextGenerator( generator );
}

void QgsFieldMappingModel::setDestinationFields( const QgsFields &destinationFields, const QMap<QString, QString> &expressions )
{
  beginResetModel();
  mMapping.clear();
  // Prepare the model data
  QStringList usedFields;
  for ( const QgsField &df : destinationFields )
  {
    Field f;
    f.field = df;
    f.field.setTypeName( qgsFieldToTypeName( df ) );
    f.originalName = df.name();
    if ( expressions.contains( f.field.name() ) )
    {
      f.expression = expressions.value( f.field.name() );
      const QgsExpression exp { f.expression };
      // if it's source field
      if ( exp.isField() && mSourceFields.names().contains( qgis::setToList( exp.referencedColumns() ).constFirst() ) )
      {
        usedFields.push_back( qgis::setToList( exp.referencedColumns() ).constFirst() );
      }
    }
    else
    {
      const QString expression { findExpressionForDestinationField( f, usedFields ) };
      if ( !expression.isEmpty() )
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

const QMap<QMetaType::Type, QString> QgsFieldMappingModel::dataTypes()
{
  static const QMap<QMetaType::Type, QString> sDataTypes {
    { QMetaType::Type::Int, QgsVariantUtils::typeToDisplayString( QMetaType::Type::Int ) },
    { QMetaType::Type::LongLong, QgsVariantUtils::typeToDisplayString( QMetaType::Type::LongLong ) },
    { QMetaType::Type::Double, QgsVariantUtils::typeToDisplayString( QMetaType::Type::Double ) },
    { QMetaType::Type::QString, QgsVariantUtils::typeToDisplayString( QMetaType::Type::QString ) },
    { QMetaType::Type::QDate, QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ) },
    { QMetaType::Type::QTime, QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ) },
    { QMetaType::Type::QDateTime, QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ) },
    { QMetaType::Type::Bool, QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ) },
    { QMetaType::Type::QByteArray, QgsVariantUtils::typeToDisplayString( QMetaType::Type::QByteArray ) },
  };
  return sDataTypes;
}

const QList<QgsVectorDataProvider::NativeType> QgsFieldMappingModel::supportedDataTypes()
{
  static const QList<QgsVectorDataProvider::NativeType> sDataTypes = QList<QgsVectorDataProvider::NativeType>() << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Int ), u"integer"_s, QMetaType::Type::Int )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::LongLong ), u"int8"_s, QMetaType::Type::LongLong )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Double ), u"double precision"_s, QMetaType::Type::Double )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QString ), u"text"_s, QMetaType::Type::QString )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), u"date"_s, QMetaType::Type::QDate )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), u"time"_s, QMetaType::Type::QTime )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), u"datetime"_s, QMetaType::Type::QDateTime )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ), u"boolean"_s, QMetaType::Type::Bool )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QByteArray ), u"binary"_s, QMetaType::Type::QByteArray )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QStringList ), u"stringlist"_s, QMetaType::Type::QStringList, 0, 0, 0, 0, QMetaType::Type::QString )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Int ), u"integerlist"_s, QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::Int )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::Double ), u"doublelist"_s, QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::Double )
                                                                                                                << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QVariantList, QMetaType::Type::LongLong ), u"integer64list"_s, QMetaType::Type::QVariantList, 0, 0, 0, 0, QMetaType::Type::LongLong );
  return sDataTypes;
}

QString QgsFieldMappingModel::qgsFieldToTypeName( const QgsField &field ) const
{
  for ( const QgsVectorDataProvider::NativeType &type : mNativeTypes )
  {
    if ( type.mType == field.type() && type.mSubType == field.subType() )
    {
      return type.mTypeName;
    }
  }

  // no exact type name matches, try to match metatype as a fallback
  for ( const QgsVectorDataProvider::NativeType &type : mNativeTypes )
  {
    if ( type.mType == field.type() && type.mSubType == field.subType() )
    {
      return type.mTypeName;
    }
  }

  // next chance -- try promoting numeric types

  // promote int -> long long
  if ( field.type() == QMetaType::Type::Int )
  {
    for ( const QgsVectorDataProvider::NativeType &type : mNativeTypes )
    {
      if ( type.mType == QMetaType::Type::LongLong )
      {
        return type.mTypeName;
      }
    }
  }
  // promote uint -> unsigned long long or long long
  if ( field.type() == QMetaType::Type::UInt )
  {
    for ( const QgsVectorDataProvider::NativeType &type : mNativeTypes )
    {
      if ( type.mType == QMetaType::Type::ULongLong || type.mType == QMetaType::Type::LongLong )
      {
        return type.mTypeName;
      }
    }
  }
  // promote float or int -> double
  if ( field.type() == QMetaType::Type::Float || field.type() == QMetaType::Type::Int )
  {
    for ( const QgsVectorDataProvider::NativeType &type : mNativeTypes )
    {
      if ( type.mType == QMetaType::Type::Double )
      {
        return type.mTypeName;
      }
    }
  }
  // last resort -- promote certain types to string
  if ( field.type() == QMetaType::Type::Float
       || field.type() == QMetaType::Type::Double
       || field.type() == QMetaType::Type::Int
       || field.type() == QMetaType::Type::UInt
       || field.type() == QMetaType::Type::LongLong
       || field.type() == QMetaType::Type::ULongLong
       || field.type() == QMetaType::Type::QDate
       || field.type() == QMetaType::Type::QTime
       || field.type() == QMetaType::Type::QDateTime )
  {
    for ( const QgsVectorDataProvider::NativeType &type : mNativeTypes )
    {
      if ( type.mType == QMetaType::Type::QString )
      {
        return type.mTypeName;
      }
    }
  }

  return QString();
}

void QgsFieldMappingModel::setFieldTypeFromName( QgsField &field, const QString &name ) const
{
  for ( const QgsVectorDataProvider::NativeType &type : mNativeTypes )
  {
    if ( type.mTypeName == name )
    {
      field.setType( type.mType );
      field.setTypeName( type.mTypeName );
      field.setSubType( type.mSubType );
      return;
    }
  }
}

QList<QgsFieldMappingModel::Field> QgsFieldMappingModel::mapping() const
{
  return mMapping;
}

QMap<QString, QgsProperty> QgsFieldMappingModel::fieldPropertyMap() const
{
  QMap<QString, QgsProperty> fieldMap;
  for ( const QgsFieldMappingModel::Field &field : mMapping )
  {
    const QgsExpression exp( field.expression );
    const bool isField = exp.isField();
    fieldMap.insert( field.originalName, isField ? QgsProperty::fromField( static_cast<const QgsExpressionNodeColumnRef *>( exp.rootNode() )->name() ) : QgsProperty::fromExpression( field.expression ) );
  }
  return fieldMap;
}

void QgsFieldMappingModel::setFieldPropertyMap( const QMap<QString, QgsProperty> &map )
{
  beginResetModel();
  for ( int i = 0; i < mMapping.count(); ++i )
  {
    Field &f = mMapping[i];
    if ( map.contains( f.field.name() ) )
    {
      const QgsProperty prop = map.value( f.field.name() );
      switch ( prop.propertyType() )
      {
        case Qgis::PropertyType::Static:
          f.expression = QgsExpression::quotedValue( prop.staticValue() );
          break;

        case Qgis::PropertyType::Field:
          f.expression = prop.field();
          break;

        case Qgis::PropertyType::Expression:
          f.expression = prop.expressionString();
          break;

        case Qgis::PropertyType::Invalid:
          f.expression.clear();
          break;
      }
    }
    else
    {
      f.expression.clear();
    }
  }
  endResetModel();
}

void QgsFieldMappingModel::appendField( const QgsField &field, const QString &expression )
{
  const int lastRow { rowCount( QModelIndex() ) };
  beginInsertRows( QModelIndex(), lastRow, lastRow );
  Field f;
  f.field = field;
  f.field.setTypeName( qgsFieldToTypeName( field ) );
  f.expression = expression;
  f.originalName = field.name();
  mMapping.push_back( f );
  endInsertRows();
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

QgsFieldMappingModel::ExpressionContextGenerator::ExpressionContextGenerator( const QgsFields &sourceFields )
  : mSourceFields( sourceFields )
{
}

QgsExpressionContext QgsFieldMappingModel::ExpressionContextGenerator::createExpressionContext() const
{
  if ( mBaseGenerator )
  {
    QgsExpressionContext ctx = mBaseGenerator->createExpressionContext();
    auto fieldMappingScope = std::make_unique<QgsExpressionContextScope>( tr( "Field Mapping" ) );
    fieldMappingScope->setFields( mSourceFields );
    ctx.appendScope( fieldMappingScope.release() );
    return ctx;
  }
  else
  {
    QgsExpressionContext ctx;
    ctx.appendScope( QgsExpressionContextUtils::globalScope() );
    ctx.setFields( mSourceFields );
    QgsFeature feature { mSourceFields };
    feature.setValid( true );
    ctx.setFeature( feature );
    return ctx;
  }
}

void QgsFieldMappingModel::ExpressionContextGenerator::setBaseExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  mBaseGenerator = generator;
}

void QgsFieldMappingModel::ExpressionContextGenerator::setSourceFields( const QgsFields &fields )
{
  mSourceFields = fields;
}
