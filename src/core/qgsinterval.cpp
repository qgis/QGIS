/***************************************************************************
  qgsinterval.cpp
  ---------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinterval.h"

#include <QString>
#include <QStringList>
#include <QMap>
#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsinterval.py.
 * See details in QEP #17
 ****************************************************************************/

QgsInterval::QgsInterval( double seconds )
  : mSeconds( seconds )
  , mValid( true )
  , mOriginalDuration( seconds )
  , mOriginalUnit( QgsUnitTypes::TemporalSeconds )
{
}

QgsInterval::QgsInterval( double duration, QgsUnitTypes::TemporalUnit unit )
  : mSeconds( duration * QgsUnitTypes::fromUnitToUnitFactor( unit, QgsUnitTypes::TemporalSeconds ) )
  , mValid( true )
  , mOriginalDuration( duration )
  , mOriginalUnit( unit )
{
}

QgsInterval::QgsInterval( double years, double months, double weeks, double days, double hours, double minutes, double seconds )
  : mSeconds( years * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::TemporalYears, QgsUnitTypes::TemporalSeconds )
              + months * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::TemporalMonths, QgsUnitTypes::TemporalSeconds )
              + weeks * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::TemporalWeeks, QgsUnitTypes::TemporalSeconds )
              + days * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::TemporalDays, QgsUnitTypes::TemporalSeconds )
              + hours * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::TemporalHours, QgsUnitTypes::TemporalSeconds )
              + minutes * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::TemporalMinutes, QgsUnitTypes::TemporalSeconds )
              + seconds )
  , mValid( true )
{
  if ( years && !months && !weeks && !days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = years;
    mOriginalUnit = QgsUnitTypes::TemporalYears;
  }
  else if ( !years && months && !weeks && !days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = months;
    mOriginalUnit = QgsUnitTypes::TemporalMonths;
  }
  else if ( !years && !months && weeks && !days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = weeks;
    mOriginalUnit = QgsUnitTypes::TemporalWeeks;
  }
  else if ( !years && !months && !weeks && days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = days;
    mOriginalUnit = QgsUnitTypes::TemporalDays;
  }
  else if ( !years && !months && !weeks && !days && hours && !minutes && !seconds )
  {
    mOriginalDuration = hours;
    mOriginalUnit = QgsUnitTypes::TemporalHours;
  }
  else if ( !years && !months && !weeks && !days && !hours && minutes && !seconds )
  {
    mOriginalDuration = minutes;
    mOriginalUnit = QgsUnitTypes::TemporalMinutes;
  }
  else if ( !years && !months && !weeks && !days && !hours && !minutes && seconds )
  {
    mOriginalDuration = seconds;
    mOriginalUnit = QgsUnitTypes::TemporalSeconds;
  }
  else if ( !years && !months && !weeks && !days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = 0;
    mOriginalUnit = QgsUnitTypes::TemporalSeconds;
  }
  else
  {
    mOriginalUnit = QgsUnitTypes::TemporalUnknownUnit;
  }
}

double QgsInterval::years() const
{
  if ( mOriginalUnit == QgsUnitTypes::TemporalYears )
    return mOriginalDuration;

  return mSeconds / YEARS;
}

void QgsInterval::setYears( double years )
{
  mSeconds = years * YEARS;
  mValid = true;
  mOriginalDuration = years;
  mOriginalUnit = QgsUnitTypes::TemporalYears;
}

double QgsInterval::months() const
{
  if ( mOriginalUnit == QgsUnitTypes::TemporalMonths )
    return mOriginalDuration;

  return mSeconds / MONTHS;
}

void QgsInterval::setMonths( double months )
{
  mSeconds = months * MONTHS;
  mValid = true;
  mOriginalDuration = months;
  mOriginalUnit = QgsUnitTypes::TemporalMonths;
}

double QgsInterval::weeks() const
{
  if ( mOriginalUnit == QgsUnitTypes::TemporalWeeks )
    return mOriginalDuration;

  return mSeconds / WEEKS;
}


void QgsInterval::setWeeks( double weeks )
{
  mSeconds = weeks * WEEKS;
  mValid = true;
  mOriginalDuration = weeks;
  mOriginalUnit = QgsUnitTypes::TemporalWeeks;
}

double QgsInterval::days() const
{
  if ( mOriginalUnit == QgsUnitTypes::TemporalDays )
    return mOriginalDuration;

  return mSeconds / DAY;
}


void QgsInterval::setDays( double days )
{
  mSeconds = days * DAY;
  mValid = true;
  mOriginalDuration = days;
  mOriginalUnit = QgsUnitTypes::TemporalDays;
}

double QgsInterval::hours() const
{
  if ( mOriginalUnit == QgsUnitTypes::TemporalHours )
    return mOriginalDuration;

  return mSeconds / HOUR;
}


void QgsInterval::setHours( double hours )
{
  mSeconds = hours * HOUR;
  mValid = true;
  mOriginalDuration = hours;
  mOriginalUnit = QgsUnitTypes::TemporalHours;
}

double QgsInterval::minutes() const
{
  if ( mOriginalUnit == QgsUnitTypes::TemporalMinutes )
    return mOriginalDuration;

  return mSeconds / MINUTE;
}

