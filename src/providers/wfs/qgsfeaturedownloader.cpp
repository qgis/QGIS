/***************************************************************************
    qgsfeaturedownloader.cpp
    ------------------------
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

#include "qgsfeaturedownloader.h"
#include "moc_qgsfeaturedownloader.cpp"
#include "qgsfeaturedownloaderimpl.h"

void QgsFeatureDownloader::run( bool serializeFeatures, long long maxFeatures )
{
  Q_ASSERT( mImpl );
  mImpl->run( serializeFeatures, maxFeatures );
}

void QgsFeatureDownloader::stop()
{
  Q_ASSERT( mImpl );
  mImpl->stop();
}
