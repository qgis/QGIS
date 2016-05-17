/***************************************************************************
       qgsfield.cpp - Describes a field in a layer or table
        --------------------------------------
       Date                 : 01-Jan-2004
       Copyright            : (C) 2004 by Gary E.Sherman
       email                : sherman at mrcc.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfield.h"
#include "qgsfield_p.h"
#include "qgis.h"
#include "qgsapplication.h"

#include <QSettings>
#include <QDataStream>
#include <QtCore/qmath.h>
#include <QIcon>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

#if 0
QgsField::QgsField( QString nam, QString typ, int len, int prec, bool num,
                    QString comment )
    : mName( nam ), mType( typ ), mLength( len ), mPrecision( prec ), mNumeric( num )
    , mComment( comment )
{
  // This function used to lower case the field name since some stores
  // use upper case (eg. shapefiles), but that caused problems with
  // attribute actions getting confused between uppercase and
  // lowercase versions of the attribute names, so just leave the
  // names how they are now.
}
#endif
QgsField::QgsField( const QString& name, QVariant::Type type,
                    const QString& typeName, int len, int prec, const QString& comment )
{
  d = new QgsFieldPrivate( name, type, typeName, len, prec, comment );
}

QgsField::QgsField( const QgsField &other )
    : d( other.d )
{

}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsField &QgsField::operator =( const QgsField & other )
{
  d = other.d;
  return *this;
}

QgsField::~QgsField()
{
}

bool QgsField::operator==( const QgsField& other ) const
{
  return *( other.d ) == *d;
}

bool QgsField::operator!=( const QgsField& other ) const
{
  return !( *this == other );
}

QString QgsField::name() const
{
  return d->name;
}

QVariant::Type QgsField::type() const
{
  return d->type;
}

QString QgsField::typeName() const
{
  return d->typeName;
}

int QgsField::length() const
{
  return d->length;
}

int QgsField::precision() const
{
  return d->precision;
}

QString QgsField::comment() const
{
  return d->comment;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

void QgsField::setName( const QString& name )
{
  d->name = name;
}

void QgsField::setType( QVariant::Type type )
{
  d->type = type;
}

void QgsField::setTypeName( const QString& typeName )
{
  d->typeName = typeName;
}

void QgsField::setLength( int len )
{
  d->length = len;
}
void QgsField::setPrecision( int precision )
{
  d->precision = precision;
}

void QgsField::setComment( const QString& comment )
{
  d->comment = comment;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

QString QgsField::displayString( const QVariant& v ) const
{
  if ( v.isNull() )
  {
    QSettings settings;
    return settings.value( "qgis/nullValue", "NULL" ).toString();
  }

  if ( d->type == QVariant::Double && d->precision > 0 )
    return QString::number( v.toDouble(), 'f', d->precision );

  return v.toString();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

bool QgsField::convertCompatible( QVariant& v ) const
{
  if ( v.isNull() )
  {
    v.convert( d->type );
    return true;
  }

  if ( d->type == QVariant::Int && v.toInt() != v.toLongLong() )
  {
    v = QVariant( d->type );
    return false;
  }

  //String representations of doubles in QVariant will return false to convert( QVariant::Int )
  //work around this by first converting to double, and then checking whether the double is convertible to int
  if ( d->type == QVariant::Int && v.canConvert( QVariant::Double ) )
  {
    bool ok = false;
    double dbl = v.toDouble( &ok );
    if ( !ok )
    {
      //couldn't convert to number
      v = QVariant( d->type );
      return false;
    }

    double round = qgsRound( dbl );
    if ( round  > INT_MAX || round < -INT_MAX )
    {
      //double too large to fit in int
      v = QVariant( d->type );
      return false;
    }
    v = QVariant( qRound( dbl ) );
    return true;
  }

  if ( !v.convert( d->type ) )
  {
    v = QVariant( d->type );
    return false;
  }

  if ( d->type == QVariant::Double && d->precision > 0 )
  {
    double s = qPow( 10, d->precision );
    double d = v.toDouble() * s;
    v = QVariant(( d < 0 ? ceil( d - 0.5 ) : floor( d + 0.5 ) ) / s );
    return true;
  }

  if ( d->type == QVariant::String && d->length > 0 && v.toString().length() > d->length )
  {
    v = v.toString().left( d->length );
    return false;
  }

  return true;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

QDataStream& operator<<( QDataStream& out, const QgsField& field )
{
  out << field.name();
  out << static_cast< quint32 >( field.type() );
  out << field.typeName();
  out << field.length();
  out << field.precision();
  out << field.comment();
  return out;
}

QDataStream& operator>>( QDataStream& in, QgsField& field )
{
  quint32 type, length, precision;
  QString name, typeName, comment;
  in >> name >> type >> typeName >> length >> precision >> comment;
  field.setName( name );
  field.setType( static_cast< QVariant::Type >( type ) );
  field.setTypeName( typeName );
  field.setLength( static_cast< int >( length ) );
  field.setPrecision( static_cast< int >( precision ) );
  field.setComment( comment );
  return in;
}

////////////////////////////////////////////////////////////////////////////


/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsFields::QgsFields()
{
  d = new QgsFieldsPrivate();
}

QgsFields::QgsFields( const QgsFields &other )
    : d( other.d )
{
}

QgsFields &QgsFields::operator =( const QgsFields & other )
{
  d = other.d;
  return *this;
}

QgsFields::~QgsFields()
{

}

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

bool QgsFields::append( const QgsField& field, FieldOrigin origin, int originIndex )
{
  if ( d->nameToIndex.contains( field.name() ) )
    return false;

  if ( originIndex == -1 && origin == OriginProvider )
    originIndex = d->fields.count();
  d->fields.append( Field( field, origin, originIndex ) );

  d->nameToIndex.insert( field.name(), d->fields.count() - 1 );
  return true;
}

bool QgsFields::appendExpressionField( const QgsField& field, int originIndex )
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

void QgsFields::extend( const QgsFields& other )
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

bool QgsFields::exists( int i ) const
{
  return i >= 0 && i < d->fields.count();
}

QgsField &QgsFields::operator[]( int i )
{
  return d->fields[i].field;
}

const QgsField &QgsFields::at( int i ) const
{
  return d->fields[i].field;
}

const QgsField &QgsFields::field( int fieldIdx ) const
{
  return d->fields[fieldIdx].field;
}

const QgsField &QgsFields::field( const QString &name ) const
{
  return d->fields[ indexFromName( name )].field;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

const QgsField &QgsFields::operator[]( int i ) const
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

int QgsFields::indexFromName( const QString &name ) const
{
  return d->nameToIndex.value( name, -1 );
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

QIcon QgsFields::iconForField( int fieldIdx ) const
{
  switch ( d->fields.at( fieldIdx ).field.type() )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldInteger.svg" );
    }
    case QVariant::Double:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldFloat.svg" );
    }
    case QVariant::String:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldText.svg" );
    }
    case QVariant::Date:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldDate.svg" );
    }
    case QVariant::DateTime:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldDateTime.svg" );
    }
    case QVariant::Time:
    {
      return QgsApplication::getThemeIcon( "/mIconFieldTime.svg" );
    }
    default:
      return QIcon();
  }
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

int QgsFields::fieldNameIndex( const QString& fieldName ) const
{
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

  return -1;
}

QgsAttributeList QgsFields::allAttributesList() const
{
  QgsAttributeList lst;
  for ( int i = 0; i < d->fields.count(); ++i )
    lst.append( i );
  return lst;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

QDataStream& operator<<( QDataStream& out, const QgsFields& fields )
{
  out << static_cast< quint32 >( fields.size() );
  for ( int i = 0; i < fields.size(); i++ )
  {
    out << fields.field( i );
  }
  return out;
}

QDataStream& operator>>( QDataStream& in, QgsFields& fields )
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
