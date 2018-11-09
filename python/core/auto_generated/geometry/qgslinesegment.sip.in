/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgslinesegment.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLineSegment2D
{
%Docstring
Represents a single 2D line segment, consisting of a 2D start and end vertex only.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgslinesegment.h"
%End
  public:

    QgsLineSegment2D( const QgsPointXY &start, const QgsPointXY &end );
%Docstring
Constructor for a QgsLineSegment2D from the specified ``start`` point to
the ``end`` point.
%End

    QgsLineSegment2D( double x1, double y1, double x2, double y2 );
%Docstring
Constructor for a QgsLineSegment2D from the point (``x1``, ``y2``) to
(``x2``, ``y2``).
%End

    double length() const;
%Docstring
Returns the length of the segment.

.. seealso:: :py:func:`lengthSquared`
%End

    double lengthSquared() const;
%Docstring
Returns the squared length of the segment.

.. seealso:: :py:func:`length`
%End

    double startX() const;
%Docstring
Returns the segment's start x-coordinate.

.. seealso:: :py:func:`start`

.. seealso:: :py:func:`startY`
%End

    double startY() const;
%Docstring
Returns the segment's start y-coordinate.

.. seealso:: :py:func:`start`

.. seealso:: :py:func:`startX`
%End

    double endX() const;
%Docstring
Returns the segment's end x-coordinate.

.. seealso:: :py:func:`end`

.. seealso:: :py:func:`endY`
%End

    double endY() const;
%Docstring
Returns the segment's end y-coordinate.

.. seealso:: :py:func:`end`

.. seealso:: :py:func:`endX`
%End

    QgsPointXY start() const;
%Docstring
Returns the segment's start point.

.. seealso:: :py:func:`end`

.. seealso:: :py:func:`startX`

.. seealso:: :py:func:`startY`
%End

    QgsPointXY end() const;
%Docstring
Returns the segment's end point.

.. seealso:: :py:func:`start`

.. seealso:: :py:func:`endX`

.. seealso:: :py:func:`endY`
%End

    void setStartX( double x );
%Docstring
Sets the segment's start ``x`` coordinate.

.. seealso:: :py:func:`setEndX`

.. seealso:: :py:func:`setStart`

.. seealso:: :py:func:`setStartY`
%End

    void setStartY( double y );
%Docstring
Sets the segment's start ``y`` coordinate.

.. seealso:: :py:func:`setEndY`

.. seealso:: :py:func:`setStart`

.. seealso:: :py:func:`setStartX`
%End

    void setEndX( double x );
%Docstring
Sets the segment's end ``x`` coordinate.

.. seealso:: :py:func:`setStartX`

.. seealso:: :py:func:`setEnd`

.. seealso:: :py:func:`setEndY`
%End

    void setEndY( double y );
%Docstring
Sets the segment's end ``y`` coordinate.

.. seealso:: :py:func:`setStartY`

.. seealso:: :py:func:`setEnd`

.. seealso:: :py:func:`setEndX`
%End

    void setStart( const QgsPointXY &start );
%Docstring
Sets the segment's ``start`` point.

.. seealso:: :py:func:`setStartX`

.. seealso:: :py:func:`setStartY`

.. seealso:: :py:func:`setEnd`
%End

    void setEnd( const QgsPointXY &end );
%Docstring
Sets the segment's ``end`` point.

.. seealso:: :py:func:`setEndX`

.. seealso:: :py:func:`setEndY`

.. seealso:: :py:func:`setStart`
%End

    int pointLeftOfLine( const QgsPointXY &point ) const;
%Docstring
Tests if a ``point`` is to the left of the line segment.

Returns -1 if the point falls to the left of the line, or +1 if the point
is to the right.

If the return value is 0, then the test was unsuccessful (e.g. due to testing a point exactly
on the line, or exactly in line with the segment) and the result is undefined.

.. seealso:: :py:func:`QgsGeometryUtils.leftOfLine`
%End

    void reverse();
%Docstring
Reverses the line segment, so that the start and end points are flipped.
%End

    bool operator==( const QgsLineSegment2D &other ) const;

    bool operator!=( const QgsLineSegment2D &other ) const;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgslinesegment.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
