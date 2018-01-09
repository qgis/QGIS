/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscurve.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsCurve: QgsAbstractGeometry
{
%Docstring
 Abstract base class for curved geometry type

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgscurve.h"
%End
  public:

    QgsCurve();
%Docstring
Constructor for QgsCurve.
%End

    virtual bool equals( const QgsCurve &other ) const = 0;
%Docstring
Checks whether this curve exactly equals another curve.

.. versionadded:: 3.0
%End

    virtual bool operator==( const QgsAbstractGeometry &other ) const;

    virtual bool operator!=( const QgsAbstractGeometry &other ) const;


    virtual QgsCurve *clone() const = 0 /Factory/;


    virtual QgsPoint startPoint() const = 0;
%Docstring
Returns the starting point of the curve.

.. seealso:: :py:func:`endPoint`
%End

    virtual QgsPoint endPoint() const = 0;
%Docstring
Returns the end point of the curve.

.. seealso:: :py:func:`startPoint`
%End

    virtual bool isClosed() const;
%Docstring
Returns true if the curve is closed.
%End

    virtual bool isRing() const;
%Docstring
Returns true if the curve is a ring.
%End

    virtual QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const = 0 /Factory/;
%Docstring
Returns a new line string geometry corresponding to a segmentized approximation
of the curve.

:param tolerance: segmentation tolerance
:param toleranceType: maximum segmentation angle or maximum difference between approximation and curve

Uses a MaximumAngle tolerance of 1 degrees by default (360
segments in a full circle)
%End

    virtual void addToPainterPath( QPainterPath &path ) const = 0;
%Docstring
Adds a curve to a painter path.
%End

    virtual void drawAsPolygon( QPainter &p ) const = 0;
%Docstring
Draws the curve as a polygon on the specified QPainter.

:param p: destination QPainter
%End

    virtual void points( QgsPointSequence &pt /Out/ ) const = 0;
%Docstring
Returns a list of points within the curve.
%End

    virtual int numPoints() const = 0;
%Docstring
Returns the number of points in the curve.
%End

    virtual void sumUpArea( double &sum /Out/ ) const = 0;
%Docstring
Sums up the area of the curve by iterating over the vertices (shoelace formula).
%End

    virtual QgsCoordinateSequence coordinateSequence() const;

    virtual bool nextVertex( QgsVertexId &id, QgsPoint &vertex /Out/ ) const;

    virtual void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex /Out/, QgsVertexId &nextVertex /Out/ ) const;

    virtual int vertexNumberFromVertexId( QgsVertexId id ) const;


    virtual bool pointAt( int node, QgsPoint &point /Out/, QgsVertexId::VertexType &type /Out/ ) const = 0;
%Docstring
Returns the point and vertex id of a point within the curve.

:param node: node number, where the first node is 0
:param point: will be set to point at corresponding node in the curve
:param type: will be set to the vertex type of the node

:return: true if node exists within the curve
%End

    virtual QgsCurve *reversed() const = 0 /Factory/;
%Docstring
Returns a reversed copy of the curve, where the direction of the curve has been flipped.

.. versionadded:: 2.14
%End

    virtual QgsAbstractGeometry *boundary() const /Factory/;


    virtual QgsCurve *segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;

%Docstring
Returns a geometry without curves. Caller takes ownership

:param tolerance: segmentation tolerance
:param toleranceType: maximum segmentation angle or maximum difference between approximation and curve*
%End

    virtual int vertexCount( int part = 0, int ring = 0 ) const;

    virtual int ringCount( int part = 0 ) const;

    virtual int partCount() const;

    virtual QgsPoint vertexAt( QgsVertexId id ) const;

    virtual QgsCurve *toCurveType() const /Factory/;


    virtual QgsRectangle boundingBox() const;


    virtual double xAt( int index ) const = 0;
%Docstring
Returns the x-coordinate of the specified node in the line string.

:param index: index of node, where the first node in the line is 0

:return: x-coordinate of node, or 0.0 if index is out of bounds

.. seealso:: :py:func:`setXAt`
%End

    virtual double yAt( int index ) const = 0;
%Docstring
Returns the y-coordinate of the specified node in the line string.

:param index: index of node, where the first node in the line is 0

:return: y-coordinate of node, or 0.0 if index is out of bounds

.. seealso:: :py:func:`setYAt`
%End

    QPolygonF asQPolygonF() const;
%Docstring
Returns a QPolygonF representing the points.
%End



  protected:

    virtual void clearCache() const;


    virtual int childCount() const;

    virtual QgsPoint childPoint( int index ) const;



};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgscurve.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
