/***************************************************************************
                             qgscurrencynumericformat.cpp
                             ----------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscurrencynumericformat.h"

#include "qgis.h"

QgsCurrencyNumericFormat::QgsCurrencyNumericFormat()
  : mPrefix( u"$"_s )
{
  setNumberDecimalPlaces( 2 );
  setShowTrailingZeros( true );
}

QString QgsCurrencyNumericFormat::id() const
{
  return u"currency"_s;
}

QString QgsCurrencyNumericFormat::visibleName() const
{
  return QObject::tr( "Currency" );
}

int QgsCurrencyNumericFormat::sortKey()
{
  return DEFAULT_SORT_KEY;
}

double QgsCurrencyNumericFormat::suggestSampleValue() const
{
  return 1234.56;
}

QString QgsCurrencyNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  const QString res = QgsBasicNumericFormat::formatDouble( value, context );
  if ( value < 0 || ( value > 0 && showPlusSign() ) )
    return res.at( 0 ) + mPrefix + res.mid( 1 ) + mSuffix;
  else
    return mPrefix + res + mSuffix;
}

QgsNumericFormat *QgsCurrencyNumericFormat::clone() const
{
  return new QgsCurrencyNumericFormat( *this );
}

QgsNumericFormat *QgsCurrencyNumericFormat::create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  auto res = std::make_unique< QgsCurrencyNumericFormat >();
  res->setConfiguration( configuration, context );
  res->mPrefix = configuration.value( u"prefix"_s, u"$"_s ).toString();
  res->mSuffix = configuration.value( u"suffix"_s, QString() ).toString();

  // override base class default for number of decimal places -- we want to default to 2, showing trailing zeros
  res->setNumberDecimalPlaces( configuration.value( u"decimals"_s, 2 ).toInt() );
  res->setShowTrailingZeros( configuration.value( u"show_trailing_zeros"_s, true ).toBool() );
  res->setRoundingType( QgsBasicNumericFormat::DecimalPlaces );

  return res.release();
}

QVariantMap QgsCurrencyNumericFormat::configuration( const QgsReadWriteContext &context ) const
{
  QVariantMap res = QgsBasicNumericFormat::configuration( context );
  res.insert( u"prefix"_s, mPrefix );
  res.insert( u"suffix"_s, mSuffix );
  return res;
}

QString QgsCurrencyNumericFormat::prefix() const
{
  return mPrefix;
}

void QgsCurrencyNumericFormat::setPrefix( const QString &prefix )
{
  mPrefix = prefix;
}

QString QgsCurrencyNumericFormat::suffix() const
{
  return mSuffix;
}

void QgsCurrencyNumericFormat::setSuffix( const QString &suffix )
{
  mSuffix = suffix;
}
