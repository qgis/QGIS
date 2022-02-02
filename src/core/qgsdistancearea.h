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
struct geod_geodesic;

/**
 * \ingroup core
 * \brief A general purpose distance and area calculator, capable of performing ellipsoid based calculations.
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
 *
 * Internally, the GeographicLib library is used to calculate all ellipsoid based measurements.
*/
class CORE_EXPORT QgsDistanceArea
{
  public:

    //! Constructor
    QgsDistanceArea();
    ~QgsDistanceArea();

    //! Copy constructor
    QgsDistanceArea( const QgsDistanceArea &other );
    QgsDistanceArea &operator=( const QgsDistanceArea &other );

    /**
     * Returns whether calculations will use the ellipsoid. Calculations will only use the
     * ellipsoid if a valid ellipsoid() has been set.
     * \see ellipsoid()
     * \since QGIS 2.14
     */
    bool willUseEllipsoid() const;

    /**
     * Sets source spatial reference system \a crs.
     * \see sourceCrs()
     * \since QGIS 2.2
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );

    /**
     * Returns the source spatial reference system.
     * \see setSourceCrs()
     * \see ellipsoidCrs()
     */
    QgsCoordinateReferenceSystem sourceCrs() const { return mCoordTransform.sourceCrs(); }

    /**
     * Returns the ellipsoid (destination) spatial reference system.
     * \see sourceCrs()
     * \see ellipsoid()
     * \since QGIS 3.6
     */
    QgsCoordinateReferenceSystem ellipsoidCrs() const { return mCoordTransform.destinationCrs(); }

    /**
     * Sets the \a ellipsoid by its acronym. Known ellipsoid acronyms can be
     * retrieved using QgsEllipsoidUtils::acronyms().
     * Calculations will only use the ellipsoid if a valid ellipsoid has been set.
     * \returns TRUE if ellipsoid was successfully set
     * \see ellipsoid()
     * \see willUseEllipsoid()
     */
    bool setEllipsoid( const QString &ellipsoid );

    /**
     * Sets ellipsoid by supplied radii. Calculations will only use the ellipsoid if
     * a valid ellipsoid been set.
     * \returns TRUE if ellipsoid was successfully set
     * \see ellipsoid()
     * \see willUseEllipsoid()
     */
    bool setEllipsoid( double semiMajor, double semiMinor );

    /**
     * Returns ellipsoid's acronym. Calculations will only use the
     * ellipsoid if a valid ellipsoid has been set.
     * \see setEllipsoid()
     * \see willUseEllipsoid()
     * \see ellipsoidCrs()
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
     * \see measureLength()
     * \see measurePerimeter()
     * \see areaUnits()
     * \since QGIS 2.12
     */
    double measureArea( const QgsGeometry &geometry ) const;

    /**
     * Measures the length of a geometry.
     * \param geometry geometry to measure
     * \returns length of geometry. For geometry collections, non curve geometries will be ignored. The units for the
     * returned distance can be retrieved by calling lengthUnits().
     * \see lengthUnits()
     * \see measureArea()
     * \see measurePerimeter()
     * \since QGIS 2.12
     */
    double measureLength( const QgsGeometry &geometry ) const;

    /**
     * Measures the perimeter of a polygon geometry.
     * \param geometry geometry to measure
     * \returns perimeter of geometry. For geometry collections, any non-polygon geometries will be ignored. The units for the
     * returned perimeter can be retrieved by calling lengthUnits().
     * \see lengthUnits()
     * \see measureArea()
     * \see measurePerimeter()
     * \since QGIS 2.12
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
     * \param p1 start point [can be Cartesian or Geographic]
     * \param distance must be in meters
     * \param azimuth - azimuth in radians, clockwise from North
     * \param projectedPoint calculated projected point
     * \return distance in mapUnits
     * \see sourceCrs()
     * \see computeSpheroidProject()
     * \note The input Point must be in the coordinate reference system being used
     * \since QGIS 3.0
     */
    double measureLineProjected( const QgsPointXY &p1, double distance = 1, double azimuth = M_PI_2, QgsPointXY *projectedPoint SIP_OUT = nullptr ) const;

    /**
     * Returns the units of distance for length calculations made by this object.
     * \see areaUnits()
     * \since QGIS 2.14
     */
    QgsUnitTypes::DistanceUnit lengthUnits() const;

    /**
     * Returns the units of area for areal calculations made by this object.
     * \see lengthUnits()
     * \since QGIS 2.14
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /**
     * Measures the area of the polygon described by a set of points.
     */
    double measurePolygon( const QVector<QgsPointXY> &points ) const;

    /**
     * Computes the bearing (in radians) between two points.
     *
     * \throws QgsCsException on invalid input coordinates
     */
    double bearing( const QgsPointXY &p1, const QgsPointXY &p2 ) const SIP_THROW( QgsCsException );

