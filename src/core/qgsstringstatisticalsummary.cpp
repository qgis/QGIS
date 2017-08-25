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
  mMinLength = INT_MAX;
  mMaxLength = 0;
  mSumLengths = 0;
  mMeanLength = 0;
}

void QgsStringStatisticalSummary::calculate( const QStringList &values )
{
  reset();

  Q_FOREACH ( const QString &string, values )
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
}

void QgsStringStatisticalSummary::calculateFromVariants( const QVariantList &values )
{
  reset();

  Q_FOREACH ( const QVariant &variant, values )
  {
    if ( variant.type() == QVariant::String )
    {
      testString( variant.toString() );
    }
  }
}

void QgsStringStatisticalSummary::testString( const QString &string )
{
  mCount++;

  if ( string.isEmpty() )
    mCountMissing++;

  if ( mStatistics & CountDistinct )
  {
    mValues << string;
  }
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
    case All:
      return QString();
  }
  return QString();
}

