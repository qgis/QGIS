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
#include <QVector>

QgsCoordinateTransformCache* QgsCoordinateTransformCache::instance()
{
  static QgsCoordinateTransformCache mInstance;
  return &mInstance;
}

QgsCoordinateTransformCache::QgsCoordinateTransformCache()
{
}

QgsCoordinateTransformCache::~QgsCoordinateTransformCache()
{
  QHash< QPair< QString, QString >, QgsCoordinateTransform* >::const_iterator tIt = mTransforms.constBegin();
  for ( ; tIt != mTransforms.constEnd(); ++tIt )
  {
    delete tIt.value();
  }
}

const QgsCoordinateTransform* QgsCoordinateTransformCache::transform( const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform, int destDatumTransform )
{
  QList< QgsCoordinateTransform* > values =
    mTransforms.values( qMakePair( srcAuthId, destAuthId ) );

  QList< QgsCoordinateTransform* >::const_iterator valIt = values.constBegin();
  for ( ; valIt != values.constEnd(); ++valIt )
  {
    if ( *valIt &&
         ( *valIt )->sourceDatumTransform() == srcDatumTransform &&
         ( *valIt )->destinationDatumTransform() == destDatumTransform )
    {
      return *valIt;
    }
  }

  //not found, insert new value
  QgsCoordinateReferenceSystem srcCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( srcAuthId );
  QgsCoordinateReferenceSystem destCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( destAuthId );
  QgsCoordinateTransform* ct = new QgsCoordinateTransform( srcCrs, destCrs );
  ct->setSourceDatumTransform( srcDatumTransform );
  ct->setDestinationDatumTransform( destDatumTransform );
  ct->initialise();
  mTransforms.insertMulti( qMakePair( srcAuthId, destAuthId ), ct );
  return ct;
}

void QgsCoordinateTransformCache::invalidateCrs( const QString& crsAuthId )
{
  //get keys to remove first
  QHash< QPair< QString, QString >, QgsCoordinateTransform* >::const_iterator it = mTransforms.constBegin();
  QVector< QPair< QString, QString > > updateList;

  for ( ; it != mTransforms.constEnd(); ++it )
  {
    if ( it.key().first == crsAuthId || it.key().second == crsAuthId )
    {
      updateList.append( it.key() );
    }
  }

  //and remove after
  QVector< QPair< QString, QString > >::const_iterator updateIt = updateList.constBegin();
  for ( ; updateIt != updateList.constEnd(); ++updateIt )
  {
    mTransforms.remove( *updateIt );
  }
}


QgsCRSCache* QgsCRSCache::instance()
{
  static QgsCRSCache mInstance;
  return &mInstance;
}

QgsCRSCache::QgsCRSCache()
    : mCRSLock( QReadWriteLock::Recursive )
    , mCRSProj4Lock( QReadWriteLock::Recursive )
    , mCRSSrsIdLock( QReadWriteLock::Recursive )
{
}

void QgsCRSCache::updateCRSCache( const QString& authid )
{
  QWriteLocker lock( &mCRSLock );
  QgsCoordinateReferenceSystem s;
  if ( s.createFromOgcWmsCrs( authid ) )
  {
    mCRS.insert( authid, s );
  }
  else
  {
    mCRS.remove( authid );
  }

  QgsCoordinateTransformCache::instance()->invalidateCrs( authid );
}

QgsCoordinateReferenceSystem QgsCRSCache::crsByAuthId( const QString& authid )
{
  return crsByOgcWmsCrs( authid );
}

