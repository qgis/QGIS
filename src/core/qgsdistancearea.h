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

#include "qgis_core.h"
#include <QVector>
#include <QReadWriteLock>
#include "qgscoordinatetransform.h"
#include "qgsunittypes.h"
#include "qgsellipsoidutils.h"

class QgsGeometry;
class QgsAbstractGeometry;
class QgsCurve;

/**
 * \ingroup core
 * A general purpose distance and area calculator, capable of performing ellipsoid based calculations.
 *
 * Measurements can either be performed on existing QgsGeometry objects, or using
 * lists of points.
 *
 * If a valid ellipsoid() has been set for the QgsDistanceArea, all calculations will be
 * performed using ellipsoidal algorithms (e.g. using Vincenty's formulas). If no
 * ellipsoid has been set, all calculations will be performed using Cartesian
 * formulas only. The behavior can be determined by calling willUseEllipsoid().
 *
 * In order to perform accurate calculations, the source coordinate reference system
 * of all measured geometries must first be specified using setSourceCrs().
 *
 * Usually, the measurements returned by QgsDistanceArea are in meters. If no valid
 * ellipsoid is set, then the units may not be meters. The units can be retrieved
 * by calling lengthUnits() and areaUnits().
*/
class CORE_EXPORT QgsDistanceArea
{
  public:

    //! Constructor
    QgsDistanceArea();

    /**
     * Returns whether calculations will use the ellipsoid. Calculations will only use the
     * ellipsoid if a valid ellipsoid() has been set.
     * \since QGIS 2.14
     * \see ellipsoid()
     */
    bool willUseEllipsoid() const;

    /**
     * Sets source spatial reference system.
     * \since QGIS 2.2
     * \see sourceCrs()
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem &srcCRS );

    /**
     * Returns the source spatial reference system.
     * \see setSourceCrs()
     */
    QgsCoordinateReferenceSystem sourceCrs() const { return mCoordTransform.sourceCrs(); }

    /**
     * Sets the \a ellipsoid by its acronym. Known ellipsoid acronyms can be
     * retrieved using QgsEllipsoidUtils::acronyms().
     * Calculations will only use the ellipsoid if a valid ellipsoid has been set.
     * \returns true if ellipsoid was successfully set
     * \see ellipsoid()
     * \see willUseEllipsoid()
     */
    bool setEllipsoid( const QString &ellipsoid );

    /**
     * Sets ellipsoid by supplied radii. Calculations will only use the ellipsoid if
     * a valid ellipsoid been set.
     * \returns true if ellipsoid was successfully set
     * \see ellipsoid()
     * \see willUseEllipsoid()
     */
    bool setEllipsoid( double semiMajor, double semiMinor );

    /**
     * Returns ellipsoid's acronym. Calculations will only use the
     * ellipsoid if a valid ellipsoid has been set.
     * \see setEllipsoid()
     * \see willUseEllipsoid()
     */
    QString ellipsoid() const { return mEllipsoid; }

    /**
     * Returns the ellipsoid's semi major axis.
     * \see ellipsoid()
     * \see ellipsoidSemiMinor()
     * \see ellipsoidInverseFlattening()
     */
    double ellipsoidSemiMajor() const { return mSemiMajor; }

    /**
     * Returns ellipsoid's semi minor axis.
     * \see ellipsoid()
     * \see ellipsoidSemiMajor()
     * \see ellipsoidInverseFlattening()
     */
    double ellipsoidSemiMinor() const { return mSemiMinor; }

    /**
     * Returns ellipsoid's inverse flattening.
     * The inverse flattening is calculated with invf = a/(a-b).
     * \see ellipsoid()
     * \see ellipsoidSemiMajor()
     * \see ellipsoidSemiMinor()
     */
    double ellipsoidInverseFlattening() const { return mInvFlattening; }

    /**
     * Measures the area of a geometry.
     * \param geometry geometry to measure
     * \returns area of geometry. For geometry collections, non surface geometries will be ignored. The units for the
     * returned area can be retrieved by calling areaUnits().
     * \since QGIS 2.12
     * \see measureLength()
     * \see measurePerimeter()
     * \see areaUnits()
     */
    double measureArea( const QgsGeometry &geometry ) const;

