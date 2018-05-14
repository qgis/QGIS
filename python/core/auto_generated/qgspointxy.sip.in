/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspointxy.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsPointXY
{
%Docstring
A class to represent a 2D point.

A QgsPointXY represents a position with X and Y coordinates.
In most scenarios it is preferable to use a QgsPoint instead which also
supports Z and M values.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspointxy.h"
%End
  public:
    QgsPointXY();

    QgsPointXY( const QgsPointXY &p );
%Docstring
Create a point from another point
%End

    QgsPointXY( double x, double y );
%Docstring
Create a point from x,y coordinates

:param x: x coordinate
:param y: y coordinate
%End

    QgsPointXY( QPointF point );
%Docstring
Create a point from a QPointF

:param point: QPointF source

.. versionadded:: 2.7
%End

    QgsPointXY( QPoint point );
%Docstring
Create a point from a QPoint

:param point: QPoint source

.. versionadded:: 2.7
%End

    QgsPointXY( const QgsPoint &point );
%Docstring
Create a new point.
Z and M values will be dropped.

.. versionadded:: 3.0
%End

    ~QgsPointXY();

    void setX( double x );
%Docstring
Sets the x value of the point

:param x: x coordinate
%End

    void setY( double y );
%Docstring
Sets the y value of the point

:param y: y coordinate
%End

    void set( double x, double y );
%Docstring
Sets the x and y value of the point
%End

    double x() const;
%Docstring
Get the x value of the point

:return: x coordinate
%End

    double y() const;
%Docstring
Get the y value of the point

:return: y coordinate
%End

    QPointF toQPointF() const;
%Docstring
Converts a point to a QPointF

:return: QPointF with same x and y values

.. versionadded:: 2.7
%End

    QString toString( int precision = -1 ) const;
%Docstring
Returns a string representation of the point (x, y) with a preset ``precision``.
If  ``precision`` is -1, then a default precision will be used.
%End

    QString asWkt() const;
%Docstring
Return the well known text representation for the point (e.g. "POINT(x y)").
The wkt is created without an SRID.
%End

    double sqrDist( double x, double y ) const;
%Docstring
Returns the squared distance between this point a specified x, y coordinate.

.. seealso:: :py:func:`distance`
%End

    double sqrDist( const QgsPointXY &other ) const;
%Docstring
Returns the squared distance between this point another point.

.. seealso:: :py:func:`distance`
%End

    double distance( double x, double y ) const;
%Docstring
Returns the distance between this point and a specified x, y coordinate.

:param x: x-coordniate
:param y: y-coordinate

.. seealso:: :py:func:`sqrDist`

.. versionadded:: 2.16
%End

    double distance( const QgsPointXY &other ) const;
%Docstring
Returns the distance between this point and another point.

:param other: other point

.. seealso:: :py:func:`sqrDist`

.. versionadded:: 2.16
%End

    double sqrDistToSegment( double x1, double y1, double x2, double y2, QgsPointXY &minDistPoint /Out/, double epsilon = DEFAULT_SEGMENT_EPSILON ) const;
%Docstring
Returns the minimum distance between this point and a segment
%End

    double azimuth( const QgsPointXY &other ) const;
%Docstring
Calculates azimuth between this point and other one (clockwise in degree, starting from north)
%End

    QgsPointXY project( double distance, double bearing ) const;
%Docstring
Returns a new point which corresponds to this point projected by a specified distance
in a specified bearing.

:param distance: distance to project
:param bearing: angle to project in, clockwise in degrees starting from north

.. versionadded:: 2.16
%End

    bool compare( const QgsPointXY &other, double epsilon = 4 * DBL_EPSILON ) const;
%Docstring
Compares this point with another point with a fuzzy tolerance

:param other: point to compare with
:param epsilon: maximum difference for coordinates between the points

:return: true if points are equal within specified tolerance

.. versionadded:: 2.9
%End

    bool operator==( const QgsPointXY &other );

    bool operator!=( const QgsPointXY &other ) const;

    void multiply( double scalar );
%Docstring
Multiply x and y by the given value
%End


    QgsVector operator-( const QgsPointXY &p ) const;

    QgsPointXY &operator+=( QgsVector v );

    QgsPointXY &operator-=( QgsVector v );

    QgsPointXY operator+( QgsVector v ) const;

    QgsPointXY operator-( QgsVector v ) const;

    QgsPointXY operator*( double scalar ) const;

    QgsPointXY operator/( double scalar ) const;

    QgsPointXY &operator*=( double scalar );

    QgsPointXY &operator/=( double scalar );

    operator QVariant() const;

    SIP_PYOBJECT __repr__();
%MethodCode
    QString str = "(" + QString::number( sipCpp->x() ) + "," + QString::number( sipCpp->y() ) + ")";
    //QString str("(%f,%f)").arg(sipCpp->x()).arg(sipCpp->y());
    sipRes = PyUnicode_FromString( str.toUtf8().data() );
%End

    int __len__();
%MethodCode
    sipRes = 2;
%End


    SIP_PYOBJECT __getitem__( int );
%MethodCode
    if ( a0 == 0 )
    {
      sipRes = Py_BuildValue( "d", sipCpp->x() );
    }
    else if ( a0 == 1 )
    {
      sipRes = Py_BuildValue( "d", sipCpp->y() );
    }
    else
    {
      QString msg = QString( "Bad index: %1" ).arg( a0 );
      PyErr_SetString( PyExc_IndexError, msg.toAscii().constData() );
    }
%End

    long __hash__() const;
%MethodCode
    sipRes = qHash( *sipCpp );
%End

}; // class QgsPoint






/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgspointxy.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
