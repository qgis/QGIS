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
#include "qgsfeedback.h"
#include "qgspointcloudblockrequest.h"

#include <QQueue>
#include <QtConcurrent/QtConcurrentMap>

struct StatsProcessor
{
    typedef QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics> result_type;

    StatsProcessor( QgsPointCloudIndex *index, QgsPointCloudRequest request, QgsFeedback *feedback )
      : mIndex( index ), mRequest( request ), mFeedback( feedback )
    {

    }

    QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics> operator()( IndexedPointCloudNode node )
    {
      std::unique_ptr<QgsPointCloudBlock> block;
      if ( mIndex->accessType() == QgsPointCloudIndex::Local )
      {
        block.reset( mIndex->nodeData( node, mRequest ) );
      }
      else
      {
        QgsPointCloudBlockRequest *request = mIndex->asyncNodeData( node, mRequest );
        QEventLoop loop;
        QObject::connect( request, &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
        QObject::connect( mFeedback, &QgsFeedback::canceled, &loop, &QEventLoop::quit );
        loop.exec();
        if ( !mFeedback->isCanceled() )
          block.reset( request->block() );
        if ( !request->block() )
        {
          QgsMessageLog::logMessage( QObject::tr( "Unable to calculate statistics for node %1, error: \"%2\"" ).arg( node.toString() ).arg( request->errorStr() ) );
          return result_type();
        }
      }

      if ( !block.get() )
      {
        return result_type();
      }

      const QgsPointCloudAttributeCollection attributesCollection = block->attributes();
      const QVector<QgsPointCloudAttribute> attributes = attributesCollection.attributes();
      const char *ptr = block->data();
      int count = block->pointCount();
      int recordSize = attributesCollection.pointRecordSize();

      QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics> statsMap;
      for ( QgsPointCloudAttribute attribute : attributes )
      {
        QgsPointCloudStatsCalculator::AttributeStatistics summary;
        summary.minimum = std::numeric_limits<double>::max();
        summary.maximum = std::numeric_limits<double>::lowest();
        summary.count = 0;
        summary.classCount.clear();
        statsMap[ attribute.name() ] = summary;
      }

      QVector<int> attributeOffsetVector;
      QSet<int> classifiableAttributesOffsetSet;
      for ( const QgsPointCloudAttribute &attribute : attributes )
      {
        int attributeOffset = 0;
        attributesCollection.find( attribute.name(), attributeOffset );
        attributeOffsetVector.push_back( attributeOffset );
        if ( attribute.name() == QStringLiteral( "Classification" ) )
        {
          classifiableAttributesOffsetSet.insert( attributeOffset );
        }
      }

      for ( int i = 0; i < count; ++i )
      {
        for ( int j = 0; j < attributes.size(); ++j )
        {
          if ( mFeedback->isCanceled() )
          {
            return result_type();
          }
          QString attributeName = attributes.at( j ).name();
          QgsPointCloudAttribute::DataType attributeType = attributes.at( j ).type();

          double attributeValue = 0;
          int attributeOffset = attributeOffsetVector[ i ];

          QgsPointCloudStatsCalculator::AttributeStatistics &stats = statsMap[ attributeName ];
          QgsPointCloudRenderContext::getAttribute( ptr, i * recordSize + attributeOffset, attributeType, attributeValue );
          stats.minimum = std::min( stats.minimum, attributeValue );
          stats.maximum = std::max( stats.maximum, attributeValue );
          stats.count++;
          if ( classifiableAttributesOffsetSet.contains( attributeOffset ) )
          {
            stats.classCount[( int )attributeValue ]++;
          }
        }
      }
      return statsMap;
    }
  private:
    QgsPointCloudIndex *mIndex = nullptr;
    QgsPointCloudRequest mRequest;
    QgsFeedback *mFeedback = nullptr;
};


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

bool QgsPointCloudStatsCalculator::calculateStats( QgsFeedback *feedback, const QVector<QgsPointCloudAttribute> &attributes, qint64 pointsLimit )
{
  using FeatureWatcher = QFutureWatcher<QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics>>;
  if ( !mIndex->isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Unable to calculate statistics of an invalid index" ) );
    return false;
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

  mFuture = QtConcurrent::mapped( nodes, StatsProcessor( mIndex, mRequest, feedback ) );
  mFutureWatcher.setFuture( mFuture );

  connect( &mFutureWatcher, &FeatureWatcher::progressValueChanged,
           this, [feedback, this]( int progressValue )
  {
    double percent = 100.0 * ( ( double )progressValue - mFutureWatcher.progressMinimum() ) / ( mFutureWatcher.progressMaximum() - mFutureWatcher.progressMinimum() );
    feedback->setProgress( percent );
  } );

  connect( feedback, &QgsFeedback::canceled, &mFutureWatcher, &FeatureWatcher::cancel );

  mFutureWatcher.waitForFinished();

  if ( mFutureWatcher.isCanceled() )
  {
    return false;
  }

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

  for ( QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics> statsMap : mFuture )
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

  return true;
}

QgsPointCloudStatsCalculator::AttributeStatistics QgsPointCloudStatsCalculator::statisticsOf( const QString &attribute )
{
  AttributeStatistics defaultVal;
  defaultVal.minimum = std::numeric_limits<double>::max();
  defaultVal.maximum = std::numeric_limits<double>::lowest();
  defaultVal.count = 0;
  return mStatisticsMap.value( attribute, defaultVal );
}

QVariant QgsPointCloudStatsCalculator::statisticsOf( const QString &attribute, QgsStatisticalSummary::Statistic statistic )
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
