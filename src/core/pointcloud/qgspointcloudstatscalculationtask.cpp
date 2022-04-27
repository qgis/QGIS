/***************************************************************************
                         qgspointcloudstatscalculationtask.cpp
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

#include "qgspointcloudstatscalculationtask.h"

#include "qgspointcloudindex.h"
#include "qgspointcloudrenderer.h"

#include <QtConcurrent/QtConcurrent>

///@cond PRIVATE

struct StatsProcessor
{
    typedef QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics> result_type;

    StatsProcessor( QgsPointCloudIndex *index, QgsPointCloudRequest request )
      : mIndex( index ), mRequest( request )
    {

    }

    QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics> operator()( IndexedPointCloudNode node )
    {
      QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics> statsMap;
      for ( QgsPointCloudAttribute attribute : mRequest.attributes().attributes() )
      {
        QgsPointCloudStatsCalculator::AttributeStatistics summary;
        summary.minimum = std::numeric_limits<double>::max();
        summary.maximum = std::numeric_limits<double>::lowest();
        summary.count = 0;
        summary.classCount.clear();
        statsMap[ attribute.name() ] = summary;
      }

      std::unique_ptr<QgsPointCloudBlock> block( mIndex->nodeData( node, mRequest ) );

      const char *ptr = block->data();
      int count = block->pointCount();
      const QgsPointCloudAttributeCollection attributes = block->attributes();
      int recordSize = attributes.pointRecordSize();

      for ( int i = 0; i < count; ++i )
      {
        for ( QgsPointCloudAttribute attribute : attributes.attributes() )
        {
          double attributeValue = 0;
          int attributeOffset = 0;
          attributes.find( attribute.name(), attributeOffset );

          QgsPointCloudStatsCalculator::AttributeStatistics &stats = statsMap[ attribute.name() ];
          QgsPointCloudRenderContext::getAttribute( ptr, i * recordSize + attributeOffset, attribute.type(), attributeValue );
          stats.minimum = std::min( stats.minimum, attributeValue );
          stats.maximum = std::max( stats.maximum, attributeValue );
          stats.count++;
          if ( attribute.name() == QStringLiteral( "Classification" ) )
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
};


QgsPointCloudStatsCalculationTask::QgsPointCloudStatsCalculationTask( QgsPointCloudIndex *index, const QVector<QgsPointCloudAttribute> &attributes, const QVector<IndexedPointCloudNode> &nodes )
  : QgsTask( tr( "Generating attributes statistics" ) ), mIndex( index ), mNodes( nodes )
{
  mAttributesRequest.setAttributes( attributes );
}

bool QgsPointCloudStatsCalculationTask::run()
{
  mFuture = QtConcurrent::mapped( mNodes, StatsProcessor( mIndex, mAttributesRequest ) );
  mFutureWatcher.setFuture( mFuture );

  int lastPercent = 0;
  while ( mFuture.isRunning() )
  {
    QThread::msleep( 100 );
    int progressValue = mFutureWatcher.progressValue();
    int percent = 100 * ( ( double )progressValue - mFutureWatcher.progressMinimum() ) / ( mFutureWatcher.progressMaximum() - mFutureWatcher.progressMinimum() );
    if ( lastPercent != percent )
    {
      setProgress( percent );
      lastPercent = percent;
    }
    if ( isCanceled() )
    {
      mFuture.cancel();
      return false;
    }
  }
  return true;
}

/// @endcond
