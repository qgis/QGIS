/***************************************************************************
                              qgsconfigcache.cpp
                              ------------------
  begin                : July 24th, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfigcache.h"
#include "qgsmapserverlogger.h"
#include "qgsprojectparser.h"
#include "qgssldparser.h"

QgsConfigCache::QgsConfigCache()
{
}

QgsConfigCache::~QgsConfigCache()
{
  QMap<QString, QgsConfigParser*>::iterator configIt = mCachedConfigurations.begin();
  for( ; configIt != mCachedConfigurations.end(); ++configIt )
  {
    delete configIt.value();
  }
}

QgsConfigParser* QgsConfigCache::searchConfiguration( const QString& filePath )
{
  QMap<QString, QgsConfigParser*>::const_iterator configIt = mCachedConfigurations.find( filePath );
  if( configIt == mCachedConfigurations.constEnd() )
  {
    QgsMSDebugMsg("Create new configuration")
    return insertConfiguration( filePath );
  }
  else
  {
    QgsMSDebugMsg("Return configuration from cash")
    return configIt.value();
  }
}

QgsConfigParser* QgsConfigCache::insertConfiguration( const QString& filePath )
{
  if( mCachedConfigurations.size() > 40 )
  {
      //remove 10 elements to avoid memory problems
      QMap<QString, QgsConfigParser*>::iterator configIt = mCachedConfigurations.begin();
      for( int i = 0; i < 10; ++i )
      {
        configIt = mCachedConfigurations.erase( configIt );
      }
  }

  //first open file
  QFile* configFile = new QFile( filePath );
  if( !configFile->exists() || !configFile->open( QIODevice::ReadOnly ) )
  {
    delete configFile;
    return 0;
  }

  //then create xml document
  QDomDocument* configDoc = new QDomDocument();
  if( !configDoc->setContent( configFile, true ) )
  {
    delete configFile;
    delete configDoc;
    return 0;
  }

  //is it an sld document or a qgis project file?
  QDomElement documentElem = configDoc->documentElement();
  QgsConfigParser* configParser = 0;
  if( documentElem.tagName() == "StyledLayerDescriptor" )
  {
    configParser = new QgsSLDParser( configDoc );
  }
  else if( documentElem.tagName() == "qgis" )
  {
    configParser = new QgsProjectParser( configDoc );
  }
  else
  {
    delete configDoc;
    return 0;
  }

  mCachedConfigurations.insert( filePath, configParser );
  delete configFile;
  return configParser;
}
