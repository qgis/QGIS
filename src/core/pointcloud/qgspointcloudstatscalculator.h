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
#include <QEventLoop>

#include "qgspointcloudrequest.h"
#include "qgsstatisticalsummary.h"
#include "qgspointcloudstatistics.h"

#define SIP_NO_FILE

class QgsPointCloudIndex;
class QgsPointCloudBlock;
class QgsPointCloudAttribute;
class IndexedPointCloudNode;
class QgsFeedback;

/**
 * \ingroup core
 * \class QgsPointCloudStatsCalculator
 *
 * \brief Class used to calculate statistics of a point cloud dataset.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudStatsCalculator : public QObject
{
    Q_OBJECT
  public:
    //! Constructor
    QgsPointCloudStatsCalculator( QgsPointCloudIndex *index );

    /**
     * Calculates the statistics of given attributes \a attributes up to new \a pointsLimit points
     * Note: the already calculated statistics are kept and another set of \a pointsLimit are processed
     */
    bool calculateStats( QgsFeedback *feedback, const QVector<QgsPointCloudAttribute> &attributes, qint64 pointsLimit = -1 );

    //! Returns the object containing the calculated statistics
    QgsPointCloudStatistics statistics() const { return mStats; }

  private:
    std::unique_ptr<QgsPointCloudIndex> mIndex = nullptr;

    QgsPointCloudStatistics mStats;
    QSet<IndexedPointCloudNode> mProcessedNodes;

    QgsPointCloudRequest mRequest;
};


#endif // QGSPOINTCLOUDSTATSCALCULATOR_H
