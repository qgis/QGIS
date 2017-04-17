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
General purpose distance and area calculator.
- calculations are done on ellipsoid
- it's possible to pass points/features in any CRS, coordinates are transformed
- two options how to use it
+ use measure() takes QgsGeometry as a parameter and calculates distance or area
+ use directly measureLine(), measurePolygon() which take list of QgsPoints
(both cases transform the coordinates from source CRS to the ellipse coords)
- returned values are in meters resp. square meters
%End

%TypeHeaderCode
#include "qgsdistancearea.h"
%End
  public:

    QgsDistanceArea();
%Docstring
Constructor
%End

    QgsDistanceArea( const QgsDistanceArea &origDA );
%Docstring
Copy constructor
%End

//     QgsDistanceArea &operator=( const QgsDistanceArea &origDA );

    void setEllipsoidalMode( bool flag );
%Docstring
 Sets whether coordinates must be projected to ellipsoid before measuring
.. note::

   for calculations to use the ellipsoid, both the ellipsoid mode must be true
 and an ellipse must be set
 \see setEllipsoid()
 \see willUseEllipsoid()
%End

    bool ellipsoidalEnabled() const;
%Docstring
 Returns whether ellipsoidal calculations are enabled
 \see willUseEllipsoid()
 \see setEllipsoidalMode()
 :rtype: bool
%End

    bool willUseEllipsoid() const;
%Docstring
 Returns whether calculations will use the ellipsoid. Calculations will only use the
 ellipsoid if ellipsoidalEnabled() is true and an ellipsoid has been set.
.. versionadded:: 2.14
 \see ellipsoidalEnabled()
 \see ellipsoid()
 :rtype: bool
%End

    void setSourceCrs( long srsid );
%Docstring
 Sets source spatial reference system (by QGIS CRS).
%End

    void setSourceCrs( const QgsCoordinateReferenceSystem &srcCRS );
%Docstring
 Sets source spatial reference system (by QGIS CRS).
.. versionadded:: 2.2
 \see sourceCrs()
%End

    void setSourceAuthId( const QString &authid );
%Docstring
 Sets source spatial reference system by authid.
%End

    QgsCoordinateReferenceSystem sourceCrs() const;
%Docstring
 Returns the source spatial reference system.
 \see setSourceCrs()
 :rtype: QgsCoordinateReferenceSystem
%End

    long sourceCrsId() const;
%Docstring
 Returns the QgsCoordinateReferenceSystem.srsid() for the CRS used during calculations.
 \see setSourceCrs()
.. versionadded:: 2.14
 :rtype: long
%End

    bool geographic() const;
%Docstring
 Returns true if a geographic (latitude/longitude based) source CRS is used.
 :rtype: bool
%End

    bool setEllipsoid( const QString &ellipsoid );
%Docstring
 Sets ellipsoid by its acronym. Calculations will only use the ellipsoid if
 both the ellipsoid has been set and ellipsoidalEnabled() is true.
 :return: true if ellipsoid was successfully set
 \see ellipsoid()
 \see setEllipsoidalMode()
 \see willUseEllipsoid()
 :rtype: bool
%End

    bool setEllipsoid( double semiMajor, double semiMinor );
%Docstring
Inverse flattening is calculated with invf = a/(a-b)
 :rtype: bool
%End

    QString ellipsoid() const;
%Docstring
 Returns ellipsoid's acronym. Calculations will only use the
 ellipsoid if ellipsoidalEnabled() is true and an ellipsoid has been set.
 \see setEllipsoid()
 \see ellipsoidalEnabled()
 \see willUseEllipsoid()
 :rtype: str
%End

    double ellipsoidSemiMajor() const;
%Docstring
 Returns the ellipsoid's semi major axis.
 :rtype: float
%End

    double ellipsoidSemiMinor() const;
%Docstring
 Returns ellipsoid's semi minor axis.
 :rtype: float
%End

    double ellipsoidInverseFlattening() const;
%Docstring
 Returns ellipsoid's inverse flattening.
 :rtype: float
%End

    double measureArea( const QgsGeometry &geometry ) const;
%Docstring
 Measures the area of a geometry.
 \param geometry geometry to measure
 :return: area of geometry. For geometry collections, non surface geometries will be ignored. The units for the
 returned area can be retrieved by calling areaUnits().
.. versionadded:: 2.12
 \see measureLength()
 \see measurePerimeter()
 \see areaUnits()
 :rtype: float
%End

    double measureLength( const QgsGeometry &geometry ) const;
%Docstring
 Measures the length of a geometry.
 \param geometry geometry to measure
 :return: length of geometry. For geometry collections, non curve geometries will be ignored. The units for the
 returned distance can be retrieved by calling lengthUnits().
.. versionadded:: 2.12
 \see lengthUnits()
 \see measureArea()
 \see measurePerimeter()
 :rtype: float
%End

    double measurePerimeter( const QgsGeometry &geometry ) const;
%Docstring
 Measures the perimeter of a polygon geometry.
 \param geometry geometry to measure
 :return: perimeter of geometry. For geometry collections, any non-polygon geometries will be ignored. The units for the
 returned perimeter can be retrieved by calling lengthUnits().
.. versionadded:: 2.12
 \see lengthUnits()
 \see measureArea()
 \see measurePerimeter()
 :rtype: float
