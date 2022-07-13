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

QgsPointCloudStatsCalculationTask::QgsPointCloudStatsCalculationTask( QgsPointCloudIndex *index, const QVector<QgsPointCloudAttribute> &attributes, qint64 pointLimit )
  : QgsTask( tr( "Generating attributes statistics" ) )
  , mCalculator( index )
  , mAttributes( attributes )
  , mPointLimit( pointLimit )
{
  mFeedback = new QgsFeedback( this );
}

bool QgsPointCloudStatsCalculationTask::run()
{
  connect( mFeedback, &QgsFeedback::progressChanged, this, &QgsPointCloudStatsCalculationTask::setProgress );
  return mCalculator.calculateStats( mFeedback, mAttributes, mPointLimit );
}

void QgsPointCloudStatsCalculationTask::cancel()
{
  mFeedback->cancel();
  QgsTask::cancel();
}

QgsPointCloudStatistics QgsPointCloudStatsCalculationTask::calculationResults() const
{
  return mCalculator.statistics();
}

/// @endcond
