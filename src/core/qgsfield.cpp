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

/*
QgsField::QgsField(QString nam, QString typ, int len, int prec, bool num,
                   QString comment)
    :mName(nam), mType(typ), mLength(len), mPrecision(prec), mNumeric(num),
     mComment(comment)
{
  // This function used to lower case the field name since some stores
  // use upper case (eg. shapefiles), but that caused problems with
  // attribute actions getting confused between uppercase and
  // lowercase versions of the attribute names, so just leave the
  // names how they are now.
}*/

QgsField::QgsField( QString name, QVariant::Type type, QString typeName, int len, int prec, QString comment )
    : mName( name ), mType( type ), mTypeName( typeName ),
    mLength( len ), mPrecision( prec ), mComment( comment )
{
}


QgsField::~QgsField()
{
}

bool QgsField::operator==( const QgsField& other ) const
{
  return (( mName == other.mName ) && ( mType == other.mType )
          && ( mLength == other.mLength ) && ( mPrecision == other.mPrecision ) );
}

bool QgsField::operator!=( const QgsField& other ) const
{
  return !( *this == other );
}


const QString & QgsField::name() const
{
  return mName;
}

QVariant::Type QgsField::type() const
{
  return mType;
}

const QString & QgsField::typeName() const
{
  return mTypeName;
}

int QgsField::length() const
{
  return mLength;
}

int QgsField::precision() const
{
  return mPrecision;
}

const QString & QgsField::comment() const
{
  return mComment;
}

void QgsField::setName( const QString & nam )
{
  mName = nam;
}

void QgsField::setType( QVariant::Type type )
{
  mType = type;
}

void QgsField::setTypeName( const QString & typeName )
{
  mTypeName = typeName;
}

void QgsField::setLength( int len )
{
  mLength = len;
}
void QgsField::setPrecision( int prec )
{
  mPrecision = prec;
}

void QgsField::setComment( const QString & comment )
{
  mComment = comment;
}

QString QgsField::displayString( const QVariant& v ) const
{
  switch ( mType )
  {
    case QVariant::Double:
      if ( mPrecision > 0 )
      {
        return QString::number( v.toDouble(), 'f', mPrecision );
      }
    default:
      return v.toString();
  }
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

void QgsFields::remove( int fieldIdx )
{
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

QList<QgsField> QgsFields::toList() const
{
  QList<QgsField> lst;
  for ( int i = 0; i < mFields.count(); ++i )
    lst.append( mFields[i].field );
  return lst;
}