%End

    double measureLine( const QList<QgsPoint> &points ) const;
%Docstring
 Measures the length of a line with multiple segments.
 \param points list of points in line
 :return: length of line. The units for the returned length can be retrieved by calling lengthUnits().
 \see lengthUnits()
 :rtype: float
%End

    double measureLine( const QgsPoint &p1, const QgsPoint &p2 ) const;
%Docstring
 Measures length of a line with one segment.
 \param p1 start of line
 \param p2 end of line
 :return: distance between points. The units for the returned distance can be retrieved by calling lengthUnits().
 \see lengthUnits()
 :rtype: float
%End


    double measureLineProjected( const QgsPoint &p1, double distance = 1, double azimuth = M_PI / 2, QgsPoint *projectedPoint /Out/ = 0 ) const;
%Docstring
 Calculates distance from one point with distance in meters and azimuth (direction)
 When the sourceCrs() is geographic, computeSpheroidProject() will be called
 otherwise QgsPoint.project() will be called after QgsUnitTypes.fromUnitToUnitFactor() has been applied to the distance
 \note:
  The input Point must be in the coordinate reference system being used
.. versionadded:: 3.0
 \param p1 start point [can be Cartesian or Geographic]
 \param distance must be in meters
 \param azimuth - azimuth in radians, clockwise from North
 \param projectedPoint calculated projected point
 :return: distance in mapUnits
 \see sourceCrs()
 \see computeSpheroidProject()
 :rtype: float
%End

    QgsUnitTypes::DistanceUnit lengthUnits() const;
%Docstring
 Returns the units of distance for length calculations made by this object.
.. versionadded:: 2.14
 \see areaUnits()
 :rtype: QgsUnitTypes.DistanceUnit
%End

    QgsUnitTypes::AreaUnit areaUnits() const;
%Docstring
 Returns the units of area for areal calculations made by this object.
.. versionadded:: 2.14
 \see lengthUnits()
 :rtype: QgsUnitTypes.AreaUnit
%End

    double measurePolygon( const QList<QgsPoint> &points ) const;
%Docstring
 Measures the area of the polygon described by a set of points.
 :rtype: float
%End

    double bearing( const QgsPoint &p1, const QgsPoint &p2 ) const;
%Docstring
 Compute the bearing (in radians) between two points.
 :rtype: float
%End

    static QString formatDistance( double distance, int decimals, QgsUnitTypes::DistanceUnit unit, bool keepBaseUnit = false );
%Docstring
 Returns an distance formatted as a friendly string.
 \param distance distance to format
 \param decimals number of decimal places to show
 \param unit unit of distance
 \param keepBaseUnit set to false to allow conversion of large distances to more suitable units, e.g., meters to
 kilometers
 :return: formatted distance string
.. versionadded:: 2.16
 \see formatArea()
 :rtype: str
%End

    static QString formatArea( double area, int decimals, QgsUnitTypes::AreaUnit unit, bool keepBaseUnit = false );
%Docstring
 Returns an area formatted as a friendly string.
 \param area area to format
 \param decimals number of decimal places to show
 \param unit unit of area
 \param keepBaseUnit set to false to allow conversion of large areas to more suitable units, e.g., square meters to
 square kilometers
 :return: formatted area string
.. versionadded:: 2.14
 \see formatDistance()
 :rtype: str
%End

    double convertLengthMeasurement( double length, QgsUnitTypes::DistanceUnit toUnits ) const;
%Docstring
 Takes a length measurement calculated by this QgsDistanceArea object and converts it to a
 different distance unit.
 \param length length value calculated by this class to convert. It is assumed that the length
 was calculated by this class, ie that its unit of length is equal to lengthUnits().
 \param toUnits distance unit to convert measurement to
 :return: converted distance
 \see convertAreaMeasurement()
.. versionadded:: 2.14
 :rtype: float
%End

    double convertAreaMeasurement( double area, QgsUnitTypes::AreaUnit toUnits ) const;
%Docstring
 Takes an area measurement calculated by this QgsDistanceArea object and converts it to a
 different areal unit.
 \param area area value calculated by this class to convert. It is assumed that the area
 was calculated by this class, ie that its unit of area is equal to areaUnits().
 \param toUnits area unit to convert measurement to
 :return: converted area
 \see convertLengthMeasurement()
.. versionadded:: 2.14
 :rtype: float
%End

    QgsPoint computeSpheroidProject( const QgsPoint &p1, double distance = 1, double azimuth = M_PI / 2 ) const;
%Docstring
 Given a location, an azimuth and a distance, computes the
 location of the projected point. Based on Vincenty's formula
 for the geodetic direct problem as described in "Geocentric
 Datum of Australia Technical Manual", Chapter 4.
.. note::

   code (and documentation) taken from rttopo project
 https://git.osgeo.org/gogs/rttopo/librttopo
 - spheroid_project.spheroid_project(...)
.. versionadded:: 3.0
 \param p1 - location of first geographic (latitude/longitude) point as degrees.
 \param distance - distance in meters.
 \param azimuth - azimuth in radians, clockwise from North
 :return: p2 - location of projected point as longitude/latitude.
 :rtype: QgsPoint
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdistancearea.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
