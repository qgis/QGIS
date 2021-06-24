/***************************************************************************
  qgsexternalstorage.h
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

void QgsExternalStorageContent::reportError( const QString &errorMsg )
{
  mStatus = Qgis::ContentStatus::Failed;
  mErrorString = errorMsg;
  emit errorOccurred( mErrorString );
}

Qgis::ContentStatus QgsExternalStorageContent::status() const
{
  return mStatus;
}

const QString &QgsExternalStorageContent::errorString() const
{
  return mErrorString;
};
