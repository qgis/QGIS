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

#include "qgsmessagelog.h"
#include "qgspointcloudattribute.h"

#include <QJsonDocument>
#include <QJsonObject>

// QgsPointCloudAttributeStatistics

void QgsPointCloudAttributeStatistics::cumulateStatistics( const QgsPointCloudAttributeStatistics &stats )
{
  minimum = std::min( minimum, stats.minimum );
  maximum = std::max( maximum, stats.maximum );

  const double newMean = ( mean * count + stats.mean * stats.count ) / ( count + stats.count );
  const double delta1 = newMean - mean;
  const double variance1 = stDev * stDev * ( count - 1 ) + count * delta1 * delta1;
  const double delta2 = newMean - stats.mean;
  const double variance2 = stats.stDev * stats.stDev * ( stats.count - 1 ) + stats.count * delta2 * delta2;
  const double variance = ( variance1 + variance2 ) / ( count + stats.count );
  stDev = std::sqrt( variance );

  mean = newMean;
  count += stats.count;

  for ( auto it = stats.classCount.constBegin(); it != stats.classCount.constEnd(); it++ )
  {
    classCount[ it.key() ] += it.value();
  }
}

int QgsPointCloudAttributeStatistics::singleClassCount( int cls ) const
{
  return classCount.value( cls, -1 );
}

// QgsPointCloudStatistics

QgsPointCloudStatistics::QgsPointCloudStatistics()
{

}

QgsPointCloudStatistics::QgsPointCloudStatistics( int sampledPointsCount, const QMap<QString, QgsPointCloudAttributeStatistics> &stats )
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

QgsPointCloudAttributeStatistics QgsPointCloudStatistics::statisticsOf( const QString &attribute ) const
{
  QgsPointCloudAttributeStatistics defaultVal;
  defaultVal.minimum = std::numeric_limits<double>::max();
  defaultVal.maximum = std::numeric_limits<double>::lowest();
  defaultVal.count = 0;
  return mStatisticsMap.value( attribute, defaultVal );
}

QList<int> QgsPointCloudStatistics::classesOf( const QString &attribute ) const
{
  if ( !mStatisticsMap.contains( attribute ) )
    return QList<int>();
  QgsPointCloudAttributeStatistics s = mStatisticsMap[ attribute ];
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
  for ( auto it = stats.mStatisticsMap.constBegin(); it != stats.mStatisticsMap.constEnd(); it++ )
  {
    const QString attribute = it.key();
    QgsPointCloudAttributeStatistics s = it.value();
    if ( mStatisticsMap.contains( attribute ) )
    {
      s.cumulateStatistics( mStatisticsMap[ attribute ] );
    }
    mStatisticsMap[ attribute ] = s;
  }
  mSampledPointsCount += stats.mSampledPointsCount;
}

QByteArray QgsPointCloudStatistics::toStatisticsJson() const
{
  QJsonObject obj;
  obj.insert( u"sampled-points"_s, QJsonValue::fromVariant( sampledPointsCount() ) );
  QJsonObject stats;
  for ( auto it = mStatisticsMap.constBegin(); it != mStatisticsMap.constEnd(); it++ )
  {
    const QgsPointCloudAttributeStatistics stat = it.value();
    stats.insert( it.key(), attributeStatisticsToJson( stat ) );
  }
  obj.insert( u"stats"_s, stats );

  QJsonDocument statsDoc( obj );
  return statsDoc.toJson( QJsonDocument::Compact );
}

QgsPointCloudStatistics QgsPointCloudStatistics::fromStatisticsJson( const QByteArray &statsByteArray )
{
  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson( statsByteArray, &error );
  if ( error.error != QJsonParseError::NoError )
  {
    QgsMessageLog::logMessage( QObject::tr( "Failed to load statistics JSON from COPC file, reason: %1" ).arg( error.errorString() ) );
    return QgsPointCloudStatistics();
  }

  QJsonObject statsJson = document.object();

  QgsPointCloudStatistics stats;
  stats.mSampledPointsCount = statsJson.value( u"sampled-points"_s ).toInt();
  if ( statsJson.contains( u"stats"_s ) )
  {
    QJsonObject statsObj = statsJson.value( u"stats"_s ).toObject();
    for ( const QString &attr : statsObj.keys() )
    {
      QJsonObject obj = statsObj.value( attr ).toObject();
      QgsPointCloudAttributeStatistics attrStats = fromAttributeStatisticsJson( obj );
      attrStats.count = stats.mSampledPointsCount;
      stats.mStatisticsMap.insert( attr, attrStats );
    }
  }
  return stats;
}

QJsonObject QgsPointCloudStatistics::attributeStatisticsToJson( const QgsPointCloudAttributeStatistics &stats )
{
  QJsonObject obj;
  obj.insert( u"minimum"_s, stats.minimum );
  obj.insert( u"maximum"_s, stats.maximum );
  obj.insert( u"mean"_s, stats.mean );
  if ( !std::isnan( stats.stDev ) )
  {
    obj.insert( u"standard-deviation"_s, stats.stDev );
  }
  QJsonObject classCount;
  for ( auto it = stats.classCount.constBegin(); it != stats.classCount.constEnd(); it++ )
  {
    classCount.insert( QString::number( it.key() ), it.value() );
  }
  obj.insert( u"class-count"_s, classCount );
  return obj;
}

QgsPointCloudAttributeStatistics QgsPointCloudStatistics::fromAttributeStatisticsJson( QJsonObject &statsJson )
{
  QgsPointCloudAttributeStatistics statsObj;
  QVariantMap m = statsJson.toVariantMap();
  statsObj.minimum = m.value( u"minimum"_s, std::numeric_limits<double>::max() ).toDouble();
  statsObj.maximum = m.value( u"maximum"_s, std::numeric_limits<double>::lowest() ).toDouble();
  statsObj.mean = m.value( u"mean"_s, 0 ).toDouble();
  statsObj.stDev = m.value( u"standard-deviation"_s, std::numeric_limits<double>::quiet_NaN() ).toDouble();
  QJsonObject classCountJson = statsJson.value( u"class-count"_s ).toObject();
  for ( const QString &key : classCountJson.keys() )
  {
    statsObj.classCount.insert( key.toInt(), classCountJson.value( key ).toInt() );
  }
  return statsObj;
}
