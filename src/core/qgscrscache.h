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

#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"
#include <QHash>
#include <QReadWriteLock>

class QgsCoordinateTransform;

/**
 * \ingroup core
 * Cache coordinate transform by authid of source/dest transformation to avoid the
overhead of initialization for each redraw*/
class CORE_EXPORT QgsCoordinateTransformCache
{
  public:
    static QgsCoordinateTransformCache *instance();

    //! QgsCoordinateTransformCache cannot be copied
    QgsCoordinateTransformCache( const QgsCoordinateTransformCache &rh ) = delete;
    //! QgsCoordinateTransformCache cannot be copied
    QgsCoordinateTransformCache &operator=( const QgsCoordinateTransformCache &rh ) = delete;

    /**
     * Returns coordinate transformation. Cache keeps ownership
        \param srcAuthId auth id string of source crs
        \param destAuthId auth id string of dest crs
        \param srcDatumTransform id of source's datum transform
        \param destDatumTransform id of destinations's datum transform
        \returns matching transform, or an invalid transform if none could be created
     */
    QgsCoordinateTransform transform( const QString &srcAuthId, const QString &destAuthId, int srcDatumTransform = -1, int destDatumTransform = -1 );

    //! Removes transformations where a changed crs is involved from the cache
    void invalidateCrs( const QString &crsAuthId );

  private:
    QMultiHash< QPair< QString, QString >, QgsCoordinateTransform > mTransforms; //same auth_id pairs might have different datum transformations

    QgsCoordinateTransformCache() = default;

#ifdef SIP_RUN
    QgsCoordinateTransformCache( const QgsCoordinateTransformCache &rh );
#endif

};

#endif // QGSCRSCACHE_H
