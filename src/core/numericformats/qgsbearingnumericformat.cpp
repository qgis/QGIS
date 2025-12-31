/***************************************************************************
                             qgsbearingnumericformat.cpp
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

#include "qgsbearingnumericformat.h"

#include "qgis.h"

QgsBearingNumericFormat::QgsBearingNumericFormat()
{
}

QString QgsBearingNumericFormat::id() const
{
  return u"bearing"_s;
}

QString QgsBearingNumericFormat::visibleName() const
{
  return QObject::tr( "Bearing" );
}

int QgsBearingNumericFormat::sortKey()
{
  return DEFAULT_SORT_KEY;
}

double QgsBearingNumericFormat::suggestSampleValue() const
{
  return 270.123;
}

QString QgsBearingNumericFormat::formatDouble( double value, const QgsNumericFormatContext &context ) const
{
  switch ( mDirectionFormat )
  {
    case UseRange0To180WithEWDirectionalSuffix:
    {
      value = fmod( value, 360.0 );
      if ( value > 180 )
        value -= 360;

      QString res = QgsBasicNumericFormat::formatDouble( std::fabs( value ), context );

      if ( res != "0"_L1 && res != "180"_L1 )
        // TODO also test for 0.000, 180.000, etc
        res += QChar( 176 ) + ( value < 0 ? QObject::tr( "W" ) : QObject::tr( "E" ) );
      else
        res += QChar( 176 );

      return res;
    }

    case UseRangeNegative180ToPositive180:
    {
      value = fmod( value, 360.0 );
      if ( value > 180 )
        value -= 360;
      if ( value < -180 )
        value += 360;

      return QgsBasicNumericFormat::formatDouble( value, context ) + QChar( 176 );
    }

    case UseRange0To360:
      value = fmod( value, 360.0 );
      if ( value < 0 )
        value += 360;
      return QgsBasicNumericFormat::formatDouble( value, context ) + QChar( 176 );
  }

  return QgsBasicNumericFormat::formatDouble( value, context );
}

QgsBearingNumericFormat *QgsBearingNumericFormat::clone() const
{
  return new QgsBearingNumericFormat( *this );
}

QgsNumericFormat *QgsBearingNumericFormat::create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const
{
  auto res = std::make_unique< QgsBearingNumericFormat >();
  res->setConfiguration( configuration, context );
  res->mDirectionFormat = static_cast< FormatDirectionOption >( configuration.value( u"direction_format"_s, 0 ).toInt() );
  return res.release();
}

QVariantMap QgsBearingNumericFormat::configuration( const QgsReadWriteContext &context ) const
{
  QVariantMap res = QgsBasicNumericFormat::configuration( context );
  res.insert( u"direction_format"_s, static_cast< int >( mDirectionFormat ) );
  return res;
}

QgsBearingNumericFormat::FormatDirectionOption QgsBearingNumericFormat::directionFormat() const
{
  return mDirectionFormat;
}

void QgsBearingNumericFormat::setDirectionFormat( FormatDirectionOption directionFormat )
{
  mDirectionFormat = directionFormat;
}

void QgsBearingNumericFormat::setConfiguration( const QVariantMap &configuration, const QgsReadWriteContext &context )
{
  QgsBasicNumericFormat::setConfiguration( configuration, context );
  mDirectionFormat = static_cast< FormatDirectionOption >( configuration.value( u"direction_format"_s, 0 ).toInt() );
}
