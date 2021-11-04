/***************************************************************************
                             qgszipitem.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgszipitem.h"
#include "qgsapplication.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include <QFileInfo>

#include <cpl_vsi.h>
#include <cpl_string.h>
#include <mutex>

QIcon QgsZipItem::iconZip()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconZip.svg" ) );
}


//-----------------------------------------------------------------------
QStringList QgsZipItem::sProviderNames = QStringList();


QgsZipItem::QgsZipItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mFilePath = path;
  init();
}

QgsZipItem::QgsZipItem( QgsDataItem *parent, const QString &name,
                        const QString &filePath, const QString &path,
                        const QString &providerKey )
  : QgsDataCollectionItem( parent, name, path, providerKey )
  , mFilePath( filePath )
{
  init();
}

void QgsZipItem::init()
{
  mType = Qgis::BrowserItemType::Collection; //Zip??
  mIconName = QStringLiteral( "/mIconZip.svg" );
  mVsiPrefix = vsiPrefix( mFilePath );

  setCapabilities( capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );

  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {
    sProviderNames << QStringLiteral( "files" );
  } );
}

bool QgsZipItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::UriList QgsZipItem::mimeUris() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "collection" );
  u.uri = path();
  u.filePath = path();
  return { u };
}

QVector<QgsDataItem *> QgsZipItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QString tmpPath;
  const QgsSettings settings;
  const QString scanZipSetting = settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString();

  mZipFileList.clear();

  QgsDebugMsgLevel( QStringLiteral( "mFilePath = %1 path = %2 name= %3 scanZipSetting= %4 vsiPrefix= %5" ).arg( mFilePath, path(), name(), scanZipSetting, mVsiPrefix ), 3 );

  // if scanZipBrowser == no: skip to the next file
  if ( scanZipSetting == QLatin1String( "no" ) )
  {
    return children;
  }

  // first get list of files
  getZipFileList();

  const QList<QgsDataItemProvider *> providers = QgsApplication::dataItemProviderRegistry()->providers();

  // loop over files inside zip
  const auto constMZipFileList = mZipFileList;
  for ( const QString &fileName : constMZipFileList )
  {
    const QFileInfo info( fileName );
    tmpPath = mVsiPrefix + mFilePath + '/' + fileName;
    QgsDebugMsgLevel( "tmpPath = " + tmpPath, 3 );

    for ( QgsDataItemProvider *provider : providers )
    {
      if ( !sProviderNames.contains( provider->name() ) )
        continue;

      // ugly hack to remove .dbf file if there is a .shp file
      if ( provider->name() == QLatin1String( "OGR" ) )
      {
        if ( info.suffix().compare( QLatin1String( "dbf" ), Qt::CaseInsensitive ) == 0 )
        {
          if ( mZipFileList.indexOf( fileName.left( fileName.count() - 4 ) + ".shp" ) != -1 )
            continue;
        }
        if ( info.completeSuffix().compare( QLatin1String( "shp.xml" ), Qt::CaseInsensitive ) == 0 )
        {
          continue;
        }
      }

      QgsDebugMsgLevel( QStringLiteral( "trying to load item %1 with %2" ).arg( tmpPath, provider->name() ), 3 );
      QgsDataItem *item = provider->createDataItem( tmpPath, this );
      if ( item )
      {
        // the item comes with zipped file name, set the name to relative path within zip file
        item->setName( fileName );
        children.append( item );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "not loaded item" ), 3 );
      }
    }
  }

  return children;
}

QgsDataItem *QgsZipItem::itemFromPath( QgsDataItem *parent, const QString &path, const QString &name )
{
  return itemFromPath( parent, path, name, path );
}

QgsDataItem *QgsZipItem::itemFromPath( QgsDataItem *parent, const QString &filePath, const QString &name, const QString &path )
{
  const QgsSettings settings;
  const QString scanZipSetting = settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString();
  QStringList zipFileList;
  const QString vsiPrefix = QgsZipItem::vsiPrefix( filePath );
  QgsZipItem *zipItem = nullptr;
  bool populated = false;

  QgsDebugMsgLevel( QStringLiteral( "path = %1 name= %2 scanZipSetting= %3 vsiPrefix= %4" ).arg( path, name, scanZipSetting, vsiPrefix ), 3 );

  // don't scan if scanZipBrowser == no
  if ( scanZipSetting == QLatin1String( "no" ) )
    return nullptr;

  // don't scan if this file is not a /vsizip/ or /vsitar/ item
  if ( ( vsiPrefix != QLatin1String( "/vsizip/" ) && vsiPrefix != QLatin1String( "/vsitar/" ) ) )
    return nullptr;

  zipItem = new QgsZipItem( parent, name, filePath, path );

  if ( zipItem )
  {
    // force populate zipItem if it has less than 10 items and is not a .tgz or .tar.gz file (slow loading)
    // for other items populating will be delayed until item is opened
    // this might be polluting the tree with empty items but is necessary for performance reasons
    // could also accept all files smaller than a certain size and add options for file count and/or size

    // first get list of files inside .zip or .tar files
    if ( path.endsWith( QLatin1String( ".zip" ), Qt::CaseInsensitive ) ||
         path.endsWith( QLatin1String( ".tar" ), Qt::CaseInsensitive ) )
    {
      zipFileList = zipItem->getZipFileList();
    }
    // force populate if less than 10 items
    if ( !zipFileList.isEmpty() && zipFileList.count() <= 10 )
    {
      zipItem->populate( zipItem->createChildren() );
      populated = true; // there is no QgsDataItem::isPopulated() function
      QgsDebugMsgLevel( QStringLiteral( "Got zipItem with %1 children, path=%2, name=%3" ).arg( zipItem->rowCount() ).arg( zipItem->path(), zipItem->name() ), 3 );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Delaying populating zipItem with path=%1, name=%2" ).arg( zipItem->path(), zipItem->name() ), 3 );
    }
  }

  // only display if has children or if is not populated
  if ( zipItem && ( !populated || zipItem->rowCount() > 0 ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "returning zipItem" ), 3 );
    return zipItem;
  }

  return nullptr;
}

QStringList QgsZipItem::getZipFileList()
{
  if ( ! mZipFileList.isEmpty() )
    return mZipFileList;

  QString tmpPath;
  const QgsSettings settings;
  const QString scanZipSetting = settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString();

  QgsDebugMsgLevel( QStringLiteral( "mFilePath = %1 name= %2 scanZipSetting= %3 vsiPrefix= %4" ).arg( mFilePath, name(), scanZipSetting, mVsiPrefix ), 3 );

  // if scanZipBrowser == no: skip to the next file
  if ( scanZipSetting == QLatin1String( "no" ) )
  {
    return mZipFileList;
  }

  // get list of files inside zip file
  QgsDebugMsgLevel( QStringLiteral( "Open file %1 with gdal vsi" ).arg( mVsiPrefix + mFilePath ), 3 );
  char **papszSiblingFiles = VSIReadDirRecursive( QString( mVsiPrefix + mFilePath ).toLocal8Bit().constData() );
  if ( papszSiblingFiles )
  {
    for ( int i = 0; papszSiblingFiles[i]; i++ )
    {
      tmpPath = papszSiblingFiles[i];
      QgsDebugMsgLevel( QStringLiteral( "Read file %1" ).arg( tmpPath ), 3 );
      // skip directories (files ending with /)
      if ( tmpPath.right( 1 ) != QLatin1String( "/" ) )
        mZipFileList << tmpPath;
    }
    CSLDestroy( papszSiblingFiles );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Error reading %1" ).arg( mFilePath ) );
  }

  return mZipFileList;
}


