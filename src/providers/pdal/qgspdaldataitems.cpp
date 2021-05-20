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
#include "qgsfileutils.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

#include <QFileInfo>

QgsPdalLayerItem::QgsPdalLayerItem( QgsDataItem *parent,
                                    const QString &name, const QString &path, const QString &uri )
  : QgsLayerItem( parent, name, path, uri, Qgis::BrowserLayerType::PointCloud, QStringLiteral( "pdal" ) )
{
  mToolTip = uri;
  setState( Qgis::BrowserItemState::Populated );
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
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "pdal" ) );
  mFileFilter = metadata->filters( QgsProviderMetadata::FilterType::FilterPointCloud );
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

  QFileInfo info( path );

  // allow only normal files
  if ( !info.isFile() )
    return nullptr;

  // Filter files by extension
  if ( !QgsFileUtils::fileMatchesFilter( path, mFileFilter ) )
    return nullptr;

  const QString name = info.fileName();

  return new QgsPdalLayerItem( parentItem, name, path, path );
}
