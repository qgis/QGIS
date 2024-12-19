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
#include "moc_qgspointcloudstatscalculator.cpp"

#include "qgspointcloudstatistics.h"

#include "qgspointcloudindex.h"
#include "qgsmessagelog.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudrequest.h"

#include "qgspointcloudrenderer.h"

#include "qgsfeedback.h"
#include "qgspointcloudblockrequest.h"

#include <QQueue>
#include <QtConcurrent/QtConcurrentMap>

struct StatsProcessor
{
    typedef QgsPointCloudStatistics result_type;
    static QMutex sStatsProcessorFeedbackMutex;

    StatsProcessor( QgsPointCloudIndex index, QgsPointCloudRequest request, QgsFeedback *feedback, double progressValue )
      : mIndex( index ), mRequest( request ), mFeedback( feedback ), mProgressValue( progressValue )
    {
    }

    StatsProcessor( const StatsProcessor &processor )
      : mIndex( processor.mIndex ), mRequest( processor.mRequest ), mFeedback( processor.mFeedback ), mProgressValue( processor.mProgressValue )
    {
    }

    StatsProcessor &operator =( const StatsProcessor &rhs )
    {
      mIndex = rhs.mIndex;
      mRequest = rhs.mRequest;
      mFeedback = rhs.mFeedback;
      mProgressValue = rhs.mProgressValue;
      return *this;
    }

    QgsPointCloudStatistics operator()( QgsPointCloudNodeId nodeId )
    {
      QgsPointCloudNode node = mIndex.getNode( nodeId );
      if ( node.pointCount() < 1 )
        return QgsPointCloudStatistics();

      std::unique_ptr<QgsPointCloudBlock> block = nullptr;
      if ( mIndex.accessType() == Qgis::PointCloudAccessType::Local )
      {
        block = mIndex.nodeData( nodeId, mRequest );
      }
      else
      {
        QgsPointCloudBlockRequest *request = mIndex.asyncNodeData( nodeId, mRequest );
        if ( request == nullptr )
        {
          QgsDebugError( QStringLiteral( "Unable to calculate statistics for node %1: Got nullptr async request" ).arg( nodeId.toString() ) );
          return QgsPointCloudStatistics();
        }
        QEventLoop loop;
        QObject::connect( request, &QgsPointCloudBlockRequest::finished, &loop, &QEventLoop::quit );
        QObject::connect( mFeedback, &QgsFeedback::canceled, &loop, &QEventLoop::quit );
        loop.exec();
        if ( !mFeedback->isCanceled() )
        {
          block = request->takeBlock();
          if ( !block )
          {
            QgsMessageLog::logMessage( QObject::tr( "Unable to calculate statistics for node %1, error: \"%2\"" ).arg( nodeId.toString(), request->errorStr() ) );
          }
        }
      }

      if ( !block.get() )
      {
        updateFeedback();
        return QgsPointCloudStatistics();
      }

      const QgsPointCloudAttributeCollection attributesCollection = block->attributes();
      const QVector<QgsPointCloudAttribute> attributes = attributesCollection.attributes();
      const char *ptr = block->data();
      int count = block->pointCount();
      int recordSize = attributesCollection.pointRecordSize();

      QMap<QString, QgsPointCloudAttributeStatistics> statsMap;
      for ( const QgsPointCloudAttribute &attribute : attributes )
      {
        QgsPointCloudAttributeStatistics summary;
        summary.minimum = std::numeric_limits<double>::max();
        summary.maximum = std::numeric_limits<double>::lowest();
        summary.count = 0;
        summary.mean = 0;
        summary.stDev = std::numeric_limits<double>::quiet_NaN();
        summary.classCount.clear();
        statsMap[ attribute.name() ] = std::move( summary );
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
             attribute.name() == QLatin1String( "PointSourceId" ) ||
             attribute.name() == QLatin1String( "Synthetic" ) ||
             attribute.name() == QLatin1String( "KeyPoint" ) ||
             attribute.name() == QLatin1String( "Withheld" ) ||
             attribute.name() == QLatin1String( "Overlap" ) )
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

          QgsPointCloudAttributeStatistics &stats = statsMap[ attributeName ];
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
      updateFeedback();
      return QgsPointCloudStatistics( count, statsMap );
    }
  private:
    QgsPointCloudIndex mIndex;
    QgsPointCloudRequest mRequest;
    QgsFeedback *mFeedback = nullptr;
    double mProgressValue = 0.0;

    void updateFeedback()
    {
      QMutexLocker locker( &sStatsProcessorFeedbackMutex );
      mFeedback->setProgress( mFeedback->progress() + mProgressValue );
    }
};

QMutex StatsProcessor::sStatsProcessorFeedbackMutex;

QgsPointCloudStatsCalculator::QgsPointCloudStatsCalculator( QgsPointCloudIndex index )
  : mIndex( index )
{

}

bool QgsPointCloudStatsCalculator::calculateStats( QgsFeedback *feedback, const QVector<QgsPointCloudAttribute> &attributes, qint64 pointsLimit )
{
  if ( !mIndex.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Unable to calculate statistics of an invalid index" ) );
    return false;
  }
  mRequest.setAttributes( attributes );

  qint64 pointCount = 0;
  QVector<QgsPointCloudNodeId> nodes;
  QQueue<QgsPointCloudNodeId> queue;
  queue.push_back( mIndex.root() );
  while ( !queue.empty() )
  {
    QgsPointCloudNode node = mIndex.getNode( queue.front() );
    queue.pop_front();
    if ( !mProcessedNodes.contains( node.id() ) )
      pointCount += node.pointCount();
    if ( pointsLimit != -1 && pointCount > pointsLimit )
      break;
    if ( !mProcessedNodes.contains( node.id() ) )
    {
      nodes.push_back( node.id() );
      mProcessedNodes.insert( node.id() );
    }
    for ( const QgsPointCloudNodeId &child : node.children() )
    {
      queue.push_back( child );
    }
  }

  feedback->setProgress( 0 );

  QVector<QgsPointCloudStatistics> list = QtConcurrent::blockingMapped( nodes, StatsProcessor( mIndex, mRequest, feedback, 100.0 / ( double )nodes.size() ) );

  for ( QgsPointCloudStatistics &s : list )
  {
    mStats.combineWith( s );
  }
  return !feedback->isCanceled() && mStats.sampledPointsCount() != 0;
}
