/***************************************************************************
                         qgspointclouddataitems.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointclouddataitems.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QFileInfo>
#include <mutex>

///@cond PRIVATE

QgsPointCloudLayerItem::QgsPointCloudLayerItem( QgsDataItem *parent,
    const QString &name, const QString &path, const QString &uri )
  : QgsLayerItem( parent, name, path, uri, QgsLayerItem::PointCloud, QStringLiteral( "pointcloud" ) )
{
  mToolTip = uri;
  setState( Populated );
}

QString QgsPointCloudLayerItem::layerName() const
{
  QFileInfo info( name() );
  return info.completeBaseName();
}

// ---------------------------------------------------------------------------
QString QgsPointCloudDataItemProvider::name()
{
  return QStringLiteral( "pointcloud" );
}

int QgsPointCloudDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsPointCloudDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return nullptr;

  QgsDebugMsgLevel( "thePath = " + path, 2 );

  // get suffix, removing .gz if present
  QFileInfo info( path );
  info.setFile( path );
  QString name = info.fileName();

  // allow only normal files
  if ( !info.isFile() )
    return nullptr;

  // Filter files by extension
  if ( !path.endsWith( QStringLiteral( "ept.json" ) ) )
    return nullptr;

  return new QgsPointCloudLayerItem( parentItem, name, path, path );
}

///@endcond
