/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaptopixel.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMapToPixel
{
%Docstring
Perform transforms between map coordinates and device coordinates.

This class can convert device coordinates to map coordinates and vice versa.
%End

%TypeHeaderCode
#include "qgsmaptopixel.h"
%End
  public:

    QgsMapToPixel( double mapUnitsPerPixel, double centerX, double centerY, int widthPixels, int heightPixels, double rotation );
%Docstring
Constructor

:param mapUnitsPerPixel: Map units per pixel
:param centerX: X coordinate of map center, in geographical units
:param centerY: Y coordinate of map center, in geographical units
:param widthPixels: Output width, in pixels
:param heightPixels: Output height, in pixels
:param rotation: clockwise rotation in degrees

.. versionadded:: 2.8
%End

    QgsMapToPixel( double mapUnitsPerPixel );
%Docstring
Constructor

:param mapUnitsPerPixel: Map units per pixel
%End

    static QgsMapToPixel fromScale( double scale, QgsUnitTypes::DistanceUnit mapUnits, double dpi = 96 );
%Docstring
Returns a new QgsMapToPixel created using a specified ``scale`` and distance unit.

:param scale: map scale denominator, e.g. 1000.0 for a 1:1000 map.
:param dpi: screen DPI
:param mapUnits: map units

:return: matching QgsMapToPixel

.. versionadded:: 3.0
%End

    QgsMapToPixel();
%Docstring
Constructor

Use setParameters to fill
%End

    QgsPointXY transform( const QgsPointXY &p ) const;
%Docstring
Transform the point from map (world) coordinates to device coordinates

:param p: Point to transform

:return: QgsPointXY in device coordinates
%End

    void transform( QgsPointXY *p ) const;

    QgsPointXY transform( qreal x, qreal y ) const;
%Docstring
Transform the point specified by x,y from map (world)
coordinates to device coordinates

:param x: x cordinate o point to transform
:param y: y coordinate of point to transform

:return: QgsPointXY in device coordinates
%End

    void transformInPlace( double &x, double &y ) const;
%Docstring
Transform device coordinates to map coordinates. Modifies the
given coordinates in place. Intended as a fast way to do the
transform.
%End



    QgsPointXY toMapCoordinates( int x, int y ) const;

    QgsPointXY toMapCoordinatesF( double x, double y ) const;
%Docstring
Transform device coordinates to map (world) coordinates
%End

    QgsPointXY toMapCoordinates( QPoint p ) const;
%Docstring
Transform device coordinates to map (world) coordinates

:param p: Point to be converted to map cooordinates

:return: QgsPointXY in map coorndiates
%End

    QgsPointXY toMapPoint( double x, double y ) const;

    void setMapUnitsPerPixel( double mapUnitsPerPixel );
%Docstring
Set map units per pixel

:param mapUnitsPerPixel: Map units per pixel
%End

    double mapUnitsPerPixel() const;
%Docstring
Return current map units per pixel
%End

    int mapWidth() const;
%Docstring
Return current map width in pixels
The information is only known if setRotation was used

.. versionadded:: 2.8
%End

    int mapHeight() const;
%Docstring
Return current map height in pixels

.. versionadded:: 2.8
%End

    void setMapRotation( double degrees, double cx, double cy );
%Docstring
Set map rotation in degrees (clockwise)

:param degrees: clockwise rotation in degrees
:param cx: X ordinate of map center in geographical units
:param cy: Y ordinate of map center in geographical units

.. versionadded:: 2.8
%End

    double mapRotation() const;
%Docstring
Return current map rotation in degrees

.. versionadded:: 2.8
%End

    void setParameters( double mapUnitsPerPixel, double centerX, double centerY, int widthPixels, int heightPixels, double rotation );
%Docstring
Set parameters for use in transforming coordinates

:param mapUnitsPerPixel: Map units per pixel
:param centerX: X coordinate of map center, in geographical units
:param centerY: Y coordinate of map center, in geographical units
:param widthPixels: Output width, in pixels
:param heightPixels: Output height, in pixels
:param rotation: clockwise rotation in degrees

.. versionadded:: 2.8
%End

    QString showParameters() const;
%Docstring
String representation of the parameters used in the transform
%End

    QTransform transform() const;

    double xCenter() const;
%Docstring
Returns the center x-coordinate for the transform.

.. seealso:: :py:func:`yCenter`

.. versionadded:: 3.0
%End

    double yCenter() const;
%Docstring
Returns the center y-coordinate for the transform.

.. seealso:: :py:func:`xCenter`

.. versionadded:: 3.0
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaptopixel.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
