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
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDate>
#include <QVariant>

#include "qgssettings.h"
#include "qgslogger.h"
#include "qgslonglongvalidator.h"
#include "qgsfields.h"
#include "qgsapplication.h"

QgsFieldValidator::QgsFieldValidator( QObject *parent, const QgsField &field, const QString &defaultValue, const QString &dateFormat )
  : QValidator( parent )
  , mField( field )
  , mDefaultValue( defaultValue )
  , mDateFormat( dateFormat )
{
  switch ( mField.type() )
  {
    case QMetaType::Type::Int:
    {
      if ( mField.length() > 0 )
      {
        const QString re = QStringLiteral( "-?\\d{0,%1}" ).arg( mField.length() );
        mValidator = new QRegularExpressionValidator( QRegularExpression( re ), parent );
      }
      else
      {
        mValidator = new QIntValidator( parent );
      }
    }
    break;

    case QMetaType::Type::Double:
    {
      if ( mField.length() > 0 && mField.precision() > 0 )
      {
        QString re;
        // Also accept locale's decimalPoint if it's not a dot
        if ( QLocale().decimalPoint() != '.' )
        {
          re = QStringLiteral( "-?\\d{0,%1}([\\.%2]\\d{0,%3})?" ).arg( mField.length() - mField.precision() ).arg( QLocale().decimalPoint() ).arg( mField.precision() );
        }
        else
        {
          re = QStringLiteral( "-?\\d{0,%1}([\\.,]\\d{0,%2})?" ).arg( mField.length() - mField.precision() ).arg( mField.precision() );
        }
        mValidator = new QRegularExpressionValidator( QRegularExpression( re ), parent );
      }
      else if ( mField.length() > 0 && mField.precision() == 0 )
      {
        const QString re = QStringLiteral( "-?\\d{0,%1}" ).arg( mField.length() );
        mValidator = new QRegularExpressionValidator( QRegularExpression( re ), parent );
      }
      else if ( mField.precision() > 0 )
      {
        QString re;
        // Also accept locale's decimalPoint if it's not a dot
        if ( QLocale().decimalPoint() != '.' )
        {
          re = QStringLiteral( "-?\\d*([\\.%1]\\d{0,%2})?" ).arg( QLocale().decimalPoint(), mField.precision() );
        }
        else
        {
          re = QStringLiteral( "-?\\d*([\\.]\\d{0,%1})?" ).arg( mField.precision() );
        }
        mValidator = new QRegularExpressionValidator( QRegularExpression( re ), parent );
      }
      else
      {
        mValidator = new QDoubleValidator( parent );
      }
    }
    break;

    case QMetaType::Type::LongLong :
      mValidator = new QgsLongLongValidator( parent );
      break;

    default:
      mValidator = nullptr;
  }

  mNullValue = QgsApplication::nullRepresentation();
}

QgsFieldValidator::~QgsFieldValidator()
{
  delete mValidator;
}

QValidator::State QgsFieldValidator::validate( QString &s, int &i ) const
{
  // empty values are considered NULL for numbers and dates and are acceptable
  if ( s.isEmpty() &&
       ( mField.type() == QMetaType::Type::Double
         || mField.type() == QMetaType::Type::Int
         || mField.type() == QMetaType::Type::LongLong
         || mField.type() == QMetaType::Type::QDate
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
    const QValidator::State result = mValidator->validate( s, i );
    return result;
  }
  else if ( mField.type() == QMetaType::Type::QString )
  {
    if ( s == mNullValue )
      return Acceptable;

    // allow entering the NULL representation, which might be longer than the actual field
    if ( mField.length() > 0 && s.size() > mField.length() )
    {
      if ( !mNullValue.isEmpty() && !s.isEmpty() && s.size() < mNullValue.size() && s == mNullValue.left( s.size() ) )
        return Intermediate;

      if ( !mDefaultValue.isEmpty() && !s.isEmpty() && s.size() < mDefaultValue.size() && s == mDefaultValue.left( s.size() ) )
        return Intermediate;

      return Invalid;
    }
  }
  else if ( mField.type() == QMetaType::Type::QDate )
  {
    return QDate::fromString( s, mDateFormat ).isValid() ? Acceptable : Intermediate;
  }
  else if ( mField.type() == QMetaType::Type::QVariantMap )
  {
    return Acceptable;
  }
  else if ( mField.type() == QMetaType::Type::User && mField.typeName().compare( QLatin1String( "geometry" ), Qt::CaseInsensitive ) == 0 )
  {
    return Acceptable;
  }
  else
  {
    QgsDebugError(
      QStringLiteral( "unsupported type %1 (%2) for validation" )
      .arg( mField.type() )
      .arg( mField.typeName() )
    );
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
  else if ( mField.type() == QMetaType::Type::QString && mField.length() > 0 && s.size() > mField.length() && s != mDefaultValue )
  {
    // if the value is longer, this must be a partial NULL representation
    s = mNullValue;
  }
  else if ( mField.type() == QMetaType::Type::QDate )
  {
    // invalid dates will also translate to NULL
    s = QString();
  }
}
