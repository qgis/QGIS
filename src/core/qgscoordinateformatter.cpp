/***************************************************************************
                        qgscoordinateformatter.cpp
                        --------------------------
  begin                : Decemeber 2015
  copyright            : (C) 2015 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscoordinateformatter.h"
#include "qgis.h"

#include <QObject> // for tr()

QString QgsCoordinateFormatter::formatX( double x, QgsCoordinateFormatter::Format format, int precision, FormatFlags flags )
{
  switch ( format )
  {
    case FormatPair:
      return formatAsPair( x, precision );

    case FormatDegreesMinutesSeconds:
      return formatXAsDegreesMinutesSeconds( x, precision, flags );

    case FormatDegreesMinutes:
      return formatXAsDegreesMinutes( x, precision, flags );

    case FormatDecimalDegrees:
      return formatXAsDegrees( x, precision, flags );
  }
  return QString(); //avoid warnings
}

QString QgsCoordinateFormatter::formatY( double y, QgsCoordinateFormatter::Format format, int precision, FormatFlags flags )
{
  switch ( format )
  {
    case FormatPair:
      return formatAsPair( y, precision );

    case FormatDegreesMinutesSeconds:
      return formatYAsDegreesMinutesSeconds( y, precision, flags );

    case FormatDegreesMinutes:
      return formatYAsDegreesMinutes( y, precision, flags );

    case FormatDecimalDegrees:
      return formatYAsDegrees( y, precision, flags );
  }
  return QString(); //avoid warnings
}

QString QgsCoordinateFormatter::format( const QgsPointXY &point, QgsCoordinateFormatter::Format format, int precision, FormatFlags flags )
{
  return QStringLiteral( "%1,%2" ).arg( formatX( point.x(), format, precision, flags ),
                                        formatY( point.y(), format, precision, flags ) );
}

QString QgsCoordinateFormatter::asPair( double x, double y, int precision )
{
  QString s = formatAsPair( x, precision );
  s += ',';
  s += formatAsPair( y, precision );
  return s;
}

QString QgsCoordinateFormatter::formatAsPair( double val, int precision )
{
  return std::isfinite( val ) ? QString::number( val, 'f', precision ) : QObject::tr( "infinite" );
}

QString QgsCoordinateFormatter::formatXAsDegreesMinutesSeconds( double val, int precision, FormatFlags flags )
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

  int precisionMultiplier = std::pow( 10.0, precision );

  int degreesX = int( std::fabs( wrappedX ) );
  double floatMinutesX = ( std::fabs( wrappedX ) - degreesX ) * 60.0;
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
  if ( flags.testFlag( FlagDegreesUseStringSuffix ) )
  {
    hemisphere = wrappedX < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
  }
  else
  {
    if ( wrappedX < 0 )
    {
      sign = QObject::tr( "-" );
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

  //pad with leading digits if required
  if ( flags.testFlag( FlagDegreesPadMinutesSeconds ) )
  {
    minutesX = QString( "%1" ).arg( intMinutesX, 2, 10, QChar( '0' ) );
    int digits = 2 + ( precision == 0 ? 0 : 1 + precision ); //1 for decimal place if required
    strSecondsX = QString( "%1" ).arg( secondsX, digits, 'f', precision, QChar( '0' ) );
  }
  else
  {
    minutesX =  QString::number( intMinutesX );
    strSecondsX = QString::number( secondsX, 'f', precision );
  }

  return sign + QString::number( degreesX ) + QChar( 176 ) +
         minutesX + QChar( 0x2032 ) +
         strSecondsX + QChar( 0x2033 ) +
         hemisphere;
}

QString QgsCoordinateFormatter::formatYAsDegreesMinutesSeconds( double val, int precision, FormatFlags flags )
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

  int precisionMultiplier = std::pow( 10.0, precision );

  int degreesY = int( std::fabs( wrappedY ) );
  double floatMinutesY = ( std::fabs( wrappedY ) - degreesY ) * 60.0;
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
  if ( flags.testFlag( FlagDegreesUseStringSuffix ) )
  {
    hemisphere = wrappedY < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( wrappedY < 0 )
    {
      sign = QObject::tr( "-" );
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

  //pad with leading digits if required
  if ( flags.testFlag( FlagDegreesPadMinutesSeconds ) )
  {
    strMinutesY = QString( "%1" ).arg( intMinutesY, 2, 10, QChar( '0' ) );
    int digits = 2 + ( precision == 0 ? 0 : 1 + precision ); //1 for decimal place if required
    strSecondsY = QString( "%1" ).arg( secondsY, digits, 'f', precision, QChar( '0' ) );
  }
  else
  {
    strMinutesY = QString::number( intMinutesY );
    strSecondsY = QString::number( secondsY, 'f', precision );
  }

  return sign + QString::number( degreesY ) + QChar( 176 ) +
         strMinutesY + QChar( 0x2032 ) +
         strSecondsY + QChar( 0x2033 ) +
         hemisphere;
}

QString QgsCoordinateFormatter::formatXAsDegreesMinutes( double val, int precision, FormatFlags flags )
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

  int precisionMultiplier = std::pow( 10.0, precision );

  //make sure rounding to specified precision doesn't create minutes >= 60
  if ( std::round( floatMinutesX * precisionMultiplier ) >= 60 * precisionMultiplier )
  {
    floatMinutesX = std::max( floatMinutesX - 60, 0.0 );
    degreesX++;
  }

  QString hemisphere;
  QString sign;
  if ( flags.testFlag( FlagDegreesUseStringSuffix ) )
  {
    hemisphere = wrappedX < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
  }
  else
  {
    if ( wrappedX < 0 )
    {
      sign = QObject::tr( "-" );
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

  //pad minutes with leading digits if required
  int digits = 2 + ( precision == 0 ? 0 : 1 + precision ); //1 for decimal place if required
  QString strMinutesX = flags.testFlag( FlagDegreesPadMinutesSeconds ) ? QString( "%1" ).arg( floatMinutesX, digits, 'f', precision, QChar( '0' ) )
                        : QString::number( floatMinutesX, 'f', precision );

  return sign + QString::number( degreesX ) + QChar( 176 ) +
         strMinutesX + QChar( 0x2032 ) +
         hemisphere;
}

QString QgsCoordinateFormatter::formatYAsDegreesMinutes( double val, int precision, FormatFlags flags )
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

  int precisionMultiplier = std::pow( 10.0, precision );

  //make sure rounding to specified precision doesn't create minutes >= 60
  if ( std::round( floatMinutesY * precisionMultiplier ) >= 60 * precisionMultiplier )
  {
    floatMinutesY = std::max( floatMinutesY - 60, 0.0 );
    degreesY++;
  }

  QString hemisphere;
  QString sign;
  if ( flags.testFlag( FlagDegreesUseStringSuffix ) )
  {
    hemisphere = wrappedY < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( wrappedY < 0 )
    {
      sign = QObject::tr( "-" );
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( degreesY == 0 && std::round( floatMinutesY * precisionMultiplier ) == 0 )
  {
    sign.clear();
    hemisphere.clear();
  }


  //pad minutes with leading digits if required
  int digits = 2 + ( precision == 0 ? 0 : 1 + precision ); //1 for decimal place if required
  QString strMinutesY = flags.testFlag( FlagDegreesPadMinutesSeconds ) ? QString( "%1" ).arg( floatMinutesY, digits, 'f', precision, QChar( '0' ) )
                        : QString::number( floatMinutesY, 'f', precision );

  return sign + QString::number( degreesY ) + QChar( 176 ) +
         strMinutesY + QChar( 0x2032 ) +
         hemisphere;
}

QString QgsCoordinateFormatter::formatXAsDegrees( double val, int precision, FormatFlags flags )
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

  double absX = std::fabs( wrappedX );

  int precisionMultiplier = std::pow( 10.0, precision );

  QString hemisphere;
  QString sign;
  if ( flags.testFlag( FlagDegreesUseStringSuffix ) )
  {
    hemisphere = wrappedX < 0 ? QObject::tr( "W" ) : QObject::tr( "E" );
  }
  else
  {
    if ( wrappedX < 0 )
    {
      sign = QObject::tr( "-" );
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

  return sign + QString::number( absX, 'f', precision ) + QChar( 176 ) + hemisphere;
}

QString QgsCoordinateFormatter::formatYAsDegrees( double val, int precision, FormatFlags flags )
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

  double absY = std::fabs( wrappedY );

  int precisionMultiplier = std::pow( 10.0, precision );

  QString hemisphere;
  QString sign;
  if ( flags.testFlag( FlagDegreesUseStringSuffix ) )
  {
    hemisphere = wrappedY < 0 ? QObject::tr( "S" ) : QObject::tr( "N" );
  }
  else
  {
    if ( wrappedY < 0 )
    {
      sign = QObject::tr( "-" );
    }
  }
  //check if coordinate is all zeros for the specified precision, and if so,
  //remove the sign and hemisphere strings
  if ( std::round( absY * precisionMultiplier ) == 0 )
  {
    sign.clear();
    hemisphere.clear();
  }

  return sign + QString::number( absY, 'f', precision ) + QChar( 176 ) + hemisphere;
}
