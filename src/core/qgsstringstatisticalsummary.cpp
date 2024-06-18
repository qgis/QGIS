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
#include "qgsvariantutils.h"
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

QgsStringStatisticalSummary::QgsStringStatisticalSummary( Qgis::StringStatistics stats )
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
  if ( QgsVariantUtils::isNull( value ) || value.userType() == QMetaType::Type::QString )
  {
    testString( value.toString() );
  }
  finalize();
}

void QgsStringStatisticalSummary::finalize()
{
  mMeanLength = mSumLengths / static_cast< double >( mCount );

  if ( mStatistics & Qgis::StringStatistic::Minority || mStatistics & Qgis::StringStatistic::Majority )
  {
    QList<int> valueCounts = mValues.values();

    if ( mStatistics & Qgis::StringStatistic::Minority )
    {
      mMinority = mValues.key( *std::min_element( valueCounts.begin(), valueCounts.end() ) );
    }
    if ( mStatistics & Qgis::StringStatistic::Majority )
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
    if ( QgsVariantUtils::isNull( variant ) || variant.userType() == QMetaType::Type::QString )
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

  if ( mStatistics & Qgis::StringStatistic::CountDistinct || mStatistics & Qgis::StringStatistic::Majority || mStatistics & Qgis::StringStatistic::Minority )
  {
    mValues[string]++;
  }
  if ( mStatistics & Qgis::StringStatistic::Min )
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
  if ( mStatistics & Qgis::StringStatistic::Max )
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
  if ( mStatistics & Qgis::StringStatistic::MeanLength )
    mSumLengths += string.length();
  mMinLength = std::min( mMinLength, static_cast<int>( string.length() ) );
  mMaxLength = std::max( mMaxLength, static_cast<int>( string.length() ) );
}

QVariant QgsStringStatisticalSummary::statistic( Qgis::StringStatistic stat ) const
{
  switch ( stat )
  {
    case Qgis::StringStatistic::Count:
      return mCount;
    case Qgis::StringStatistic::CountDistinct:
      return mValues.count();
    case Qgis::StringStatistic::CountMissing:
      return mCountMissing;
    case Qgis::StringStatistic::Min:
      return mMin;
    case Qgis::StringStatistic::Max:
      return mMax;
    case Qgis::StringStatistic::MinimumLength:
      return mMinLength;
    case Qgis::StringStatistic::MaximumLength:
      return mMaxLength;
    case Qgis::StringStatistic::MeanLength:
      return mMeanLength;
    case Qgis::StringStatistic::Minority:
      return mMinority;
    case Qgis::StringStatistic::Majority:
      return mMajority;
    case Qgis::StringStatistic::All:
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

QString QgsStringStatisticalSummary::displayName( Qgis::StringStatistic statistic )
{
  switch ( statistic )
  {
    case Qgis::StringStatistic::Count:
      return QObject::tr( "Count" );
    case Qgis::StringStatistic::CountDistinct:
      return QObject::tr( "Count (distinct)" );
    case Qgis::StringStatistic::CountMissing:
      return QObject::tr( "Count (missing)" );
    case Qgis::StringStatistic::Min:
      return QObject::tr( "Minimum" );
    case Qgis::StringStatistic::Max:
      return QObject::tr( "Maximum" );
    case Qgis::StringStatistic::MinimumLength:
      return QObject::tr( "Minimum length" );
    case Qgis::StringStatistic::MaximumLength:
      return QObject::tr( "Maximum length" );
    case Qgis::StringStatistic::MeanLength:
      return QObject::tr( "Mean length" );
    case Qgis::StringStatistic::Minority:
      return QObject::tr( "Minority" );
    case Qgis::StringStatistic::Majority:
      return QObject::tr( "Majority" );
    case Qgis::StringStatistic::All:
      return QString();
  }
  return QString();
}

