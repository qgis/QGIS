/***************************************************************************
                             qgscoordinatenumericformat.cpp
                             --------------------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscoordinatenumericformat.h"
#include "qgis.h"
#include "qgscoordinateformatter.h"

#include <memory>
#include <iostream>
#include <locale>
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


QgsGeographicCoordinateNumericFormat::QgsGeographicCoordinateNumericFormat()
{
}

QString QgsGeographicCoordinateNumericFormat::id() const
{
  return QStringLiteral( "geographiccoordinate" );
}

QString QgsGeographicCoordinateNumericFormat::visibleName() const
{
  return QObject::tr( "Geographic Coordinate" );
}

int QgsGeographicCoordinateNumericFormat::sortKey()
{
  return DEFAULT_SORT_KEY;
}

double QgsGeographicCoordinateNumericFormat::suggestSampleValue() const
{
  return 3.7555;
}

QString QgsGeographicCoordinateNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  const QChar decimal = decimalSeparator().isNull() ? context.decimalSeparator() : decimalSeparator();
  std::basic_stringstream<wchar_t> os;
  os.imbue( std::locale( os.getloc(), new formatter( thousandsSeparator().isNull() ? context.thousandsSeparator() : thousandsSeparator(),
                         false,
                         decimal ) ) );

  switch ( context.interpretation() )
  {
    case QgsNumericFormatContext::Interpretation::Latitude:
      return formatLatitude( value, os, context );

    case QgsNumericFormatContext::Interpretation::Generic:
    case QgsNumericFormatContext::Interpretation::Longitude:
      return formatLongitude( value, os, context );
  }
  BUILTIN_UNREACHABLE
}

QgsGeographicCoordinateNumericFormat *QgsGeographicCoordinateNumericFormat::clone() const
{
  return new QgsGeographicCoordinateNumericFormat( *this );
}

QgsNumericFormat *QgsGeographicCoordinateNumericFormat::create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  std::unique_ptr< QgsGeographicCoordinateNumericFormat > res = std::make_unique< QgsGeographicCoordinateNumericFormat >();
  res->setConfiguration( configuration, context );
  res->mAngleFormat = qgsEnumKeyToValue( configuration.value( QStringLiteral( "angle_format" ) ).toString(), AngleFormat::DecimalDegrees );
  res->mShowLeadingZeros = configuration.value( QStringLiteral( "show_leading_zeros" ), false ).toBool();
  res->mShowLeadingDegreeZeros = configuration.value( QStringLiteral( "show_leading_degree_zeros" ), false ).toBool();
  res->mUseSuffix = configuration.value( QStringLiteral( "show_suffix" ), false ).toBool();
  return res.release();
}

QVariantMap QgsGeographicCoordinateNumericFormat::configuration( const QgsReadWriteContext &context ) const
{
  QVariantMap res = QgsBasicNumericFormat::configuration( context );
  res.insert( QStringLiteral( "angle_format" ), qgsEnumValueToKey( mAngleFormat ) );
  res.insert( QStringLiteral( "show_leading_zeros" ), mShowLeadingZeros );
  res.insert( QStringLiteral( "show_leading_degree_zeros" ), mShowLeadingDegreeZeros );
  res.insert( QStringLiteral( "show_suffix" ), mUseSuffix );
  return res;
}

QgsGeographicCoordinateNumericFormat::AngleFormat QgsGeographicCoordinateNumericFormat::angleFormat() const
{
  return mAngleFormat;
}

void QgsGeographicCoordinateNumericFormat::setAngleFormat( QgsGeographicCoordinateNumericFormat::AngleFormat format )
{
  mAngleFormat = format;
}

void QgsGeographicCoordinateNumericFormat::setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext &context )
{
  QgsBasicNumericFormat::setConfiguration( configuration, context );
  mAngleFormat = qgsEnumKeyToValue( configuration.value( QStringLiteral( "angle_format" ) ).toString(), AngleFormat::DecimalDegrees );
  mShowLeadingZeros = configuration.value( QStringLiteral( "show_leading_zeros" ), false ).toBool();
  mShowLeadingDegreeZeros = configuration.value( QStringLiteral( "show_leading_degree_zeros" ), false ).toBool();
  mUseSuffix = configuration.value( QStringLiteral( "show_suffix" ), false ).toBool();
}

bool QgsGeographicCoordinateNumericFormat::showLeadingZeros() const
{
  return mShowLeadingZeros;
}

void QgsGeographicCoordinateNumericFormat::setShowLeadingZeros( bool newShowLeadingZeros )
{
  mShowLeadingZeros = newShowLeadingZeros;
}

bool QgsGeographicCoordinateNumericFormat::showDegreeLeadingZeros() const
{
  return mShowLeadingDegreeZeros;
}

void QgsGeographicCoordinateNumericFormat::setShowDegreeLeadingZeros( bool show )
{
  mShowLeadingDegreeZeros = show;
}

bool QgsGeographicCoordinateNumericFormat::showDirectionalSuffix() const
{
  return mUseSuffix;
}

void QgsGeographicCoordinateNumericFormat::setShowDirectionalSuffix( bool show )
{
  mUseSuffix = show;
}

QString QgsGeographicCoordinateNumericFormat::formatLongitude( double value, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const
{
  switch ( mAngleFormat )
  {
    case QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutesSeconds:
      return formatLongitudeAsDegreesMinutesSeconds( value, ss, context );
    case QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutes:
      return formatLongitudeAsDegreesMinutes( value, ss,  context );
    case QgsGeographicCoordinateNumericFormat::AngleFormat::DecimalDegrees:
      return formatLongitudeAsDegrees( value, ss, context );
  }
  BUILTIN_UNREACHABLE
}

QString QgsGeographicCoordinateNumericFormat::formatLatitude( double value, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const
{
  switch ( mAngleFormat )
  {
    case QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutesSeconds:
      return formatLatitudeAsDegreesMinutesSeconds( value, ss, context );
    case QgsGeographicCoordinateNumericFormat::AngleFormat::DegreesMinutes:
      return formatLatitudeAsDegreesMinutes( value, ss, context );
    case QgsGeographicCoordinateNumericFormat::AngleFormat::DecimalDegrees:
      return formatLatitudeAsDegrees( value, ss, context );
  }
  BUILTIN_UNREACHABLE
}

QString QgsGeographicCoordinateNumericFormat::formatLatitudeAsDegreesMinutesSeconds( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const
{
  //first, limit latitude to -180 to 180 degree range
  double wrappedY = std::fmod( val, 180.0 );
  //next, wrap around latitudes > 90 or < -90 degrees, so that eg "110S" -> "70N"
  if ( wrappedY > 90.0 )
  {
    wrappedY = wrappedY - 180.0;
  }
  else if ( wrappedY < -90.0 )
  {
    wrappedY = wrappedY + 180.0;
  }

  const double precisionMultiplier = std::pow( 10.0, numberDecimalPlaces() );

  int degreesY = int( std::fabs( wrappedY ) );
  const double floatMinutesY = ( std::fabs( wrappedY ) - degreesY ) * 60.0;
  int intMinutesY = int( floatMinutesY );
  double secondsY = ( floatMinutesY - intMinutesY ) * 60.0;

  //make sure rounding to specified precision doesn't create seconds >= 60
  if ( std::round( secondsY * precisionMultiplier ) >= 60 * precisionMultiplier )
  {
    secondsY = std::max( secondsY - 60, 0.0 );
    intMinutesY++;
    if ( intMinutesY >= 60 )
    {
      intMinutesY -= 60;
      degreesY++;
    }
  }

  QString hemisphere;
  QString sign;
  if ( mUseSuffix )
  {
    hemisphere = wrappedY < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( wrappedY < 0 )
    {
      sign = context.negativeSign();
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( degreesY == 0 && intMinutesY == 0 && std::round( secondsY * precisionMultiplier ) == 0 )
  {
    sign = QString();
    hemisphere.clear();
  }

  QString strMinutesY;
  QString strSecondsY;

  ss << std::fixed << std::setprecision( 0 );
  ss << intMinutesY;

  strMinutesY = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  ss << std::fixed << std::setprecision( numberDecimalPlaces() );
  ss << secondsY;
  strSecondsY = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  trimTrailingZeros( strSecondsY, context );

  //pad with leading digits if required
  if ( mShowLeadingZeros && intMinutesY < 10 )
    strMinutesY = '0' + strMinutesY;

  if ( mShowLeadingZeros && secondsY < 10 )
    strSecondsY = '0' + strSecondsY;

  ss << std::fixed << std::setprecision( 0 );
  ss << degreesY;
  QString degreesYStr = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  if ( mShowLeadingDegreeZeros )
    degreesYStr = QString( QStringLiteral( "00" ) + degreesYStr ).right( 2 );

  return sign + degreesYStr + QChar( 176 ) +
         strMinutesY + QChar( 0x2032 ) +
         strSecondsY + QChar( 0x2033 ) +
         hemisphere;
}

QString QgsGeographicCoordinateNumericFormat::formatLongitudeAsDegreesMinutesSeconds( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const
{
  //first, limit longitude to -360 to 360 degree range
  double wrappedX = std::fmod( val, 360.0 );
  //next, wrap around longitudes > 180 or < -180 degrees, so that eg "190E" -> "170W"
  if ( wrappedX > 180.0 )
  {
    wrappedX = wrappedX - 360.0;
  }
  else if ( wrappedX < -180.0 )
  {
    wrappedX = wrappedX + 360.0;
  }

  const double precisionMultiplier = std::pow( 10.0, numberDecimalPlaces() );

  int degreesX = int( std::fabs( wrappedX ) );
  const double floatMinutesX = ( std::fabs( wrappedX ) - degreesX ) * 60.0;
  int intMinutesX = int( floatMinutesX );
  double secondsX = ( floatMinutesX - intMinutesX ) * 60.0;

  //make sure rounding to specified precision doesn't create seconds >= 60
  if ( std::round( secondsX * precisionMultiplier ) >= 60 * precisionMultiplier )
  {
    secondsX = std::max( secondsX - 60, 0.0 );
    intMinutesX++;
    if ( intMinutesX >= 60 )
    {
      intMinutesX -= 60;
      degreesX++;
    }
  }

  QString hemisphere;
  QString sign;
  if ( mUseSuffix )
  {
    hemisphere = wrappedX < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
  }
  else
  {
    if ( wrappedX < 0 )
    {
      sign = context.negativeSign();
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( degreesX == 0 && intMinutesX == 0 && std::round( secondsX * precisionMultiplier ) == 0 )
  {
    sign.clear();
    hemisphere.clear();
  }

  //also remove directional prefix from 180 degree longitudes
  if ( degreesX == 180 && intMinutesX == 0 && std::round( secondsX * precisionMultiplier ) == 0 )
  {
    hemisphere.clear();
  }

  QString minutesX;
  QString strSecondsX;

  ss << std::fixed << std::setprecision( 0 );
  ss << intMinutesX;

  minutesX = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  ss << std::fixed << std::setprecision( numberDecimalPlaces() );
  ss << secondsX;
  strSecondsX = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  trimTrailingZeros( strSecondsX, context );

  //pad with leading digits if required
  if ( mShowLeadingZeros && intMinutesX < 10 )
    minutesX = '0' + minutesX;

  if ( mShowLeadingZeros && secondsX < 10 )
    strSecondsX = '0' + strSecondsX;

  ss << std::fixed << std::setprecision( 0 );
  ss << degreesX;
  QString degreesXStr = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  if ( mShowLeadingDegreeZeros )
    degreesXStr = QString( QStringLiteral( "000" ) + degreesXStr ).right( 3 );

  return sign + degreesXStr + QChar( 176 ) +
         minutesX + QChar( 0x2032 ) +
         strSecondsX + QChar( 0x2033 ) +
         hemisphere;
}

QString QgsGeographicCoordinateNumericFormat::formatLatitudeAsDegreesMinutes( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const
{
  //first, limit latitude to -180 to 180 degree range
  double wrappedY = std::fmod( val, 180.0 );
  //next, wrap around latitudes > 90 or < -90 degrees, so that eg "110S" -> "70N"
  if ( wrappedY > 90.0 )
  {
    wrappedY = wrappedY - 180.0;
  }
  else if ( wrappedY < -90.0 )
  {
    wrappedY = wrappedY + 180.0;
  }

  int degreesY = int( std::fabs( wrappedY ) );
  double floatMinutesY = ( std::fabs( wrappedY ) - degreesY ) * 60.0;

  const double precisionMultiplier = std::pow( 10.0, numberDecimalPlaces() );

  //make sure rounding to specified precision doesn't create minutes >= 60
  if ( std::round( floatMinutesY * precisionMultiplier ) >= 60 * precisionMultiplier )
  {
    floatMinutesY = std::max( floatMinutesY - 60, 0.0 );
    degreesY++;
  }

  QString hemisphere;
  QString sign;
  if ( mUseSuffix )
  {
    hemisphere = wrappedY < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( wrappedY < 0 )
    {
      sign = context.negativeSign();
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( degreesY == 0 && std::round( floatMinutesY * precisionMultiplier ) == 0 )
  {
    sign.clear();
    hemisphere.clear();
  }

  ss << std::fixed << std::setprecision( numberDecimalPlaces() );
  ss << floatMinutesY;
  QString strMinutesY = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  trimTrailingZeros( strMinutesY, context );

  //pad with leading digits if required
  if ( mShowLeadingZeros && floatMinutesY < 10 )
    strMinutesY = '0' + strMinutesY;

  ss << std::fixed << std::setprecision( 0 );
  ss << degreesY;
  QString degreesYStr = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  if ( mShowLeadingDegreeZeros )
    degreesYStr = QString( QStringLiteral( "00" ) + degreesYStr ).right( 2 );

  return sign + degreesYStr + QChar( 176 ) +
         strMinutesY + QChar( 0x2032 ) +
         hemisphere;
}

QString QgsGeographicCoordinateNumericFormat::formatLongitudeAsDegreesMinutes( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const
{
  //first, limit longitude to -360 to 360 degree range
  double wrappedX = std::fmod( val, 360.0 );
  //next, wrap around longitudes > 180 or < -180 degrees, so that eg "190E" -> "170W"
  if ( wrappedX > 180.0 )
  {
    wrappedX = wrappedX - 360.0;
  }
  else if ( wrappedX < -180.0 )
  {
    wrappedX = wrappedX + 360.0;
  }

  int degreesX = int( std::fabs( wrappedX ) );
  double floatMinutesX = ( std::fabs( wrappedX ) - degreesX ) * 60.0;

  const double precisionMultiplier = std::pow( 10.0, numberDecimalPlaces() );

  //make sure rounding to specified precision doesn't create minutes >= 60
  if ( std::round( floatMinutesX * precisionMultiplier ) >= 60 * precisionMultiplier )
  {
    floatMinutesX = std::max( floatMinutesX - 60, 0.0 );
    degreesX++;
  }

  QString hemisphere;
  QString sign;
  if ( mUseSuffix )
  {
    hemisphere = wrappedX < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
  }
  else
  {
    if ( wrappedX < 0 )
    {
      sign = context.negativeSign();
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( degreesX == 0 && std::round( floatMinutesX * precisionMultiplier ) == 0 )
  {
    sign.clear();
    hemisphere.clear();
  }

  //also remove directional prefix from 180 degree longitudes
  if ( degreesX == 180 && std::round( floatMinutesX * precisionMultiplier ) == 0 )
  {
    hemisphere.clear();
  }

  ss << std::fixed << std::setprecision( numberDecimalPlaces() );
  ss << floatMinutesX;
  QString strMinutesX = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  trimTrailingZeros( strMinutesX, context );

  //pad with leading digits if required
  if ( mShowLeadingZeros && floatMinutesX < 10 )
    strMinutesX = '0' + strMinutesX;

  ss << std::fixed << std::setprecision( 0 );
  ss << degreesX;
  QString degreesXStr = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  if ( mShowLeadingDegreeZeros )
    degreesXStr = QString( QStringLiteral( "000" ) + degreesXStr ).right( 3 );

  return sign + degreesXStr + QChar( 176 ) +
         strMinutesX + QChar( 0x2032 ) +
         hemisphere;
}

QString QgsGeographicCoordinateNumericFormat::formatLatitudeAsDegrees( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const
{
  //first, limit latitude to -180 to 180 degree range
  double wrappedY = std::fmod( val, 180.0 );
  //next, wrap around latitudes > 90 or < -90 degrees, so that eg "110S" -> "70N"
  if ( wrappedY > 90.0 )
  {
    wrappedY = wrappedY - 180.0;
  }
  else if ( wrappedY < -90.0 )
  {
    wrappedY = wrappedY + 180.0;
  }

  const double absY = std::fabs( wrappedY );

  const double precisionMultiplier = std::pow( 10.0, numberDecimalPlaces() );

  QString hemisphere;
  QString sign;
  if ( mUseSuffix )
  {
    hemisphere = wrappedY < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( wrappedY < 0 )
    {
      sign = context.negativeSign();
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( std::round( absY * precisionMultiplier ) == 0 )
  {
    sign.clear();
    hemisphere.clear();
  }

  ss << std::fixed << std::setprecision( numberDecimalPlaces() );
  ss << absY;
  QString strDegreesY = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  trimTrailingZeros( strDegreesY, context );

  if ( mShowLeadingDegreeZeros && absY < 10 )
    strDegreesY = '0' + strDegreesY;

  return sign + strDegreesY + QChar( 176 ) + hemisphere;
}

QString QgsGeographicCoordinateNumericFormat::formatLongitudeAsDegrees( double val, std::basic_stringstream<wchar_t> &ss, const QgsNumericFormatContext &context ) const
{
  //first, limit longitude to -360 to 360 degree range
  double wrappedX = std::fmod( val, 360.0 );
  //next, wrap around longitudes > 180 or < -180 degrees, so that eg "190E" -> "170W"
  if ( wrappedX > 180.0 )
  {
    wrappedX = wrappedX - 360.0;
  }
  else if ( wrappedX < -180.0 )
  {
    wrappedX = wrappedX + 360.0;
  }

  const double absX = std::fabs( wrappedX );

  const double precisionMultiplier = std::pow( 10.0, numberDecimalPlaces() );

  QString hemisphere;
  QString sign;
  if ( mUseSuffix )
  {
    hemisphere = wrappedX < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
  }
  else
  {
    if ( wrappedX < 0 )
    {
      sign = context.negativeSign();
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( std::round( absX * precisionMultiplier ) == 0 )
  {
    sign.clear();
    hemisphere.clear();
  }

  //also remove directional prefix from 180 degree longitudes
  if ( std::round( absX * precisionMultiplier ) == 180 * precisionMultiplier )
  {
    sign.clear();
    hemisphere.clear();
  }

  ss << std::fixed << std::setprecision( numberDecimalPlaces() );
  ss << absX;
  QString strDegreesX = QString::fromStdWString( ss.str() );
  ss.str( std::wstring() );

  trimTrailingZeros( strDegreesX, context );

  if ( mShowLeadingDegreeZeros && absX < 100 )
    strDegreesX = '0' + strDegreesX;
  if ( mShowLeadingDegreeZeros && absX < 10 )
    strDegreesX = '0' + strDegreesX;

  return sign + strDegreesX + QChar( 176 ) + hemisphere;
}

void QgsGeographicCoordinateNumericFormat::trimTrailingZeros( QString &input, const QgsNumericFormatContext &context ) const
{
  const QChar decimal = decimalSeparator().isNull() ? context.decimalSeparator() : decimalSeparator();
  if ( !showTrailingZeros() && input.contains( decimal ) )
  {
    int trimPoint = input.length() - 1;

    while ( input.at( trimPoint ) == context.zeroDigit() )
      trimPoint--;

    if ( input.at( trimPoint ) == decimal )
      trimPoint--;

    input.truncate( trimPoint + 1 );
  }
}
