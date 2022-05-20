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
#include <QJsonObject>

#include "qgspointcloudattribute.h"

QgsPointCloudStatistics::QgsPointCloudStatistics()
{

}

QgsPointCloudStatistics::QgsPointCloudStatistics( const QJsonObject &obj )
{
  mSampledPointsCount = obj.value( QStringLiteral( "sampled-points" ) ).toInt();
  if ( obj.contains( QStringLiteral( "stats" ) ) )
  {
    QJsonObject stats = obj.value( QStringLiteral( "stats" ) ).toObject();
    for ( QString attr : stats.keys() )
    {
      QJsonObject obj = stats.value( attr ).toObject();
      QgsPointCloudStatistics::AttributeStatistics attrStats = fromJsonToStats( obj );
      mStatisticsMap.insert( attr, attrStats );
    }
  }
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

QJsonObject QgsPointCloudStatistics::toJson() const
{
  QJsonObject obj;
  obj.insert( QStringLiteral( "sampled-points" ), QJsonValue::fromVariant( sampledPointsCount() ) );
  QJsonObject stats;
  for ( QString &attr : mStatisticsMap.keys() )
  {
    QgsPointCloudStatistics::AttributeStatistics stat = mStatisticsMap.value( attr );
    stats.insert( attr, QgsPointCloudStatistics::fromStatsToJson( stat ) );
  }
  obj.insert( QStringLiteral( "stats" ), stats );
  return obj;
}

QJsonObject QgsPointCloudStatistics::fromStatsToJson( QgsPointCloudStatistics::AttributeStatistics &stats )
{
  QJsonObject obj;
  obj.insert( QStringLiteral( "minimum" ), stats.minimum );
  obj.insert( QStringLiteral( "maximum" ), stats.maximum );
  obj.insert( QStringLiteral( "mean" ), stats.mean );
  if ( !std::isnan( stats.stDev ) )
  {
    obj.insert( QStringLiteral( "stDev" ), stats.stDev );
  }
  obj.insert( QStringLiteral( "count" ), stats.count );
  QJsonObject classCount;
  for ( int c : stats.classCount.keys() )
  {
    classCount.insert( QString::number( c ), stats.classCount[c] );
  }
  obj.insert( QStringLiteral( "class-count" ), classCount );
  return obj;
}

QgsPointCloudStatistics::AttributeStatistics QgsPointCloudStatistics::fromJsonToStats( QJsonObject &statsJson )
{
  QgsPointCloudStatistics::AttributeStatistics statsObj;
  QVariantMap m = statsJson.toVariantMap();
  statsObj.minimum = m.value( QStringLiteral( "minimum" ), std::numeric_limits<double>::max() ).toDouble();
  statsObj.maximum = m.value( QStringLiteral( "maximum" ), std::numeric_limits<double>::lowest() ).toDouble();
  statsObj.mean = m.value( QStringLiteral( "mean" ), 0 ).toDouble();
  statsObj.stDev = m.value( QStringLiteral( "stDev" ), std::numeric_limits<double>::quiet_NaN() ).toDouble();
  statsObj.count = m.value( QStringLiteral( "count" ), 0 ).toDouble();
  QJsonObject classCountJson = statsJson.value( QStringLiteral( "class-count" ) ).toObject();
  for ( QString key : classCountJson.keys() )
  {
    statsObj.classCount.insert( key.toInt(), classCountJson.value( key ).toInt() );
  }
  return statsObj;
}
