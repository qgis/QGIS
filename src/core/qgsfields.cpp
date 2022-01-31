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
#include "qgsfields_p.h"
#include "qgsapplication.h"
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

bool QgsFields::append( const QgsField &field, FieldOrigin origin, int originIndex )
{
  if ( d->nameToIndex.contains( field.name() ) )
    return false;

  if ( originIndex == -1 && origin == OriginProvider )
    originIndex = d->fields.count();
  d->fields.append( Field( field, origin, originIndex ) );

  d->nameToIndex.insert( field.name(), d->fields.count() - 1 );
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

  d->fields.append( Field( field, OriginExpression, originIndex ) );

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

QgsFields::FieldOrigin QgsFields::fieldOrigin( int fieldIdx ) const
{
  if ( !exists( fieldIdx ) )
    return OriginUnknown;

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
      case QgsFields::OriginExpression:
        return QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) );

      case QgsFields::OriginJoin:
        return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/join.svg" ) );

      default:
        return iconForFieldType( d->fields.at( fieldIdx ).field.type(), d->fields.at( fieldIdx ).field.subType() );
    }
  }
  return iconForFieldType( d->fields.at( fieldIdx ).field.type(), d->fields.at( fieldIdx ).field.subType() );
}

QIcon QgsFields::iconForFieldType( QVariant::Type type, QVariant::Type subType )
{
  switch ( type )
  {
    case QVariant::Bool:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldBool.svg" ) );
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldInteger.svg" ) );
    case QVariant::Double:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldFloat.svg" ) );
    case QVariant::String:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldText.svg" ) );
    case QVariant::Date:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDate.svg" ) );
    case QVariant::DateTime:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDateTime.svg" ) );
    case QVariant::Time:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldTime.svg" ) );
    case QVariant::ByteArray:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldBinary.svg" ) );
    case QVariant::List:
    {
      switch ( subType )
      {
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArrayInteger.svg" ) );
        case QVariant::Double:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArrayFloat.svg" ) );
        case QVariant::String:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArrayString.svg" ) );
        default:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArray.svg" ) );
      }
    }
    case QVariant::StringList:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldArrayString.svg" ) );
    case QVariant::Map:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldJson.svg" ) );
    default:
      return QIcon();
  }
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

int QgsFields::lookupField( const QString &fieldName ) const
{
  if ( fieldName.isEmpty() ) //shortcut
    return -1;

  for ( int idx = 0; idx < count(); ++idx )
  {
    if ( d->fields[idx].field.name() == fieldName )
      return idx;
  }

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
