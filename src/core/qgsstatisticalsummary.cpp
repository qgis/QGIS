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


QgsStatisticalSummary::QgsStatisticalSummary( Statistics stats )
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
}

void QgsStatisticalSummary::calculate( const QList<double> &values )
{
  reset();

  foreach ( double value, values )
  {
    mCount++;
    mSum += value;
    mMin = qMin( mMin, value );
    mMax = qMax( mMax, value );

    if ( mStatistics & QgsStatisticalSummary::Majority || mStatistics & QgsStatisticalSummary::Minority || mStatistics & QgsStatisticalSummary::Variety )
      mValueCount.insert( value, mValueCount.value( value, 0 ) + 1 );
  }

  if ( mCount == 0 )
    return;

  mMean = mSum / mCount;

  if ( mStatistics & QgsStatisticalSummary::StDev )
  {
    double sumSquared = 0;
    foreach ( double value, values )
    {
      double diff = value - mMean;
      sumSquared += diff * diff;
    }
    mStdev = qPow( sumSquared / values.count(), 0.5 );
    mSampleStdev = qPow( sumSquared / ( values.count() - 1 ), 0.5 );
  }

  QList<double> sorted;
  if ( mStatistics & QgsStatisticalSummary::Median
       || mStatistics & QgsStatisticalSummary::FirstQuartile
       || mStatistics & QgsStatisticalSummary::ThirdQuartile
       || mStatistics & QgsStatisticalSummary::InterQuartileRange )
  {
    sorted = values;
    qSort( sorted.begin(), sorted.end() );
    bool even = ( mCount % 2 ) < 1;
    if ( even )
    {
      mMedian = ( sorted[mCount / 2 - 1] + sorted[mCount / 2] ) / 2.0;
    }
    else //odd
    {
      mMedian = sorted[( mCount + 1 ) / 2 - 1];
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
        mFirstQuartile = ( sorted[halfCount / 2 - 1] + sorted[halfCount / 2] ) / 2.0;
      }
      else //odd
      {
        mFirstQuartile = sorted[( halfCount  + 1 ) / 2 - 1];
      }
    }
    else
    {
      int halfCount = mCount / 2 + 1;
      bool even = ( halfCount % 2 ) < 1;
      if ( even )
      {
        mFirstQuartile = ( sorted[halfCount / 2 - 1] + sorted[halfCount / 2] ) / 2.0;
      }
      else //odd
      {
        mFirstQuartile = sorted[( halfCount  + 1 ) / 2 - 1];
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
        mThirdQuartile = ( sorted[ halfCount + halfCount / 2 - 1] + sorted[ halfCount + halfCount / 2] ) / 2.0;
      }
      else //odd
      {
        mThirdQuartile = sorted[( halfCount + 1 ) / 2 - 1 + halfCount ];
      }
    }
    else
    {
      int halfCount = mCount / 2 + 1;
      bool even = ( halfCount % 2 ) < 1;
      if ( even )
      {
        mThirdQuartile = ( sorted[ halfCount + halfCount / 2 - 2 ] + sorted[ halfCount + halfCount / 2 - 1 ] ) / 2.0;
      }
      else //odd
      {
        mThirdQuartile = sorted[( halfCount + 1 ) / 2 - 2 + halfCount ];
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

