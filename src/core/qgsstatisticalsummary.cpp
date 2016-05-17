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
#include <limits>
#include <qmath.h>
#include <QString>
#include <QObject>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

QgsStatisticalSummary::QgsStatisticalSummary( const Statistics& stats )
    : mStatistics( stats )
{
  reset();
}

QgsStatisticalSummary::~QgsStatisticalSummary()
{

}

void QgsStatisticalSummary::reset()
{
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
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

void QgsStatisticalSummary::calculate( const QList<double> &values )
{
  reset();

  Q_FOREACH ( double value, values )
  {
    addValue( value );
  }

  finalize();
}

void QgsStatisticalSummary::addValue( double value )
{
  mCount++;
  mSum += value;
  mMin = qMin( mMin, value );
  mMax = qMax( mMax, value );

  if ( mStatistics & QgsStatisticalSummary::Majority || mStatistics & QgsStatisticalSummary::Minority || mStatistics & QgsStatisticalSummary::Variety )
    mValueCount.insert( value, mValueCount.value( value, 0 ) + 1 );

  if ( mStatistics & QgsStatisticalSummary::StDev || mStatistics & QgsStatisticalSummary::StDevSample ||
       mStatistics & QgsStatisticalSummary::Median || mStatistics & QgsStatisticalSummary::FirstQuartile ||
       mStatistics & QgsStatisticalSummary::ThirdQuartile || mStatistics & QgsStatisticalSummary::InterQuartileRange )
    mValues << value;
}

void QgsStatisticalSummary::addVariant( const QVariant& value )
{
  bool convertOk = false;
  if ( !value.isValid() || value.isNull() )
    mMissing++;
  else
  {
    double val = value.toDouble( &convertOk );
    if ( convertOk )
      addValue( val );
    else
      mMissing++;
  }
}

void QgsStatisticalSummary::finalize()
{
  if ( mCount == 0 )
    return;

  mMean = mSum / mCount;

  if ( mStatistics & QgsStatisticalSummary::StDev || mStatistics & QgsStatisticalSummary::StDevSample )
  {
    double sumSquared = 0;
    Q_FOREACH ( double value, mValues )
    {
      double diff = value - mMean;
      sumSquared += diff * diff;
    }
    mStdev = qPow( sumSquared / mValues.count(), 0.5 );
    mSampleStdev = qPow( sumSquared / ( mValues.count() - 1 ), 0.5 );
  }

  if ( mStatistics & QgsStatisticalSummary::Median
       || mStatistics & QgsStatisticalSummary::FirstQuartile
       || mStatistics & QgsStatisticalSummary::ThirdQuartile
       || mStatistics & QgsStatisticalSummary::InterQuartileRange )
  {
    qSort( mValues.begin(), mValues.end() );
    bool even = ( mCount % 2 ) < 1;
    if ( even )
    {
      mMedian = ( mValues[mCount / 2 - 1] + mValues[mCount / 2] ) / 2.0;
    }
    else //odd
    {
      mMedian = mValues[( mCount + 1 ) / 2 - 1];
    }
  }

  if ( mStatistics & QgsStatisticalSummary::FirstQuartile
       || mStatistics & QgsStatisticalSummary::InterQuartileRange )
  {
    if (( mCount % 2 ) < 1 )
    {
      int halfCount = mCount / 2;
      bool even = ( halfCount % 2 ) < 1;
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
      int halfCount = mCount / 2 + 1;
      bool even = ( halfCount % 2 ) < 1;
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

  if ( mStatistics & QgsStatisticalSummary::ThirdQuartile
       || mStatistics & QgsStatisticalSummary::InterQuartileRange )
  {
    if (( mCount % 2 ) < 1 )
    {
      int halfCount = mCount / 2;
      bool even = ( halfCount % 2 ) < 1;
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
      int halfCount = mCount / 2 + 1;
      bool even = ( halfCount % 2 ) < 1;
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

  if ( mStatistics & QgsStatisticalSummary::Minority || mStatistics & QgsStatisticalSummary::Majority )
  {
    QList<int> valueCounts = mValueCount.values();
    qSort( valueCounts.begin(), valueCounts.end() );
    if ( mStatistics & QgsStatisticalSummary::Minority )
    {
      mMinority = mValueCount.key( valueCounts.first() );
    }
    if ( mStatistics & QgsStatisticalSummary::Majority )
    {
      mMajority = mValueCount.key( valueCounts.last() );
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
    case Count:
      return mCount;
    case CountMissing:
      return mMissing;
    case Sum:
      return mSum;
    case Mean:
      return mMean;
    case Median:
      return mMedian;
    case StDev:
      return mStdev;
    case StDevSample:
      return mSampleStdev;
    case Min:
      return mMin;
    case Max:
      return mMax;
    case Range:
      return mMax - mMin;
    case Minority:
      return mMinority;
    case Majority:
      return mMajority;
    case Variety:
      return mValueCount.count();
    case FirstQuartile:
      return mFirstQuartile;
    case ThirdQuartile:
      return mThirdQuartile;
    case InterQuartileRange:
      return mThirdQuartile - mFirstQuartile;
    case All:
      return 0;
  }
  return 0;
}

QString QgsStatisticalSummary::displayName( QgsStatisticalSummary::Statistic statistic )
{
  switch ( statistic )
  {
    case Count:
      return QObject::tr( "Count" );
    case CountMissing:
      return QObject::tr( "Count (missing)" );
    case Sum:
      return QObject::tr( "Sum" );
    case Mean:
      return QObject::tr( "Mean" );
    case Median:
      return QObject::tr( "Median" );
    case StDev:
      return QObject::tr( "St dev (pop)" );
    case StDevSample:
      return QObject::tr( "St dev (sample)" );
    case Min:
      return QObject::tr( "Minimum" );
    case Max:
      return QObject::tr( "Maximum" );
    case Range:
      return QObject::tr( "Range" );
    case Minority:
      return QObject::tr( "Minority" );
    case Majority:
      return QObject::tr( "Majority" );
    case Variety:
      return QObject::tr( "Variety" );
    case FirstQuartile:
      return QObject::tr( "Q1" );
    case ThirdQuartile:
      return QObject::tr( "Q3" );
    case InterQuartileRange:
      return QObject::tr( "IQR" );
    case All:
      return QString();
  }
  return QString();
}

