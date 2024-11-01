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
#include "qgsunittypes.h"

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
  , mOriginalUnit( Qgis::TemporalUnit::Seconds )
{
}

QgsInterval::QgsInterval( std::chrono::milliseconds milliseconds )
  : mSeconds( static_cast<double>( milliseconds.count() ) / 1000.0 )
  , mValid( true )
  , mOriginalDuration( static_cast<double>( milliseconds.count() ) )
  , mOriginalUnit( Qgis::TemporalUnit::Milliseconds )
{
}

QgsInterval::QgsInterval( double duration, Qgis::TemporalUnit unit )
  : mSeconds( duration * QgsUnitTypes::fromUnitToUnitFactor( unit, Qgis::TemporalUnit::Seconds ) )
  , mValid( true )
  , mOriginalDuration( duration )
  , mOriginalUnit( unit )
{
}

QgsInterval::QgsInterval( double years, double months, double weeks, double days, double hours, double minutes, double seconds )
  : mSeconds( years * QgsUnitTypes::fromUnitToUnitFactor( Qgis::TemporalUnit::Years, Qgis::TemporalUnit::Seconds )
              + months * QgsUnitTypes::fromUnitToUnitFactor( Qgis::TemporalUnit::Months, Qgis::TemporalUnit::Seconds )
              + weeks * QgsUnitTypes::fromUnitToUnitFactor( Qgis::TemporalUnit::Weeks, Qgis::TemporalUnit::Seconds )
              + days * QgsUnitTypes::fromUnitToUnitFactor( Qgis::TemporalUnit::Days, Qgis::TemporalUnit::Seconds )
              + hours * QgsUnitTypes::fromUnitToUnitFactor( Qgis::TemporalUnit::Hours, Qgis::TemporalUnit::Seconds )
              + minutes * QgsUnitTypes::fromUnitToUnitFactor( Qgis::TemporalUnit::Minutes, Qgis::TemporalUnit::Seconds )
              + seconds )
  , mValid( true )
{
  if ( years && !months && !weeks && !days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = years;
    mOriginalUnit = Qgis::TemporalUnit::Years;
  }
  else if ( !years && months && !weeks && !days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = months;
    mOriginalUnit = Qgis::TemporalUnit::Months;
  }
  else if ( !years && !months && weeks && !days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = weeks;
    mOriginalUnit = Qgis::TemporalUnit::Weeks;
  }
  else if ( !years && !months && !weeks && days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = days;
    mOriginalUnit = Qgis::TemporalUnit::Days;
  }
  else if ( !years && !months && !weeks && !days && hours && !minutes && !seconds )
  {
    mOriginalDuration = hours;
    mOriginalUnit = Qgis::TemporalUnit::Hours;
  }
  else if ( !years && !months && !weeks && !days && !hours && minutes && !seconds )
  {
    mOriginalDuration = minutes;
    mOriginalUnit = Qgis::TemporalUnit::Minutes;
  }
  else if ( !years && !months && !weeks && !days && !hours && !minutes && seconds )
  {
    mOriginalDuration = seconds;
    mOriginalUnit = Qgis::TemporalUnit::Seconds;
  }
  else if ( !years && !months && !weeks && !days && !hours && !minutes && !seconds )
  {
    mOriginalDuration = 0;
    mOriginalUnit = Qgis::TemporalUnit::Seconds;
  }
  else
  {
    mOriginalUnit = Qgis::TemporalUnit::Unknown;
  }
}

double QgsInterval::years() const
{
  if ( mOriginalUnit == Qgis::TemporalUnit::Years )
    return mOriginalDuration;

  return mSeconds / YEARS;
}

void QgsInterval::setYears( double years )
{
  mSeconds = years * YEARS;
  mValid = true;
  mOriginalDuration = years;
  mOriginalUnit = Qgis::TemporalUnit::Years;
}

double QgsInterval::months() const
{
  if ( mOriginalUnit == Qgis::TemporalUnit::Months )
    return mOriginalDuration;

  return mSeconds / MONTHS;
}

void QgsInterval::setMonths( double months )
{
  mSeconds = months * MONTHS;
  mValid = true;
  mOriginalDuration = months;
  mOriginalUnit = Qgis::TemporalUnit::Months;
}

