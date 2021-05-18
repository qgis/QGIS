/***************************************************************************
    qgsauthmethodmetadata.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsauthmethodmetadata.h"


QgsAuthMethodMetadata::QgsAuthMethodMetadata(QString const &key, QString const &description, QString const &library , const QString &guiLibrary)
  : mKey( key )
  , mDescription( description )
  , mLibrary( library )
  , mGuiLibrary( guiLibrary )
{}

QString QgsAuthMethodMetadata::key() const
{
  return mKey;
}

QString QgsAuthMethodMetadata::description() const
{
  return mDescription;
}

QString QgsAuthMethodMetadata::library() const
{
  return mLibrary;
}

QString QgsAuthMethodMetadata::guiLibrary() const
{
  return mGuiLibrary;
}

