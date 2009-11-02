/***************************************************************************
  qgsdistancearea.h - Distance and area calculations on the ellipsoid
 ---------------------------------------------------------------------------
  Date                 : September 2005
  Copyright            : (C) 2005 by Martin Dobias
  email                : won.der at centrum.sk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSDISTANCEAREA_H
#define QGSDISTANCEAREA_H

#include <QList>
#include "qgscoordinatetransform.h"

class QgsGeometry;

/** \ingroup core
General purpose distance and area calculator.
- calculations are done on ellipsoid
- it's possible to pass points/features in any CRS, coordinates are transformed
- two options how to use it
  + use measure() takes QgsGeometry as a parameter and calculates distance or area
  + use directly measureLine(), measurePolygon() which take list of QgsPoints
  (both cases transform the coordinates from source CRS to the ellipse coords)
- returned values are in meters resp. square meters
*/
class CORE_EXPORT QgsDistanceArea
{
  public:
    //! Constructor
    QgsDistanceArea();

    //! Destructor
    ~QgsDistanceArea();

    //! sets whether coordinates must be projected to ellipsoid before measuring
    void setProjectionsEnabled( bool flag );

    //! returns projections enabled flag
    bool hasCrsTransformEnabled() { return mProjectionsEnabled; }

    //! sets source spatial reference system (by QGIS CRS)
    void setSourceCrs( long srsid );

    //! sets source spatial reference system (by EpsgCrsId)
    void setSourceEpsgCrsId( long epsgId );

    //! returns source spatial reference system
    long sourceCrs() { return mSourceRefSys; }
    //! What sort of coordinate system is being used?
    bool geographic() { return mCoordTransform->sourceCrs().geographicFlag(); }

    //! sets ellipsoid by its acronym
    bool setEllipsoid( const QString& ellipsoid );

    //! returns ellipsoid's acronym
    const QString& ellipsoid() { return mEllipsoid; }

    //! returns ellipsoid's semi major axis
    double ellipsoidSemiMajor() { return mSemiMajor; }
    //! returns ellipsoid's semi minor axis
    double ellipsoidSemiMinor() { return mSemiMinor; }
    //! returns ellipsoid's inverse flattening
    double ellipsoidInverseFlattening() { return mInvFlattening; }

    //! general measurement (line distance or polygon area)
    double measure( QgsGeometry* geometry );

    //! measures line with more segments
    double measureLine( const QList<QgsPoint>& points );
    //! measures line with one segment
    double measureLine( const QgsPoint& p1, const QgsPoint& p2 );

    //! measures polygon area
    double measurePolygon( const QList<QgsPoint>& points );

    //! compute bearing - in radians
    double bearing( const QgsPoint& p1, const QgsPoint& p2 );

    static QString textUnit( double value, int decimals, QGis::UnitType u, bool isArea );

  protected:

    //! measures line distance, line points are extracted from WKB
    unsigned char* measureLine( unsigned char* feature, double* area, bool hasZptr = false );
    //! measures polygon area, vertices are extracted from WKB
    unsigned char* measurePolygon( unsigned char* feature, double* area, bool hasZptr = false );

    /**
      calculates distance from two points on ellipsoid
      based on inverse Vincenty's formulae

      Points p1 and p2 are expected to be in degrees and in currently used ellipsoid

      @note if course1 is not NULL, bearing (in radians) from first point is calculated
            (the same for course2)
      @return distance in meters
     */
    double computeDistanceBearing( const QgsPoint& p1, const QgsPoint& p2,
                                   double* course1 = NULL, double* course2 = NULL );


    /**
     calculates area of polygon on ellipsoid
     algorithm has been taken from GRASS: gis/area_poly1.c

    */
    double computePolygonArea( const QList<QgsPoint>& points );

    double computePolygonFlatArea( const QList<QgsPoint>& points );

    /**
      precalculates some values
      (must be called always when changing ellipsoid)
    */
    void computeAreaInit();

  private:

    //! used for transforming coordinates from source CRS to ellipsoid's coordinates
    QgsCoordinateTransform* mCoordTransform;

    //! indicates whether we will transform coordinates
    bool mProjectionsEnabled;

    //! id of the source spatial reference system
    long mSourceRefSys;

    //! ellipsoid acronym (from table tbl_ellipsoids)
    QString mEllipsoid;

    //! ellipsoid parameters
    double mSemiMajor, mSemiMinor, mInvFlattening;

    // utility functions for polygon area measurement

    double getQ( double x );
    double getQbar( double x );

    // temporary area measurement stuff

    double m_QA, m_QB, m_QC;
    double m_QbarA, m_QbarB, m_QbarC, m_QbarD;
    double m_AE;  /* a^2(1-e^2) */
    double m_Qp;  /* Q at the north pole */
    double m_E;   /* area of the earth */
    double m_TwoPI;

};

#endif
