/***************************************************************************
    qgsmdaldataitems.cpp
    ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmdaldataitems.h"
#include "qgsmdalprovider.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QFileInfo>


QgsMdalLayerItem::QgsMdalLayerItem( QgsDataItem *parent,
                                    const QString &name, const QString &path, const QString &uri )
  : QgsLayerItem( parent, name, path, uri, QgsLayerItem::Mesh, QStringLiteral( "mdal" ) )
{
  mToolTip = uri;
  setState( Populated );
}

QString QgsMdalLayerItem::layerName() const
{
  QFileInfo info( name() );
  return info.completeBaseName();
}

// ---------------------------------------------------------------------------
static QStringList sExtensions = QStringList();

QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::File;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return nullptr;

  QgsDebugMsgLevel( "thePath = " + path, 2 );

  // get suffix, removing .gz if present
  QFileInfo info( path );
  QString suffix = info.suffix().toLower();
  // extract basename with extension
  info.setFile( path );
  QString name = info.fileName();

  // allow only normal files
  if ( !info.isFile() )
    return nullptr;

  // get supported extensions
  if ( sExtensions.isEmpty() )
  {
    // TODO ask MDAL for extensions !
    sExtensions << QStringLiteral( "2dm" )
                << QStringLiteral( "grb" )
                << QStringLiteral( "grb2" )
                << QStringLiteral( "bin" )
                << QStringLiteral( "grib" )
                << QStringLiteral( "grib1" )
                << QStringLiteral( "grib2" )
                << QStringLiteral( "nc" );
  }

  // Filter files by extension
  if ( !sExtensions.contains( suffix ) )
    return nullptr;

  return new QgsMdalLayerItem( parentItem, name, path, path );
}

