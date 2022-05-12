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

#include <limits>

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

QList<int> QgsPointCloudStatistics::classesOf( const QString &attribute ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return QList<int>();
  AttributeStatistics s = mStatisticsMap[ attribute ];
  return s.classCount.keys();
}

QMap<int, int> QgsPointCloudStatistics::availableClasses( const QString &attribute ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return QMap<int, int>();
  return mStatisticsMap[ attribute ].classCount;
}

double QgsPointCloudStatistics::minimum( const QString &attribute ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return std::numeric_limits<double>::quiet_NaN();
  return mStatisticsMap[ attribute ].minimum;
}

double QgsPointCloudStatistics::maximum( const QString &attribute ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return std::numeric_limits<double>::quiet_NaN();
  return mStatisticsMap[ attribute ].maximum;
}

double QgsPointCloudStatistics::mean( const QString &attribute ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return std::numeric_limits<double>::quiet_NaN();
  return mStatisticsMap[ attribute ].mean;
}

double QgsPointCloudStatistics::stDev( const QString &attribute ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return std::numeric_limits<double>::quiet_NaN();
  return mStatisticsMap[ attribute ].stDev;
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
