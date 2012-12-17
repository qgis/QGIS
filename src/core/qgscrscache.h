/***************************************************************************
                              qgscrscache.h
                              --------------
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

#ifndef QGSCRSCACHE_H
#define QGSCRSCACHE_H

#include "qgscoordinatereferencesystem.h"
#include <QHash>

class QgsCoordinateTransform;

/**Cache coordinate transform by authid of source/dest transformation to avoid the
overhead of initialisation for each redraw*/
class CORE_EXPORT QgsCoordinateTransformCache
{
  public:
    static QgsCoordinateTransformCache* instance();
    ~QgsCoordinateTransformCache();
    /**Returns coordinate transformation. Cache keeps ownership
        @param srcAuthId auth id string of source crs
        @param destAuthId auth id string of dest crs*/
    const QgsCoordinateTransform* transform( const QString& srcAuthId, const QString& destAuthId );

  private:
    static QgsCoordinateTransformCache* mInstance;
    QHash< QPair< QString, QString >, QgsCoordinateTransform* > mTransforms;
};

class CORE_EXPORT QgsCRSCache
{
  public:
    static QgsCRSCache* instance();
    ~QgsCRSCache();
    /**Returns the CRS for authid, e.g. 'EPSG:4326' (or an invalid CRS in case of error)*/
    const QgsCoordinateReferenceSystem& crsByAuthId( const QString& authid );
    const QgsCoordinateReferenceSystem& crsByEpsgId( long epgs );

  protected:
    QgsCRSCache();

  private:
    static QgsCRSCache* mInstance;
    QHash< QString, QgsCoordinateReferenceSystem > mCRS;
    /**CRS that is not initialised (returned in case of error)*/
    QgsCoordinateReferenceSystem mInvalidCRS;
};

#endif // QGSCRSCACHE_H
