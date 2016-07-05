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
#include <QReadWriteLock>

class QgsCoordinateTransform;

/** \ingroup core
 * Cache coordinate transform by authid of source/dest transformation to avoid the
overhead of initialization for each redraw*/
class CORE_EXPORT QgsCoordinateTransformCache
{
  public:
    static QgsCoordinateTransformCache* instance();

    ~QgsCoordinateTransformCache();
    /** Returns coordinate transformation. Cache keeps ownership
        @param srcAuthId auth id string of source crs
        @param destAuthId auth id string of dest crs
        @param srcDatumTransform id of source's datum transform
        @param destDatumTransform id of destinations's datum transform
     */
    const QgsCoordinateTransform* transform( const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform = -1, int destDatumTransform = -1 );
    /** Removes transformations where a changed crs is involved from the cache*/
    void invalidateCrs( const QString& crsAuthId );

  private:
    QMultiHash< QPair< QString, QString >, QgsCoordinateTransform* > mTransforms; //same auth_id pairs might have different datum transformations

    QgsCoordinateTransformCache();
    QgsCoordinateTransformCache( const QgsCoordinateTransformCache& rh );
    QgsCoordinateTransformCache& operator=( const QgsCoordinateTransformCache& rh );
};



/** \ingroup core
 * \class QgsCRSCache
 * \brief Caches QgsCoordinateReferenceSystem construction, which may be expensive.
 *
 * QgsCRSCache maintains a cache of previously constructed coordinate systems, so that
 * creating a new CRS from the cache can reuse previously calculated parameters. The
 * constructors for QgsCoordinateReferenceSystem can be expensive, so it's recommended
 * to use QgsCRSCache instead of directly calling the QgsCoordinateReferenceSystem
 * constructors.
 */

class CORE_EXPORT QgsCRSCache
{
  public:

    //! Returns a pointer to the QgsCRSCache singleton
    static QgsCRSCache* instance();

    /** Returns the CRS for authid, e.g. 'EPSG:4326' (or an invalid CRS in case of error)
     * @deprecated use crsByOgcWmsCrs() instead
    */
    Q_DECL_DEPRECATED QgsCoordinateReferenceSystem crsByAuthId( const QString& authid );

    /** Returns the CRS from a given OGC WMS-format Coordinate Reference System string.
     * @param ogcCrs OGR compliant CRS definition, eg "EPSG:4326"
     * @returns matching CRS, or an invalid CRS if string could not be matched
     * @note added in QGIS 2.16
     * @see QgsCoordinateReferenceSystem::createFromOgcWmsCrs()
    */
    QgsCoordinateReferenceSystem crsByOgcWmsCrs( const QString& ogcCrs ) const;

    /** Returns the CRS from a given EPSG ID.
     * @param epsg epsg CRS ID
     * @returns matching CRS, or an invalid CRS if string could not be matched
    */
    QgsCoordinateReferenceSystem crsByEpsgId( long epsg ) const;

    /** Returns the CRS from a proj4 style formatted string.
     * @param proj4 proj4 format string
     * @returns matching CRS, or an invalid CRS if string could not be matched
     * @note added in QGIS 2.16
     * @see QgsCoordinateReferenceSystem::createFromProj4()
    */
    QgsCoordinateReferenceSystem crsByProj4( const QString& proj4 ) const;

    /** Returns the CRS from a WKT spatial ref sys definition string.
     * @param wkt WKT for the desired spatial reference system.
     * @returns matching CRS, or an invalid CRS if string could not be matched
     * @note added in QGIS 2.16
     * @see QgsCoordinateReferenceSystem::createFromWkt()
    */
    QgsCoordinateReferenceSystem crsByWkt( const QString& wkt ) const;

    /** Returns the CRS from a specified QGIS SRS ID.
     * @param srsId internal QGIS SRS ID
     * @returns matching CRS, or an invalid CRS if ID could not be found
     * @note added in QGIS 2.16
     * @see QgsCoordinateReferenceSystem::createFromSrsId()
    */
    QgsCoordinateReferenceSystem crsBySrsId( long srsId ) const;

    /** Updates the cached definition of a CRS. Should be called if the definition of a user-created
     * CRS has been changed.
     * @param authid CRS auth ID, eg "EPSG:4326" or "USER:100009"
     */
    void updateCRSCache( const QString& authid );

  protected:
    QgsCRSCache();

  private:

    mutable QReadWriteLock mCRSLock;
    mutable QHash< QString, QgsCoordinateReferenceSystem > mCRS;
    mutable QReadWriteLock mCRSProj4Lock;
    mutable QHash< QString, QgsCoordinateReferenceSystem > mCRSProj4;
    mutable QReadWriteLock mCRSWktLock;
    mutable QHash< QString, QgsCoordinateReferenceSystem > mCRSWkt;
    mutable QReadWriteLock mCRSSrsIdLock;
    mutable QHash< long, QgsCoordinateReferenceSystem > mCRSSrsId;

    /** CRS that is not initialized (returned in case of error)*/
    QgsCoordinateReferenceSystem mInvalidCRS;

    QgsCRSCache( const QgsCRSCache& other );
};

#endif // QGSCRSCACHE_H