QgsCoordinateReferenceSystem QgsCRSCache::crsByOgcWmsCrs( const QString& ogcCrs ) const
{
  mCRSLock.lockForRead();
  QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = mCRS.constFind( ogcCrs );
  if ( crsIt == mCRS.constEnd() )
  {
    mCRSLock.unlock();
    mCRSLock.lockForWrite();
    QgsCoordinateReferenceSystem s;
    if ( ! s.createFromOgcWmsCrs( ogcCrs ) )
    {
      QgsCoordinateReferenceSystem result = mCRS.insert( ogcCrs, mInvalidCRS ).value();
      mCRSLock.unlock();
      return result;
    }
    else
    {
      QgsCoordinateReferenceSystem result = mCRS.insert( ogcCrs, s ).value();
      mCRSLock.unlock();
      return result;
    }
  }
  else
  {
    QgsCoordinateReferenceSystem result = crsIt.value();
    mCRSLock.unlock();
    return result;
  }
}

QgsCoordinateReferenceSystem QgsCRSCache::crsByEpsgId( long epsg ) const
{
  return crsByOgcWmsCrs( "EPSG:" + QString::number( epsg ) );
}

QgsCoordinateReferenceSystem QgsCRSCache::crsByProj4( const QString& proj4 ) const
{
  mCRSProj4Lock.lockForRead();
  QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = mCRSProj4.constFind( proj4 );
  if ( crsIt == mCRSProj4.constEnd() )
  {
    mCRSProj4Lock.unlock();
    mCRSProj4Lock.lockForWrite();
    QgsCoordinateReferenceSystem s;
    if ( ! s.createFromProj4( proj4 ) )
    {
      QgsCoordinateReferenceSystem result = mCRSProj4.insert( proj4, mInvalidCRS ).value();
      mCRSProj4Lock.unlock();
      return result;
    }
    else
    {
      QgsCoordinateReferenceSystem result = mCRSProj4.insert( proj4, s ).value();
      mCRSProj4Lock.unlock();
      return result;
    }
  }
  else
  {
    QgsCoordinateReferenceSystem result = crsIt.value();
    mCRSProj4Lock.unlock();
    return result;
  }
}

QgsCoordinateReferenceSystem QgsCRSCache::crsByWkt( const QString& wkt ) const
{
  mCRSWktLock.lockForRead();
  QHash< QString, QgsCoordinateReferenceSystem >::const_iterator crsIt = mCRSWkt.constFind( wkt );
  if ( crsIt == mCRSWkt.constEnd() )
  {
    mCRSWktLock.unlock();
    mCRSWktLock.lockForWrite();
    QgsCoordinateReferenceSystem s;
    if ( ! s.createFromWkt( wkt ) )
    {
      QgsCoordinateReferenceSystem result = mCRSWkt.insert( wkt, mInvalidCRS ).value();
      mCRSWktLock.unlock();
      return result;
    }
    else
    {
      QgsCoordinateReferenceSystem result = mCRSWkt.insert( wkt, s ).value();
      mCRSWktLock.unlock();
      return result;
    }
  }
  else
  {
    QgsCoordinateReferenceSystem result = crsIt.value();
    mCRSWktLock.unlock();
    return result;
  }
}

QgsCoordinateReferenceSystem QgsCRSCache::crsBySrsId( long srsId ) const
{
  mCRSSrsIdLock.lockForRead();
  QHash< long, QgsCoordinateReferenceSystem >::const_iterator crsIt = mCRSSrsId.constFind( srsId );
  if ( crsIt == mCRSSrsId.constEnd() )
  {
    mCRSSrsIdLock.unlock();
    mCRSSrsIdLock.lockForWrite();
    QgsCoordinateReferenceSystem s;
    if ( ! s.createFromSrsId( srsId ) )
    {
      QgsCoordinateReferenceSystem result = mCRSSrsId.insert( srsId, mInvalidCRS ).value();
      mCRSSrsIdLock.unlock();
      return result;
    }
    else
    {
      QgsCoordinateReferenceSystem result = mCRSSrsId.insert( srsId, s ).value();
      mCRSSrsIdLock.unlock();
      return result;
    }
  }
  else
  {
    QgsCoordinateReferenceSystem result = crsIt.value();
    mCRSSrsIdLock.unlock();
    return result;
  }
}