    /**
     * Measures the length of a geometry.
     * \param geometry geometry to measure
     * \returns length of geometry. For geometry collections, non curve geometries will be ignored. The units for the
     * returned distance can be retrieved by calling lengthUnits().
     * \since QGIS 2.12
     * \see lengthUnits()
     * \see measureArea()
     * \see measurePerimeter()
     */
    double measureLength( const QgsGeometry &geometry ) const;

    /**
     * Measures the perimeter of a polygon geometry.
     * \param geometry geometry to measure
     * \returns perimeter of geometry. For geometry collections, any non-polygon geometries will be ignored. The units for the
     * returned perimeter can be retrieved by calling lengthUnits().
     * \since QGIS 2.12
     * \see lengthUnits()
     * \see measureArea()
     * \see measurePerimeter()
     */
    double measurePerimeter( const QgsGeometry &geometry ) const;

    /**
     * Measures the length of a line with multiple segments.
     * \param points list of points in line
     * \returns length of line. The units for the returned length can be retrieved by calling lengthUnits().
     * \see lengthUnits()
     */
    double measureLine( const QVector<QgsPointXY> &points ) const;

    /**
     * Measures the distance between two points.
     * \param p1 start of line
     * \param p2 end of line
     * \returns distance between points. The units for the returned distance can be retrieved by calling lengthUnits().
     * \see lengthUnits()
     */
    double measureLine( const QgsPointXY &p1, const QgsPointXY &p2 ) const;

    /**
     * Calculates the distance from one point with distance in meters and azimuth (direction)
     * When the sourceCrs() is geographic, computeSpheroidProject() will be called
     * otherwise QgsPoint.project() will be called after QgsUnitTypes::fromUnitToUnitFactor() has been applied to the distance
     * \note:
     *  The input Point must be in the coordinate reference system being used
     * \since QGIS 3.0
     * \param p1 start point [can be Cartesian or Geographic]
     * \param distance must be in meters
     * \param azimuth - azimuth in radians, clockwise from North
     * \param projectedPoint calculated projected point
     * \return distance in mapUnits
     * \see sourceCrs()
     * \see computeSpheroidProject()
     */
    double measureLineProjected( const QgsPointXY &p1, double distance = 1, double azimuth = M_PI_2, QgsPointXY *projectedPoint SIP_OUT = nullptr ) const;

    /**
     * Returns the units of distance for length calculations made by this object.
     * \since QGIS 2.14
     * \see areaUnits()
     */
    QgsUnitTypes::DistanceUnit lengthUnits() const;

    /**
     * Returns the units of area for areal calculations made by this object.
     * \since QGIS 2.14
     * \see lengthUnits()
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /**
     * Measures the area of the polygon described by a set of points.
     */
    double measurePolygon( const QVector<QgsPointXY> &points ) const;

    /**
     * Computes the bearing (in radians) between two points.
     */
    double bearing( const QgsPointXY &p1, const QgsPointXY &p2 ) const;

    /**
     * Returns an distance formatted as a friendly string.
     * \param distance distance to format
     * \param decimals number of decimal places to show
     * \param unit unit of distance
     * \param keepBaseUnit set to false to allow conversion of large distances to more suitable units, e.g., meters to
     * kilometers
     * \returns formatted distance string
     * \since QGIS 2.16
     * \see formatArea()
     */
    static QString formatDistance( double distance, int decimals, QgsUnitTypes::DistanceUnit unit, bool keepBaseUnit = false );

    /**
     * Returns an area formatted as a friendly string.
     * \param area area to format
     * \param decimals number of decimal places to show
     * \param unit unit of area
     * \param keepBaseUnit set to false to allow conversion of large areas to more suitable units, e.g., square meters to
     * square kilometers
     * \returns formatted area string
     * \since QGIS 2.14
     * \see formatDistance()
     */
    static QString formatArea( double area, int decimals, QgsUnitTypes::AreaUnit unit, bool keepBaseUnit = false );

