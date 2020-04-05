/***************************************************************************
  qgsstringstatisticalsummary.cpp
  -------------------------------
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

#include "qgsstringstatisticalsummary.h"
#include <QObject>
#include <limits>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsstringstatisticalsummary.py.
 * See details in QEP #17
 ****************************************************************************/

QgsStringStatisticalSummary::QgsStringStatisticalSummary( QgsStringStatisticalSummary::Statistics stats )
  : mStatistics( stats )
{
  reset();
}

void QgsStringStatisticalSummary::reset()
{
  mCount = 0;
  mValues.clear();
  mCountMissing = 0;
  mMin.clear();
  mMax.clear();
  mMinLength = std::numeric_limits<int>::max();
  mMaxLength = 0;
  mSumLengths = 0;
  mMeanLength = 0;
  mMinority = QString();
  mMajority = QString();
  mFirst = QString();
  mLast = QString();
  mMode.clear();
}

void QgsStringStatisticalSummary::calculate( const QStringList &values )
{
  reset();

  const auto constValues = values;
  for ( const QString &string : constValues )
  {
    testString( string );
  }

  finalize();
}

void QgsStringStatisticalSummary::addString( const QString &string )
{
  testString( string );
}

void QgsStringStatisticalSummary::addValue( const QVariant &value )
{
  if ( value.isNull() )
  {
    addString( QString() );
  }
  else if ( value.type() == QVariant::String )
  {
    addString( value.toString() );
  }
}

void QgsStringStatisticalSummary::finalize()
{
  mMeanLength = mSumLengths / static_cast< double >( mCount );

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

    for ( const QString key : mValues.keys() )
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

void QgsStringStatisticalSummary::calculateFromVariants( const QVariantList &values )
{
  reset();

  const auto constValues = values;
  for ( const QVariant &variant : constValues )
  {
    if ( variant.isNull() )
    {
      testString( QString() );
    }
    else if ( variant.type() == QVariant::String )
    {
      testString( variant.toString() );
    }
  }

  finalize();
}

void QgsStringStatisticalSummary::testString( const QString &string )
{
  if ( string.isNull() )
  {
    mCountMissing++;
    return;
  }

  mCount++;

  if ( mStatistics & CountDistinct || mStatistics & Majority || mStatistics & Minority || mStatistics & Mode )
    mValues[string]++;

  if ( mStatistics & Min )
  {
    if ( !mMin.isEmpty() && !string.isEmpty() )
    {
      mMin = std::min( mMin, string );
    }
    else if ( mMin.isEmpty() && !string.isEmpty() )
    {
      mMin = string;
    }
  }
  if ( mStatistics & Max )
  {
    if ( !mMax.isEmpty() && !string.isEmpty() )
    {
      mMax = std::max( mMax, string );
    }
    else if ( mMax.isEmpty() && !string.isEmpty() )
    {
      mMax = string;
    }
  }

  if ( mStatistics & MeanLength )
    mSumLengths += string.length();

  mMinLength = std::min( mMinLength, string.length() );
  mMaxLength = std::max( mMaxLength, string.length() );

  if ( mStatistics & First )
    if ( mFirst.isNull() )
      mFirst = string;

  if ( mStatistics & Last )
    mLast = string;
}

QVariant QgsStringStatisticalSummary::statistic( QgsStringStatisticalSummary::Statistic stat ) const
{
  switch ( stat )
  {
    case Count:
      return mCount;
    case CountDistinct:
      return mValues.count();
    case CountMissing:
      return mCountMissing;
    case Min:
      return mMin;
    case Max:
      return mMax;
    case MinimumLength:
      return mMinLength;
    case MaximumLength:
      return mMaxLength;
    case MeanLength:
      return mMeanLength;
    case Minority:
      return mMinority;
    case Majority:
      return mMajority;
    case First:
      return mFirst;
    case Last:
      return mLast;
    case Mode:
      return mMode;
    case All:
      return 0;
  }
  return 0;
}

QString QgsStringStatisticalSummary::displayName( QgsStringStatisticalSummary::Statistic statistic )
{
  switch ( statistic )
  {
    case Count:
      return QObject::tr( "Count" );
    case CountDistinct:
      return QObject::tr( "Count (distinct)" );
    case CountMissing:
      return QObject::tr( "Count (missing)" );
    case Min:
      return QObject::tr( "Minimum" );
    case Max:
      return QObject::tr( "Maximum" );
    case MinimumLength:
      return QObject::tr( "Minimum length" );
    case MaximumLength:
      return QObject::tr( "Maximum length" );
    case MeanLength:
      return QObject::tr( "Mean length" );
    case Minority:
      return QObject::tr( "Minority" );
    case Majority:
      return QObject::tr( "Majority" );
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

