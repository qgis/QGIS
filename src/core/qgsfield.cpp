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

#include <QSettings>
#include <QtCore/qmath.h>

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

QgsField::QgsField( QString name, QVariant::Type type, QString typeName, int len, int prec, QString comment )
{
  d = new QgsField::FieldData( name, type, typeName, len, prec, comment );
}

QgsField::QgsField( const QgsField &other )
    : d( other.d )
{

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

const QString & QgsField::name() const
{
  return d->name;
}

QVariant::Type QgsField::type() const
{
  return d->type;
}

const QString & QgsField::typeName() const
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

const QString & QgsField::comment() const
{
  return d->comment;
}

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

////////////////////////////////////////////////////////////////////////////

void QgsFields::clear()
{
  mFields.clear();
  mNameToIndex.clear();
}

bool QgsFields::append( const QgsField& field, FieldOrigin origin, int originIndex )
{
  if ( mNameToIndex.contains( field.name() ) )
    return false;

  if ( originIndex == -1 && origin == OriginProvider )
    originIndex = mFields.count();
  mFields.append( Field( field, origin, originIndex ) );

  mNameToIndex.insert( field.name(), mFields.count() - 1 );
  return true;
}

bool QgsFields::appendExpressionField( const QgsField& field, int originIndex )
{
  if ( mNameToIndex.contains( field.name() ) )
    return false;

  mFields.append( Field( field, OriginExpression, originIndex ) );

  mNameToIndex.insert( field.name(), mFields.count() - 1 );
  return true;
}

void QgsFields::remove( int fieldIdx )
{
  if ( !exists( fieldIdx ) )
    return;

  mNameToIndex.remove( mFields[fieldIdx].field.name() );
  mFields.remove( fieldIdx );
}

void QgsFields::extend( const QgsFields& other )
{
  for ( int i = 0; i < other.count(); ++i )
  {
    append( other.at( i ), other.fieldOrigin( i ), other.fieldOriginIndex( i ) );
  }
}

QgsFields::FieldOrigin QgsFields::fieldOrigin( int fieldIdx ) const
{
  if ( !exists( fieldIdx ) )
    return OriginUnknown;

  return mFields[fieldIdx].origin;
}

QList<QgsField> QgsFields::toList() const
{
  QList<QgsField> lst;
  for ( int i = 0; i < mFields.count(); ++i )
    lst.append( mFields[i].field );
  return lst;
}

int QgsFields::fieldNameIndex( const QString& fieldName ) const
{
  for ( int idx = 0; idx < count(); ++idx )
  {
    if ( QString::compare( mFields[idx].field.name(), fieldName, Qt::CaseInsensitive ) == 0 )
    {
      return idx;
    }
  }
  return -1;
}

QgsAttributeList QgsFields::allAttributesList() const
{
  QgsAttributeList lst;
  for ( int i = 0; i < mFields.count(); ++i )
    lst.append( i );
  return lst;
}
