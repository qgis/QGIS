/***************************************************************************
                         qgspointcloudstatscalculator.h
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

#ifndef QGSPOINTCLOUDSTATSCALCULATOR_H
#define QGSPOINTCLOUDSTATSCALCULATOR_H

#include "qgis_core.h"

#include <QVariant>
#include <QSet>
#include <QFuture>
#include <QFutureWatcher>

#include "qgspointcloudrequest.h"

class QgsPointCloudIndex;
class QgsPointCloudBlock;
class QgsPointCloudAttribute;
class IndexedPointCloudNode;

#include "qgstaskmanager.h"

class QgsPointCloudStatsCalculationTask;

class CORE_EXPORT QgsPointCloudStatsCalculator : public QObject
{
    Q_OBJECT
  public:
    struct AttributeStatistics
    {
      double minimum = std::numeric_limits<double>::max();
      double maximum = std::numeric_limits<double>::lowest();
      int count = 0;
      QMap<int, int> classCount;

      void cumulateStatistics( const AttributeStatistics &stats )
      {
        minimum = std::min( minimum, stats.minimum );
        maximum = std::min( maximum, stats.maximum );
        count += stats.count;
      }
    };

    //! Constructor
    QgsPointCloudStatsCalculator( QgsPointCloudIndex *index );

    //! Clears the statistics of all attributes
    void clear();

    //! Clears the statistics of given attributes \a attributes
    void clear( const QVector<QgsPointCloudAttribute> &attributes );

    /**
     * Calculates the statistics of given attributes \a attributes up to new \a pointsLimit points
     * Note: the already calculated statistics are kept and another set of \a pointsLimit are processed
     */
    void calculateStats( const QVector<QgsPointCloudAttribute> &attributes, qint64 pointsLimit = -1 );

    /**
     * Returns the calculated statistics of each attribute processed
     */
    QMap<QString, AttributeStatistics> statistics() const { return mStatisticsMap; }

    /**
     * Returns the calculated statistics of attribute \a attribute
     */
    AttributeStatistics statisticsOf( const QString &attribute );
  signals:
    void statisticsCalculated();
  private slots:
    void statsCalculationFinished();
  private:
    QgsPointCloudIndex *mIndex = nullptr;

    QMap<QString, AttributeStatistics> mStatisticsMap;
    QSet<IndexedPointCloudNode> mProcessedNodes;

    long mStatsCalculationTaskId = 0;
    QgsPointCloudRequest mRequest;
};

class CORE_EXPORT QgsPointCloudStatsCalculationTask : public QgsTask
{
    Q_OBJECT

  public:
    QgsPointCloudStatsCalculationTask( QgsPointCloudIndex *index, const QVector<QgsPointCloudAttribute> &attributes, const QVector<IndexedPointCloudNode> &nodes );

    bool run() override;

    QgsPointCloudIndex *mIndex = nullptr;
    QVector<IndexedPointCloudNode> mNodes;
    QgsPointCloudRequest mAttributesRequest;
    QFuture<QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics>> mFuture;
    QFutureWatcher<QMap<QString, QgsPointCloudStatsCalculator::AttributeStatistics>> mFutureWatcher;
};

#endif // QGSPOINTCLOUDSTATSCALCULATOR_H
