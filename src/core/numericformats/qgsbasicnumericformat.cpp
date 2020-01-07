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
#include "qgis.h"
#include <memory>
#include <iostream>
#include <locale>
#include <iomanip>

struct formatter : std::numpunct<char>
{
  formatter( QChar thousands, bool showThousands, QChar decimal )
    : mThousands( thousands.toLatin1() )
    , mDecimal( decimal.toLatin1() )
    , mShowThousands( showThousands )
  {}
  char do_decimal_point() const override {return mDecimal;}
  char do_thousands_sep() const override {return mThousands;}
  string_type do_grouping() const override { return mShowThousands ? "\3" : "\0"; }

  char mThousands;
  char mDecimal;
  bool mShowThousands = true;
};

QgsBasicNumericFormat::QgsBasicNumericFormat()
{
}

QString QgsBasicNumericFormat::id() const
{
  return QStringLiteral( "basic" );
}

QString QgsBasicNumericFormat::visibleName() const
{
  return QObject::tr( "Number" );
}

QString QgsBasicNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  std::ostringstream os;
  os.imbue( std::locale( os.getloc(), new formatter( context.thousandsSeparator(), mShowThousandsSeparator, context.decimalSeparator() ) ) );

  if ( !mUseScientific )
    os << std::fixed << std::setprecision( mNumberDecimalPlaces );
  else
    os << std::scientific << std::setprecision( mNumberDecimalPlaces );

  os << value;
  QString res = QString::fromStdString( os.str() );

  if ( mShowPlusSign && value > 0 )
    res.prepend( '+' );

  if ( !mShowTrailingZeros && res.contains( context.decimalSeparator() ) )
  {
    int trimPoint = res.length() - 1;
    int ePoint = 0;
    if ( mUseScientific )
    {
      while ( res.at( trimPoint ) != 'e' && res.at( trimPoint ) != 'E' )
        trimPoint--;
      ePoint = trimPoint;
      trimPoint--;
    }

    while ( res.at( trimPoint ) == '0' )
      trimPoint--;

    if ( res.at( trimPoint ) == context.decimalSeparator() )
      trimPoint--;

    QString original = res;
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
  std::unique_ptr< QgsBasicNumericFormat > res = qgis::make_unique< QgsBasicNumericFormat >();
  res->setConfiguration( configuration, context );
  return res.release();
}

QVariantMap QgsBasicNumericFormat::configuration( const QgsReadWriteContext & ) const
{
  QVariantMap res;
  res.insert( QStringLiteral( "decimals" ), mNumberDecimalPlaces );
  res.insert( QStringLiteral( "show_thousand_separator" ), mShowThousandsSeparator );
  res.insert( QStringLiteral( "show_plus" ), mShowPlusSign );
  res.insert( QStringLiteral( "show_trailing_zeros" ), mShowTrailingZeros );
  return res;
}

void QgsBasicNumericFormat::setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext & )
{
  mNumberDecimalPlaces = configuration.value( QStringLiteral( "decimals" ), 6 ).toInt();
  mShowThousandsSeparator = configuration.value( QStringLiteral( "show_thousand_separator" ), true ).toBool();
  mShowPlusSign = configuration.value( QStringLiteral( "show_plus" ), false ).toBool();
  mShowTrailingZeros = configuration.value( QStringLiteral( "show_trailing_zeros" ), false ).toBool();
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
