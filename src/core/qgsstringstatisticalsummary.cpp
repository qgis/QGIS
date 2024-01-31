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
#include <QString>
#include <QStringList>
#include <QObject>
#include <QVariant>
#include <QVariantList>
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
  if ( value.type() == QVariant::String )
  {
    testString( value.toString() );
  }
  finalize();
}

void QgsStringStatisticalSummary::finalize()
{
  mMeanLength = mSumLengths / static_cast< double >( mCount );

  if ( mStatistics & Statistic::Minority || mStatistics & Statistic::Majority )
  {
    QList<int> valueCounts = mValues.values();

    if ( mStatistics & Statistic::Minority )
    {
      mMinority = mValues.key( *std::min_element( valueCounts.begin(), valueCounts.end() ) );
    }
    if ( mStatistics & Statistic::Majority )
    {
      mMajority = mValues.key( *std::max_element( valueCounts.begin(), valueCounts.end() ) );
    }
  }
}

void QgsStringStatisticalSummary::calculateFromVariants( const QVariantList &values )
{
  reset();

  const auto constValues = values;
  for ( const QVariant &variant : constValues )
  {
    if ( variant.type() == QVariant::String )
    {
      testString( variant.toString() );
    }
  }

  finalize();
}

void QgsStringStatisticalSummary::testString( const QString &string )
{
  mCount++;

  if ( string.isEmpty() )
    mCountMissing++;

  if ( mStatistics & Statistic::CountDistinct || mStatistics & Statistic::Majority || mStatistics & Statistic::Minority )
  {
    mValues[string]++;
  }
  if ( mStatistics & Statistic::Min )
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
  if ( mStatistics & Statistic::Max )
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
  if ( mStatistics & Statistic::MeanLength )
    mSumLengths += string.length();
  mMinLength = std::min( mMinLength, static_cast<int>( string.length() ) );
  mMaxLength = std::max( mMaxLength, static_cast<int>( string.length() ) );
}

QVariant QgsStringStatisticalSummary::statistic( QgsStringStatisticalSummary::Statistic stat ) const
{
  switch ( stat )
  {
    case Statistic::Count:
      return mCount;
    case Statistic::CountDistinct:
      return mValues.count();
    case Statistic::CountMissing:
      return mCountMissing;
    case Statistic::Min:
      return mMin;
    case Statistic::Max:
      return mMax;
    case Statistic::MinimumLength:
      return mMinLength;
    case Statistic::MaximumLength:
      return mMaxLength;
    case Statistic::MeanLength:
      return mMeanLength;
    case Statistic::Minority:
      return mMinority;
    case Statistic::Majority:
      return mMajority;
    case Statistic::All:
      return 0;
  }
  return 0;
}

QSet<QString> QgsStringStatisticalSummary::distinctValues() const
{
  QSet< QString > res;
  res.reserve( mValues.size() );
  for ( auto it = mValues.begin(); it != mValues.end(); ++it )
  {
    res.insert( it.key() );
  }
  return res;
}

QString QgsStringStatisticalSummary::displayName( QgsStringStatisticalSummary::Statistic statistic )
{
  switch ( statistic )
  {
    case Statistic::Count:
      return QObject::tr( "Count" );
    case Statistic::CountDistinct:
      return QObject::tr( "Count (distinct)" );
    case Statistic::CountMissing:
      return QObject::tr( "Count (missing)" );
    case Statistic::Min:
      return QObject::tr( "Minimum" );
    case Statistic::Max:
      return QObject::tr( "Maximum" );
    case Statistic::MinimumLength:
      return QObject::tr( "Minimum length" );
    case Statistic::MaximumLength:
      return QObject::tr( "Maximum length" );
    case Statistic::MeanLength:
      return QObject::tr( "Mean length" );
    case Statistic::Minority:
      return QObject::tr( "Minority" );
    case Statistic::Majority:
      return QObject::tr( "Majority" );
    case Statistic::All:
      return QString();
  }
  return QString();
}

