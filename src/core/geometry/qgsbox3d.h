/***************************************************************************
                             qgsbox3d.h
                             ----------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBOX3D_H
#define QGSBOX3D_H

#include "qgis_core.h"
#include "qgsrectangle.h"

#include <QVector3D>

#include "qgspoint.h"

class QgsVector3D;

/**
 * \ingroup core
 * \brief A 3-dimensional box composed of x, y, z coordinates.
 *
 * A box composed of x/y/z minimum and maximum values. It is often used to return the 3D
 * extent of a geometry or collection of geometries.
 *
 * \note In QGIS 3.34 this class was renamed from QgsBox3d to QgsBox3D. The old QgsBox3d name
 * remains available in PyQGIS for compatibility.
 *
 * \see QgsRectangle
 */
class CORE_EXPORT QgsBox3D
{
    Q_GADGET

  public:

    /**
    * Constructor for QgsBox3D which accepts the ranges of x/y/z coordinates. If \a normalize is FALSE then
    * the normalization step will not be applied automatically.
    */
#ifndef SIP_RUN
    QgsBox3D( double xmin = std::numeric_limits<double>::quiet_NaN(), double ymin = std::numeric_limits<double>::quiet_NaN(), double zmin = std::numeric_limits<double>::quiet_NaN(),
              double xmax = std::numeric_limits<double>::quiet_NaN(), double ymax = std::numeric_limits<double>::quiet_NaN(), double zmax = std::numeric_limits<double>::quiet_NaN(),
              bool normalize = true );

    /**
     * Constructs a QgsBox3D from two points representing opposite corners of the box.
     * The box is normalized after construction. If \a normalize is FALSE then
     * the normalization step will not be applied automatically.
     */
    QgsBox3D( const QgsPoint &p1, const QgsPoint &p2, bool normalize = true );

    /**
     * Constructs a QgsBox3D from two 3D vectors representing opposite corners of the box.
     * The box is normalized after construction. If \a normalize is FALSE then
     * the normalization step will not be applied automatically.
     * \since QGIS 3.42
     */
    QgsBox3D( const QgsVector3D &corner1, const QgsVector3D &corner2, bool normalize = true );

