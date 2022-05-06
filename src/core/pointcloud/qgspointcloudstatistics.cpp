/***************************************************************************
                         qgspointcloudstatistics.h
                         --------------------
    begin                : May 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudstatistics.h"

#include "qgspointcloudattribute.h"

QgsPointCloudStatistics::QgsPointCloudStatistics()
{

}

QgsPointCloudStatistics::QgsPointCloudStatistics( int sampledPointsCount, const QMap<QString, AttributeStatistics> &stats )
  : mSampledPointsCount( sampledPointsCount ), mStatisticsMap( stats )
{

}

void QgsPointCloudStatistics::clear()
{
  mStatisticsMap.clear();
}

void QgsPointCloudStatistics::clear( const QVector<QgsPointCloudAttribute> &attributes )
{
  for ( QgsPointCloudAttribute attribute : attributes )
  {
    mStatisticsMap.remove( attribute.name() );
  }
}

QgsPointCloudStatistics::AttributeStatistics QgsPointCloudStatistics::statisticsOf( const QString &attribute ) const
{
  AttributeStatistics defaultVal;
  defaultVal.minimum = std::numeric_limits<double>::max();
  defaultVal.maximum = std::numeric_limits<double>::lowest();
  defaultVal.count = 0;
  return mStatisticsMap.value( attribute, defaultVal );
}

QVariant QgsPointCloudStatistics::statisticOf( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return QVariant();
  const AttributeStatistics &stats = mStatisticsMap[ attribute ];
  switch ( statistic )
  {
    case QgsStatisticalSummary::Count:
      return stats.count >= 0 ? QVariant( stats.count ) : QVariant();

    case QgsStatisticalSummary::Min:
      return stats.minimum;

    case QgsStatisticalSummary::Max:
      return stats.maximum;

    case QgsStatisticalSummary::Range:
      return QVariant( stats.maximum - stats.minimum );

    case QgsStatisticalSummary::Mean:
    case QgsStatisticalSummary::StDev:
    case QgsStatisticalSummary::CountMissing:
    case QgsStatisticalSummary::Sum:
    case QgsStatisticalSummary::Median:
    case QgsStatisticalSummary::StDevSample:
    case QgsStatisticalSummary::Minority:
    case QgsStatisticalSummary::Majority:
    case QgsStatisticalSummary::Variety:
    case QgsStatisticalSummary::FirstQuartile:
    case QgsStatisticalSummary::ThirdQuartile:
    case QgsStatisticalSummary::InterQuartileRange:
    case QgsStatisticalSummary::First:
    case QgsStatisticalSummary::Last:
    case QgsStatisticalSummary::All:
      return QVariant();
  }
  return QVariant();
}

QVariantList QgsPointCloudStatistics::classesOf( const QString &attribute ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return QVariantList();
  AttributeStatistics s = mStatisticsMap[ attribute ];
  QVariantList classes;
  for ( int c : s.classCount.keys() )
  {
    classes.push_back( c );
  }
  return classes;
}

QVariant QgsPointCloudStatistics::classStatisticOf( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const
{
  // For now we only calculate the count of a class
  if ( statistic != QgsStatisticalSummary::Statistic::Count || !mStatisticsMap.contains( attribute ) )
    return QVariant();
  AttributeStatistics s = mStatisticsMap[ attribute ];
  return s.classCount.value( value.toInt(), 0 );
}

void QgsPointCloudStatistics::combineWith( const QgsPointCloudStatistics &stats )
{
  for ( QString attribute : stats.mStatisticsMap.keys() )
  {
    AttributeStatistics s = stats.mStatisticsMap[ attribute ];
    if ( mStatisticsMap.contains( attribute ) )
    {
      s.cumulateStatistics( mStatisticsMap[ attribute ] );
    }
    mStatisticsMap[ attribute ] = s;
  }
  mSampledPointsCount += stats.mSampledPointsCount;
}
