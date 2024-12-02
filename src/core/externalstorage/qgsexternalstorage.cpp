/***************************************************************************
  qgsexternalstorage.cpp
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalstorage.h"
#include "moc_qgsexternalstorage.cpp"

void QgsExternalStorageContent::reportError( const QString &errorMsg )
{
  setStatus( Qgis::ContentStatus::Failed );
  mErrorString = errorMsg;
  emit errorOccurred( mErrorString );
}

void QgsExternalStorageContent::setStatus( Qgis::ContentStatus status )
{
  mStatus = status;
}

Qgis::ContentStatus QgsExternalStorageContent::status() const
{
  return mStatus;
}

const QString &QgsExternalStorageContent::errorString() const
{
  return mErrorString;
};

QgsExternalStorageStoredContent *QgsExternalStorage::store( const QString &filePath, const QString &url, const QString &authCfg, Qgis::ActionStart storingMode ) const
{
  QgsExternalStorageStoredContent *content = doStore( filePath, url, authCfg );
  if ( storingMode == Qgis::ActionStart::Immediate )
    content->store();

  return content;
}

QgsExternalStorageFetchedContent *QgsExternalStorage::fetch( const QString &url, const QString &authCfg, Qgis::ActionStart fetchingMode ) const
{
  QgsExternalStorageFetchedContent *content = doFetch( url, authCfg );
  if ( fetchingMode == Qgis::ActionStart::Immediate )
    content->fetch();

  return content;
}
