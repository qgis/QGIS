/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsbox3d.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsBox3d
{
%Docstring
A 3-dimensional box composed of x, y, z coordinates.

A box composed of x/y/z minimum and maximum values. It is often used to return the 3D
extent of a geometry or collection of geometries.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsRectangle`
%End

%TypeHeaderCode
#include "qgsbox3d.h"
%End
  public:

    QgsBox3d( double xmin = 0, double ymin = 0, double zmin = 0, double xmax = 0, double ymax = 0, double zmax = 0 );
%Docstring
Constructor for QgsBox3D which accepts the ranges of x/y/z coordinates.
%End

    QgsBox3d( const QgsPoint &p1, const QgsPoint &p2 );
%Docstring
Constructs a QgsBox3D from two points representing opposite corners of the box.
The box is normalized after construction.
%End

    QgsBox3d( const QgsRectangle &rect );
%Docstring
Constructs a QgsBox3D from a rectangle.
Z Minimum and Z Maximum are set to 0.0.
%End

    void setXMinimum( double x );
%Docstring
Sets the minimum ``x`` value.

.. seealso:: :py:func:`xMinimum`

.. seealso:: :py:func:`setXMaximum`
%End

    void setXMaximum( double x );
%Docstring
Sets the maximum ``x`` value.

.. seealso:: :py:func:`xMaximum`

.. seealso:: :py:func:`setXMinimum`
%End

    double xMinimum() const;
%Docstring
Returns the minimum x value.

.. seealso:: :py:func:`setXMinimum`

.. seealso:: :py:func:`xMaximum`
%End

    double xMaximum() const;
%Docstring
Returns the maximum x value.

.. seealso:: :py:func:`setXMaximum`

.. seealso:: :py:func:`xMinimum`
%End

    void setYMinimum( double y );
%Docstring
Sets the minimum ``y`` value.

.. seealso:: :py:func:`yMinimum`

.. seealso:: :py:func:`setYMaximum`
%End

    void setYMaximum( double y );
%Docstring
Sets the maximum ``y`` value.

.. seealso:: :py:func:`yMaximum`

.. seealso:: :py:func:`setYMinimum`
%End

    double yMinimum() const;
%Docstring
Returns the minimum y value.

.. seealso:: :py:func:`setYMinimum`

.. seealso:: :py:func:`yMaximum`
%End

    double yMaximum() const;
%Docstring
Returns the maximum y value.

.. seealso:: :py:func:`setYMaximum`

.. seealso:: :py:func:`yMinimum`
%End

    void setZMinimum( double z );
%Docstring
Sets the minimum ``z`` value.

.. seealso:: :py:func:`zMinimum`

.. seealso:: :py:func:`setZMaximum`
%End

    void setZMaximum( double z );
%Docstring
Sets the maximum ``z`` value.

.. seealso:: :py:func:`zMaximum`

.. seealso:: :py:func:`setZMinimum`
%End

    double zMinimum() const;
%Docstring
Returns the minimum z value.

.. seealso:: :py:func:`setZMinimum`

.. seealso:: :py:func:`zMaximum`
%End

    double zMaximum() const;
%Docstring
Returns the maximum z value.

.. seealso:: :py:func:`setZMaximum`

.. seealso:: :py:func:`zMinimum`
%End

    void normalize();
%Docstring
Normalize the box so it has non-negative width/height/depth.
%End

    double width() const;
%Docstring
Returns the width of the box.

.. seealso:: :py:func:`height`

.. seealso:: :py:func:`depth`
%End

    double height() const;
%Docstring
Returns the height of the box.

.. seealso:: :py:func:`width`

.. seealso:: :py:func:`depth`
%End

    double depth() const;
%Docstring
Returns the depth of the box.

.. seealso:: :py:func:`width`

.. seealso:: :py:func:`height`
%End

    double volume() const;
%Docstring
Returns the volume of the box.
%End

    QgsBox3d intersect( const QgsBox3d &other ) const;
%Docstring
Returns the intersection of this box and another 3D box.
%End

    bool is2d() const;
%Docstring
Returns true if the box can be considered a 2-dimensional box, i.e.
it has equal minimum and maximum z values.
%End

    bool intersects( const QgsBox3d &other ) const;
%Docstring
Returns true if box intersects with another box.
%End

    bool contains( const QgsBox3d &other ) const;
%Docstring
Returns true when box contains other box.
%End

    bool contains( const QgsPoint &point ) const;
%Docstring
Returns true when box contains a ``point``.

If the point is a 2D point (no z-coordinate), then the containment test
will be performed on the x/y extent of the box only.
%End

    QgsRectangle toRectangle() const;
%Docstring
Converts the box to a 2D rectangle.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsbox3d.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
