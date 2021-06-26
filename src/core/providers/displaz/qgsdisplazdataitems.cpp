/***************************************************************************
    QgsDisplazdataitems.cpp
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
#include "qgsdisplazdataitems.h"
#include "qgsdisplazprovider.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QFileInfo>
#include <mutex>


QgsDisplazLayerItem::QgsDisplazLayerItem( QgsDataItem *parent,
                                    const QString &name, const QString &path, const QString &uri )
  : QgsLayerItem( parent, name, path, uri, QgsLayerItem::PointCloud, QStringLiteral( "displaz" ) )
{
  mToolTip = uri;
  setState( Populated );
}

QString QgsDisplazLayerItem::layerName() const
{
  QFileInfo info( name() );
  return info.completeBaseName();
}

// ---------------------------------------------------------------------------
static QStringList sExtensions = QStringList();

 int dataCapabilities()
{
  return QgsDataProvider::File;
}

 QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
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

  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    QgsDisplazProvider::filePointCloudExtensions( sExtensions);
  } 
  ); //  sExtension 静态，只调用一次

  // Filter files by extension
  if ( !sExtensions.contains( suffix ) )
    return nullptr;

  return new QgsDisplazLayerItem( parentItem, name, path, path );
}


 // -------------------------wp--------------------------------------------------
 ///static QStringList sExtensions = QStringList();

 QString QgsDisplazDataItemProvider::name()
 {
	 return QStringLiteral("displaz");
 }

 int QgsDisplazDataItemProvider::capabilities() const
 {
	 return QgsDataProvider::File;
 }

 QgsDataItem *QgsDisplazDataItemProvider::createDataItem(const QString &path, QgsDataItem *parentItem)
 {
	 if (path.isEmpty())
		 return nullptr;

	 QgsDebugMsgLevel("thePath = " + path, 2);

	 // get suffix, removing .gz if present
	 QFileInfo info(path);
	 QString suffix = info.suffix().toLower();
	 // extract basename with extension
	 info.setFile(path);
	 QString name = info.fileName();

	 // allow only normal files
	 if (!info.isFile())
		 return nullptr;

	 static std::once_flag initialized;
	 std::call_once(initialized, [=]()
	 {
		// QStringList meshExtensions;
		 //QStringList datasetsExtensions;
		 QgsDisplazProvider::filePointCloudExtensions(sExtensions);
		 
	 });

	 // Filter files by extension
	 if (!sExtensions.contains(suffix))
		 return nullptr;

	 return new QgsDisplazLayerItem(parentItem, name, path, path);
 }
