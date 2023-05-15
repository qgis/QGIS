/***************************************************************************
    qgsogrconnpool.cpp
    ---------------------
    begin                : May 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrconnpool.h"
///@cond PRIVATE

#include "qgslogger.h"


void QgsOgrConnPoolGroup::connectionCreate( const QString &connectionInfo, QgsOgrConn *&connection )
{
  connection = new QgsOgrConn;

  const QVariantMap parts = QgsOgrProviderMetadata().decodeUri( connectionInfo );
  const QString fullPath = parts.value( QStringLiteral( "vsiPrefix" ) ).toString()
                           + parts.value( QStringLiteral( "path" ) ).toString()
                           + parts.value( QStringLiteral( "vsiSuffix" ) ).toString();
  const QStringList openOptions = parts.value( QStringLiteral( "openOptions" ) ).toStringList();
  char **papszOpenOptions = nullptr;
  for ( const QString &option : std::as_const( openOptions ) )
  {
    papszOpenOptions = CSLAddString( papszOpenOptions,
                                     option.toUtf8().constData() );
  }
  connection->ds = QgsOgrProviderUtils::GDALOpenWrapper( fullPath.toUtf8().constData(), false, papszOpenOptions, nullptr );
  CSLDestroy( papszOpenOptions );
  connection->path = connectionInfo;
  connection->valid = true;
}

void QgsOgrConnPoolGroup::connectionDestroy( QgsOgrConn *connection )
{
  destroyOgrConn( connection );
}

void QgsOgrConnPoolGroup::invalidateConnection( QgsOgrConn *connection )
{
  connection->valid = false;
}

bool QgsOgrConnPoolGroup::connectionIsValid( QgsOgrConn *connection )
{
  return connection->valid;
}

QgsOgrConnPool *QgsOgrConnPool::sInstance = nullptr;

// static public
QgsOgrConnPool *QgsOgrConnPool::instance()
{
  if ( ! sInstance ) sInstance = new QgsOgrConnPool();
  return sInstance;
}

// static public
void QgsOgrConnPool::cleanupInstance()
{
  delete sInstance;
  sInstance = nullptr;
}

QgsOgrConnPool::QgsOgrConnPool() : QgsConnectionPool<QgsOgrConn *, QgsOgrConnPoolGroup>()
{
  QgsDebugCall;
}

QgsOgrConnPool::~QgsOgrConnPool()
{
  QgsDebugCall;
}

QString QgsOgrConnPool::connectionToName( QgsOgrConn *connection )
{
  return connection->path;
}


///@endcond
