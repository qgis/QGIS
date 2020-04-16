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
#include "qgis.h"
#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QObject>
#include <QVariant>
#include <QDebug>
#include <QVariantList>
#include <limits>
#include <cmath>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsdatetimestatisticalsummary.py.
 * See details in QEP #17
 ****************************************************************************/

QgsDateTimeStatisticalSummary::QgsDateTimeStatisticalSummary( QgsDateTimeStatisticalSummary::Statistics stats )
  : mStatistics( stats )
{
  reset();
}

void QgsDateTimeStatisticalSummary::reset()
{
  mCount = 0;
  mValues.clear();
  mAllValues.clear();
  mCountMissing = 0;
  mSumMSec = 0;
  mMean = QDateTime();
  mMedian = QDateTime();
  mStDev = QgsInterval();
  mSampleStDev = QgsInterval();
  mMin = QDateTime();
  mMax = QDateTime();
  mMinority = QDateTime();
  mMajority = QDateTime();
  mFirstQuartile = QDateTime();
  mThirdQuartile = QDateTime();
  mFirst = QDateTime();
  mLast = QDateTime();
  mMode.clear();
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
  if ( value.type() == QVariant::DateTime )
  {
    testDateTime( value.toDateTime() );
  }
  else if ( value.type() == QVariant::Date )
  {
    QDate date = value.toDate();
    testDateTime( date.isValid() ? QDateTime( date, QTime( 0, 0, 0 ) )
                  : QDateTime() );
  }
  else if ( value.type() == QVariant::Time )
  {
    mIsTimes = true;
    QTime time = value.toTime();
    testDateTime( time.isValid() ? QDateTime( QDate::fromJulianDay( 0 ), time )
                  : QDateTime() );
  }
  else //not a date
  {
    mCountMissing++;
  }
}

