/***************************************************************************
                             qgsfractionnumericformat.cpp
                             ----------------------------
    begin                : March 2020
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

#include "qgsfractionnumericformat.h"
#include "qgis.h"
#include <QString>
#include <memory>
#include <iostream>
#include <locale>
#include <sstream>
#include <iomanip>

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

QgsFractionNumericFormat::QgsFractionNumericFormat()
{
}

QString QgsFractionNumericFormat::id() const
{
  return QStringLiteral( "fraction" );
}

QString QgsFractionNumericFormat::visibleName() const
{
  return QObject::tr( "Fraction" );
}

int QgsFractionNumericFormat::sortKey()
{
  return 100;
}

QString QgsFractionNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  std::basic_stringstream<wchar_t> os;
  os.imbue( std::locale( os.getloc(), new formatter( mThousandsSeparator.isNull() ? context.thousandsSeparator() : mThousandsSeparator,
                         mShowThousandsSeparator,
                         context.decimalSeparator() ) ) );

  unsigned long long num;
  unsigned long long den;
  int sign;

  QString res;

  const double fixed = std::floor( std::fabs( value ) );
  const bool success = doubleToVulgarFraction( std::fabs( value ) - fixed, num, den, sign );
  if ( success )
  {
    if ( mUseDedicatedUnicode && num == 1 && den == 2 )
      res = QChar( 0xBD );  //½
    else if ( mUseDedicatedUnicode && num == 1 && den == 3 )
      res = QChar( 0x2153 );  //⅓
    else if ( mUseDedicatedUnicode && num == 2 && den == 3 )
      res = QChar( 0x2154 );  //⅔
    else if ( mUseDedicatedUnicode && num == 1 && den == 4 )
      res = QChar( 0xBC );  //¼
    else if ( mUseDedicatedUnicode && num == 3 && den == 4 )
      res = QChar( 0xBE );  //¾
    else if ( mUseDedicatedUnicode && num == 1 && den == 5 )
      res = QChar( 0x2155 );  //⅕
    else if ( mUseDedicatedUnicode && num == 2 && den == 5 )
      res = QChar( 0x2156 );  //⅖
    else if ( mUseDedicatedUnicode && num == 3 && den == 5 )
      res = QChar( 0x2157 );  //⅗
    else if ( mUseDedicatedUnicode && num == 4 && den == 5 )
      res = QChar( 0x2158 );  //⅘
    else if ( mUseDedicatedUnicode && num == 1 && den == 6 )
      res = QChar( 0x2159 ); //⅙
    else if ( mUseDedicatedUnicode && num == 5 && den == 6 )
      res = QChar( 0x215A ); //⅚
    else if ( mUseDedicatedUnicode && num == 1 && den == 7 )
      res = QChar( 0x2150 ); //⅐
    else if ( mUseDedicatedUnicode && num == 1 && den == 8 )
      res = QChar( 0x215B ); //⅛
    else if ( mUseDedicatedUnicode && num == 3 && den == 8 )
      res = QChar( 0x215C ); //⅜
    else if ( mUseDedicatedUnicode && num == 5 && den == 8 )
      res = QChar( 0x215D ); //⅝
    else if ( mUseDedicatedUnicode && num == 7 && den == 8 )
      res = QChar( 0x215E ); //⅞
    else if ( mUseDedicatedUnicode && num == 1 && den == 9 )
      res = QChar( 0x2151 ); //⅑
    else if ( mUseDedicatedUnicode && num == 1 && den == 10 )
      res = QChar( 0x2152 ); //⅒
    else if ( mUseUnicodeSuperSubscript )
      res = num == 0 ? QString() : QStringLiteral( "%1%2%3" ).arg( toUnicodeSuperscript( QString::number( num ) ),
            QChar( 0x002F ), // "SOLIDUS" character
            toUnicodeSubscript( QString::number( den ) ) );
    else
      res = num == 0 ? QString() : QStringLiteral( "%2/%3" ).arg( num ).arg( den );
    if ( fixed )
    {
      os << std::fixed << std::setprecision( 0 );
      os << fixed;
      res.prepend( QString::fromStdWString( os.str() ) + ' ' );
      res = res.trimmed();
    }
    if ( res.isEmpty() )
      res = QString::number( 0 );

    if ( value < 0 )
      res.prepend( context.negativeSign() );
  }
  else
  {
    os << std::fixed << std::setprecision( 10 );
    os << value;
    res = QString::fromStdWString( os.str() );
  }

  if ( value > 0 && mShowPlusSign )
  {
    res.prepend( context.positiveSign() );
  }

  return res;
}

QgsNumericFormat *QgsFractionNumericFormat::clone() const
{
  return new QgsFractionNumericFormat( *this );
}

QgsNumericFormat *QgsFractionNumericFormat::create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  std::unique_ptr< QgsFractionNumericFormat > res = std::make_unique< QgsFractionNumericFormat >();
  res->setConfiguration( configuration, context );
  return res.release();
}

QVariantMap QgsFractionNumericFormat::configuration( const QgsReadWriteContext & ) const
{
  QVariantMap res;
  res.insert( QStringLiteral( "show_thousand_separator" ), mShowThousandsSeparator );
  res.insert( QStringLiteral( "show_plus" ), mShowPlusSign );
  res.insert( QStringLiteral( "thousand_separator" ), mThousandsSeparator.isNull() ? QVariant() : QVariant::fromValue( mThousandsSeparator ) );
  res.insert( QStringLiteral( "use_dedicated_unicode" ), mUseDedicatedUnicode );
  res.insert( QStringLiteral( "use_unicode_supersubscript" ), mUseUnicodeSuperSubscript );
  return res;
}

double QgsFractionNumericFormat::suggestSampleValue() const
{
  return 1234.75;
}

bool QgsFractionNumericFormat::useDedicatedUnicodeCharacters() const
{
  return mUseDedicatedUnicode;
}

void QgsFractionNumericFormat::setUseDedicatedUnicodeCharacters( bool enabled )
{
  mUseDedicatedUnicode = enabled;
}

bool QgsFractionNumericFormat::useUnicodeSuperSubscript() const
{
  return mUseUnicodeSuperSubscript;
}

void QgsFractionNumericFormat::setUseUnicodeSuperSubscript( bool enabled )
{
  mUseUnicodeSuperSubscript = enabled;
}

void QgsFractionNumericFormat::setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext & )
{
  mShowThousandsSeparator = configuration.value( QStringLiteral( "show_thousand_separator" ), true ).toBool();
  mShowPlusSign = configuration.value( QStringLiteral( "show_plus" ), false ).toBool();
  mThousandsSeparator = configuration.value( QStringLiteral( "thousand_separator" ), QChar() ).toChar();
  mUseDedicatedUnicode = configuration.value( QStringLiteral( "use_dedicated_unicode" ), false ).toBool();
  mUseUnicodeSuperSubscript = configuration.value( QStringLiteral( "use_unicode_supersubscript" ), true ).toBool();
}

bool QgsFractionNumericFormat::showThousandsSeparator() const
{
  return mShowThousandsSeparator;
}

void QgsFractionNumericFormat::setShowThousandsSeparator( bool showThousandsSeparator )
{
  mShowThousandsSeparator = showThousandsSeparator;
}

bool QgsFractionNumericFormat::showPlusSign() const
{
  return mShowPlusSign;
}

void QgsFractionNumericFormat::setShowPlusSign( bool showPlusSign )
{
  mShowPlusSign = showPlusSign;
}

QChar QgsFractionNumericFormat::thousandsSeparator() const
{
  return mThousandsSeparator;
}

void QgsFractionNumericFormat::setThousandsSeparator( QChar character )
{
  mThousandsSeparator = character;
}

QString QgsFractionNumericFormat::toUnicodeSuperscript( const QString &input )
{
  QString res = input;
  for ( int i = 0; i < input.size(); ++i )
  {
    const QChar c = input.at( i );
    if ( c == '0' )
      res[i] =  QChar( 0x2070 ); //⁰
    else if ( c == '1' )
      res[i] =  QChar( 0x00B9 ); //¹
    else if ( c == '2' )
      res[i] =  QChar( 0x00B2 ); //²
    else if ( c == '3' )
      res[i] =  QChar( 0x00B3 ); //³
    else if ( c == '4' )
      res[i] =  QChar( 0x2074 ); //⁴
    else if ( c == '5' )
      res[i] =  QChar( 0x2075 ); //⁵
    else if ( c == '6' )
      res[i] =  QChar( 0x2076 ); //⁶
    else if ( c == '7' )
      res[i] =  QChar( 0x2077 ); //⁷
    else if ( c == '8' )
      res[i] =  QChar( 0x2078 ); //⁸
    else if ( c == '9' )
      res[i] =  QChar( 0x2079 ); //⁹
  }
  return res;
}

QString QgsFractionNumericFormat::toUnicodeSubscript( const QString &input )
{
  QString res = input;
  for ( int i = 0; i < input.size(); ++i )
  {
    const QChar c = input.at( i );
    if ( c == '0' )
      res[i] =  QChar( 0x2080 ); //₀
    else if ( c == '1' )
      res[i] =  QChar( 0x2081 ); //₁
    else if ( c == '2' )
      res[i] =  QChar( 0x2082 ); //₂
    else if ( c == '3' )
      res[i] =  QChar( 0x2083 ); //₃
    else if ( c == '4' )
      res[i] =  QChar( 0x2084 ); //₄
    else if ( c == '5' )
      res[i] =  QChar( 0x2085 ); //₅
    else if ( c == '6' )
      res[i] =  QChar( 0x2086 ); //₆
    else if ( c == '7' )
      res[i] =  QChar( 0x2087 ); //₇
    else if ( c == '8' )
      res[i] =  QChar( 0x2088 ); //₈
    else if ( c == '9' )
      res[i] =  QChar( 0x2089 ); //₉
  }
  return res;
}
