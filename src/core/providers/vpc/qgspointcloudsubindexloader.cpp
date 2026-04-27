/***************************************************************************
    qgspointcloudsubindexloader.cpp
    ---------------------
    begin                : March 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudsubindexloader.h"

#include "qgscopcpointcloudindex.h"
#include "qgseptpointcloudindex.h"

#include <QString>
#include <QtConcurrentRun>

#include "moc_qgspointcloudsubindexloader.cpp"

using namespace Qt::StringLiterals;

QgsPointCloudSubIndexLoader::QgsPointCloudSubIndexLoader( const QString &uri, int id, bool emitDataChanged, QObject *parent )
  : QObject( parent )
  , mUri( uri )
  , mId( id )
  , mEmitDataChanged( emitDataChanged )
{
  mFutureWatcher = new QFutureWatcher<QgsPointCloudIndex>( this );
  connect( mFutureWatcher, &QFutureWatcher<QgsPointCloudIndex>::finished, this, &QgsPointCloudSubIndexLoader::onFutureFinished );
}

QgsPointCloudSubIndexLoader::~QgsPointCloudSubIndexLoader()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    disconnect( mFutureWatcher, &QFutureWatcher<QgsPointCloudIndex>::finished, this, &QgsPointCloudSubIndexLoader::onFutureFinished );
    mFutureWatcher->waitForFinished();
  }
}

void QgsPointCloudSubIndexLoader::start()
{
  const QFuture<QgsPointCloudIndex> future = QtConcurrent::run( loadSubIndex, mUri );
  mFutureWatcher->setFuture( future );
}

QgsPointCloudIndex QgsPointCloudSubIndexLoader::loadSubIndex( const QString &uri )
{
  QgsPointCloudIndex pc;
  if ( uri.endsWith( u"copc.laz"_s, Qt::CaseSensitivity::CaseInsensitive ) )
    pc = QgsPointCloudIndex( new QgsCopcPointCloudIndex() );
  else if ( uri.endsWith( u"ept.json"_s, Qt::CaseSensitivity::CaseInsensitive ) )
    pc = QgsPointCloudIndex( new QgsEptPointCloudIndex() );
  else
    return pc;

  pc.load( uri );
  return pc;
}

void QgsPointCloudSubIndexLoader::onFutureFinished()
{
  mIndex = mFutureWatcher->result();
  emit finished( mId );
}