    /**
     * Constructs a QgsBox3D from a rectangle.
     * If \a normalize is FALSE then the normalization step will not be applied automatically.
     */
    QgsBox3D( const QgsRectangle &rect,
              double zMin = std::numeric_limits<double>::quiet_NaN(), double zMax = std::numeric_limits<double>::quiet_NaN(),
              bool normalize = true );

#else
    QgsBox3D( SIP_PYOBJECT x SIP_TYPEHINT( Optional[Union[QgsPoint, QgsVector3D, QgsRectangle, float]] ) = Py_None, SIP_PYOBJECT y SIP_TYPEHINT( Optional[QgsPoint, QgsVector3D, float] ) = Py_None, SIP_PYOBJECT z SIP_TYPEHINT( Optional[Union[bool, float]] ) = Py_None, SIP_PYOBJECT x2 SIP_TYPEHINT( Optional[Union[bool, float]] ) = Py_None, SIP_PYOBJECT y2 SIP_TYPEHINT( Optional[float] ) = Py_None, SIP_PYOBJECT z2 SIP_TYPEHINT( Optional[float] ) = Py_None, SIP_PYOBJECT n SIP_TYPEHINT( Optional[bool] ) = Py_None ) [( double x = 0.0, double y = 0.0, double z = 0.0, double x2 = 0.0, double y2 = 0.0, double z2 = 0.0, bool n = true )];
    % MethodCode
    if ( sipCanConvertToType( a0, sipType_QgsRectangle, SIP_NOT_NONE ) && a4 == Py_None && a5 == Py_None && a6 == Py_None )
    {
      int state;
      sipIsErr = 0;

      QgsRectangle *p = reinterpret_cast<QgsRectangle *>( sipConvertToType( a0, sipType_QgsRectangle, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
      if ( sipIsErr )
      {
        sipReleaseType( p, sipType_QgsRectangle, state );
      }
      else
      {
        double z1 = a1 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a1 );
        double z2 = a2 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a2 );
        bool n = a3 == Py_None ? true : PyObject_IsTrue( a3 );

        sipCpp = new QgsBox3D( *p, z1, z2, n );
      }
    }
    else if ( sipCanConvertToType( a0, sipType_QgsPoint, SIP_NOT_NONE ) && sipCanConvertToType( a1, sipType_QgsPoint, SIP_NOT_NONE ) && a3 == Py_None && a4 == Py_None && a5 == Py_None && a6 == Py_None )
    {
      int state;
      sipIsErr = 0;

      QgsPoint *pt1 = reinterpret_cast<QgsPoint *>( sipConvertToType( a0, sipType_QgsPoint, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
      if ( sipIsErr )
      {
        sipReleaseType( pt1, sipType_QgsPoint, state );
      }
      else
      {
        QgsPoint *pt2 = reinterpret_cast<QgsPoint *>( sipConvertToType( a1, sipType_QgsPoint, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
        if ( sipIsErr )
        {
          sipReleaseType( pt2, sipType_QgsPoint, state );
        }
        else
        {
          bool n = a2 == Py_None ? true : PyObject_IsTrue( a2 );
          sipCpp = new QgsBox3D( *pt1, *pt2, n );
        }
      }
    }
    else if ( sipCanConvertToType( a0, sipType_QgsVector3D, SIP_NOT_NONE ) && sipCanConvertToType( a1, sipType_QgsVector3D, SIP_NOT_NONE ) && a3 == Py_None && a4 == Py_None && a5 == Py_None && a6 == Py_None )
    {
      int state;
      sipIsErr = 0;

      QgsVector3D *corner1 = reinterpret_cast<QgsVector3D *>( sipConvertToType( a0, sipType_QgsVector3D, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
      if ( sipIsErr )
      {
        sipReleaseType( corner1, sipType_QgsVector3D, state );
      }
      else
      {
        QgsVector3D *corner2 = reinterpret_cast<QgsVector3D *>( sipConvertToType( a1, sipType_QgsVector3D, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
        if ( sipIsErr )
        {
          sipReleaseType( corner2, sipType_QgsVector3D, state );
        }
        else
        {
          bool n = a2 == Py_None ? true : PyObject_IsTrue( a2 );
          sipCpp = new QgsBox3D( *corner1, *corner2, n );
        }
      }
    }
    else if (
      ( a0 == Py_None || PyFloat_AsDouble( a0 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a1 == Py_None || PyFloat_AsDouble( a1 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a2 == Py_None || PyFloat_AsDouble( a2 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a3 == Py_None || PyFloat_AsDouble( a3 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a4 == Py_None || PyFloat_AsDouble( a3 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a5 == Py_None || PyFloat_AsDouble( a3 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a6 == Py_None || PyFloat_AsDouble( a3 ) != -1.0 || !PyErr_Occurred() ) )
    {
      double x1 = a0 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a0 );
      double y1 = a1 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a1 );
      double z1 = a2 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a2 );
      double x2 = a3 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a3 );
      double y2 = a4 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a4 );
      double z2 = a5 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a5 );
      bool n = a6 == Py_None ? true : PyObject_IsTrue( a6 );
      sipCpp = new QgsBox3D( x1, y1, z1, x2, y2, z2, n );
    }
    else // Invalid ctor arguments
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type in constructor arguments." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    % End
#endif

    /**
     * Sets the box from a set of (x,y,z) minimum and maximum coordinates.
     *
     * \since QGIS 3.40
     */
    void set( double xMin, double yMin, double zMin, double xMax, double yMax, double zMax, bool normalize = true )
    {
      mBounds2d.set( xMin, yMin, xMax, yMax, false );
      mZmin = zMin;
      mZmax = zMax;
      if ( normalize )
      {
        QgsBox3D::normalize();
      }
    }

    /**
     * Sets the minimum \a x value.
     * \see xMinimum()
     * \see setXMaximum()
     */
    void setXMinimum( double x ) SIP_HOLDGIL;

    /**
     * Sets the maximum \a x value.
     * \see xMaximum()
     * \see setXMinimum()
     */
    void setXMaximum( double x ) SIP_HOLDGIL;

    /**
     * Returns the minimum x value.
     * \see setXMinimum()
     * \see xMaximum()
     */
    double xMinimum() const SIP_HOLDGIL { return mBounds2d.xMinimum(); }

    /**
     * Returns the maximum x value.
     * \see setXMaximum()
     * \see xMinimum()
     */
    double xMaximum() const SIP_HOLDGIL { return mBounds2d.xMaximum(); }

    /**
     * Sets the minimum \a y value.
     * \see yMinimum()
     * \see setYMaximum()
     */
    void setYMinimum( double y ) SIP_HOLDGIL;

    /**
     * Sets the maximum \a y value.
     * \see yMaximum()
     * \see setYMinimum()
     */
    void setYMaximum( double y ) SIP_HOLDGIL;

    /**
     * Returns the minimum y value.
     * \see setYMinimum()
     * \see yMaximum()
     */
    double yMinimum() const SIP_HOLDGIL { return mBounds2d.yMinimum(); }

    /**
     * Returns the maximum y value.
     * \see setYMaximum()
     * \see yMinimum()
     */
    double yMaximum() const SIP_HOLDGIL { return mBounds2d.yMaximum(); }

    /**
     * Sets the minimum \a z value.
     * \see zMinimum()
     * \see setZMaximum()
     */
    void setZMinimum( double z ) SIP_HOLDGIL;

    /**
     * Sets the maximum \a z value.
     * \see zMaximum()
     * \see setZMinimum()
     */
    void setZMaximum( double z ) SIP_HOLDGIL;

    /**
     * Returns the minimum z value.
     * \see setZMinimum()
     * \see zMaximum()
     */
    double zMinimum() const SIP_HOLDGIL { return mZmin; }

    /**
     * Returns the maximum z value.
     * \see setZMaximum()
     * \see zMinimum()
     */
    double zMaximum() const SIP_HOLDGIL { return mZmax; }

    /**
     * Mark a box as being null (holding no spatial information).
     *
     * \since QGIS 3.34
     */
    void setNull() SIP_HOLDGIL;

    /**
     * Normalize the box so it has non-negative width/height/depth.
     */
    void normalize() SIP_HOLDGIL;

    /**
     * Returns the width of the box.
     * \see height()
     * \see depth()
     */
    double width() const SIP_HOLDGIL { return mBounds2d.width(); }

    /**
     * Returns the height of the box.
     * \see width()
     * \see depth()
     */
    double height() const SIP_HOLDGIL { return mBounds2d.height(); }

    /**
     * Returns the depth of the box.
     * \see width()
     * \see height()
     */
    double depth() const SIP_HOLDGIL { return mZmax - mZmin; }

    /**
     * Returns the center of the box as a vector.
     *
     * \since QGIS 3.34
     */
    QgsVector3D center() const SIP_HOLDGIL;

    /**
     * Returns the area of the box.
     *
     * \since QGIS 3.40
     */
    double area() const SIP_HOLDGIL { return mBounds2d.area(); }

    /**
     * Returns the volume of the box.
     */
    double volume() const SIP_HOLDGIL { return mBounds2d.area() * ( mZmax - mZmin ); }

    /**
     * Returns the intersection of this box and another 3D box.
     */
    QgsBox3D intersect( const QgsBox3D &other ) const SIP_HOLDGIL;

    /**
     * Returns TRUE if the box can be considered a 2-dimensional box, i.e.
     * it has equal minimum and maximum z values.
     */
    bool is2d() const SIP_HOLDGIL;

    /**
     * Returns TRUE if the box can be considered a 3-dimensional box, i.e.
     * it has valid minimum and maximum z values.
     * If the box is not normalized, this returns FALSE.
     *
     * \since QGIS 3.34
     */
    bool is3D() const SIP_HOLDGIL;

    /**
     * Returns TRUE if box intersects with another box.
     */
    bool intersects( const QgsBox3D &other ) const SIP_HOLDGIL;

    /**
     * Returns TRUE when box contains other box.
     */
    bool contains( const QgsBox3D &other ) const SIP_HOLDGIL;

    /**
     * Returns TRUE when box contains a \a point.
     *
     * If the point is a 2D point (no z-coordinate), then the containment test
     * will be performed on the x/y extent of the box only.
     */
    bool contains( const QgsPoint &point ) const SIP_HOLDGIL;

    /**
     * Returns TRUE when box contains a point (\a x, \a y, \a z).
     * A point on the border of the box will also return TRUE
     *
     * If the point is a 2D point (no z-coordinate), then the containment test
     * will be performed on the x/y extent of the box only.
     *
     * \since QGIS 3.34
     */
    bool contains( double x, double y, double z ) const SIP_HOLDGIL;

    /**
     * Expands the bbox so that it covers both the original rectangle and the given rectangle.
     *
     * \since QGIS 3.34
     */
    void combineWith( const QgsBox3D &box ) SIP_HOLDGIL;

    /**
     * Expands the bbox so that it covers both the original rectangle and the given point.
     *
     * \since QGIS 3.34
     */
    void combineWith( double x, double y, double z ) SIP_HOLDGIL;

    /**
     * Converts the box to a 2D rectangle.
     */
    QgsRectangle toRectangle() const SIP_HOLDGIL { return mBounds2d; }

    /**
     * Returns the smallest distance between the box and the point \a point
     * (returns 0 if the point is inside the box)
     *
     * \since QGIS 3.18
     */
    double distanceTo( const  QVector3D &point ) const SIP_HOLDGIL;

    bool operator==( const QgsBox3D &other ) const SIP_HOLDGIL;

    /**
     * Scale the rectangle around a \a center QgsPoint.
     *
     * If no \a center point is specified then the current center of the box will be used.
     *
     * \since QGIS 3.26
     */
    void scale( double scaleFactor, const QgsPoint &center = QgsPoint() ) SIP_HOLDGIL;

    /**
     * Scale the rectangle around a center coordinates.
     *
     * \since QGIS 3.26
     */
    void scale( double scaleFactor, double centerX, double centerY, double centerZ ) SIP_HOLDGIL;

    /**
     * Grows the box in place by the specified amount in all dimensions.
     * \since QGIS 3.42
     */
    void grow( double delta );

    /**
     * Test if the box is null (holding no spatial information).
     *
     * A null box is also an empty box.
     *
     * \see setNull()
     *
     * \since QGIS 3.34
     */
    bool isNull() const SIP_HOLDGIL;

    /**
     * Returns TRUE if the box is empty.
     * An empty box may still be non-null if it contains valid
     * spatial information (e.g. bounding box of a point or of a vertical
     * or horizontal segment).
     *
     * \since QGIS 3.34
     */
    bool isEmpty() const SIP_HOLDGIL;

    /**
     * Returns a string representation of form xmin,ymin,zmin : xmax,ymax,zmax
     * Coordinates will be truncated to the specified precision.
     * If the specified precision is less than 0, a suitable minimum precision is used.
     *
     * \since QGIS 3.34
     */
    QString toString( int precision = 16 ) const SIP_HOLDGIL;

    /**
     * Returns an array of all box corners as 3D vectors.
     */
    QVector< QgsVector3D > corners() const SIP_HOLDGIL;

    /**
     * Returns a box offset from this one in the direction of the reversed vector.
     * \since QGIS 3.34
     */
    QgsBox3D operator-( const QgsVector3D &v ) const SIP_HOLDGIL;

    /**
     * Returns a box offset from this one in the direction of the vector.
     * \since QGIS 3.34
     */
    QgsBox3D operator+( const QgsVector3D &v ) const SIP_HOLDGIL;

    /**
     * Moves this box in the direction of the reversed vector.
     * \since QGIS 3.34
     */
    QgsBox3D &operator-=( const QgsVector3D &v ) SIP_HOLDGIL;

    /**
     * Moves this box in the direction of the vector.
     * \since QGIS 3.34
     */
    QgsBox3D &operator+=( const QgsVector3D &v ) SIP_HOLDGIL;


#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsBox3D(%1, %2, %3, %4, %5, %6)>" )
                  .arg( sipCpp->xMinimum() )
                  .arg( sipCpp->yMinimum() )
                  .arg( sipCpp->zMinimum() )
                  .arg( sipCpp->xMaximum() )
                  .arg( sipCpp->yMaximum() )
                  .arg( sipCpp->zMaximum() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    QgsRectangle mBounds2d;
    double mZmin = std::numeric_limits<double>::quiet_NaN();
    double mZmax = std::numeric_limits<double>::quiet_NaN();

};

#endif // QGSBOX3D_H
