/***************************************************************************
                         qgspdaldataitems.cpp
                         --------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspdaldataitems.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QFileInfo>
#include <mutex>

QgsPdalLayerItem::QgsPdalLayerItem( QgsDataItem *parent,
                                    const QString &name, const QString &path, const QString &uri )
  : QgsLayerItem( parent, name, path, uri, QgsLayerItem::PointCloud, QStringLiteral( "pdal" ) )
{
  mToolTip = uri;
  mIconName = QStringLiteral( "mIconPdalLayer.svg" );
  setState( Populated );
}

QString QgsPdalLayerItem::layerName() const
{
  QFileInfo info( name() );
  return info.completeBaseName();
}

// ---------------------------------------------------------------------------
QgsPdalDataItemProvider::QgsPdalDataItemProvider():
  QgsDataItemProvider()
{
}

QString QgsPdalDataItemProvider::name()
{
  return QStringLiteral( "pdal" );
}

int QgsPdalDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsPdalDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return nullptr;

  QgsDebugMsgLevel( "thePath = " + path, 2 );

  // get suffix, removing .gz if present
  QFileInfo info( path );
  info.setFile( path );

  // allow only normal files
  if ( !info.isFile() )
    return nullptr;

  // Filter files by extension
  // TODO get file filter list from PDAL library
  if ( !path.endsWith( QStringLiteral( "laz" ) ) )
    return nullptr;

  QString name = info.baseName() + QStringLiteral( ".laz" );

  return new QgsPdalLayerItem( parentItem, name, path, path );
}