    /**
     * Returns an distance formatted as a friendly string.
     * \param distance distance to format
     * \param decimals number of decimal places to show
     * \param unit unit of distance
     * \param keepBaseUnit set to FALSE to allow conversion of large distances to more suitable units, e.g., meters to
     * kilometers
     * \returns formatted distance string
     * \see formatArea()
     * \since QGIS 2.16
     */
    static QString formatDistance( double distance, int decimals, QgsUnitTypes::DistanceUnit unit, bool keepBaseUnit = false );

    /**
     * Returns an area formatted as a friendly string.
     * \param area area to format
     * \param decimals number of decimal places to show
     * \param unit unit of area
     * \param keepBaseUnit set to FALSE to allow conversion of large areas to more suitable units, e.g., square meters to
     * square kilometers
     * \returns formatted area string
     * \see formatDistance()
     * \since QGIS 2.14
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
     * location of the projected point.
     *
     * \param p1 - location of first geographic (latitude/longitude) point as degrees.
     * \param distance - distance in meters.
     * \param azimuth - azimuth in radians, clockwise from North
     * \return p2 - location of projected point as longitude/latitude.
     *
     * \since QGIS 3.0
     */
    QgsPointXY computeSpheroidProject( const QgsPointXY &p1, double distance = 1, double azimuth = M_PI_2 ) const;

    /**
     * Calculates the geodesic line between \a p1 and \a p2, which represents the shortest path on the
     * ellipsoid between these two points.
     *
     * The ellipsoid settings defined on this QgsDistanceArea object will be used during the calculations.
     *
     * \a p1 and \a p2 must be in the sourceCrs() of this QgsDistanceArea object. The returned line
     * will also be in this same CRS.
     *
     * The \a interval parameter gives the maximum distance between points on the computed line.
     * This argument is always specified in meters. A shorter distance results in a denser line,
     * at the cost of extra computing time.
     *
     * If the geodesic line crosses the antimeridian (+/- 180 degrees longitude) and \a breakLine is TRUE, then
     * the line will be split into two parts, broken at the antimeridian. In this case the function
     * will return two lines, corresponding to the portions at either side of the antimeridian.
     *
     * \since QGIS 3.6
     */
    QVector<QVector<QgsPointXY> > geodesicLine( const QgsPointXY &p1, const QgsPointXY &p2, double interval, bool breakLine = false ) const;

    /**
     * Calculates the latitude at which the geodesic line joining \a p1 and \a p2 crosses
     * the antimeridian (longitude +/- 180 degrees).
     *
     * The ellipsoid settings defined on this QgsDistanceArea object will be used during the calculations.
     *
     * \a p1 and \a p2 must be in the ellipsoidCrs() of this QgsDistanceArea object. The returned latitude
     * will also be in this same CRS.
     *
     * \param p1 Starting point, in ellipsoidCrs()
     * \param p2 Ending point, in ellipsoidCrs()
     * \param fractionAlongLine will be set to the fraction along the geodesic line joining \a p1 to \a p2 at which the antimeridian crossing occurs.
     *
     * \returns the latitude at which the geodesic crosses the antimeridian
     * \see splitGeometryAtAntimeridian()
     *
     * \since QGIS 3.6
     */
    double latitudeGeodesicCrossesAntimeridian( const QgsPointXY &p1, const QgsPointXY &p2, double &fractionAlongLine SIP_OUT ) const;

    /**
     * Splits a (Multi)LineString \a geometry at the antimeridian (longitude +/- 180 degrees).
     * The returned geometry will always be a multi-part geometry.
     *
     * Whenever line segments in the input geometry cross the antimeridian, they will be
     * split into two segments, with the latitude of the breakpoint being determined using a geodesic
     * line connecting the points either side of this segment.
     *
     * The ellipsoid settings defined on this QgsDistanceArea object will be used during the calculations.
     *
     * \a geometry must be in the sourceCrs() of this QgsDistanceArea object. The returned geometry
     * will also be in this same CRS.
     *
     * If \a geometry contains M or Z values, these will be linearly interpolated for the new vertices
     * created at the antimeridian.
     *
     * \note Non-(Multi)LineString geometries will be returned unchanged.
     *
     * \see latitudeGeodesicCrossesAntimeridian()
     * \since QGIS 3.6
     */
    QgsGeometry splitGeometryAtAntimeridian( const QgsGeometry &geometry ) const;

  private:

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
    void computeAreaInit() const;

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

    mutable std::unique_ptr< geod_geodesic > mGeod;

    // utility functions for polygon area measurement

    double measure( const QgsAbstractGeometry *geomV2, MeasureType type = Default ) const;
    double measureLine( const QgsCurve *curve ) const;
    double measurePolygon( const QgsCurve *curve ) const;

};

#endif

