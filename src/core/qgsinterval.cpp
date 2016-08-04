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
#include "qgis.h"
#include <QString>
#include <QStringList>
#include <QMap>
#include <QObject>
#include <QDebug>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsinterval.py.
 * See details in QEP #17
 ****************************************************************************/

QgsInterval::QgsInterval()
    : mSeconds( 0.0 )
    , mValid( false )
{

}

QgsInterval::QgsInterval( double seconds )
    : mSeconds( seconds )
    , mValid( true )
{ }

bool QgsInterval::operator==( const QgsInterval& other ) const
{
  if ( !mValid && !other.mValid )
    return true;
  else if ( mValid && other.mValid )
    return qgsDoubleNear( mSeconds, other.mSeconds );
  else
    return false;
}

QgsInterval QgsInterval::fromString( const QString& string )
{
  int seconds = 0;
  QRegExp rx( "([-+]?\\d?\\.?\\d+\\s+\\S+)", Qt::CaseInsensitive );
  QStringList list;
  int pos = 0;

  while (( pos = rx.indexIn( string, pos ) ) != -1 )
  {
    list << rx.cap( 1 );
    pos += rx.matchedLength();
  }

  QMap<int, QStringList> map;
  map.insert( 1, QStringList() << "second" << "seconds" << QObject::tr( "second|seconds", "list of words separated by | which reference years" ).split( '|' ) );
  map.insert( 0 + MINUTE, QStringList() << "minute" << "minutes" << QObject::tr( "minute|minutes", "list of words separated by | which reference minutes" ).split( '|' ) );
  map.insert( 0 + HOUR, QStringList() << "hour" << "hours" << QObject::tr( "hour|hours", "list of words separated by | which reference minutes hours" ).split( '|' ) );
  map.insert( 0 + DAY, QStringList() << "day" << "days" << QObject::tr( "day|days", "list of words separated by | which reference days" ).split( '|' ) );
  map.insert( 0 + WEEKS, QStringList() << "week" << "weeks" << QObject::tr( "week|weeks", "wordlist separated by | which reference weeks" ).split( '|' ) );
  map.insert( 0 + MONTHS, QStringList() << "month" << "months" << QObject::tr( "month|months", "list of words separated by | which reference months" ).split( '|' ) );
  map.insert( 0 + YEARS, QStringList() << "year" << "years" << QObject::tr( "year|years", "list of words separated by | which reference years" ).split( '|' ) );

  Q_FOREACH ( const QString& match, list )
  {
    QStringList split = match.split( QRegExp( "\\s+" ) );
    bool ok;
    double value = split.at( 0 ).toDouble( &ok );
    if ( !ok )
    {
      continue;
    }

    bool matched = false;
    QMap<int, QStringList>::const_iterator it = map.constBegin();
    for ( ; it != map.constEnd(); ++it )
    {
      int duration = it.key();
      Q_FOREACH ( const QString& name, it.value() )
      {
        if ( match.contains( name, Qt::CaseInsensitive ) )
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

QDebug operator<<( QDebug dbg, const QgsInterval& interval )
{
  if ( !interval.isValid() )
    dbg.nospace() << "QgsInterval()";
  else
    dbg.nospace() << "QgsInterval(" << interval.seconds() << ")";
  return dbg.maybeSpace();
}

QgsInterval operator-( const QDateTime& dt1, const QDateTime& dt2 )
{
  qint64 mSeconds = dt2.msecsTo( dt1 );
  return QgsInterval( mSeconds / 1000.0 );
}

QDateTime operator+( const QDateTime& start, const QgsInterval& interval )
{
  return start.addMSecs( static_cast<qint64>( interval.seconds() * 1000.0 ) );
}

QgsInterval operator-( const QDate& date1, const QDate& date2 )
{
  qint64 seconds = static_cast< qint64 >( date2.daysTo( date1 ) ) * 24 * 60 * 60;
  return QgsInterval( seconds );
}

QgsInterval operator-( const QTime& time1, const QTime& time2 )
{
  qint64 mSeconds = time2.msecsTo( time1 );
  return QgsInterval( mSeconds / 1000.0 );
}
