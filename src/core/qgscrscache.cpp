/***************************************************************************
                              qgscrscache.cpp
                              ---------------
  begin                : September 6th, 2011
  copyright            : (C) 2011 by Marco Hugentobler
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

#include "qgscrscache.h"
#include "qgscoordinatetransform.h"

QgsCoordinateTransformCache* QgsCoordinateTransformCache::mInstance = 0;

QgsCoordinateTransformCache* QgsCoordinateTransformCache::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsCoordinateTransformCache();
  }
  return mInstance;
}

QgsCoordinateTransformCache::~QgsCoordinateTransformCache()
{
  QHash< QPair< QString, QString >, QgsCoordinateTransform* >::const_iterator tIt = mTransforms.constBegin();
  for ( ; tIt != mTransforms.constEnd(); ++tIt )
  {
    delete tIt.value();
  }
  delete mInstance;
}

const QgsCoordinateTransform* QgsCoordinateTransformCache::transform( const QString& srcAuthId, const QString& destAuthId )
{
  QHash< QPair< QString, QString >, QgsCoordinateTransform* >::const_iterator ctIt =
    mTransforms.find( qMakePair( srcAuthId, destAuthId ) );
  if ( ctIt == mTransforms.constEnd() )
  {
    const QgsCoordinateReferenceSystem& srcCrs = QgsCRSCache::instance()->crsByAuthId( srcAuthId );
    const QgsCoordinateReferenceSystem& destCrs = QgsCRSCache::instance()->crsByAuthId( destAuthId );
    QgsCoordinateTransform* ct = new QgsCoordinateTransform( srcCrs, destCrs );
    mTransforms.insert( qMakePair( srcAuthId, destAuthId ), ct );
    return ct;
  }
  else
  {
    return ctIt.value();
  }
}

QgsCRSCache* QgsCRSCache::mInstance = 0;

QgsCRSCache* QgsCRSCache::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsCRSCache();
  }
  return mInstance;
}

QgsCRSCache::QgsCRSCache()
{
}

QgsCRSCache::~QgsCRSCache()
{
  delete mInstance;
}

const QgsCoordinateReferenceSystem& QgsCRSCache::crsByAuthId( const QString& authid )
{
  QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = mCRS.find( authid );
  if ( crsIt == mCRS.constEnd() )
  {
    QgsCoordinateReferenceSystem s;
    if ( ! s.createFromOgcWmsCrs( authid ) )
    {
      return mInvalidCRS;
    }
    return mCRS.insert( authid, s ).value();
  }
  else
  {
    return crsIt.value();
  }
}

const QgsCoordinateReferenceSystem& QgsCRSCache::crsByEpsgId( long epsg )
{
  return crsByAuthId( "EPSG:" + QString::number( epsg ) );
}
