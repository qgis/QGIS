/***************************************************************************
                             qgsbasicnumericformat.cpp
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

#include "qgsbasicnumericformat.h"

#include <iomanip>
#include <iostream>
#include <locale>
#include <memory>

#include "qgis.h"

namespace QgsBasicNumericFormat_ns
{
  struct formatter : std::numpunct<wchar_t>
  {
    formatter( QChar thousands, bool showThousands, QChar decimal )
      : mThousands( thousands.unicode() )
      , mDecimal( decimal.unicode() )
      , mShowThousands( showThousands )
    {}
    wchar_t do_decimal_point() const override { return mDecimal; }
    wchar_t do_thousands_sep() const override { return mThousands; }
    std::string do_grouping() const override { return mShowThousands ? "\3" : "\0"; }

    wchar_t mThousands;
    wchar_t mDecimal;
    bool mShowThousands = true;
  };
}

QgsBasicNumericFormat::QgsBasicNumericFormat()
{
}

QString QgsBasicNumericFormat::id() const
{
  return u"basic"_s;
}

QString QgsBasicNumericFormat::visibleName() const
{
  return QObject::tr( "Number" );
}

int QgsBasicNumericFormat::sortKey()
{
  return 1;
}

QString QgsBasicNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  const QChar decimal = mDecimalSeparator.isNull() ? context.decimalSeparator() : mDecimalSeparator;
  std::basic_stringstream<wchar_t> os;
  os.imbue( std::locale( os.getloc(), new QgsBasicNumericFormat_ns::formatter( mThousandsSeparator.isNull() ? context.thousandsSeparator() : mThousandsSeparator,
                         mShowThousandsSeparator,
                         decimal ) ) );

  if ( !mUseScientific )
  {
    switch ( mRoundingType )
    {
      case DecimalPlaces:
        os << std::fixed << std::setprecision( mNumberDecimalPlaces );
        if ( qgsDoubleNear( value, 0 ) )
          os << 0.0;
        else
          os << value;

        break;

      case SignificantFigures:
      {
        if ( qgsDoubleNear( value, 0 ) )
        {
          os << std::fixed << std::setprecision( mNumberDecimalPlaces - 1 ) << 0.0;
        }
        else
        {
          // digits before decimal point
          const int d = std::floor( std::log10( value < 0 ? -value : value ) ) + 1;
          const double order = std::pow( 10.0, mNumberDecimalPlaces - d );
          os << std::fixed << std::setprecision( std::max( mNumberDecimalPlaces - d, 0 ) ) << std::round( value * order ) / order;
        }
        break;
      }
    }
  }
  else
  {
    os << std::scientific << std::setprecision( mNumberDecimalPlaces );
    if ( qgsDoubleNear( value, 0 ) )
      os << 0.0;
    else
      os << value;
  }

  QString res = QString::fromStdWString( os.str() );

  if ( mShowPlusSign && value > 0 )
    res.prepend( context.positiveSign() );

  if ( !mShowTrailingZeros && res.contains( decimal ) )
  {
    int trimPoint = res.length() - 1;
    int ePoint = 0;
    if ( mUseScientific )
    {
      while ( res.at( trimPoint ).toUpper() != context.exponential().toUpper() )
        trimPoint--;
      ePoint = trimPoint;
      trimPoint--;
    }

    while ( res.at( trimPoint ) == context.zeroDigit() )
      trimPoint--;

    if ( res.at( trimPoint ) == decimal )
      trimPoint--;

    const QString original = res;
    res.truncate( trimPoint + 1 );
    if ( mUseScientific )
      res += original.mid( ePoint );
  }

  return res;
}

QgsNumericFormat *QgsBasicNumericFormat::clone() const
{
  return new QgsBasicNumericFormat( *this );
}

QgsNumericFormat *QgsBasicNumericFormat::create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  auto res = std::make_unique< QgsBasicNumericFormat >();
  res->setConfiguration( configuration, context );
  return res.release();
}

QVariantMap QgsBasicNumericFormat::configuration( const QgsReadWriteContext & ) const
{
  QVariantMap res;
  res.insert( u"decimals"_s, mNumberDecimalPlaces );
  res.insert( u"show_thousand_separator"_s, mShowThousandsSeparator );
  res.insert( u"show_plus"_s, mShowPlusSign );
  res.insert( u"show_trailing_zeros"_s, mShowTrailingZeros );
  res.insert( u"rounding_type"_s, static_cast< int >( mRoundingType ) );
  res.insert( u"thousand_separator"_s, mThousandsSeparator.isNull() ? QVariant() : QVariant::fromValue( mThousandsSeparator ) );
  res.insert( u"decimal_separator"_s, mDecimalSeparator.isNull() ? QVariant() : QVariant::fromValue( mDecimalSeparator ) );
  return res;
}

void QgsBasicNumericFormat::setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext & )
{
  mNumberDecimalPlaces = configuration.value( u"decimals"_s, 6 ).toInt();
  mShowThousandsSeparator = configuration.value( u"show_thousand_separator"_s, true ).toBool();
  mShowPlusSign = configuration.value( u"show_plus"_s, false ).toBool();
  mShowTrailingZeros = configuration.value( u"show_trailing_zeros"_s, false ).toBool();
  mRoundingType = static_cast< RoundingType >( configuration.value( u"rounding_type"_s, static_cast< int >( DecimalPlaces ) ).toInt() );
  mThousandsSeparator = configuration.value( u"thousand_separator"_s, QChar() ).toChar();
  mDecimalSeparator = configuration.value( u"decimal_separator"_s, QChar() ).toChar();
}

int QgsBasicNumericFormat::numberDecimalPlaces() const
{
  return mNumberDecimalPlaces;
}

void QgsBasicNumericFormat::setNumberDecimalPlaces( int numberDecimalPlaces )
{
  mNumberDecimalPlaces = numberDecimalPlaces;
}

bool QgsBasicNumericFormat::showThousandsSeparator() const
{
  return mShowThousandsSeparator;
}

void QgsBasicNumericFormat::setShowThousandsSeparator( bool showThousandsSeparator )
{
  mShowThousandsSeparator = showThousandsSeparator;
}

bool QgsBasicNumericFormat::showPlusSign() const
{
  return mShowPlusSign;
}

void QgsBasicNumericFormat::setShowPlusSign( bool showPlusSign )
{
  mShowPlusSign = showPlusSign;
}

bool QgsBasicNumericFormat::showTrailingZeros() const
{
  return mShowTrailingZeros;
}

void QgsBasicNumericFormat::setShowTrailingZeros( bool showTrailingZeros )
{
  mShowTrailingZeros = showTrailingZeros;
}

QgsBasicNumericFormat::RoundingType QgsBasicNumericFormat::roundingType() const
{
  return mRoundingType;
}

void QgsBasicNumericFormat::setRoundingType( QgsBasicNumericFormat::RoundingType type )
{
  mRoundingType = type;
}

QChar QgsBasicNumericFormat::thousandsSeparator() const
{
  return mThousandsSeparator;
}

void QgsBasicNumericFormat::setThousandsSeparator( QChar character )
{
  mThousandsSeparator = character;
}

QChar QgsBasicNumericFormat::decimalSeparator() const
{
  return mDecimalSeparator;
}

void QgsBasicNumericFormat::setDecimalSeparator( QChar character )
{
  mDecimalSeparator = character;
}
