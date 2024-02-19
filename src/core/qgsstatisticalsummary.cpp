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

QgsStatisticalSummary::QgsStatisticalSummary( Qgis::Statistics stats )
  : mStatistics( stats )
{
  reset();
}

void QgsStatisticalSummary::setStatistics( Qgis::Statistics stats )
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

  mRequiresHisto = mStatistics & Qgis::Statistic::Majority || mStatistics & Qgis::Statistic::Minority || mStatistics & Qgis::Statistic::Variety;

  mRequiresAllValueStorage = mStatistics & Qgis::Statistic::StDev || mStatistics & Qgis::Statistic::StDevSample ||
                             mStatistics & Qgis::Statistic::Median || mStatistics & Qgis::Statistic::FirstQuartile ||
                             mStatistics & Qgis::Statistic::ThirdQuartile || mStatistics & Qgis::Statistic::InterQuartileRange;
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

  if ( mStatistics & Qgis::Statistic::StDev || mStatistics & Qgis::Statistic::StDevSample )
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

  if ( mStatistics & Qgis::Statistic::Median
       || mStatistics & Qgis::Statistic::FirstQuartile
       || mStatistics & Qgis::Statistic::ThirdQuartile
       || mStatistics & Qgis::Statistic::InterQuartileRange )
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

  if ( mStatistics & Qgis::Statistic::FirstQuartile
       || mStatistics & Qgis::Statistic::InterQuartileRange )
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

  if ( mStatistics & Qgis::Statistic::ThirdQuartile
       || mStatistics & Qgis::Statistic::InterQuartileRange )
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

  if ( mStatistics & Qgis::Statistic::Minority || mStatistics & Qgis::Statistic::Majority )
  {
    QList<int> valueCounts = mValueCount.values();

    if ( mStatistics & Qgis::Statistic::Minority )
    {
      mMinority = mValueCount.key( *std::min_element( valueCounts.begin(), valueCounts.end() ) );
    }
    if ( mStatistics & Qgis::Statistic::Majority )
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

double QgsStatisticalSummary::statistic( Qgis::Statistic stat ) const
{
  switch ( stat )
  {
    case Qgis::Statistic::Count:
      return mCount;
    case Qgis::Statistic::CountMissing:
      return mMissing;
    case Qgis::Statistic::Sum:
      return mSum;
    case Qgis::Statistic::Mean:
      return mMean;
    case Qgis::Statistic::Median:
      return mMedian;
    case Qgis::Statistic::StDev:
      return mStdev;
    case Qgis::Statistic::StDevSample:
      return mSampleStdev;
    case Qgis::Statistic::Min:
      return mMin;
    case Qgis::Statistic::Max:
      return mMax;
    case Qgis::Statistic::Range:
      return mMax - mMin;
    case Qgis::Statistic::Minority:
      return mMinority;
    case Qgis::Statistic::Majority:
      return mMajority;
    case Qgis::Statistic::Variety:
      return mValueCount.count();
    case Qgis::Statistic::FirstQuartile:
      return mFirstQuartile;
    case Qgis::Statistic::ThirdQuartile:
      return mThirdQuartile;
    case Qgis::Statistic::InterQuartileRange:
      return mThirdQuartile - mFirstQuartile;
    case Qgis::Statistic::First:
      return mFirst;
    case Qgis::Statistic::Last:
      return mLast;
    case Qgis::Statistic::All:
      return 0;
  }
  return 0;
}

QString QgsStatisticalSummary::displayName( Qgis::Statistic statistic )
{
  switch ( statistic )
  {
    case Qgis::Statistic::Count:
      return QObject::tr( "Count" );
    case Qgis::Statistic::CountMissing:
      return QObject::tr( "Count (missing)" );
    case Qgis::Statistic::Sum:
      return QObject::tr( "Sum" );
    case Qgis::Statistic::Mean:
      return QObject::tr( "Mean" );
    case Qgis::Statistic::Median:
      return QObject::tr( "Median" );
    case Qgis::Statistic::StDev:
      return QObject::tr( "St dev (pop)" );
    case Qgis::Statistic::StDevSample:
      return QObject::tr( "St dev (sample)" );
    case Qgis::Statistic::Min:
      return QObject::tr( "Minimum" );
    case Qgis::Statistic::Max:
      return QObject::tr( "Maximum" );
    case Qgis::Statistic::Range:
      return QObject::tr( "Range" );
    case Qgis::Statistic::Minority:
      return QObject::tr( "Minority" );
    case Qgis::Statistic::Majority:
      return QObject::tr( "Majority" );
    case Qgis::Statistic::Variety:
      return QObject::tr( "Variety" );
    case Qgis::Statistic::FirstQuartile:
      return QObject::tr( "Q1" );
    case Qgis::Statistic::ThirdQuartile:
      return QObject::tr( "Q3" );
    case Qgis::Statistic::InterQuartileRange:
      return QObject::tr( "IQR" );
    case Qgis::Statistic::First:
      return QObject::tr( "First" );
    case Qgis::Statistic::Last:
      return QObject::tr( "Last" );
    case Qgis::Statistic::All:
      return QString();
  }
  return QString();
}

QString QgsStatisticalSummary::shortName( Qgis::Statistic statistic )
{
  switch ( statistic )
  {
    case Qgis::Statistic::Count:
      return QStringLiteral( "count" );
    case Qgis::Statistic::CountMissing:
      return QStringLiteral( "countmissing" );
    case Qgis::Statistic::Sum:
      return QStringLiteral( "sum" );
    case Qgis::Statistic::Mean:
      return QStringLiteral( "mean" );
    case Qgis::Statistic::Median:
      return QStringLiteral( "median" );
    case Qgis::Statistic::StDev:
      return QStringLiteral( "stdev" );
    case Qgis::Statistic::StDevSample:
      return QStringLiteral( "stdevsample" );
    case Qgis::Statistic::Min:
      return QStringLiteral( "min" );
    case Qgis::Statistic::Max:
      return QStringLiteral( "max" );
    case Qgis::Statistic::Range:
      return QStringLiteral( "range" );
    case Qgis::Statistic::Minority:
      return QStringLiteral( "minority" );
    case Qgis::Statistic::Majority:
      return QStringLiteral( "majority" );
    case Qgis::Statistic::Variety:
      return QStringLiteral( "variety" );
    case Qgis::Statistic::FirstQuartile:
      return QStringLiteral( "q1" );
    case Qgis::Statistic::ThirdQuartile:
      return QStringLiteral( "q3" );
    case Qgis::Statistic::InterQuartileRange:
      return QStringLiteral( "iqr" );
    case Qgis::Statistic::First:
      return QStringLiteral( "first" );
    case Qgis::Statistic::Last:
      return QStringLiteral( "last" );
    case Qgis::Statistic::All:
      return QString();
  }
  return QString();
}

