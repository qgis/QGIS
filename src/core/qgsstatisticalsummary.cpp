/***************************************************************************
  qgsstatisticalsummary.cpp
  --------------------------------------
  Date                 : May 2015
  Copyright            : (C) 2015 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstatisticalsummary.h"
#include "qgsvariantutils.h"
#include <limits>
#include <QString>
#include <QObject>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsStatisticalSummary::QgsStatisticalSummary( Statistics stats )
  : mStatistics( stats )
{
  reset();
}

void QgsStatisticalSummary::setStatistics( QgsStatisticalSummary::Statistics stats )
{
  mStatistics = stats;
  reset();
}

void QgsStatisticalSummary::reset()
{
  mFirst = std::numeric_limits<double>::quiet_NaN();
  mLast = std::numeric_limits<double>::quiet_NaN();
  mCount = 0;
  mMissing = 0;
  mSum = 0;
  mMean = 0;
  mMedian = 0;
  mMin = std::numeric_limits<double>::max();
  mMax = -std::numeric_limits<double>::max();
  mStdev = 0;
  mSampleStdev = 0;
  mMinority = 0;
  mMajority = 0;
  mFirstQuartile = 0;
  mThirdQuartile = 0;
  mValueCount.clear();
  mValues.clear();

  mRequiresHisto = mStatistics & QgsStatisticalSummary::Statistic::Majority || mStatistics & QgsStatisticalSummary::Statistic::Minority || mStatistics & QgsStatisticalSummary::Statistic::Variety;

  mRequiresAllValueStorage = mStatistics & QgsStatisticalSummary::Statistic::StDev || mStatistics & QgsStatisticalSummary::Statistic::StDevSample ||
                             mStatistics & QgsStatisticalSummary::Statistic::Median || mStatistics & QgsStatisticalSummary::Statistic::FirstQuartile ||
                             mStatistics & QgsStatisticalSummary::Statistic::ThirdQuartile || mStatistics & QgsStatisticalSummary::Statistic::InterQuartileRange;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

void QgsStatisticalSummary::calculate( const QList<double> &values )
{
  reset();

  for ( const double value : values )
  {
    addValue( value );
  }

  finalize();
}

void QgsStatisticalSummary::addValue( double value )
{
  if ( mCount == 0 )
    mFirst = value;
  mCount++;
  mSum += value;
  mMin = std::min( mMin, value );
  mMax = std::max( mMax, value );
  mLast = value;

  if ( mRequiresHisto )
    mValueCount.insert( value, mValueCount.value( value, 0 ) + 1 );

  if ( mRequiresAllValueStorage )
    mValues << value;
}

void QgsStatisticalSummary::addVariant( const QVariant &value )
{
  bool convertOk = false;
  if ( QgsVariantUtils::isNull( value ) )
    mMissing++;
  else
  {
    const double val = value.toDouble( &convertOk );
    if ( convertOk )
      addValue( val );
    else
      mMissing++;
  }
}

void QgsStatisticalSummary::finalize()
{
  if ( mCount == 0 )
  {
    mFirst = std::numeric_limits<double>::quiet_NaN();
    mLast = std::numeric_limits<double>::quiet_NaN();
    mMin = std::numeric_limits<double>::quiet_NaN();
    mMax = std::numeric_limits<double>::quiet_NaN();
    mMean = std::numeric_limits<double>::quiet_NaN();
    mMedian = std::numeric_limits<double>::quiet_NaN();
    mStdev = std::numeric_limits<double>::quiet_NaN();
    mSampleStdev = std::numeric_limits<double>::quiet_NaN();
    mMinority = std::numeric_limits<double>::quiet_NaN();
    mMajority = std::numeric_limits<double>::quiet_NaN();
    mFirstQuartile = std::numeric_limits<double>::quiet_NaN();
    mThirdQuartile = std::numeric_limits<double>::quiet_NaN();
    return;
  }

  mMean = mSum / mCount;

  if ( mStatistics & QgsStatisticalSummary::Statistic::StDev || mStatistics & QgsStatisticalSummary::Statistic::StDevSample )
  {
    double sumSquared = 0;
    const auto constMValues = mValues;
    for ( const double value : constMValues )
    {
      const double diff = value - mMean;
      sumSquared += diff * diff;
    }
    mStdev = std::pow( sumSquared / mValues.count(), 0.5 );
    mSampleStdev = std::pow( sumSquared / ( mValues.count() - 1 ), 0.5 );
  }

  if ( mStatistics & QgsStatisticalSummary::Statistic::Median
       || mStatistics & QgsStatisticalSummary::Statistic::FirstQuartile
       || mStatistics & QgsStatisticalSummary::Statistic::ThirdQuartile
       || mStatistics & QgsStatisticalSummary::Statistic::InterQuartileRange )
  {
    std::sort( mValues.begin(), mValues.end() );
    const bool even = ( mCount % 2 ) < 1;
    if ( even )
    {
      mMedian = ( mValues[mCount / 2 - 1] + mValues[mCount / 2] ) / 2.0;
    }
    else //odd
    {
      mMedian = mValues[( mCount + 1 ) / 2 - 1];
    }
  }

  if ( mStatistics & QgsStatisticalSummary::Statistic::FirstQuartile
       || mStatistics & QgsStatisticalSummary::Statistic::InterQuartileRange )
  {
    if ( ( mCount % 2 ) < 1 )
    {
      const int halfCount = mCount / 2;
      const bool even = ( halfCount % 2 ) < 1;
      if ( even )
      {
        mFirstQuartile = ( mValues[halfCount / 2 - 1] + mValues[halfCount / 2] ) / 2.0;
      }
      else //odd
      {
        mFirstQuartile = mValues[( halfCount  + 1 ) / 2 - 1];
      }
    }
    else
    {
      const int halfCount = mCount / 2 + 1;
      const bool even = ( halfCount % 2 ) < 1;
      if ( even )
      {
        mFirstQuartile = ( mValues[halfCount / 2 - 1] + mValues[halfCount / 2] ) / 2.0;
      }
      else //odd
      {
        mFirstQuartile = mValues[( halfCount  + 1 ) / 2 - 1];
      }
    }
  }

  if ( mStatistics & QgsStatisticalSummary::Statistic::ThirdQuartile
       || mStatistics & QgsStatisticalSummary::Statistic::InterQuartileRange )
  {
    if ( ( mCount % 2 ) < 1 )
    {
      const int halfCount = mCount / 2;
      const bool even = ( halfCount % 2 ) < 1;
      if ( even )
      {
        mThirdQuartile = ( mValues[ halfCount + halfCount / 2 - 1] + mValues[ halfCount + halfCount / 2] ) / 2.0;
      }
      else //odd
      {
        mThirdQuartile = mValues[( halfCount + 1 ) / 2 - 1 + halfCount ];
      }
    }
    else
    {
      const int halfCount = mCount / 2 + 1;
      const bool even = ( halfCount % 2 ) < 1;
      if ( even )
      {
        mThirdQuartile = ( mValues[ halfCount + halfCount / 2 - 2 ] + mValues[ halfCount + halfCount / 2 - 1 ] ) / 2.0;
      }
      else //odd
      {
        mThirdQuartile = mValues[( halfCount + 1 ) / 2 - 2 + halfCount ];
      }
    }
  }

  if ( mStatistics & QgsStatisticalSummary::Statistic::Minority || mStatistics & QgsStatisticalSummary::Statistic::Majority )
  {
    QList<int> valueCounts = mValueCount.values();

    if ( mStatistics & QgsStatisticalSummary::Statistic::Minority )
    {
      mMinority = mValueCount.key( *std::min_element( valueCounts.begin(), valueCounts.end() ) );
    }
    if ( mStatistics & QgsStatisticalSummary::Statistic::Majority )
    {
      mMajority = mValueCount.key( *std::max_element( valueCounts.begin(), valueCounts.end() ) );
    }
  }

}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

double QgsStatisticalSummary::statistic( QgsStatisticalSummary::Statistic stat ) const
{
  switch ( stat )
  {
    case Statistic::Count:
      return mCount;
    case Statistic::CountMissing:
      return mMissing;
    case Statistic::Sum:
      return mSum;
    case Statistic::Mean:
      return mMean;
    case Statistic::Median:
      return mMedian;
    case Statistic::StDev:
      return mStdev;
    case Statistic::StDevSample:
      return mSampleStdev;
    case Statistic::Min:
      return mMin;
    case Statistic::Max:
      return mMax;
    case Statistic::Range:
      return mMax - mMin;
    case Statistic::Minority:
      return mMinority;
    case Statistic::Majority:
      return mMajority;
    case Statistic::Variety:
      return mValueCount.count();
    case Statistic::FirstQuartile:
      return mFirstQuartile;
    case Statistic::ThirdQuartile:
      return mThirdQuartile;
    case Statistic::InterQuartileRange:
      return mThirdQuartile - mFirstQuartile;
    case Statistic::First:
      return mFirst;
    case Statistic::Last:
      return mLast;
    case Statistic::All:
      return 0;
  }
  return 0;
}

QString QgsStatisticalSummary::displayName( QgsStatisticalSummary::Statistic statistic )
{
  switch ( statistic )
  {
    case Statistic::Count:
      return QObject::tr( "Count" );
    case Statistic::CountMissing:
      return QObject::tr( "Count (missing)" );
    case Statistic::Sum:
      return QObject::tr( "Sum" );
    case Statistic::Mean:
      return QObject::tr( "Mean" );
    case Statistic::Median:
      return QObject::tr( "Median" );
    case Statistic::StDev:
      return QObject::tr( "St dev (pop)" );
    case Statistic::StDevSample:
      return QObject::tr( "St dev (sample)" );
    case Statistic::Min:
      return QObject::tr( "Minimum" );
    case Statistic::Max:
      return QObject::tr( "Maximum" );
    case Statistic::Range:
      return QObject::tr( "Range" );
    case Statistic::Minority:
      return QObject::tr( "Minority" );
    case Statistic::Majority:
      return QObject::tr( "Majority" );
    case Statistic::Variety:
      return QObject::tr( "Variety" );
    case Statistic::FirstQuartile:
      return QObject::tr( "Q1" );
    case Statistic::ThirdQuartile:
      return QObject::tr( "Q3" );
    case Statistic::InterQuartileRange:
      return QObject::tr( "IQR" );
    case Statistic::First:
      return QObject::tr( "First" );
    case Statistic::Last:
      return QObject::tr( "Last" );
    case Statistic::All:
      return QString();
  }
  return QString();
}

QString QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic statistic )
{
  switch ( statistic )
  {
    case Statistic::Count:
      return QStringLiteral( "count" );
    case Statistic::CountMissing:
      return QStringLiteral( "countmissing" );
    case Statistic::Sum:
      return QStringLiteral( "sum" );
    case Statistic::Mean:
      return QStringLiteral( "mean" );
    case Statistic::Median:
      return QStringLiteral( "median" );
    case Statistic::StDev:
      return QStringLiteral( "stdev" );
    case Statistic::StDevSample:
      return QStringLiteral( "stdevsample" );
    case Statistic::Min:
      return QStringLiteral( "min" );
    case Statistic::Max:
      return QStringLiteral( "max" );
    case Statistic::Range:
      return QStringLiteral( "range" );
    case Statistic::Minority:
      return QStringLiteral( "minority" );
    case Statistic::Majority:
      return QStringLiteral( "majority" );
    case Statistic::Variety:
      return QStringLiteral( "variety" );
    case Statistic::FirstQuartile:
      return QStringLiteral( "q1" );
    case Statistic::ThirdQuartile:
      return QStringLiteral( "q3" );
    case Statistic::InterQuartileRange:
      return QStringLiteral( "iqr" );
    case Statistic::First:
      return QStringLiteral( "first" );
    case Statistic::Last:
      return QStringLiteral( "last" );
    case Statistic::All:
      return QString();
  }
  return QString();
}