    /**
     * Takes a length measurement calculated by this QgsDistanceArea object and converts it to a
     * different distance unit.
     * \param length length value calculated by this class to convert. It is assumed that the length
     * was calculated by this class, ie that its unit of length is equal to lengthUnits().
     * \param toUnits distance unit to convert measurement to
     * \returns converted distance
     * \see convertAreaMeasurement()
     * \since QGIS 2.14
     */
    double convertLengthMeasurement( double length, QgsUnitTypes::DistanceUnit toUnits ) const;

    /**
     * Takes an area measurement calculated by this QgsDistanceArea object and converts it to a
     * different areal unit.
     * \param area area value calculated by this class to convert. It is assumed that the area
     * was calculated by this class, ie that its unit of area is equal to areaUnits().
     * \param toUnits area unit to convert measurement to
     * \returns converted area
     * \see convertLengthMeasurement()
     * \since QGIS 2.14
     */
    double convertAreaMeasurement( double area, QgsUnitTypes::AreaUnit toUnits ) const;

    /**
     * Given a location, an azimuth and a distance, computes the
     * location of the projected point. Based on Vincenty's formula
     * for the geodetic direct problem as described in "Geocentric
     * Datum of Australia Technical Manual", Chapter 4.
     * \note code (and documentation) taken from rttopo project
     * https://git.osgeo.org/gogs/rttopo/librttopo
     * - spheroid_project.spheroid_project(...)
     * -  Valid bounds checking for degrees (latitude=+- 85.05115) is based values used for
     * -> 'WGS84 Web Mercator (Auxiliary Sphere)' calculations
     * --> latitudes outside these bounds cause the calculations to become unstable and can return invalid results
     * \since QGIS 3.0
     * \param p1 - location of first geographic (latitude/longitude) point as degrees.
     * \param distance - distance in meters.
     * \param azimuth - azimuth in radians, clockwise from North
     * \return p2 - location of projected point as longitude/latitude.
     */
    QgsPointXY computeSpheroidProject( const QgsPointXY &p1, double distance = 1, double azimuth = M_PI_2 ) const;

  private:

    /**
     * Calculates distance from two points on ellipsoid
     * based on inverse Vincenty's formulae
     *
     * Points p1 and p2 are expected to be in degrees and in currently used ellipsoid
     *
     * \note if course1 is not NULL, bearing (in radians) from first point is calculated
     * (the same for course2)
     * \returns distance in meters
     */
    double computeDistanceBearing( const QgsPointXY &p1, const QgsPointXY &p2,
                                   double *course1 = nullptr, double *course2 = nullptr ) const;

    /**
     * Calculates area of polygon on ellipsoid
     * algorithm has been taken from GRASS: gis/area_poly1.c
     */
    double computePolygonArea( const QVector<QgsPointXY> &points ) const;

    double computePolygonFlatArea( const QVector<QgsPointXY> &points ) const;

    /**
     * Precalculates some values
     * (must be called always when changing ellipsoid)
     */
    void computeAreaInit();

    void setFromParams( const QgsEllipsoidUtils::EllipsoidParameters &params );

    enum MeasureType
    {
      Default,
      Area,
      Length
    };

    //! used for transforming coordinates from source CRS to ellipsoid's coordinates
    QgsCoordinateTransform mCoordTransform;

    //! ellipsoid acronym (from table tbl_ellipsoids)
    QString mEllipsoid;

    //! ellipsoid parameters
    double mSemiMajor, mSemiMinor, mInvFlattening;

    // utility functions for polygon area measurement

    double getQ( double x ) const;
    double getQbar( double x ) const;

    double measure( const QgsAbstractGeometry *geomV2, MeasureType type = Default ) const;
    double measureLine( const QgsCurve *curve ) const;
    double measurePolygon( const QgsCurve *curve ) const;

    // temporary area measurement stuff

    double m_QA, m_QB, m_QC;
    double m_QbarA, m_QbarB, m_QbarC, m_QbarD;
    double m_AE;  /* a^2(1-e^2) */
    double m_Qp;  /* Q at the north pole */
    double m_E;   /* area of the earth */
    double m_TwoPI;

};

#endif

