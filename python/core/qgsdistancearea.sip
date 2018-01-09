/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdistancearea.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsDistanceArea
{
%Docstring
A general purpose distance and area calculator, capable of performing ellipsoid based calculations.

Measurements can either be performed on existing QgsGeometry objects, or using
lists of points.

If a valid ellipsoid() has been set for the QgsDistanceArea, all calculations will be
performed using ellipsoidal algorithms (e.g. using Vincenty's formulas). If no
ellipsoid has been set, all calculations will be performed using Cartesian
formulas only. The behavior can be determined by calling willUseEllipsoid().

In order to perform accurate calculations, the source coordinate reference system
of all measured geometries must first be specified using setSourceCrs().

Usually, the measurements returned by QgsDistanceArea are in meters. If no valid
ellipsoid is set, then the units may not be meters. The units can be retrieved
by calling lengthUnits() and areaUnits().
%End

%TypeHeaderCode
#include "qgsdistancearea.h"
%End
  public:

    QgsDistanceArea();
%Docstring
Constructor
%End

    bool willUseEllipsoid() const;
%Docstring
Returns whether calculations will use the ellipsoid. Calculations will only use the
ellipsoid if a valid ellipsoid() has been set.

.. versionadded:: 2.14

.. seealso:: :py:func:`ellipsoid`
%End

    void setSourceCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context );
%Docstring
Sets source spatial reference system ``crs``.

.. versionadded:: 2.2

.. seealso:: :py:func:`sourceCrs`
%End

    QgsCoordinateReferenceSystem sourceCrs() const;
%Docstring
Returns the source spatial reference system.

.. seealso:: :py:func:`setSourceCrs`
%End

    bool setEllipsoid( const QString &ellipsoid );
%Docstring
Sets the ``ellipsoid`` by its acronym. Known ellipsoid acronyms can be
retrieved using :py:func:`QgsEllipsoidUtils.acronyms()`
Calculations will only use the ellipsoid if a valid ellipsoid has been set.

:return: true if ellipsoid was successfully set

.. seealso:: :py:func:`ellipsoid`

.. seealso:: :py:func:`willUseEllipsoid`
%End

    bool setEllipsoid( double semiMajor, double semiMinor );
%Docstring
Sets ellipsoid by supplied radii. Calculations will only use the ellipsoid if
a valid ellipsoid been set.

:return: true if ellipsoid was successfully set

.. seealso:: :py:func:`ellipsoid`

.. seealso:: :py:func:`willUseEllipsoid`
%End

    QString ellipsoid() const;
%Docstring
Returns ellipsoid's acronym. Calculations will only use the
ellipsoid if a valid ellipsoid has been set.

.. seealso:: :py:func:`setEllipsoid`

.. seealso:: :py:func:`willUseEllipsoid`
%End

    double ellipsoidSemiMajor() const;
%Docstring
Returns the ellipsoid's semi major axis.

.. seealso:: :py:func:`ellipsoid`

.. seealso:: :py:func:`ellipsoidSemiMinor`

.. seealso:: :py:func:`ellipsoidInverseFlattening`
%End

    double ellipsoidSemiMinor() const;
%Docstring
Returns ellipsoid's semi minor axis.

.. seealso:: :py:func:`ellipsoid`

.. seealso:: :py:func:`ellipsoidSemiMajor`

.. seealso:: :py:func:`ellipsoidInverseFlattening`
%End

    double ellipsoidInverseFlattening() const;
%Docstring
Returns ellipsoid's inverse flattening.
The inverse flattening is calculated with invf = a/(a-b).

.. seealso:: :py:func:`ellipsoid`

.. seealso:: :py:func:`ellipsoidSemiMajor`

.. seealso:: :py:func:`ellipsoidSemiMinor`
%End

    double measureArea( const QgsGeometry &geometry ) const;
%Docstring
Measures the area of a geometry.

:param geometry: geometry to measure

:return: area of geometry. For geometry collections, non surface geometries will be ignored. The units for the
returned area can be retrieved by calling areaUnits().

.. versionadded:: 2.12

.. seealso:: :py:func:`measureLength`

.. seealso:: :py:func:`measurePerimeter`

.. seealso:: :py:func:`areaUnits`
%End

    double measureLength( const QgsGeometry &geometry ) const;
%Docstring
Measures the length of a geometry.

:param geometry: geometry to measure

:return: length of geometry. For geometry collections, non curve geometries will be ignored. The units for the
returned distance can be retrieved by calling lengthUnits().

.. versionadded:: 2.12

.. seealso:: :py:func:`lengthUnits`

.. seealso:: :py:func:`measureArea`

.. seealso:: :py:func:`measurePerimeter`
%End

    double measurePerimeter( const QgsGeometry &geometry ) const;
%Docstring
Measures the perimeter of a polygon geometry.

:param geometry: geometry to measure

:return: perimeter of geometry. For geometry collections, any non-polygon geometries will be ignored. The units for the
returned perimeter can be retrieved by calling lengthUnits().

.. versionadded:: 2.12

.. seealso:: :py:func:`lengthUnits`

.. seealso:: :py:func:`measureArea`

.. seealso:: :py:func:`measurePerimeter`
%End

    double measureLine( const QVector<QgsPointXY> &points ) const;
%Docstring
Measures the length of a line with multiple segments.

:param points: list of points in line

:return: length of line. The units for the returned length can be retrieved by calling lengthUnits().

.. seealso:: :py:func:`lengthUnits`
%End

    double measureLine( const QgsPointXY &p1, const QgsPointXY &p2 ) const;
%Docstring
Measures the distance between two points.

:param p1: start of line
:param p2: end of line

:return: distance between points. The units for the returned distance can be retrieved by calling lengthUnits().

.. seealso:: :py:func:`lengthUnits`
%End

    double measureLineProjected( const QgsPointXY &p1, double distance = 1, double azimuth = M_PI_2, QgsPointXY *projectedPoint /Out/ = 0 ) const;
%Docstring
Calculates the distance from one point with distance in meters and azimuth (direction)
When the sourceCrs() is geographic, computeSpheroidProject() will be called
otherwise QgsPoint.project() will be called after QgsUnitTypes.fromUnitToUnitFactor() has been applied to the distance

:param p1: start point [can be Cartesian or Geographic]
:param distance: must be in meters
:param azimuth: - azimuth in radians, clockwise from North
:param projectedPoint: calculated projected point

:return: distance in mapUnits

.. seealso:: :py:func:`sourceCrs`

.. seealso:: :py:func:`computeSpheroidProject`

.. note::

   The input Point must be in the coordinate reference system being used

.. versionadded:: 3.0
%End

    QgsUnitTypes::DistanceUnit lengthUnits() const;
%Docstring
Returns the units of distance for length calculations made by this object.

.. seealso:: :py:func:`areaUnits`

.. versionadded:: 2.14
%End

    QgsUnitTypes::AreaUnit areaUnits() const;
%Docstring
Returns the units of area for areal calculations made by this object.

.. versionadded:: 2.14

.. seealso:: :py:func:`lengthUnits`
%End

    double measurePolygon( const QVector<QgsPointXY> &points ) const;
%Docstring
Measures the area of the polygon described by a set of points.
%End

    double bearing( const QgsPointXY &p1, const QgsPointXY &p2 ) const;
%Docstring
Computes the bearing (in radians) between two points.
%End

    static QString formatDistance( double distance, int decimals, QgsUnitTypes::DistanceUnit unit, bool keepBaseUnit = false );
%Docstring
Returns an distance formatted as a friendly string.

:param distance: distance to format
:param decimals: number of decimal places to show
:param unit: unit of distance
:param keepBaseUnit: set to false to allow conversion of large distances to more suitable units, e.g., meters to
kilometers

:return: formatted distance string

.. versionadded:: 2.16

.. seealso:: :py:func:`formatArea`
%End

    static QString formatArea( double area, int decimals, QgsUnitTypes::AreaUnit unit, bool keepBaseUnit = false );
%Docstring
Returns an area formatted as a friendly string.

:param area: area to format
:param decimals: number of decimal places to show
:param unit: unit of area
:param keepBaseUnit: set to false to allow conversion of large areas to more suitable units, e.g., square meters to
square kilometers

:return: formatted area string

.. versionadded:: 2.14

.. seealso:: :py:func:`formatDistance`
%End

    double convertLengthMeasurement( double length, QgsUnitTypes::DistanceUnit toUnits ) const;
%Docstring
Takes a length measurement calculated by this QgsDistanceArea object and converts it to a
different distance unit.

:param length: length value calculated by this class to convert. It is assumed that the length
was calculated by this class, ie that its unit of length is equal to lengthUnits().
:param toUnits: distance unit to convert measurement to

:return: converted distance

.. seealso:: :py:func:`convertAreaMeasurement`

.. versionadded:: 2.14
%End

    double convertAreaMeasurement( double area, QgsUnitTypes::AreaUnit toUnits ) const;
%Docstring
Takes an area measurement calculated by this QgsDistanceArea object and converts it to a
different areal unit.

:param area: area value calculated by this class to convert. It is assumed that the area
was calculated by this class, ie that its unit of area is equal to areaUnits().
:param toUnits: area unit to convert measurement to

:return: converted area

.. seealso:: :py:func:`convertLengthMeasurement`

.. versionadded:: 2.14
%End

    QgsPointXY computeSpheroidProject( const QgsPointXY &p1, double distance = 1, double azimuth = M_PI_2 ) const;
%Docstring
Given a location, an azimuth and a distance, computes the
location of the projected point. Based on Vincenty's formula
for the geodetic direct problem as described in "Geocentric
Datum of Australia Technical Manual", Chapter 4.

:param p1: - location of first geographic (latitude/longitude) point as degrees.
:param distance: - distance in meters.
:param azimuth: - azimuth in radians, clockwise from North

:return: p2 - location of projected point as longitude/latitude.

.. note::

   code (and documentation) taken from rttopo project
https://git.osgeo.org/gogs/rttopo/librttopo
- spheroid_project.spheroid_project(...)
-  Valid bounds checking for degrees (latitude=+- 85.05115) is based values used for
-> 'WGS84 Web Mercator (Auxiliary Sphere)' calculations
--> latitudes outside these bounds cause the calculations to become unstable and can return invalid results

.. versionadded:: 3.0
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdistancearea.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
