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

    //! Copy constructor
    QgsDistanceArea( const QgsDistanceArea &origDA );

    //! Assignment operator
    QgsDistanceArea & operator=( const QgsDistanceArea & origDA );

    //! sets whether coordinates must be projected to ellipsoid before measuring
    void setEllipsoidalMode( bool flag );

    //! returns projections enabled flag
    bool ellipsoidalEnabled() const { return mEllipsoidalMode; }

    //! sets source spatial reference system (by QGIS CRS)
    void setSourceCrs( long srsid );

    /**
     * Sets source spatial reference system (by QGIS CRS)
     * @note: missing in Python bindings in QGIS < 2.2
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem& srcCRS );

    //! sets source spatial reference system by authid
    void setSourceAuthId( QString authid );

    //! returns source spatial reference system
    long sourceCrs() const { return mCoordTransform->sourceCrs().srsid(); }
    //! What sort of coordinate system is being used?
    bool geographic() const { return mCoordTransform->sourceCrs().geographicFlag(); }

    //! sets ellipsoid by its acronym
    bool setEllipsoid( const QString& ellipsoid );

    //! Sets ellipsoid by supplied radii
    // Inverse flattening is calculated with invf = a/(a-b)
    bool setEllipsoid( double semiMajor, double semiMinor );

    //! returns ellipsoid's acronym
    const QString& ellipsoid() const { return mEllipsoid; }

    //! returns ellipsoid's semi major axis
    double ellipsoidSemiMajor() const { return mSemiMajor; }
    //! returns ellipsoid's semi minor axis
    double ellipsoidSemiMinor() const { return mSemiMinor; }
    //! returns ellipsoid's inverse flattening
    double ellipsoidInverseFlattening() const { return mInvFlattening; }

    //! general measurement (line distance or polygon area)
    double measure( QgsGeometry* geometry );

    //! measures perimeter of polygon
    double measurePerimeter( QgsGeometry* geometry );

    //! measures line
    double measureLine( const QList<QgsPoint>& points );

    //! measures line with one segment
    double measureLine( const QgsPoint& p1, const QgsPoint& p2 );

    //! measures polygon area
    double measurePolygon( const QList<QgsPoint>& points );

    //! compute bearing - in radians
    double bearing( const QgsPoint& p1, const QgsPoint& p2 );

    static QString textUnit( double value, int decimals, QGis::UnitType u, bool isArea, bool keepBaseUnit = false );

    //! Helper for conversion between physical units
    void convertMeasurement( double &measure, QGis::UnitType &measureUnits, QGis::UnitType displayUnits, bool isArea );

  protected:
    //! measures line distance, line points are extracted from WKB
    // @note available in python bindings
    const unsigned char* measureLine( const unsigned char* feature, double* area, bool hasZptr = false );
    //! measures polygon area and perimeter, vertices are extracted from WKB
    // @note available in python bindings
    const unsigned char* measurePolygon( const unsigned char* feature, double* area, double* perimeter, bool hasZptr = false );

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

    //! uses flat / planimetric / Euclidean distance
    double computeDistanceFlat( const QgsPoint& p1, const QgsPoint& p2 );

    //! calculate distance with given coordinates (does not do a transform anymore)
    double computeDistance( const QList<QgsPoint>& points );

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
    //! Copy helper
    void _copy( const QgsDistanceArea & origDA );

    //! used for transforming coordinates from source CRS to ellipsoid's coordinates
    QgsCoordinateTransform* mCoordTransform;

    //! indicates whether we will transform coordinates
    bool mEllipsoidalMode;

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