double QgsInterval::weeks() const
{
  if ( mOriginalUnit == Qgis::TemporalUnit::Weeks )
    return mOriginalDuration;

  return mSeconds / WEEKS;
}


void QgsInterval::setWeeks( double weeks )
{
  mSeconds = weeks * WEEKS;
  mValid = true;
  mOriginalDuration = weeks;
  mOriginalUnit = Qgis::TemporalUnit::Weeks;
}

double QgsInterval::days() const
{
  if ( mOriginalUnit == Qgis::TemporalUnit::Days )
    return mOriginalDuration;

  return mSeconds / DAY;
}


void QgsInterval::setDays( double days )
{
  mSeconds = days * DAY;
  mValid = true;
  mOriginalDuration = days;
  mOriginalUnit = Qgis::TemporalUnit::Days;
}

double QgsInterval::hours() const
{
  if ( mOriginalUnit == Qgis::TemporalUnit::Hours )
    return mOriginalDuration;

  return mSeconds / HOUR;
}


void QgsInterval::setHours( double hours )
{
  mSeconds = hours * HOUR;
  mValid = true;
  mOriginalDuration = hours;
  mOriginalUnit = Qgis::TemporalUnit::Hours;
}

double QgsInterval::minutes() const
{
  if ( mOriginalUnit == Qgis::TemporalUnit::Minutes )
    return mOriginalDuration;

  return mSeconds / MINUTE;
}

void QgsInterval::setMinutes( double minutes )
{
  mSeconds = minutes * MINUTE;
  mValid = true;
  mOriginalDuration = minutes;
  mOriginalUnit = Qgis::TemporalUnit::Minutes;
}

void QgsInterval::setSeconds( double seconds )
{
  mSeconds = seconds;
  mValid = true;
  mOriginalDuration = seconds;
  mOriginalUnit = Qgis::TemporalUnit::Seconds;
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

  const thread_local QMap<int, QStringList> map { {
    { 1, QStringList() << QStringLiteral( "second" ) << QStringLiteral( "seconds" ) << QObject::tr( "second|seconds", "list of words separated by | which reference years" ).split( '|' ) },
    { 0 + MINUTE, QStringList() << QStringLiteral( "minute" ) << QStringLiteral( "minutes" ) << QObject::tr( "minute|minutes", "list of words separated by | which reference minutes" ).split( '|' ) },
    { 0 + HOUR, QStringList() << QStringLiteral( "hour" ) << QStringLiteral( "hours" ) << QObject::tr( "hour|hours", "list of words separated by | which reference minutes hours" ).split( '|' ) },
    { 0 + DAY, QStringList() << QStringLiteral( "day" ) << QStringLiteral( "days" ) << QObject::tr( "day|days", "list of words separated by | which reference days" ).split( '|' ) },
    { 0 + WEEKS, QStringList() << QStringLiteral( "week" ) << QStringLiteral( "weeks" ) << QObject::tr( "week|weeks", "wordlist separated by | which reference weeks" ).split( '|' ) },
    { 0 + MONTHS, QStringList() << QStringLiteral( "month" ) << QStringLiteral( "months" ) << QStringLiteral( "mon" ) << QObject::tr( "month|months|mon", "list of words separated by | which reference months" ).split( '|' ) },
    { 0 + YEARS, QStringList() << QStringLiteral( "year" ) << QStringLiteral( "years" ) << QObject::tr( "year|years", "list of words separated by | which reference years" ).split( '|' ) },
  } };

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

#if QT_VERSION < QT_VERSION_CHECK( 6, 4, 0 )

QgsInterval operator-( const QDateTime &dt1, const QDateTime &dt2 )
{
  const qint64 mSeconds = dt2.msecsTo( dt1 );
  return QgsInterval( mSeconds / 1000.0 );
}

#endif

QDateTime operator+( const QDateTime &start, const QgsInterval &interval )
{
  return start.addMSecs( static_cast<qint64>( interval.seconds() * 1000.0 ) );
}

QgsInterval operator-( QDate date1, QDate date2 )
{
  const qint64 seconds = static_cast<qint64>( date2.daysTo( date1 ) ) * 24 * 60 * 60;
  return QgsInterval( seconds );
}

QgsInterval operator-( QTime time1, QTime time2 )
{
  const qint64 mSeconds = time2.msecsTo( time1 );
  return QgsInterval( mSeconds / 1000.0 );
}