void QgsDateTimeStatisticalSummary::finalize()
{
  if ( mCount == 0 )
    return;

  if ( mStatistics & Mean || mStatistics & StDev || mStatistics & StDevSample )
  {

    if ( mIsTimes )
    {
      const QTime meanTime = QTime( 0, 0 ).addMSecs(  mSumMSec / mCount ).addMSecs( mFirst.time().msecsSinceStartOfDay() );
      mMean = QDateTime( QDate::fromJulianDay( 0 ), meanTime );
    }
    else
    {
      mMean = mFirst.addMSecs( mSumMSec / mCount );
    }

    if ( mStatistics & StDev || mStatistics & StDevSample )
    {
      long long sumSecsSquared = 0;

      for ( const QDateTime dt : mValues.keys() )
      {
        long long diff = mMean.secsTo( dt );
        // square and multiply by the times of occurrences
        sumSecsSquared += diff * diff * mValues.value( dt );
      }

      if ( mStatistics & StDev )
        mStDev = QgsInterval( std::pow( sumSecsSquared / mCount, 0.5 ) );
      
      if ( mStatistics & StDevSample )
        mSampleStDev = QgsInterval( std::pow( sumSecsSquared / ( mCount - 1 ), 0.5 ) );
    }
  }

  if ( mStatistics & Median || mStatistics & FirstQuartile || mStatistics & ThirdQuartile || mStatistics & InterQuartileRange )
  {
    std::sort( mAllValues.begin(), mAllValues.end() );
  }

  if ( mStatistics & Median )
  {
    bool isEven = ( mCount % 2 ) < 1;

    if ( isEven )
    {
      QDateTime leftDt = mAllValues[mCount / 2 - 1];
      QDateTime rightDt = mAllValues[mCount / 2];

      mMedian = leftDt.addMSecs( leftDt.msecsTo( rightDt ) / 2.0 );
    }
    else //odd
    {
      mMedian = mAllValues[( mCount + 1 ) / 2 - 1];
    }
  }


  if ( mStatistics & FirstQuartile || mStatistics & InterQuartileRange )
  {
    if ( ( mCount % 2 ) < 1 )
    {
      int halfCount = mCount / 2;
      bool isEven = ( halfCount % 2 ) < 1;
      if ( isEven )
      {
        QDateTime leftDt = mAllValues[halfCount / 2 - 1];
        QDateTime rightDt = mAllValues[halfCount / 2];

        mFirstQuartile = leftDt.addMSecs( leftDt.msecsTo( rightDt ) / 2.0 );
      }
      else //odd
      {
        mFirstQuartile = mAllValues[( halfCount + 1 ) / 2 - 1];
      }
    }
    else
    {
      int halfCount = mCount / 2 + 1;
      bool isEven = ( halfCount % 2 ) < 1;
      if ( isEven )
      {
        QDateTime leftDt = mAllValues[halfCount / 2 - 1];
        QDateTime rightDt = mAllValues[halfCount / 2];

        mFirstQuartile = leftDt.addMSecs( leftDt.msecsTo( rightDt ) / 2.0 );
      }
      else //odd
      {
        mFirstQuartile = mAllValues[( halfCount + 1 ) / 2 - 1];
      }
    }
  }

  if ( mStatistics & ThirdQuartile || mStatistics & InterQuartileRange )
  {
    if ( ( mCount % 2 ) < 1 )
    {
      int halfCount = mCount / 2;
      bool isEven = ( halfCount % 2 ) < 1;
      if ( isEven )
      {
        QDateTime leftDt = mAllValues[halfCount + halfCount / 2 - 1];
        QDateTime rightDt = mAllValues[halfCount + halfCount / 2];

        mThirdQuartile = leftDt.addMSecs( leftDt.msecsTo( rightDt ) / 2.0 );
      }
      else //odd
      {
        mThirdQuartile = mAllValues[( halfCount + 1 ) / 2 - 1 + halfCount ];
      }
    }
    else
    {
      int halfCount = mCount / 2 + 1;
      bool isEven = ( halfCount % 2 ) < 1;
      if ( isEven )
      {
        QDateTime leftDt = mAllValues[halfCount + halfCount / 2 - 2];
        QDateTime rightDt = mAllValues[halfCount + halfCount / 2 - 1];

        mThirdQuartile = leftDt.addMSecs( leftDt.msecsTo( rightDt ) / 2.0 );
      }
      else //odd
      {
        mThirdQuartile = mAllValues[( halfCount + 1 ) / 2 - 2 + halfCount ];
      }
    }
  }

  if ( mStatistics & Minority || mStatistics & Majority )
  {
    QList<int> valueCounts = mValues.values();

    if ( mStatistics & Minority )
    {
      mMinority = mValues.key( *std::min_element( valueCounts.begin(), valueCounts.end() ) );
    }
    if ( mStatistics & Majority )
    {
      mMajority = mValues.key( *std::max_element( valueCounts.begin(), valueCounts.end() ) );
    }
  }

  if ( mStatistics & Mode )
  {
    int maxOccurrences = 0;

    for ( const QDateTime key : mValues.keys() )
    {
      int occurrences = mValues[key];

      if ( occurrences > maxOccurrences )
      {
        mMode.clear();
        maxOccurrences = occurrences;
      }
      else if ( occurrences == 0 || occurrences < maxOccurrences )
      {
        continue;
      }

      mMode.append( key );
    }
  }
}

void QgsDateTimeStatisticalSummary::testDateTime( const QDateTime &dateTime )
{
  if ( ! dateTime.isValid() || dateTime.isNull() )
  {
    mCountMissing++;
    return;
  }

  mCount++;

  if ( mStatistics & CountDistinct || mStatistics & StDev || mStatistics & StDevSample || mStatistics & Minority || mStatistics & Majority || mStatistics & Mode )
  {
    mValues[dateTime]++;
  }
  if ( mStatistics & Min || mStatistics & Range )
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
  if ( mStatistics & Max || mStatistics & Range )
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

  if ( mStatistics & First || mStatistics & Mean || mStatistics & StDev || mStatistics & StDevSample )
    if ( mFirst.isNull() )
      mFirst = dateTime;

  if ( mStatistics & Last )
    mLast = dateTime;

  if ( mStatistics & Mean || mStatistics & StDev || mStatistics & StDevSample )
    mSumMSec += mFirst.msecsTo( dateTime );

  if ( mStatistics & Median || mStatistics & FirstQuartile || mStatistics & ThirdQuartile || mStatistics & InterQuartileRange )
    mAllValues.append( dateTime );
}

