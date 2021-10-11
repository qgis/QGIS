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
#include "qgis_sip.h"
#include <QThread>

#include "qgsgeometry.h"

class QgsCurvePolygon;

/**
 * \ingroup core
 * \class QgsGeometryValidator
 */
class CORE_EXPORT QgsGeometryValidator : public QThread
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGeometryValidator.
     */
    QgsGeometryValidator( const QgsGeometry &geometry, QVector<QgsGeometry::Error> *errors = nullptr, Qgis::GeometryValidationEngine method = Qgis::GeometryValidationEngine::QgisInternal );
    ~QgsGeometryValidator() override;

    void run() override;
    void stop();

    /**
     * Validate geometry and produce a list of geometry errors.
     * This method blocks the thread until the validation is finished.
     */
    static void validateGeometry( const QgsGeometry &geometry, QVector<QgsGeometry::Error> &errors SIP_OUT, Qgis::GeometryValidationEngine method = Qgis::GeometryValidationEngine::QgisInternal );

  signals:

    /**
     * Sent when an error has been found during the validation process.
     *
     * The \a error contains details about the error.
     */
    void errorFound( const QgsGeometry::Error &error );

    /**
     * Sent when the validation is finished.
     *
     * The result is in a human readable \a summary, mentioning
     * if the validation has been aborted, successfully been validated
     * or how many errors have been found.
     *
     * \since QGIS 3.6
     */
    void validationFinished( const QString &summary );

  public slots:
    void addError( const QgsGeometry::Error & );

  private:
    void validatePolyline( int i, const QgsLineString *line, bool ring = false );
    void validatePolygon( int partIndex, const QgsCurvePolygon *polygon );
    void checkRingIntersections( int partIndex0, int ringIndex0, const QgsLineString *ring0, int partIndex1, int ringIndex1, const QgsLineString *ring1 );
    double distLine2Point( double px, double py, QgsVector v, double qX, double qY );
    bool intersectLines( double px, double py, QgsVector v, double qx, double qy, QgsVector w, double &sX, double &sY );
    bool ringInRing( const QgsCurve *inside, const QgsCurve *outside );
    bool pointInRing( const QgsCurve *ring, double pX, double pY );

    QgsGeometry mGeometry;
    QVector<QgsGeometry::Error> *mErrors;
    bool mStop;
    int mErrorCount;
    Qgis::GeometryValidationEngine mMethod = Qgis::GeometryValidationEngine::QgisInternal;
};

#endif
