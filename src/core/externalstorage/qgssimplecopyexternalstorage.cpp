/***************************************************************************
  qgssimplecopyexternalstorage.cpp
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssimplecopyexternalstorage_p.h"

#include "qgscopyfiletask.h"
#include "qgsapplication.h"

#include <QFileInfo>

///@cond PRIVATE

QgsSimpleCopyExternalStorageStoredContent::QgsSimpleCopyExternalStorageStoredContent( const QString &filePath, const QString &url, const QString &authcfg )
{
  Q_UNUSED( authcfg );

  mCopyTask = new QgsCopyFileTask( filePath, url );

  connect( mCopyTask, &QgsTask::taskCompleted, this, [ = ]
  {
    mUrl = mCopyTask->destination();
    mStatus = Qgis::ContentStatus::Finished;
    emit stored();
  } );

  connect( mCopyTask, &QgsTask::taskTerminated, this, [ = ]
  {
    reportError( mCopyTask->errorString() );
  } );

  connect( mCopyTask, &QgsTask::progressChanged, this, [ = ]( double progress )
  {
    emit progressChanged( progress );
  } );
}

void QgsSimpleCopyExternalStorageStoredContent::store()
{
  mStatus = Qgis::ContentStatus::Running;
  QgsApplication::taskManager()->addTask( mCopyTask );
}

void QgsSimpleCopyExternalStorageStoredContent::cancel()
{
  if ( !mCopyTask )
    return;

  disconnect( mCopyTask, &QgsTask::taskTerminated, this, nullptr );
  connect( mCopyTask, &QgsTask::taskTerminated, this, [ = ]
  {
    mStatus = Qgis::ContentStatus::Canceled;
    emit canceled();
  } );

  mCopyTask->cancel();
}

QString QgsSimpleCopyExternalStorageStoredContent::url() const
{
  return mUrl;
}

QgsSimpleCopyExternalStorageFetchedContent::QgsSimpleCopyExternalStorageFetchedContent( const QString &filePath )
  : mFilePath( filePath )
{
}

void QgsSimpleCopyExternalStorageFetchedContent::fetch()
{
  // no fetching process, we read directly from its location
  if ( !QFileInfo::exists( mFilePath ) )
  {
    reportError( tr( "File '%1' does not exist" ).arg( mFilePath ) );
  }
  else
  {
    mStatus = Qgis::ContentStatus::Finished;
    mResultFilePath = mFilePath;
    emit fetched();
  }
}

QString QgsSimpleCopyExternalStorageFetchedContent::filePath() const
{
  return mResultFilePath;
}

QString QgsSimpleCopyExternalStorage::type() const
{
  return QStringLiteral( "SimpleCopy" );
};

QString QgsSimpleCopyExternalStorage::displayName() const
{
  return QObject::tr( "Simple copy" );
};

QgsExternalStorageStoredContent *QgsSimpleCopyExternalStorage::doStore( const QString &filePath, const QString &url, const QString &authcfg ) const
{
  return new QgsSimpleCopyExternalStorageStoredContent( filePath, url, authcfg );
};

QgsExternalStorageFetchedContent *QgsSimpleCopyExternalStorage::doFetch( const QString &url, const QString &authConfig ) const
{
  Q_UNUSED( authConfig );

  return new QgsSimpleCopyExternalStorageFetchedContent( url );
}

///@endcond
