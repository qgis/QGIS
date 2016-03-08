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
#include "qgswkbptr.h"
#include "qgsunittypes.h"

class QgsGeometry;
class QgsAbstractGeometryV2;
class QgsCurveV2;

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

    /** Sets whether coordinates must be projected to ellipsoid before measuring
     * @note for calculations to use the ellipsoid, both the ellipsoid mode must be true
     * and an ellipse must be set
     * @see setEllipsoid()
     * @see willUseEllipsoid()
     */
    void setEllipsoidalMode( bool flag );

    /** Returns whether ellipsoidal calculations are enabled
     * @see willUseEllipsoid()
     * @see setEllipsoidalMode()
     */
    bool ellipsoidalEnabled() const { return mEllipsoidalMode; }

    /** Returns whether calculations will use the ellipsoid. Calculations will only use the
     * ellipsoid if ellipsoidalEnabled() is true and an ellipsoid has been set.
     * @note added in QGIS 2.14
     * @see ellipsoidalEnabled()
     * @see ellipsoid()
     */
    bool willUseEllipsoid() const;

    //! sets source spatial reference system (by QGIS CRS)
    void setSourceCrs( long srsid );

    /**
     * Sets source spatial reference system (by QGIS CRS)
     * @note: missing in Python bindings in QGIS < 2.2
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem& srcCRS );

    //! sets source spatial reference system by authid
    void setSourceAuthId( const QString& authid );

    //! returns source spatial reference system
    //! @deprecated use sourceCrsId() instead
    // TODO QGIS 3.0 - make sourceCrs() return QgsCoordinateReferenceSystem
    Q_DECL_DEPRECATED long sourceCrs() const { return mCoordTransform->sourceCrs().srsid(); }

    /** Returns the QgsCoordinateReferenceSystem::srsid() for the CRS used during calculations.
     * @see setSourceCrs()
     * @note added in QGIS 2.14
     */
    long sourceCrsId() const { return mCoordTransform->sourceCrs().srsid(); }

    //! What sort of coordinate system is being used?
    bool geographic() const { return mCoordTransform->sourceCrs().geographicFlag(); }

    /** Sets ellipsoid by its acronym. Calculations will only use the ellipsoid if
     * both the ellipsoid has been set and ellipsoidalEnabled() is true.
     * @returns true if ellipsoid was successfully set
     * @see ellipsoid()
     * @see setEllipsoidalMode()
     * @see willUseEllipsoid()
     */
    bool setEllipsoid( const QString& ellipsoid );

    /** Sets ellipsoid by supplied radii. Calculations will only use the ellipsoid if
     * both the ellipsoid has been set and ellipsoidalEnabled() is true.
     * @returns true if ellipsoid was successfully set
     * @see ellipsoid()
     * @see setEllipsoidalMode()
     * @see willUseEllipsoid()
     */
    // Inverse flattening is calculated with invf = a/(a-b)
    bool setEllipsoid( double semiMajor, double semiMinor );

    /** Returns ellipsoid's acronym. Calculations will only use the
     * ellipsoid if ellipsoidalEnabled() is true and an ellipsoid has been set.
     * @see setEllipsoid()
     * @see ellipsoidalEnabled()
     * @see willUseEllipsoid()
     */
    QString ellipsoid() const { return mEllipsoid; }

    //! returns ellipsoid's semi major axis
    double ellipsoidSemiMajor() const { return mSemiMajor; }
    //! returns ellipsoid's semi minor axis
    double ellipsoidSemiMinor() const { return mSemiMinor; }
    //! returns ellipsoid's inverse flattening
    double ellipsoidInverseFlattening() const { return mInvFlattening; }

    /** General measurement (line distance or polygon area)
     * @deprecated use measureArea() or measureLength() methods instead, as this method
     * is unpredictable for geometry collections
     */
    Q_DECL_DEPRECATED double measure( const QgsGeometry* geometry ) const;

    /** Measures the area of a geometry.
     * @param geometry geometry to measure
     * @returns area of geometry. For geometry collections, non surface geometries will be ignored. The units for the
     * returned area can be retrieved by calling areaUnits().
     * @note added in QGIS 2.12
     * @see measureLength()
     * @see measurePerimeter()
     * @see areaUnits()
     */
    double measureArea( const QgsGeometry* geometry ) const;

    /** Measures the length of a geometry.
     * @param geometry geometry to measure
     * @returns length of geometry. For geometry collections, non curve geometries will be ignored. The units for the
     * returned distance can be retrieved by calling lengthUnits().
     * @note added in QGIS 2.12
     * @see lengthUnits()
     * @see measureArea()
     * @see measurePerimeter()
     */
    double measureLength( const QgsGeometry* geometry ) const;

    /** Measures the perimeter of a polygon geometry.
     * @param geometry geometry to measure
     * @returns perimeter of geometry. For geometry collections, any non-polygon geometries will be ignored. The units for the
     * returned perimeter can be retrieved by calling lengthUnits().
     * @note added in QGIS 2.12
     * @see lengthUnits()
     * @see measureArea()
     * @see measurePerimeter()
     */
    double measurePerimeter( const QgsGeometry *geometry ) const;

    /** Measures the length of a line with multiple segments.
     * @param points list of points in line
     * @returns length of line. The units for the returned length can be retrieved by calling lengthUnits().
     * @see lengthUnits()
     */
    double measureLine( const QList<QgsPoint>& points ) const;

    /** Measures length of a line with one segment.
     * @param p1 start of line
     * @param p2 end of line
     * @returns distance between points. The units for the returned distance can be retrieved by calling lengthUnits().
     * @see lengthUnits()
     */
    double measureLine( const QgsPoint& p1, const QgsPoint& p2 ) const;

    /** Measures length of line with one segment and returns units of distance.
     * @param p1 start of line
     * @param p2 end of line
     * @param units will be set to units of measure
     * @returns calculated distance between points. Distance units are stored in units parameter.
     * @note added in QGIS 2.12
     */
    double measureLine( const QgsPoint& p1, const QgsPoint& p2, QGis::UnitType& units ) const;

    /** Returns the units of distance for length calculations made by this object.
     * @note added in QGIS 2.14
     * @see areaUnits()
     */
    QGis::UnitType lengthUnits() const;

    /** Returns the units of area for areal calculations made by this object.
     * @note added in QGIS 2.14
     * @see lengthUnits()
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    //! measures polygon area
    double measurePolygon( const QList<QgsPoint>& points ) const;

    //! compute bearing - in radians
    double bearing( const QgsPoint& p1, const QgsPoint& p2 ) const;

    /** Returns a measurement formatted as a friendly string
     * @param value value of measurement
     * @param decimals number of decimal places to show
     * @param u unit of measurement
     * @param isArea set to true if measurement is an area measurement
     * @param keepBaseUnit set to false to allow conversion of large distances to more suitable units, eg meters
     * to kilometers
     * @return formatted measurement string
     * @deprecated use formatDistance() or formatArea() instead
     */
    Q_DECL_DEPRECATED static QString textUnit( double value, int decimals, QGis::UnitType u, bool isArea, bool keepBaseUnit = false );

    /** Returns an distance formatted as a friendly string.
     * @param distance distance to format
     * @param decimals number of decimal places to show
     * @param unit unit of distance
     * @param keepBaseUnit set to false to allow conversion of large distances to more suitable units, eg meters to
     * kilometers
     * @returns formatted distance string
     * @note added in QGIS 2.16
     * @see formatArea()
     */
    static QString formatDistance( double distance, int decimals, QGis::UnitType unit, bool keepBaseUnit = false );

    /** Returns an area formatted as a friendly string.
     * @param area area to format
     * @param decimals number of decimal places to show
     * @param unit unit of area
     * @param keepBaseUnit set to false to allow conversion of large areas to more suitable units, eg square meters to
     * square kilometers
     * @returns formatted area string
     * @note added in QGIS 2.14
     * @see formatDistance()
     */
    static QString formatArea( double area, int decimals, QgsUnitTypes::AreaUnit unit, bool keepBaseUnit = false );

    //! Helper for conversion between physical units
    // TODO QGIS 3.0 - remove this method, as its behaviour is non-intuitive.
    void convertMeasurement( double &measure, QGis::UnitType &measureUnits, QGis::UnitType displayUnits, bool isArea ) const;

    /** Takes a length measurement calculated by this QgsDistanceArea object and converts it to a
     * different distance unit.
     * @param length length value calculated by this class to convert. It is assumed that the length
     * was calculated by this class, ie that its unit of length is equal to lengthUnits().
     * @param toUnits distance unit to convert measurement to
     * @returns converted distance
     * @see convertAreaMeasurement()
     * @note added in QGIS 2.14
     */
    double convertLengthMeasurement( double length, QGis::UnitType toUnits ) const;

    /** Takes an area measurement calculated by this QgsDistanceArea object and converts it to a
     * different areal unit.
     * @param area area value calculated by this class to convert. It is assumed that the area
     * was calculated by this class, ie that its unit of area is equal to areaUnits().
     * @param toUnits area unit to convert measurement to
     * @returns converted area
     * @see convertLengthMeasurement()
     * @note added in QGIS 2.14
     */
    double convertAreaMeasurement( double area, QgsUnitTypes::AreaUnit toUnits ) const;

  protected:
    //! measures polygon area and perimeter, vertices are extracted from WKB
    // @note not available in python bindings
    QgsConstWkbPtr measurePolygon( QgsConstWkbPtr feature, double* area, double* perimeter, bool hasZptr = false ) const;

    /**
     * calculates distance from two points on ellipsoid
     * based on inverse Vincenty's formulae
     *
     * Points p1 and p2 are expected to be in degrees and in currently used ellipsoid
     *
     * @note if course1 is not NULL, bearing (in radians) from first point is calculated
     * (the same for course2)
     * @return distance in meters
     */
    double computeDistanceBearing( const QgsPoint& p1, const QgsPoint& p2,
                                   double* course1 = nullptr, double* course2 = nullptr ) const;

    //! uses flat / planimetric / Euclidean distance
    double computeDistanceFlat( const QgsPoint& p1, const QgsPoint& p2 ) const;

    //! calculate distance with given coordinates (does not do a transform anymore)
    double computeDistance( const QList<QgsPoint>& points ) const;

    /**
     * calculates area of polygon on ellipsoid
     * algorithm has been taken from GRASS: gis/area_poly1.c
     */
    double computePolygonArea( const QList<QgsPoint>& points ) const;

    double computePolygonFlatArea( const QList<QgsPoint>& points ) const;

    /**
     * precalculates some values
     * (must be called always when changing ellipsoid)
     */
    void computeAreaInit();

  private:

    enum MeasureType
    {
      Default,
      Area,
      Length
    };

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

    double getQ( double x ) const;
    double getQbar( double x ) const;

    double measure( const QgsAbstractGeometryV2* geomV2, MeasureType type = Default ) const;
    double measureLine( const QgsCurveV2* curve ) const;
    double measurePolygon( const QgsCurveV2* curve ) const;

    // temporary area measurement stuff

    double m_QA, m_QB, m_QC;
    double m_QbarA, m_QbarB, m_QbarC, m_QbarD;
    double m_AE;  /* a^2(1-e^2) */
    double m_Qp;  /* Q at the north pole */
    double m_E;   /* area of the earth */
    double m_TwoPI;

};

#endif

