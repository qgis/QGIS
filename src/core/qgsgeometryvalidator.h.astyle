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

#include "qgis_core.h"
#include "qgis.h"
#include <QThread>
#include "qgsgeometry.h"

/** \ingroup core
 * \class QgsGeometryValidator
 */
class CORE_EXPORT QgsGeometryValidator : public QThread
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGeometryValidator.
     */
    QgsGeometryValidator( const QgsGeometry &geoemtry, QList<QgsGeometry::Error> *errors = nullptr, QgsGeometry::ValidationMethod method = QgsGeometry::ValidatorQgisInternal );
    ~QgsGeometryValidator();

    void run() override;
    void stop();

    //! Validate geometry and produce a list of geometry errors
    static void validateGeometry( const QgsGeometry &geometry, QList<QgsGeometry::Error> &errors SIP_OUT, QgsGeometry::ValidationMethod method = QgsGeometry::ValidatorQgisInternal );

  signals:
    void errorFound( const QgsGeometry::Error & );

  public slots:
    void addError( const QgsGeometry::Error & );

  private:
    void validatePolyline( int i, QgsPolyline polyline, bool ring = false );
    void validatePolygon( int i, const QgsPolygon &polygon );
    void checkRingIntersections( int p0, int i0, const QgsPolyline &ring0, int p1, int i1, const QgsPolyline &ring1 );
    double distLine2Point( const QgsPointXY &p, QgsVector v, const QgsPointXY &q );
    bool intersectLines( const QgsPointXY &p, QgsVector v, const QgsPointXY &q, QgsVector w, QgsPointXY &s );
    bool ringInRing( const QgsPolyline &inside, const QgsPolyline &outside );
    bool pointInRing( const QgsPolyline &ring, const QgsPointXY &p );

    QgsGeometry mGeometry;
    QList<QgsGeometry::Error> *mErrors;
    bool mStop;
    int mErrorCount;
    QgsGeometry::ValidationMethod mMethod = QgsGeometry::ValidatorQgisInternal;
};

#endif
