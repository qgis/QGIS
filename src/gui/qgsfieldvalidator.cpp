/***************************************************************************
                         qgsfieldvalidator.cpp  -  description
                             -------------------
    begin                : March 2011
    copyright            : (C) 2011 by SunilRajKiran-kCube
    email                : sunilraj.kiran@kcubeconsulting.com

  adapted version of QValidator for QgsField
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldvalidator.h"

#include <QValidator>
#include <QRegExpValidator>
#include <QDate>
#include <QVariant>
#include <QSettings>

#include "qgslogger.h"
#include "qgslonglongvalidator.h"
#include "qgsfield.h"

QgsFieldValidator::QgsFieldValidator( QObject *parent, const QgsField &field, QString defaultValue, QString dateFormat )
    : QValidator( parent )
    , mField( field )
    , mDefaultValue( defaultValue )
    , mDateFormat( dateFormat )
{
  switch ( mField.type() )
  {
    case QVariant::Int:
    {
      if ( mField.length() > 0 )
      {
        QString re = QString( "-?\\d{0,%1}" ).arg( mField.length() );
        mValidator = new QRegExpValidator( QRegExp( re ), parent );
      }
      else
      {
        mValidator = new QIntValidator( parent );
      }
    }
    break;

    case QVariant::Double:
    {
      if ( mField.length() > 0 && mField.precision() > 0 )
      {
        QString re = QString( "-?\\d{0,%1}(\\.\\d{0,%2})?" ).arg( mField.length() - mField.precision() ).arg( mField.precision() );
        mValidator = new QRegExpValidator( QRegExp( re ), parent );
      }
      else if ( mField.length() > 0 && mField.precision() == 0 )
      {
        QString re = QString( "-?\\d{0,%1}" ).arg( mField.length() );
        mValidator = new QRegExpValidator( QRegExp( re ), parent );
      }
      else if ( mField.precision() > 0 )
      {
        QString re = QString( "-?\\d*(\\.\\d{0,%1})?" ).arg( mField.precision() );
        mValidator = new QRegExpValidator( QRegExp( re ), parent );
      }
      else
      {
        mValidator = new QDoubleValidator( parent );
      }
    }
    break;

    case QVariant::LongLong :
      mValidator = new QgsLongLongValidator( parent );
      break;

    default:
      mValidator = 0;
  }

  QSettings settings;
  mNullValue = settings.value( "qgis/nullValue", "NULL" ).toString();
}

QgsFieldValidator::~QgsFieldValidator()
{
  delete mValidator;
}

QValidator::State QgsFieldValidator::validate( QString &s, int &i ) const
{
  // empty values are considered NULL for numbers and dates and are acceptable
  if ( s.isEmpty() &&
       ( mField.type() == QVariant::Double
         || mField.type() == QVariant::Int
         || mField.type() == QVariant::LongLong
         || mField.type() == QVariant::Date
       )
     )
  {
    return Acceptable;
  }

  if ( s == mDefaultValue )
    return Acceptable;

  // delegate to the child validator if any
  if ( mValidator )
  {
    QValidator::State result = mValidator->validate( s, i );
    return result;
  }
  else if ( mField.type() == QVariant::String )
  {
    // allow to enter the NULL representation, which might be
    // longer than the actual field
    if ( mNullValue.size() > 0 && s.size() > 0 && s.size() < mNullValue.size() && s == mNullValue.left( s.size() ) )
      return Intermediate;

    if ( mDefaultValue.size() > 0 && s.size() > 0 && s.size() < mDefaultValue.size() && s == mDefaultValue.left( s.size() ) )
      return Intermediate;

    if ( s == mNullValue )
      return Acceptable;

    if ( mField.length() > 0 && s.size() > mField.length() )
      return Invalid;
  }
  else if ( mField.type() == QVariant::Date )
  {
    return QDate::fromString( s, mDateFormat ).isValid() ? Acceptable : Intermediate;
  }
  else
  {
    QgsDebugMsg( QString( "unsupported type %1 for validation" ).arg( mField.type() ) );
    return Invalid;
  }

  return Acceptable;
}

void QgsFieldValidator::fixup( QString &s ) const
{
  if ( mValidator )
  {
    mValidator->fixup( s );
  }
  else if ( mField.type() == QVariant::String && mField.length() > 0 && s.size() > mField.length() && s != mDefaultValue )
  {
    // if the value is longer, this must be a partial NULL representation
    s = mNullValue;
  }
  else if ( mField.type() == QVariant::Date )
  {
    // invalid dates will also translate to NULL
    s = "";
  }
}
