/***************************************************************************
                         qgseptdataitems.cpp
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

#include "qgseptdataitems.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsfileutils.h"

#include <QFileInfo>
#include <mutex>

///@cond PRIVATE

QgsEptLayerItem::QgsEptLayerItem( QgsDataItem *parent,
                                  const QString &name, const QString &path, const QString &uri )
  : QgsLayerItem( parent, name, path, uri, Qgis::BrowserLayerType::PointCloud, QStringLiteral( "ept" ) )
{
  mToolTip = uri;
  setState( Qgis::BrowserItemState::Populated );
}

QString QgsEptLayerItem::layerName() const
{
  QFileInfo info( name() );
  return info.completeBaseName();
}

// ---------------------------------------------------------------------------
QgsEptDataItemProvider::QgsEptDataItemProvider()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ept" ) );
  mFileFilter = metadata->filters( QgsProviderMetadata::FilterType::FilterPointCloud );
}

QString QgsEptDataItemProvider::name()
{
  return QStringLiteral( "ept" );
}

int QgsEptDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsEptDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return nullptr;

  QgsDebugMsgLevel( "thePath = " + path, 2 );

  const QFileInfo info( path );

  // allow only normal files
  if ( !info.isFile() )
    return nullptr;

  // Filter files by extension
  if ( !QgsFileUtils::fileMatchesFilter( path, mFileFilter ) )
    return nullptr;

  QString name = info.dir().dirName();

  return new QgsEptLayerItem( parentItem, name, path, path );
}

///@endcond
