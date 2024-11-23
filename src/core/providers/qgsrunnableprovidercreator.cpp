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
#include "moc_qgsrunnableprovidercreator.cpp"

#include <QDebug>
#include <QThread>

#include "qgsproviderregistry.h"
#include "qgsruntimeprofiler.h"

QgsRunnableProviderCreator::QgsRunnableProviderCreator( const QString &layerId, const QString &providerKey, const QString &dataSource, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
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
  // should use thread-local profiler
  QgsScopedRuntimeProfile profile( "Create data providers/" + mLayerId, QStringLiteral( "projectload" ) );
  mDataProvider.reset( QgsProviderRegistry::instance()->createProvider( mProviderKey, mDataSource, mOptions, mFlags ) );
  mDataProvider->moveToThread( QObject::thread() );
  emit providerCreated( mDataProvider->isValid(), mLayerId );
}

QgsDataProvider *QgsRunnableProviderCreator::dataProvider()
{
  return mDataProvider.release();
}
