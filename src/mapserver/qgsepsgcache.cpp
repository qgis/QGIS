/***************************************************************************
                              qgsepsgcache.cpp
                              ----------------
  begin                : June 9th, 2010
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

#include "qgsepsgcache.h"

QgsEPSGCache* QgsEPSGCache::mInstance = 0;

QgsEPSGCache* QgsEPSGCache::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsEPSGCache();
  }
  return mInstance;
}

QgsEPSGCache::QgsEPSGCache()
{
}

QgsEPSGCache::~QgsEPSGCache()
{
  delete mInstance;
}

const QgsCoordinateReferenceSystem& QgsEPSGCache::searchCRS( long epsg )
{
  QHash< long, QgsCoordinateReferenceSystem >::const_iterator crsIt = mCRS.find( epsg );
  if ( crsIt == mCRS.constEnd() )
  {
    QgsCoordinateReferenceSystem s;
    if ( ! s.createFromEpsg( epsg ) )
    {
      return mInvalidCRS;
    }
    return mCRS.insert( epsg, s ).value();
  }
  else
  {
    return crsIt.value();
  }
}