void QgsInterval::setMinutes( double minutes )
{
  mSeconds = minutes * MINUTE;
  mValid = true;
  mOriginalDuration = minutes;
  mOriginalUnit = QgsUnitTypes::TemporalMinutes;
}

void QgsInterval::setSeconds( double seconds )
{
  mSeconds = seconds;
  mValid = true;
  mOriginalDuration = seconds;
  mOriginalUnit = QgsUnitTypes::TemporalSeconds;
}

QgsInterval QgsInterval::fromString( const QString &string )
{
  double seconds = 0;
  const thread_local QRegularExpression rx( "([-+]?\\d*\\.?\\d+\\s+\\S+)", QRegularExpression::CaseInsensitiveOption );
  const thread_local QRegularExpression rxtime( ".* \\d{1,2}(:)\\d{1,2}(:)\\d{1,2}.*", QRegularExpression::CaseInsensitiveOption );

  const QRegularExpressionMatch matchtime = rxtime.match( string );
  QString modedString = QString( string );
  if ( matchtime.hasMatch() ) //some part of the string contains 00:00:00 style duration
  {
    // Get the second occurrence of : (minutes)
    modedString.replace( matchtime.capturedStart( 2 ), 1, " minutes " );
    // Get the first occurrence of : (hours)
    modedString.replace( matchtime.capturedStart( 1 ), 1, " hours " );
    modedString.append( " seconds" );
  }

  QStringList list;
  int pos = 0;
  QRegularExpressionMatch match = rx.match( modedString );
  while ( match.hasMatch() )
  {
    list << match.captured( 1 );
    pos = match.capturedStart() + match.capturedLength();
    match = rx.match( modedString, pos );
  }

  const thread_local QMap<int, QStringList> map{{
      {1, QStringList() << QStringLiteral( "second" ) << QStringLiteral( "seconds" ) << QObject::tr( "second|seconds", "list of words separated by | which reference years" ).split( '|' )},
      { 0 + MINUTE, QStringList() << QStringLiteral( "minute" ) << QStringLiteral( "minutes" ) << QObject::tr( "minute|minutes", "list of words separated by | which reference minutes" ).split( '|' ) },
      {0 + HOUR, QStringList() << QStringLiteral( "hour" ) << QStringLiteral( "hours" ) << QObject::tr( "hour|hours", "list of words separated by | which reference minutes hours" ).split( '|' )},
      {0 + DAY, QStringList() << QStringLiteral( "day" ) << QStringLiteral( "days" ) << QObject::tr( "day|days", "list of words separated by | which reference days" ).split( '|' )},
      {0 + WEEKS, QStringList() << QStringLiteral( "week" ) << QStringLiteral( "weeks" ) << QObject::tr( "week|weeks", "wordlist separated by | which reference weeks" ).split( '|' )},
      {0 + MONTHS, QStringList() << QStringLiteral( "month" ) << QStringLiteral( "months" ) << QStringLiteral( "mon" ) << QObject::tr( "month|months|mon", "list of words separated by | which reference months" ).split( '|' )},
      {0 + YEARS, QStringList() << QStringLiteral( "year" ) << QStringLiteral( "years" ) << QObject::tr( "year|years", "list of words separated by | which reference years" ).split( '|' )},
    }};

  const thread_local QRegularExpression splitRx( "\\s+" );

  for ( const QString &match : std::as_const( list ) )
  {
    const QStringList split = match.split( splitRx );
    bool ok;
    const double value = split.at( 0 ).toDouble( &ok );
    if ( !ok )
    {
      continue;
    }

    bool matched = false;
    QMap<int, QStringList>::const_iterator it = map.constBegin();
    for ( ; it != map.constEnd(); ++it )
    {
      const int duration = it.key();
      const QStringList durationIdentifiers = it.value();
      for ( const QString &identifier : durationIdentifiers )
      {
        if ( match.contains( identifier, Qt::CaseInsensitive ) )
        {
          matched = true;
          break;
        }
      }

      if ( matched )
      {
        seconds += value * duration;
        break;
      }
    }
  }

  // If we can't parse the string at all then we just return invalid
  if ( seconds == 0 )
    return QgsInterval();

  return QgsInterval( seconds );
}

QDebug operator<<( QDebug dbg, const QgsInterval &interval )
{
  if ( !interval.isValid() )
    dbg.nospace() << "QgsInterval()";
  else
    dbg.nospace() << "QgsInterval(" << interval.seconds() << ")";
  return dbg.maybeSpace();
}

QgsInterval operator-( const QDateTime &dt1, const QDateTime &dt2 )
{
  const qint64 mSeconds = dt2.msecsTo( dt1 );
  return QgsInterval( mSeconds / 1000.0 );
}

QDateTime operator+( const QDateTime &start, const QgsInterval &interval )
{
  return start.addMSecs( static_cast<qint64>( interval.seconds() * 1000.0 ) );
}

QgsInterval operator-( QDate date1, QDate date2 )
{
  const qint64 seconds = static_cast< qint64 >( date2.daysTo( date1 ) ) * 24 * 60 * 60;
  return QgsInterval( seconds );
}

QgsInterval operator-( QTime time1, QTime time2 )
{
  const qint64 mSeconds = time2.msecsTo( time1 );
  return QgsInterval( mSeconds / 1000.0 );
}
