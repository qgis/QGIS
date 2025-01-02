/***************************************************************************
  qgsdatetimestatisticalsummary.cpp
  ---------------------------------
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

#include "qgsdatetimestatisticalsummary.h"
#include "qgsvariantutils.h"
#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QObject>
#include <QVariant>
#include <QVariantList>
#include <limits>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsdatetimestatisticalsummary.py.
 * See details in QEP #17
 ****************************************************************************/

QgsDateTimeStatisticalSummary::QgsDateTimeStatisticalSummary( Qgis::DateTimeStatistics stats )
  : mStatistics( stats )
{
  reset();
}

void QgsDateTimeStatisticalSummary::reset()
{
  mCount = 0;
  mValues.clear();
  mCountMissing = 0;
  mMin = QDateTime();
  mMax = QDateTime();
  mIsTimes = false;
}

void QgsDateTimeStatisticalSummary::calculate( const QVariantList &values )
{
  reset();

  const auto constValues = values;
  for ( const QVariant &variant : constValues )
  {
    addValue( variant );
  }
  finalize();
}

void QgsDateTimeStatisticalSummary::addValue( const QVariant &value )
{

  if ( value.userType() == QMetaType::Type::QDateTime )
  {
    testDateTime( value.toDateTime(), QgsVariantUtils::isNull( value ) );
  }
  else if ( value.userType() == QMetaType::Type::QDate )
  {
    const QDate date = value.toDate();
    testDateTime( date.isValid() ? QDateTime( date, QTime( 0, 0, 0 ) )
                  : QDateTime(), QgsVariantUtils::isNull( value ) );
  }
  else if ( value.userType() == QMetaType::Type::QTime )
  {
    mIsTimes = true;
    const QTime time = value.toTime();
    testDateTime( time.isValid() ? QDateTime( QDate::fromJulianDay( 0 ), time )
                  : QDateTime(), QgsVariantUtils::isNull( value ) );
  }
  else //not a date
  {
    mCountMissing++;
    mCount++;
  }

  // QTime?
}

void QgsDateTimeStatisticalSummary::finalize()
{
  //nothing to do for now - this method has been added for forward compatibility
  //if statistics are implemented which require a post-calculation step
}

void QgsDateTimeStatisticalSummary::testDateTime( const QDateTime &dateTime, bool isNull )
{
  mCount++;

  if ( !dateTime.isValid() || isNull )
    mCountMissing++;

  if ( mStatistics & Qgis::DateTimeStatistic::CountDistinct )
  {
    mValues << dateTime;
  }
  if ( mStatistics & Qgis::DateTimeStatistic::Min || mStatistics & Qgis::DateTimeStatistic::Range )
  {
    if ( mMin.isValid() && dateTime.isValid() )
    {
      mMin = std::min( mMin, dateTime );
    }
    else if ( !mMin.isValid() && dateTime.isValid() )
    {
      mMin = dateTime;
    }
  }
  if ( mStatistics & Qgis::DateTimeStatistic::Max || mStatistics & Qgis::DateTimeStatistic::Range )
  {
    if ( mMax.isValid() && dateTime.isValid() )
    {
      mMax = std::max( mMax, dateTime );
    }
    else if ( !mMax.isValid() && dateTime.isValid() )
    {
      mMax = dateTime;
    }
  }
}

QVariant QgsDateTimeStatisticalSummary::statistic( Qgis::DateTimeStatistic stat ) const
{
  switch ( stat )
  {
    case Qgis::DateTimeStatistic::Count:
      return mCount;
    case Qgis::DateTimeStatistic::CountDistinct:
      return mValues.count();
    case Qgis::DateTimeStatistic::CountMissing:
      return mCountMissing;
    case Qgis::DateTimeStatistic::Min:
      return mIsTimes ? QVariant( mMin.time() ) : QVariant( mMin );
    case Qgis::DateTimeStatistic::Max:
      return mIsTimes ? QVariant( mMax.time() ) : QVariant( mMax );
    case Qgis::DateTimeStatistic::Range:
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
      return mIsTimes ? QVariant::fromValue( mMax.time() - mMin.time() ) : QVariant::fromValue( mMax - mMin );
#else
      return mIsTimes ? QVariant::fromValue( mMax.time() - mMin.time() ) : QVariant::fromValue( QgsInterval( static_cast< double >( ( mMax - mMin ).count() ) / 1000.0 ) );
#endif
    case Qgis::DateTimeStatistic::All:
      return 0;
  }
  return 0;
}

QgsInterval QgsDateTimeStatisticalSummary::range() const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
  return mMax - mMin;
#else
  return QgsInterval( static_cast< double >( ( mMax - mMin ).count() ) / 1000.0 );
#endif
}

QString QgsDateTimeStatisticalSummary::displayName( Qgis::DateTimeStatistic statistic )
{
  switch ( statistic )
  {
    case Qgis::DateTimeStatistic::Count:
      return QObject::tr( "Count" );
    case Qgis::DateTimeStatistic::CountDistinct:
      return QObject::tr( "Count (distinct)" );
    case Qgis::DateTimeStatistic::CountMissing:
      return QObject::tr( "Count (missing)" );
    case Qgis::DateTimeStatistic::Min:
      return QObject::tr( "Minimum (earliest)" );
    case Qgis::DateTimeStatistic::Max:
      return QObject::tr( "Maximum (latest)" );
    case Qgis::DateTimeStatistic::Range:
      return QObject::tr( "Range (interval)" );
    case Qgis::DateTimeStatistic::All:
      return QString();
  }
  return QString();
}

