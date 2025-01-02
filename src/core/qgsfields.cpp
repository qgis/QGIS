/***************************************************************************
  qgsfields.cpp - QgsFields

 ---------------------
 begin                : 22.9.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfields.h"
#include "moc_qgsfields.cpp"
#include "qgsfields_p.h"
#include "qgsapplication.h"
#include "qgsvariantutils.h"
#include <QIcon>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsFields::QgsFields()
{
  d = new QgsFieldsPrivate();
}

QgsFields::QgsFields( const QgsFields &other ) //NOLINT
  : d( other.d )
{
}

QgsFields &QgsFields::operator =( const QgsFields &other )  //NOLINT
{
  d = other.d;
  return *this;
}

QgsFields::QgsFields( const QList<QgsField> &fields )
{
  d = new QgsFieldsPrivate();
  for ( const QgsField &field : fields )
  {
    append( field );
  }
}

QgsFields::~QgsFields() //NOLINT
{}

void QgsFields::clear()
{
  d->fields.clear();
  d->nameToIndex.clear();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

bool QgsFields::append( const QgsField &field, Qgis::FieldOrigin origin, int originIndex )
{
  if ( d->nameToIndex.contains( field.name() ) )
    return false;

  if ( originIndex == -1 && origin == Qgis::FieldOrigin::Provider )
    originIndex = d->fields.count();
  d->fields.append( Field( field, origin, originIndex ) );

  d->nameToIndex.insert( field.name(), d->fields.count() - 1 );
  return true;
}

bool QgsFields::append( const QList<QgsField> &fields, Qgis::FieldOrigin origin )
{
  for ( const QgsField &field : fields )
  {
    if ( d->nameToIndex.contains( field.name() ) )
      return false;
  }

  for ( const QgsField &field : fields )
  {
    append( field, origin );
  }
  return true;
}

bool QgsFields::append( const QgsFields &fields )
{
  for ( const QgsField &field : fields )
  {
    if ( d->nameToIndex.contains( field.name() ) )
      return false;
  }

  for ( int i = 0; i < fields.size(); ++ i )
  {
    append( fields.at( i ), fields.fieldOrigin( i ), fields.fieldOriginIndex( i ) );
  }
  return true;
}

bool QgsFields::rename( int fieldIdx, const QString &name )
{
  if ( !exists( fieldIdx ) )
    return false;

  if ( name.isEmpty() )
    return false;

  if ( d->nameToIndex.contains( name ) )
    return false;

  const QString oldName = d->fields[ fieldIdx ].field.name();
  d->fields[ fieldIdx ].field.setName( name );
  d->nameToIndex.remove( oldName );
  d->nameToIndex.insert( name, fieldIdx );
  return true;
}

bool QgsFields::appendExpressionField( const QgsField &field, int originIndex )
{
  if ( d->nameToIndex.contains( field.name() ) )
    return false;

  d->fields.append( Field( field, Qgis::FieldOrigin::Expression, originIndex ) );

  d->nameToIndex.insert( field.name(), d->fields.count() - 1 );
  return true;
}

void QgsFields::remove( int fieldIdx )
{
  if ( !exists( fieldIdx ) )
    return;

  d->fields.remove( fieldIdx );
  d->nameToIndex.clear();
  for ( int idx = 0; idx < count(); ++idx )
  {
    d->nameToIndex.insert( d->fields.at( idx ).field.name(), idx );
  }
}

void QgsFields::extend( const QgsFields &other )
{
  for ( int i = 0; i < other.count(); ++i )
  {
    append( other.at( i ), other.fieldOrigin( i ), other.fieldOriginIndex( i ) );
  }
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

bool QgsFields::isEmpty() const
{
  return d->fields.isEmpty();
}

int QgsFields::count() const
{
  return d->fields.count();
}

int QgsFields::size() const
{
  return d->fields.count();
}

QStringList QgsFields::names() const
{
  QStringList lst;
  for ( int i = 0; i < d->fields.count(); ++i )
  {
    lst.append( d->fields[i].field.name() );
  }
  return lst;
}

bool QgsFields::exists( int i ) const
{
  return i >= 0 && i < d->fields.count();
}

QgsField &QgsFields::operator[]( int i )
{
  return d->fields[i].field;
}

QgsField QgsFields::at( int i ) const
{
  return d->fields[i].field;
}

QgsField QgsFields::field( int fieldIdx ) const
{
  return d->fields[fieldIdx].field;
}

QgsField QgsFields::field( const QString &name ) const
{
  return d->fields[ indexFromName( name )].field;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsField QgsFields::operator[]( int i ) const
{
  return d->fields[i].field;
}

Qgis::FieldOrigin QgsFields::fieldOrigin( int fieldIdx ) const
{
  if ( !exists( fieldIdx ) )
    return Qgis::FieldOrigin::Unknown;

  return d->fields[fieldIdx].origin;
}

int QgsFields::fieldOriginIndex( int fieldIdx ) const
{
  return d->fields[fieldIdx].originIndex;
}

int QgsFields::indexFromName( const QString &fieldName ) const
{
  return d->nameToIndex.value( fieldName, -1 );
}

int QgsFields::indexOf( const QString &fieldName ) const
{
  return d->nameToIndex.value( fieldName, -1 );
}

QList<QgsField> QgsFields::toList() const
{
  QList<QgsField> lst;
  for ( int i = 0; i < d->fields.count(); ++i )
    lst.append( d->fields[i].field );
  return lst;
}

bool QgsFields::operator==( const QgsFields &other ) const
{
  return d->fields == other.d->fields;
}

QgsFields::const_iterator QgsFields::constBegin() const noexcept
{
  if ( d->fields.isEmpty() )
    return const_iterator();

  return const_iterator( &d->fields.first() );
}

QgsFields::const_iterator QgsFields::constEnd() const noexcept
{
  if ( d->fields.isEmpty() )
    return const_iterator();

  return const_iterator( &d->fields.last() + 1 );
}

QgsFields::const_iterator QgsFields::begin() const noexcept
{
  if ( d->fields.isEmpty() )
    return const_iterator();

  return const_iterator( &d->fields.first() );
}

QgsFields::const_iterator QgsFields::end() const noexcept
{
  if ( d->fields.isEmpty() )
    return const_iterator();

  return const_iterator( &d->fields.last() + 1 );
}

QgsFields::iterator QgsFields::begin()
{
  if ( d->fields.isEmpty() )
    return iterator();

  d.detach();
  return iterator( &d->fields.first() );
}

QgsFields::iterator QgsFields::end()
{
  if ( d->fields.isEmpty() )
    return iterator();

  d.detach();
  return iterator( &d->fields.last() + 1 );
}

QIcon QgsFields::iconForField( int fieldIdx, bool considerOrigin ) const
{
  if ( considerOrigin )
  {
    switch ( fieldOrigin( fieldIdx ) )
    {
      case Qgis::FieldOrigin::Expression:
        return QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) );

      case Qgis::FieldOrigin::Join:
        return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/join.svg" ) );

      default:
        return iconForFieldType( d->fields.at( fieldIdx ).field.type(), d->fields.at( fieldIdx ).field.subType(), d->fields.at( fieldIdx ).field.typeName() );
    }
  }
  return iconForFieldType( d->fields.at( fieldIdx ).field.type(), d->fields.at( fieldIdx ).field.subType(), d->fields.at( fieldIdx ).field.typeName() );
}

QIcon QgsFields::iconForFieldType( QMetaType::Type type, QMetaType::Type subType, const QString &typeString )
{
  switch ( type )
  {
    case QMetaType::Type::Bool:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldBool.svg" ) );
    case QMetaType::Type::Int:
    case QMetaType::Type::UInt:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::ULongLong:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldInteger.svg" ) );
    case QMetaType::Type::Double:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldFloat.svg" ) );
    case QMetaType::Type::QString:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldText.svg" ) );
    case QMetaType::Type::QDate:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDate.svg" ) );
    case QMetaType::Type::QDateTime:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDateTime.svg" ) );
    case QMetaType::Type::QTime:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldTime.svg" ) );
    case QMetaType::Type::QByteArray:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldBinary.svg" ) );
    case QMetaType::Type::QVariantList:
    {
      switch ( subType )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::UInt:
        case QMetaType::Type::LongLong:
        case QMetaType::Type::ULongLong:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArrayInteger.svg" ) );
        case QMetaType::Type::Double:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArrayFloat.svg" ) );
        case QMetaType::Type::QString:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArrayString.svg" ) );
        default:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArray.svg" ) );
      }
    }
    case QMetaType::Type::QStringList:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArrayString.svg" ) );
    case QMetaType::Type::QVariantMap:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldJson.svg" ) );
    case QMetaType::Type::User:
      if ( typeString.compare( QLatin1String( "geometry" ) ) == 0 )
      {
        return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldGeometry.svg" ) );
      }
      else
      {
        return QIcon();
      }

    default:
      return QIcon();
  }
}

QIcon QgsFields::iconForFieldType( QVariant::Type type, QVariant::Type subType, const QString &typeString )
{
  return iconForFieldType( QgsVariantUtils::variantTypeToMetaType( type ), QgsVariantUtils::variantTypeToMetaType( subType ), typeString );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

int QgsFields::lookupField( const QString &fieldName ) const
{
  for ( int idx = 0; idx < count(); ++idx )
  {
    if ( d->fields[idx].field.name() == fieldName )
      return idx;
  }

  if ( fieldName.isEmpty() )
    return -1;

  for ( int idx = 0; idx < count(); ++idx )
  {
    if ( QString::compare( d->fields[idx].field.name(), fieldName, Qt::CaseInsensitive ) == 0 )
      return idx;
  }

  for ( int idx = 0; idx < count(); ++idx )
  {
    const QString alias = d->fields[idx].field.alias();
    if ( !alias.isEmpty() && QString::compare( alias, fieldName, Qt::CaseInsensitive ) == 0 )
      return idx;
  }

  return -1;
}

QgsAttributeList QgsFields::allAttributesList() const
{
  const int count = d->fields.count();
  QgsAttributeList lst;
  lst.reserve( count );
  for ( int i = 0; i < count; ++i )
    lst.append( i );
  return lst;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

QDataStream &operator<<( QDataStream &out, const QgsFields &fields )
{
  out << static_cast< quint32 >( fields.size() );
  for ( int i = 0; i < fields.size(); i++ )
  {
    out << fields.field( i );
  }
  return out;
}

QDataStream &operator>>( QDataStream &in, QgsFields &fields )
{
  fields.clear();
  quint32 size;
  in >> size;
  for ( quint32 i = 0; i < size; i++ )
  {
    QgsField field;
    in >> field;
    fields.append( field );
  }
  return in;
}
