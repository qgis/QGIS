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

#include "qgspointcloudstatistics.h"

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
    typedef QgsPointCloudStatistics result_type;

    StatsProcessor( QgsPointCloudIndex *index, QgsPointCloudRequest request, QgsFeedback *feedback )
      : mIndex( index ), mRequest( request ), mFeedback( feedback )
    {

    }

    QgsPointCloudStatistics operator()( IndexedPointCloudNode node )
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
          return QgsPointCloudStatistics();
        }
      }

      if ( !block.get() )
      {
        return QgsPointCloudStatistics();
      }

      const QgsPointCloudAttributeCollection attributesCollection = block->attributes();
      const QVector<QgsPointCloudAttribute> attributes = attributesCollection.attributes();
      const char *ptr = block->data();
      int count = block->pointCount();
      int recordSize = attributesCollection.pointRecordSize();

      QMap<QString, QgsPointCloudStatistics::AttributeStatistics> statsMap;
      for ( QgsPointCloudAttribute attribute : attributes )
      {
        QgsPointCloudStatistics::AttributeStatistics summary;
        summary.minimum = std::numeric_limits<double>::max();
        summary.maximum = std::numeric_limits<double>::lowest();
        summary.count = 0;
        summary.mean = 0;
        summary.stDev = std::numeric_limits<double>::quiet_NaN();
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
        if ( attribute.name() == QLatin1String( "ScannerChannel" ) ||
             attribute.name() == QLatin1String( "ReturnNumber" ) ||
             attribute.name() == QLatin1String( "NumberOfReturns" ) ||
             attribute.name() == QLatin1String( "ScanDirectionFlag" ) ||
             attribute.name() == QLatin1String( "Classification" ) ||
             attribute.name() == QLatin1String( "EdgeOfFlightLine" ) ||
             attribute.name() == QLatin1String( "PointSourceId" ) )
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
            return QgsPointCloudStatistics();
          }
          QString attributeName = attributes.at( j ).name();
          QgsPointCloudAttribute::DataType attributeType = attributes.at( j ).type();

          double attributeValue = 0;
          int attributeOffset = attributeOffsetVector[ j ];

          QgsPointCloudStatistics::AttributeStatistics &stats = statsMap[ attributeName ];
          QgsPointCloudRenderContext::getAttribute( ptr, i * recordSize + attributeOffset, attributeType, attributeValue );
          stats.minimum = std::min( stats.minimum, attributeValue );
          stats.maximum = std::max( stats.maximum, attributeValue );
          stats.mean += attributeValue / count;
          // TODO: add stDev calculation
          stats.count++;
          if ( classifiableAttributesOffsetSet.contains( attributeOffset ) )
          {
            stats.classCount[( int )attributeValue ]++;
          }
        }
      }
      return QgsPointCloudStatistics( count, statsMap );
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

bool QgsPointCloudStatsCalculator::calculateStats( QgsFeedback *feedback, const QVector<QgsPointCloudAttribute> &attributes, qint64 pointsLimit )
{
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
    {
      nodes.push_back( node );
      mProcessedNodes.insert( node );
    }
    for ( IndexedPointCloudNode child : mIndex->nodeChildren( node ) )
    {
      queue.push_back( child );
    }
  }

  mFuture = QtConcurrent::mapped( nodes, StatsProcessor( mIndex, mRequest, feedback ) );
  mFutureWatcher.setFuture( mFuture );

  connect( &mFutureWatcher, &QFutureWatcher<QgsPointCloudStatistics>::progressValueChanged,
           this, [feedback, this]( int progressValue )
  {
    double percent = 100.0 * ( ( double )progressValue - mFutureWatcher.progressMinimum() ) / ( mFutureWatcher.progressMaximum() - mFutureWatcher.progressMinimum() );
    feedback->setProgress( percent );
  } );

  connect( feedback, &QgsFeedback::canceled, &mFutureWatcher, &QFutureWatcher<QgsPointCloudStatistics>::cancel );

  mFutureWatcher.waitForFinished();

  if ( mFutureWatcher.isCanceled() )
  {
    return false;
  }

  for ( QgsPointCloudStatistics s : mFuture )
  {
    mStats.combineWith( s );
  }

  // fetch X, Y & Z stats directly from the index
  QVector<QString> coordinateAttributes;
  coordinateAttributes.push_back( QStringLiteral( "X" ) );
  coordinateAttributes.push_back( QStringLiteral( "Y" ) );
  coordinateAttributes.push_back( QStringLiteral( "Z" ) );

  QMap<QString, QgsPointCloudStatistics::AttributeStatistics> statsMap = mStats.statisticsMap();
  for ( QString attribute : coordinateAttributes )
  {
    QgsPointCloudStatistics::AttributeStatistics s;
    QVariant min = mIndex->metadataStatistic( attribute, QgsStatisticalSummary::Min );
    QVariant max = mIndex->metadataStatistic( attribute, QgsStatisticalSummary::Max );
    if ( !min.isValid() )
      continue;
    s.minimum = min.toDouble();
    s.maximum = max.toDouble();
    s.count = mIndex->metadataStatistic( attribute, QgsStatisticalSummary::Count ).toInt();
    s.mean = mIndex->metadataStatistic( attribute, QgsStatisticalSummary::Mean ).toInt();
    s.stDev = mIndex->metadataStatistic( attribute, QgsStatisticalSummary::StDev ).toInt();
    QVariantList classes = mIndex->metadataClasses( attribute );
    for ( QVariant c : classes )
    {
      s.classCount[ c.toInt() ] = mIndex->metadataClassStatistic( attribute, c, QgsStatisticalSummary::Count ).toInt();
    }
    statsMap[ attribute ] = s;
  }

  mStats = QgsPointCloudStatistics( pointCount, statsMap );

  return true;
}
