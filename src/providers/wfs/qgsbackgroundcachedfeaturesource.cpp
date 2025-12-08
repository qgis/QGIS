/***************************************************************************
    qgsbackgroundcachedfeaturesource.cpp
    ------------------------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbackgroundcachedfeaturesource.h"

#include "qgsbackgroundcachedfeatureiterator.h"
#include "qgsbackgroundcachedshareddata.h"

QgsBackgroundCachedFeatureSource::QgsBackgroundCachedFeatureSource(
  std::shared_ptr<QgsBackgroundCachedSharedData> shared
)
  : mShared( std::move( shared ) )
{
}

QgsFeatureIterator QgsBackgroundCachedFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsBackgroundCachedFeatureIterator( this, false, mShared, request ) );
}