QVariant QgsDateTimeStatisticalSummary::statistic( QgsDateTimeStatisticalSummary::Statistic stat ) const
{
  switch ( stat )
  {
    case Count:
      return mCount;
    case CountDistinct:
      return mValues.count();
    case CountMissing:
      return mCountMissing;
    case Mean:
      return mIsTimes ? QVariant( mMean.time() ) : QVariant( mMean );
    case Median:
      return mIsTimes ? QVariant( mMedian.time() ) : QVariant( mMedian );
    case StDev:
      return mStDev;
    case StDevSample:
      return mSampleStDev;
    case Min:
      return mIsTimes ? QVariant( mMin.time() ) : QVariant( mMin );
    case Max:
      return mIsTimes ? QVariant( mMax.time() ) : QVariant( mMax );
    case Range:
      return mIsTimes ? QVariant::fromValue( mMax.time() - mMin.time() ) : QVariant::fromValue( mMax - mMin );
    case Minority:
      return mIsTimes ? QVariant( mMinority.time() ) : QVariant( mMinority );
    case Majority:
      return mIsTimes ? QVariant( mMajority.time() ) : QVariant( mMajority );
    case FirstQuartile:
      return mIsTimes ? QVariant( mFirstQuartile.time() ) : QVariant( mFirstQuartile );
    case ThirdQuartile:
      return mIsTimes ? QVariant( mThirdQuartile.time() ) : QVariant( mThirdQuartile );
    case InterQuartileRange:
      return mIsTimes ? QVariant( mThirdQuartile.time() - mFirstQuartile.time() ) : QVariant( mThirdQuartile - mFirstQuartile );
    case First:
      return mIsTimes ? QVariant( mFirst.time() ) : QVariant( mFirst );
    case Last:
      return mIsTimes ? QVariant( mLast.time() ) : QVariant( mLast );
    case Mode:
    {
      if ( mIsTimes )
      {
        QList<QTime> modeTimeList;
        for ( const QDateTime &dt : mMode )
        {
          modeTimeList.append( dt.time() );
        }

        return qgis::toVariantList( modeTimeList );
      }
      else
      {
        return qgis::toVariantList( mMode );
      }

    }
    case All:
      return 0;
  }
  return 0;
}

QString QgsDateTimeStatisticalSummary::displayName( QgsDateTimeStatisticalSummary::Statistic statistic )
{
  switch ( statistic )
  {
    case Count:
      return QObject::tr( "Count" );
    case CountDistinct:
      return QObject::tr( "Count (distinct)" );
    case CountMissing:
      return QObject::tr( "Count (missing)" );
    case Mean:
      return QObject::tr( "Mean" );
    case Median:
      return QObject::tr( "Median" );
    case StDev:
      return QObject::tr( "St dev (pop)" );
    case StDevSample:
      return QObject::tr( "St dev (sample)" );
    case Min:
      return QObject::tr( "Minimum (earliest)" );
    case Max:
      return QObject::tr( "Maximum (latest)" );
    case Range:
      return QObject::tr( "Range (interval)" );
    case Minority:
      return QObject::tr( "Minority" );
    case Majority:
      return QObject::tr( "Majority" );
    case FirstQuartile:
      return QObject::tr( "Q1" );
    case ThirdQuartile:
      return QObject::tr( "Q3" );
    case InterQuartileRange:
      return QObject::tr( "IQR" );
    case First:
      return QObject::tr( "First" );
    case Last:
      return QObject::tr( "Last" );
    case Mode:
      return QObject::tr( "Mode" );
    case All:
      return QString();
  }
  return QString();
}

