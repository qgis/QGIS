/***************************************************************************
                         qgspointcloudstatscalculator.cpp
                         --------------------
    begin                : April 2022
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

#include "qgspointcloudstatscalculator.h"

#include "qgspointcloudindex.h"
#include "qgsmessagelog.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrequest.h"

#include "qgspointcloudrenderer.h"

#include "qgsapplication.h"
#include "qgspointcloudstatscalculationtask.h"

#include <QQueue>
#include <QtConcurrent/QtConcurrentMap>

QgsPointCloudStatsCalculator::QgsPointCloudStatsCalculator( QgsPointCloudIndex *index )
  : mIndex( index )
{

}

void QgsPointCloudStatsCalculator::clear()
{
  mStatisticsMap.clear();
  mProcessedNodes.clear();
}

void QgsPointCloudStatsCalculator::clear( const QVector<QgsPointCloudAttribute> &attributes )
{
  for ( QgsPointCloudAttribute attribute : attributes )
  {
    mStatisticsMap.remove( attribute.name() );
  }
  mProcessedNodes.clear();
}

void QgsPointCloudStatsCalculator::calculateStats( const QVector<QgsPointCloudAttribute> &attributes, qint64 pointsLimit )
{
  if ( mStatsCalculationTaskId != 0 )
    return;
  if ( !mIndex->isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Unable to calculate statistics of an invalid index" ) );
    return;
  }
  mRequest.setAttributes( attributes );

  //
  qint64 pointCount = 0;
  QVector<IndexedPointCloudNode> nodes;
  QQueue<IndexedPointCloudNode> queue;
  queue.push_back( mIndex->root() );
  while ( !queue.empty() )
  {
    IndexedPointCloudNode node = queue.front();
    queue.pop_front();
    if ( !mProcessedNodes.contains( node ) )
      pointCount += mIndex->nodePointCount( node );
    if ( pointsLimit != -1 && pointCount > pointsLimit )
      break;
    if ( !mProcessedNodes.contains( node ) )
      nodes.push_back( node );
    for ( IndexedPointCloudNode child : mIndex->nodeChildren( node ) )
    {
      queue.push_back( child );
    }
  }

  QgsPointCloudStatsCalculationTask *task = new QgsPointCloudStatsCalculationTask( mIndex, attributes, nodes );
  QObject::connect( task, &QgsTask::taskCompleted, this, &QgsPointCloudStatsCalculator::statsCalculationFinished );
  QObject::connect( task, &QgsTask::taskTerminated, this, [this]
  {
    QgsMessageLog::logMessage( QStringLiteral( "Statistics generation cancelled" ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Info );
    mStatsCalculationTaskId = 0;
  } );

  mStatsCalculationTaskId = QgsApplication::taskManager()->addTask( task );
}

void QgsPointCloudStatsCalculator::statsCalculationFinished()
{
  QgsPointCloudStatsCalculationTask *task = qobject_cast<QgsPointCloudStatsCalculationTask *>( QObject::sender() );
  if ( !task )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Statistics generation error: Invalid task" ), QObject::tr( "Point clouds" ), Qgis::MessageLevel::Info );
    return;
  }

  QVector<QgsPointCloudAttribute> attributes = mRequest.attributes().attributes();
  // TODO: use reduce
  for ( QgsPointCloudAttribute attribute : attributes )
  {
    if ( mStatisticsMap.contains( attribute.name() ) )
      continue;
    AttributeStatistics summary;
    summary.minimum = std::numeric_limits<double>::max();
    summary.maximum = std::numeric_limits<double>::lowest();
    mStatisticsMap[ attribute.name() ] = summary;
  }

  for ( QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics> statsMap : task->mFuture )
  {
    for ( QgsPointCloudAttribute attribute : attributes )
    {
      QgsPointCloudStatsCalculator::AttributeStatistics &cumulatedStats = mStatisticsMap[ attribute.name() ];
      QgsPointCloudStatsCalculator::AttributeStatistics &stats = statsMap[ attribute.name() ];
      cumulatedStats.minimum = std::min( cumulatedStats.minimum, stats.minimum );
      cumulatedStats.maximum = std::max( cumulatedStats.maximum, stats.maximum );
      cumulatedStats.count += stats.count;
      for ( int key : stats.classCount.keys() )
      {
        cumulatedStats.classCount.insert( key, cumulatedStats.classCount.value( key, 0 ) + stats.classCount.value( key ) );
      }
    }
  }

  mStatsCalculationTaskId = 0;

  emit statisticsCalculated();
}

QgsPointCloudStatsCalculator::AttributeStatistics QgsPointCloudStatsCalculator::statisticsOf( const QString &attribute )
{
  AttributeStatistics defaultVal;
  defaultVal.minimum = std::numeric_limits<double>::max();
  defaultVal.maximum = std::numeric_limits<double>::lowest();
  defaultVal.count = 0;
  return mStatisticsMap.value( attribute, defaultVal );
}
