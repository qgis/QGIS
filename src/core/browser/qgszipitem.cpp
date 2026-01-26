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

#include <cpl_string.h>
#include <cpl_vsi.h>
#include <mutex>

#include "qgsapplication.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsgdalutils.h"
#include "qgssettings.h"

#include <QFileInfo>

#include "moc_qgszipitem.cpp"

QIcon QgsZipItem::iconZip()
{
  return QgsApplication::getThemeIcon( u"/mIconZip.svg"_s );
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
  mIconName = u"/mIconZip.svg"_s;
  mVsiPrefix = QgsGdalUtils::vsiPrefixForPath( mFilePath );

  setCapabilities( capabilities2() | Qgis::BrowserItemCapability::ItemRepresentsFile );

  static std::once_flag initialized;
  std::call_once( initialized, []
  {
    sProviderNames << u"files"_s;
  } );
}

bool QgsZipItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::UriList QgsZipItem::mimeUris() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = u"collection"_s;
  u.uri = path();
  u.filePath = path();
  return { u };
}

QString QgsZipItem::vsiPrefix( const QString &uri )
{
  return QgsGdalUtils::vsiPrefixForPath( uri );
}

QVector<QgsDataItem *> QgsZipItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QString tmpPath;
  const QgsSettings settings;
  const QString scanZipSetting = settings.value( u"qgis/scanZipInBrowser2"_s, "basic" ).toString();

  mZipFileList.clear();

  QgsDebugMsgLevel( u"mFilePath = %1 path = %2 name= %3 scanZipSetting= %4 vsiPrefix= %5"_s.arg( mFilePath, path(), name(), scanZipSetting, mVsiPrefix ), 3 );

  // if scanZipBrowser == no: skip to the next file
  if ( scanZipSetting == "no"_L1 )
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
      if ( provider->name() == "OGR"_L1 )
      {
        if ( info.suffix().compare( "dbf"_L1, Qt::CaseInsensitive ) == 0 )
        {
          if ( mZipFileList.indexOf( fileName.left( fileName.count() - 4 ) + ".shp" ) != -1 )
            continue;
        }
        if ( info.completeSuffix().compare( "shp.xml"_L1, Qt::CaseInsensitive ) == 0 )
        {
          continue;
        }
      }

      QgsDebugMsgLevel( u"trying to load item %1 with %2"_s.arg( tmpPath, provider->name() ), 3 );
      QgsDataItem *item = provider->createDataItem( tmpPath, this );
      if ( item )
      {
        // the item comes with zipped file name, set the name to relative path within zip file
        item->setName( fileName );
        children.append( item );
      }
      else
      {
        QgsDebugMsgLevel( u"not loaded item"_s, 3 );
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
  const QString scanZipSetting = settings.value( u"qgis/scanZipInBrowser2"_s, "basic" ).toString();
  QStringList zipFileList;
  const QString vsiPrefix = QgsGdalUtils::vsiPrefixForPath( filePath );
  bool populated = false;

  QgsDebugMsgLevel( u"path = %1 name= %2 scanZipSetting= %3 vsiPrefix= %4"_s.arg( path, name, scanZipSetting, vsiPrefix ), 3 );

  // don't scan if scanZipBrowser == no
  if ( scanZipSetting == "no"_L1 )
    return nullptr;

  // don't scan if this file is not a vsi container archive item
  if ( !QgsGdalUtils::isVsiArchivePrefix( vsiPrefix ) )
    return nullptr;

  auto zipItem = std::make_unique< QgsZipItem >( parent, name, filePath, path );
  // force populate zipItem if it has less than 10 items and is not a .tgz or .tar.gz file (slow loading)
  // for other items populating will be delayed until item is opened
  // this might be polluting the tree with empty items but is necessary for performance reasons
  // could also accept all files smaller than a certain size and add options for file count and/or size

  // first get list of files inside .zip or .tar files
  if ( path.endsWith( ".zip"_L1, Qt::CaseInsensitive ) ||
       path.endsWith( ".tar"_L1, Qt::CaseInsensitive ) )
  {
    zipFileList = zipItem->getZipFileList();
  }
  // force populate if less than 10 items
  if ( !zipFileList.isEmpty() && zipFileList.count() <= 10 )
  {
    zipItem->populate( zipItem->createChildren() );
    populated = true; // there is no QgsDataItem::isPopulated() function
    QgsDebugMsgLevel( u"Got zipItem with %1 children, path=%2, name=%3"_s.arg( zipItem->rowCount() ).arg( zipItem->path(), zipItem->name() ), 3 );
  }
  else
  {
    QgsDebugMsgLevel( u"Delaying populating zipItem with path=%1, name=%2"_s.arg( zipItem->path(), zipItem->name() ), 3 );
  }

  // only display if has children or if is not populated
  if ( !populated || zipItem->rowCount() > 0 )
  {
    QgsDebugMsgLevel( u"returning zipItem"_s, 3 );
    return zipItem.release();
  }

  return nullptr;
}

QStringList QgsZipItem::getZipFileList()
{
  if ( ! mZipFileList.isEmpty() )
    return mZipFileList;

  QString tmpPath;
  const QgsSettings settings;
  const QString scanZipSetting = settings.value( u"qgis/scanZipInBrowser2"_s, "basic" ).toString();

  QgsDebugMsgLevel( u"mFilePath = %1 name= %2 scanZipSetting= %3 vsiPrefix= %4"_s.arg( mFilePath, name(), scanZipSetting, mVsiPrefix ), 3 );

  // if scanZipBrowser == no: skip to the next file
  if ( scanZipSetting == "no"_L1 )
  {
    return mZipFileList;
  }

  // get list of files inside zip file
  QgsDebugMsgLevel( u"Open file %1 with gdal vsi"_s.arg( mVsiPrefix + mFilePath ), 3 );
  char **papszSiblingFiles = VSIReadDirRecursive( QString( mVsiPrefix + mFilePath ).toUtf8().constData() );
  if ( papszSiblingFiles )
  {
    for ( int i = 0; papszSiblingFiles[i]; i++ )
    {
      tmpPath = papszSiblingFiles[i];
      QgsDebugMsgLevel( u"Read file %1"_s.arg( tmpPath ), 3 );
      // skip directories (files ending with /)
      if ( tmpPath.right( 1 ) != "/"_L1 )
        mZipFileList << tmpPath;
    }
    CSLDestroy( papszSiblingFiles );
  }
  else
  {
    QgsDebugError( u"Error reading %1"_s.arg( mFilePath ) );
  }

  return mZipFileList;
}


