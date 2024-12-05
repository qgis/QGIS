/***************************************************************************
   qgshanaconnectionpool.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanaconnection.h"
#include "qgshanaconnectionpool.h"
#include "moc_qgshanaconnectionpool.cpp"
#include "qgshanasettings.h"
#include "qgshanautils.h"
#include "qgslogger.h"

QgsHanaConnectionPoolGroup::QgsHanaConnectionPoolGroup( const QString &name )
  : QgsConnectionPoolGroup<QgsHanaConnection *>( name )
{
  initTimer( this );
}

QBasicMutex QgsHanaConnectionPool::sMutex;
std::shared_ptr<QgsHanaConnectionPool> QgsHanaConnectionPool::sInstance;

QgsHanaConnection *QgsHanaConnectionPool::getConnection( const QString &connInfo )
{
  std::shared_ptr<QgsConnectionPool> instance;
  {
    QMutexLocker lock( &sMutex );
    if ( !sInstance )
      sInstance = std::shared_ptr<QgsHanaConnectionPool>( new QgsHanaConnectionPool() );
    instance = sInstance;
  }
  return instance->acquireConnection( connInfo );
}

void QgsHanaConnectionPool::returnConnection( QgsHanaConnection *conn )
{
  QMutexLocker lock( &sMutex );
  if ( sInstance )
    sInstance->releaseConnection( conn );
  else
    qgsConnectionPool_ConnectionDestroy( conn );
}

void QgsHanaConnectionPool::cleanupInstance()
{
  QMutexLocker lock( &sMutex );
  if ( sInstance )
    sInstance.reset();
}

QgsHanaConnectionPool::QgsHanaConnectionPool()
  : QgsConnectionPool<QgsHanaConnection *, QgsHanaConnectionPoolGroup>()
{
  QgsDebugCall;
}

QgsHanaConnectionPool::~QgsHanaConnectionPool()
{
  QgsDebugCall;
}

QgsHanaConnectionRef::QgsHanaConnectionRef( const QgsDataSourceUri &uri )
{
  mConnection = std::unique_ptr<QgsHanaConnection>(
    QgsHanaConnectionPool::getConnection( QgsHanaUtils::connectionInfo( uri ) )
  );
}

QgsHanaConnectionRef::QgsHanaConnectionRef( const QString &name )
{
  QgsHanaSettings settings( name, true );
  mConnection = std::unique_ptr<QgsHanaConnection>(
    QgsHanaConnectionPool::getConnection( QgsHanaUtils::connectionInfo( settings.toDataSourceUri() ) )
  );
}

QgsHanaConnectionRef::~QgsHanaConnectionRef()
{
  if ( mConnection )
    QgsHanaConnectionPool::returnConnection( mConnection.release() );
}
