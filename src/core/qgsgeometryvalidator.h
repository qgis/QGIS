/***************************************************************************
  qgsgeometryvalidator.h - geometry validation thread
  -------------------------------------------------------------------
Date                 : 03.01.2012
Copyright            : (C) 2012 by Juergen E. Fischer
email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYVALIDATOR_H
#define QGSGEOMETRYVALIDATOR_H

#include <QThread>
#include "qgsgeometry.h"

class CORE_EXPORT QgsGeometryValidator : public QThread
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGeometryValidator( const QgsGeometry *g, QList<QgsGeometry::Error> *errors = 0 );
    ~QgsGeometryValidator();

    void run() override;
    void stop();

    /** Validate geometry and produce a list of geometry errors */
    static void validateGeometry( const QgsGeometry *g, QList<QgsGeometry::Error> &errors );

    /** Attempts to create a valid representation of a given invalid geometry without loosing any of the input vertices.
     * Already-valid geometries are returned w/out further intervention.
     * In case of full or partial dimensional collapses, the output geometry may be a collection of 
     * lower-to-equal dimension geometries or a geometry of lower dimension.
     * Single polygons may become multi-geometries in case of self-intersections.
     * @note added in 2.4
     **/
    static GEOSGeometry *makeValidGeometry( const GEOSGeometry *g );

    /** Attempts to create a valid representation of a given invalid geometry without loosing any of the input vertices.
     * Already-valid geometries are returned w/out further intervention.
     * In case of full or partial dimensional collapses, the output geometry may be a collection of 
     * lower-to-equal dimension geometries or a geometry of lower dimension.
     * Single polygons may become multi-geometries in case of self-intersections.
     * @note added in 2.4
     **/
    static QgsGeometry *makeValidGeometry( const QgsGeometry *g );

  signals:
    void errorFound( QgsGeometry::Error );

  public slots:
    void addError( QgsGeometry::Error );

  private:
    void validatePolyline( int i, QgsPolyline polyline, bool ring = false );
    void validatePolygon( int i, const QgsPolygon &polygon );
    void checkRingIntersections( int p0, int i0, const QgsPolyline &ring0, int p1, int i1, const QgsPolyline &ring1 );
    double distLine2Point( QgsPoint p, QgsVector v, QgsPoint q );
    bool intersectLines( QgsPoint p, QgsVector v, QgsPoint q, QgsVector w, QgsPoint &s );
    bool ringInRing( const QgsPolyline &inside, const QgsPolyline &outside );
    bool pointInRing( const QgsPolyline &ring, const QgsPoint &p );

    QgsGeometry mG;
    QList<QgsGeometry::Error> *mErrors;
    bool mStop;
    int mErrorCount;
}; // class QgsGeometryValidator

#endif
