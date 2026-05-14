/***************************************************************************
  qgsrunnableprovidercreator.cpp - QgsRunnableProviderCreator

 ---------------------
 begin                : 20.3.2023
 copyright            : (C) 2023 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrunnableprovidercreator.h"

#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgsruntimeprofiler.h"
#include "qgsthreadingutils.h"

#include <QDebug>
#include <QString>
#include <QThread>

#include "moc_qgsrunnableprovidercreator.cpp"

using namespace Qt::StringLiterals;

QgsRunnableProviderCreator::QgsRunnableProviderCreator(
  const QString &layerId, const QString &providerKey, const QString &dataSource, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags
)
  : mLayerId( layerId )
  , mProviderKey( providerKey )
  , mDataSource( dataSource )
  , mOptions( options )
  , mFlags( flags )
{
  setAutoDelete( false );
}

void QgsRunnableProviderCreator::run()
{
  QgsScopedThreadName threadName( u"pcreate:%1"_s.arg( mProviderKey ) );
  QgsDebugMsgLevel( u"Creating provider in parallel for %1: %2 - %3 in thread %4"_s.arg( mLayerId, mProviderKey, mDataSource, QgsThreadingUtils::threadDescription( QThread::currentThread() ) ), 2 );

  // should use thread-local profiler
  QgsScopedRuntimeProfile profile( "Create data providers/" + mLayerId, u"projectload"_s );
  mDataProvider.reset( QgsProviderRegistry::instance()->createProvider( mProviderKey, mDataSource, mOptions, mFlags ) );

  QgsDebugMsgLevel( u"Created provider for %1: %2 - %3 belongs to thread %4"_s.arg( mLayerId, mProviderKey, mDataSource, QgsThreadingUtils::threadDescription( mDataProvider->thread() ) ), 2 );

  // detach from thread, so that the creator of QgsRunnableProviderCreator can "pull" the data provider
  // (we can't push it to a thread here, because we don't know what the target thread for the owner will be)
  mDataProvider->moveToThread( nullptr );

  emit providerCreated( mDataProvider->isValid(), mLayerId );
}

QgsDataProvider *QgsRunnableProviderCreator::dataProvider()
{
  return mDataProvider.release();
}
