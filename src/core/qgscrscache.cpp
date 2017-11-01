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

QgsCoordinateTransformCache *QgsCoordinateTransformCache::instance()
{
  static QgsCoordinateTransformCache sInstance;
  return &sInstance;
}

QgsCoordinateTransform QgsCoordinateTransformCache::transform( const QString &srcAuthId, const QString &destAuthId, int srcDatumTransform, int destDatumTransform )
{
  QList< QgsCoordinateTransform > values =
    mTransforms.values( qMakePair( srcAuthId, destAuthId ) );

  QList< QgsCoordinateTransform >::const_iterator valIt = values.constBegin();
  for ( ; valIt != values.constEnd(); ++valIt )
  {
    if ( ( *valIt ).isValid() &&
         ( *valIt ).sourceDatumTransform() == srcDatumTransform &&
         ( *valIt ).destinationDatumTransform() == destDatumTransform )
    {
      return *valIt;
    }
  }

  //not found, insert new value
  QgsCoordinateReferenceSystem srcCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( srcAuthId );
  QgsCoordinateReferenceSystem destCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( destAuthId );
  QgsCoordinateTransform ct = QgsCoordinateTransform( srcCrs, destCrs );
  ct.setSourceDatumTransform( srcDatumTransform );
  ct.setDestinationDatumTransform( destDatumTransform );
  ct.initialize();
  mTransforms.insertMulti( qMakePair( srcAuthId, destAuthId ), ct );
  return ct;
}

void QgsCoordinateTransformCache::invalidateCrs( const QString &crsAuthId )
{
  //get keys to remove first
  QHash< QPair< QString, QString >, QgsCoordinateTransform >::const_iterator it = mTransforms.constBegin();
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
