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
 \see QgsRectangle
%End

%TypeHeaderCode
#include "qgsbox3d.h"
%End
  public:

    QgsBox3d( double xmin = 0, double ymin = 0, double mZmin = 0, double xmax = 0, double ymax = 0, double mZmax = 0 );
%Docstring
 Constructor for QgsBox3D which accepts the ranges of x/y/z coordinates.
%End

    QgsBox3d( const QgsPointV2 &p1, const QgsPointV2 &p2 );
%Docstring
 Constructs a QgsBox3D from two points representing opposite corners of the box.
 The box is normalized after construction.
%End

    void setXMinimum( double x );
%Docstring
 Sets the minimum x value.
 \see xMinimum()
 \see setXMaximum()
%End

    void setXMaximum( double x );
%Docstring
 Sets the maximum x value.
 \see xMaximum()
 \see setXMinimum()
%End

    double xMinimum() const;
%Docstring
 Returns the minimum x value.
 \see setXMinimum()
 \see xMaximum()
 :rtype: float
%End

    double xMaximum() const;
%Docstring
 Returns the maximum x value.
 \see setXMaximum()
 \see xMinimum()
 :rtype: float
%End

    void setYMinimum( double y );
%Docstring
 Sets the minimum y value.
 \see yMinimum()
 \see setYMaximum()
%End

    void setYMaximum( double y );
%Docstring
 Sets the maximum y value.
 \see yMaximum()
 \see setYMinimum()
%End

    double yMinimum() const;
%Docstring
 Returns the minimum y value.
 \see setYMinimum()
 \see yMaximum()
 :rtype: float
%End

    double yMaximum() const;
%Docstring
 Returns the maximum y value.
 \see setYMaximum()
 \see yMinimum()
 :rtype: float
%End

    void setZMinimum( double z );
%Docstring
 Sets the minimum z value.
 \see zMinimum()
 \see setZMaximum()
%End

    void setZMaximum( double z );
%Docstring
 Sets the maximum z value.
 \see zMaximum()
 \see setZMinimum()
%End

    double zMinimum() const;
%Docstring
 Returns the minimum z value.
 \see setZMinimum()
 \see zMaximum()
 :rtype: float
%End

    double zMaximum() const;
%Docstring
 Returns the maximum z value.
 \see setZMaximum()
 \see zMinimum()
 :rtype: float
%End

    void normalize();
%Docstring
 Normalize the box so it has non-negative width/height/depth.
%End

    double width() const;
%Docstring
 Returns the width of the box.
 \see height()
 \see depth()
 :rtype: float
%End

    double height() const;
%Docstring
 Returns the height of the box.
 \see width()
 \see depth()
 :rtype: float
%End

    double depth() const;
%Docstring
 Returns the depth of the box.
 \see width()
 \see height()
 :rtype: float
%End

    double volume() const;
%Docstring
 Returns the volume of the box.
 :rtype: float
%End

    QgsBox3d intersect( const QgsBox3d &other ) const;
%Docstring
 Returns the intersection of this box and another 3D box.
 :rtype: QgsBox3d
%End

    bool intersects( const QgsBox3d &other ) const;
%Docstring
 Returns true if box intersects with another box.
 :rtype: bool
%End

    bool contains( const QgsBox3d &other ) const;
%Docstring
 Returns true when box contains other box.
 :rtype: bool
%End

    bool contains( const QgsPointV2 &point ) const;
%Docstring
 Returns true when box contains a point.

 If the point is a 2D point (no z-coordinate), then the containment test
 will be performed on the x/y extent of the box only.
 :rtype: bool
%End

    QgsRectangle toRectangle() const;
%Docstring
 Converts the box to a 2D rectangle.
 :rtype: QgsRectangle
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsbox3d.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
