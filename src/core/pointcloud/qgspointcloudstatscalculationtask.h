/***************************************************************************
                         qgspointcloudstatscalculationtask.h
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

#ifndef QGSPOINTCLOUDSTATSCALCULATIONTASK_H
#define QGSPOINTCLOUDSTATSCALCULATIONTASK_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QObject>

#include "qgstaskmanager.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudstatscalculator.h"

#define SIP_NO_FILE

class QgsPointCloudStatsCalculationTask : public QgsTask
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

/// @endcond

#endif // QGSPOINTCLOUDSTATSCALCULATIONTASK_H
