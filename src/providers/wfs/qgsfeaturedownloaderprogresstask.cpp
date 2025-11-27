/***************************************************************************
    qgsfeaturedownloaderprogresstask.cpp
    ------------------------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeaturedownloaderprogresstask.h"
#include "moc_qgsfeaturedownloaderprogresstask.cpp"

#include "qgsapplication.h"

#include <QThreadPool>

QgsFeatureDownloaderProgressTask::QgsFeatureDownloaderProgressTask( const QString &description, long long totalCount )
  : QgsTask( description, QgsTask::CanCancel | QgsTask::CancelWithoutPrompt | QgsTask::Silent )
  , mTotalCount( totalCount )
{
}

bool QgsFeatureDownloaderProgressTask::run()
{
  QgsApplication::taskManager()->threadPool()->releaseThread();
  mNotFinishedMutex.lock();
  if ( !mAlreadyFinished )
  {
    mNotFinishedWaitCondition.wait( &mNotFinishedMutex );
  }
  mNotFinishedMutex.unlock();

  QgsApplication::taskManager()->threadPool()->reserveThread();
  return true;
}

void QgsFeatureDownloaderProgressTask::cancel()
{
  emit canceled();

  QgsTask::cancel();
}

void QgsFeatureDownloaderProgressTask::finalize()
{
  const QMutexLocker lock( &mNotFinishedMutex );
  mAlreadyFinished = true;

  mNotFinishedWaitCondition.wakeAll();
}

void QgsFeatureDownloaderProgressTask::setDownloaded( long long count )
{
  setProgress( static_cast<double>( count ) / static_cast<double>( mTotalCount ) * 100 );
}
