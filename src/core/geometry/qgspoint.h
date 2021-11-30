/***************************************************************************
                         qgspointv2.h
                         --------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINT_H
#define QGSPOINT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractgeometry.h"
#include "qgsrectangle.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsgeometry.cpp.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \ingroup core
 * \brief Point geometry type, with support for z-dimension and m-values.
 *
 * A QgsPoint represents a 2, 3 or 4-dimensional position, with X and Y and optional
 * Z or M coordinates. Since it supports these additional dimensions, QgsPoint is
 * used as the low-level storage of geometry coordinates throughout QGIS.
 *
 * In some scenarios it is preferable to use the QgsPointXY class instead, which is
 * lighter and has smaller memory requirements compared to QgsPoint. See the QgsPointXY
 * documentation for examples of situations where it is appropriate to use QgsPointXY
 * instead of QgsPoint.
 *
 * \see QgsPointXY
 * \since QGIS 3.0, (previously QgsPointV2 since QGIS 2.10)
 */
class CORE_EXPORT QgsPoint: public QgsAbstractGeometry
{
    Q_GADGET

    Q_PROPERTY( double x READ x WRITE setX )
    Q_PROPERTY( double y READ y WRITE setY )
    Q_PROPERTY( double z READ z WRITE setZ )
    Q_PROPERTY( double m READ m WRITE setM )

  public:

    /**
     * Construct a point with the provided initial coordinate values.
     *
     * If \a wkbType is set to `QgsWkbTypes::Point`, `QgsWkbTypes::PointZ`, `QgsWkbTypes::PointM` or `QgsWkbTypes::PointZM`
     * the type will be set accordingly. If it is left to the default `QgsWkbTypes::Unknown`, the type will be set
     * based on the following rules:
     *
     * - If only x and y are specified, the type will be a 2D point.
     * - If any or both of the Z and M are specified, the appropriate type will be created.
     *
     * \code{.py}
     *   pt = QgsPoint(43.4, 5.3)
     *   pt.asWkt() # Point(43.4 5.3)
     *
     *   pt_z = QgsPoint(120, 343, 77)
     *   pt_z.asWkt() # PointZ(120 343 77)
     *
     *   pt_m = QgsPoint(33, 88, m=5)
     *   pt_m.m() # 5
     *   pt_m.wkbType() # 2001 (QgsWkbTypes.PointM)
     *
     *   pt = QgsPoint(30, 40, wkbType=QgsWkbTypes.PointZ)
     *   pt.z() # nan
     *   pt.wkbType() # 1001 (QgsWkbTypes.PointZ)
     * \endcode
     */
#ifndef SIP_RUN
    QgsPoint( double x = std::numeric_limits<double>::quiet_NaN(), double y = std::numeric_limits<double>::quiet_NaN(), double z = std::numeric_limits<double>::quiet_NaN(), double m = std::numeric_limits<double>::quiet_NaN(), QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown );
#else
    QgsPoint( SIP_PYOBJECT x SIP_TYPEHINT( Optional[Union[QgsPoint, QPointF, float]] ) = Py_None, SIP_PYOBJECT y SIP_TYPEHINT( Optional[float] ) = Py_None, SIP_PYOBJECT z SIP_TYPEHINT( Optional[float] ) = Py_None, SIP_PYOBJECT m SIP_TYPEHINT( Optional[float] ) = Py_None, SIP_PYOBJECT wkbType SIP_TYPEHINT( Optional[int] ) = Py_None ) [( double x = 0.0, double y = 0.0, double z = 0.0, double m = 0.0, QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown )];
    % MethodCode
    if ( sipCanConvertToType( a0, sipType_QgsPointXY, SIP_NOT_NONE ) && a1 == Py_None && a2 == Py_None && a3 == Py_None && a4 == Py_None )
    {
      int state;
      sipIsErr = 0;

      QgsPointXY *p = reinterpret_cast<QgsPointXY *>( sipConvertToType( a0, sipType_QgsPointXY, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
      if ( sipIsErr )
      {
        sipReleaseType( p, sipType_QgsPointXY, state );
      }
      else
      {
        sipCpp = new sipQgsPoint( QgsPoint( *p ) );
      }
    }
    else if ( sipCanConvertToType( a0, sipType_QPointF, SIP_NOT_NONE ) && a1 == Py_None && a2 == Py_None && a3 == Py_None && a4 == Py_None )
    {
      int state;
      sipIsErr = 0;

      QPointF *p = reinterpret_cast<QPointF *>( sipConvertToType( a0, sipType_QPointF, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
      if ( sipIsErr )
      {
        sipReleaseType( p, sipType_QPointF, state );
      }
      else
      {
        sipCpp = new sipQgsPoint( QgsPoint( *p ) );
      }
    }
    else if (
      ( a0 == Py_None || PyFloat_AsDouble( a0 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a1 == Py_None || PyFloat_AsDouble( a1 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a2 == Py_None || PyFloat_AsDouble( a2 ) != -1.0 || !PyErr_Occurred() ) &&
      ( a3 == Py_None || PyFloat_AsDouble( a3 ) != -1.0 || !PyErr_Occurred() ) )
    {
      double x = a0 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a0 );
      double y = a1 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a1 );
      double z = a2 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a2 );
      double m = a3 == Py_None ? std::numeric_limits<double>::quiet_NaN() : PyFloat_AsDouble( a3 );
      QgsWkbTypes::Type wkbType = a4 == Py_None ? QgsWkbTypes::Unknown : static_cast<QgsWkbTypes::Type>( sipConvertToEnum( a4, sipType_QgsWkbTypes_Type ) );
      sipCpp = new sipQgsPoint( QgsPoint( x, y, z, m, wkbType ) );
    }
    else // Invalid ctor arguments
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type in constructor arguments." ).toUtf8().constData() );
      sipIsErr = 1;
    }
    % End
#endif

    /**
     * Construct a QgsPoint from a QgsPointXY object
     */
    explicit QgsPoint( const QgsPointXY &p ) SIP_SKIP;

    /**
     * Construct a QgsPoint from a QPointF
     */
    explicit QgsPoint( QPointF p ) SIP_SKIP;

    /**
     * Create a new point with the given wkbtype and values.
     *
     * \note Not available in Python bindings
     */
    explicit QgsPoint( QgsWkbTypes::Type wkbType, double x = std::numeric_limits<double>::quiet_NaN(), double y = std::numeric_limits<double>::quiet_NaN(), double z = std::numeric_limits<double>::quiet_NaN(), double m = std::numeric_limits<double>::quiet_NaN() ) SIP_SKIP;

    bool operator==( const QgsAbstractGeometry &other ) const override SIP_HOLDGIL
    {
      const QgsPoint *pt = qgsgeometry_cast< const QgsPoint * >( &other );
      if ( !pt )
        return false;

      const QgsWkbTypes::Type type = wkbType();

      if ( pt->wkbType() != type )
        return false;

      const bool nan1X = std::isnan( mX );
      const bool nan2X = std::isnan( pt->x() );
      if ( nan1X != nan2X )
        return false;
      if ( !nan1X && !qgsDoubleNear( mX, pt->x(), 1E-8 ) )
        return false;

      const bool nan1Y = std::isnan( mY );
      const bool nan2Y = std::isnan( pt->y() );
      if ( nan1Y != nan2Y )
        return false;
      if ( !nan1Y && !qgsDoubleNear( mY, pt->y(), 1E-8 ) )
        return false;

      if ( QgsWkbTypes::hasZ( type ) )
      {
        const bool nan1Z = std::isnan( mZ );
        const bool nan2Z = std::isnan( pt->z() );
        if ( nan1Z != nan2Z )
          return false;
        if ( !nan1Z && !qgsDoubleNear( mZ, pt->z(), 1E-8 ) )
          return false;
      }

      if ( QgsWkbTypes::hasM( type ) )
      {
        const bool nan1M = std::isnan( mM );
        const bool nan2M = std::isnan( pt->m() );
        if ( nan1M != nan2M )
          return false;
        if ( !nan1M && !qgsDoubleNear( mM, pt->m(), 1E-8 ) )
          return false;
      }

      return true;
    }

    bool operator!=( const QgsAbstractGeometry &other ) const override SIP_HOLDGIL
    {
      return !operator==( other );
    }

    /**
     * Returns the point's x-coordinate.
     * \see setX()
     * \see rx()
     */
    double x() const SIP_HOLDGIL { return mX; }

    /**
     * Returns the point's y-coordinate.
     * \see setY()
     * \see ry()
     */
    double y() const SIP_HOLDGIL { return mY; }

    /**
     * Returns the point's z-coordinate.
     * \see setZ()
     * \see rz()
     */
    double z() const SIP_HOLDGIL { return mZ; }

    /**
     * Returns the point's m value.
     * \see setM()
     * \see rm()
     */
    double m() const SIP_HOLDGIL { return mM; }

    /**
     * Returns a reference to the x-coordinate of this point.
     * Using a reference makes it possible to directly manipulate x in place.
     * \see x()
     * \see setX()
     * \note not available in Python bindings
     */
    double &rx() SIP_SKIP { clearCache(); return mX; }

    /**
     * Returns a reference to the y-coordinate of this point.
     * Using a reference makes it possible to directly manipulate y in place.
     * \see y()
     * \see setY()
     * \note not available in Python bindings
     */
    double &ry() SIP_SKIP { clearCache(); return mY; }

    /**
     * Returns a reference to the z-coordinate of this point.
     * Using a reference makes it possible to directly manipulate z in place.
     * \see z()
     * \see setZ()
     * \note not available in Python bindings
     */
    double &rz() SIP_SKIP { clearCache(); return mZ; }

    /**
     * Returns a reference to the m value of this point.
     * Using a reference makes it possible to directly manipulate m in place.
     * \see m()
     * \see setM()
     * \note not available in Python bindings
     */
    double &rm() SIP_SKIP { clearCache(); return mM; }

    /**
     * Sets the point's x-coordinate.
     * \see x()
     * \see rx()
     */
    void setX( double x ) SIP_HOLDGIL
    {
      clearCache();
      mX = x;
    }

    /**
     * Sets the point's y-coordinate.
     * \see y()
     * \see ry()
     */
    void setY( double y ) SIP_HOLDGIL
    {
      clearCache();
      mY = y;
    }

    /**
     * Sets the point's z-coordinate.
     * \note calling this will have no effect if the point does not contain a z-dimension. Use addZValue() to
     * add a z value and force the point to have a z dimension.
     * \see z()
     * \see rz()
     */
    void setZ( double z ) SIP_HOLDGIL
    {
      if ( !is3D() )
        return;
      clearCache();
      mZ = z;
    }

    /**
     * Sets the point's m-value.
     * \note calling this will have no effect if the point does not contain a m-dimension. Use addMValue() to
     * add a m value and force the point to have an m dimension.
     * \see m()
     * \see rm()
     */
    void setM( double m ) SIP_HOLDGIL
    {
      if ( !isMeasure() )
        return;
      clearCache();
      mM = m;
    }

    /**
     * Returns the point as a QPointF.
     * \since QGIS 2.14
     */
    QPointF toQPointF() const SIP_HOLDGIL
    {
      return QPointF( mX, mY );
    }

    /**
     * Returns the Cartesian 2D distance between this point and a specified x, y coordinate. In certain
     * cases it may be more appropriate to call the faster distanceSquared() method, e.g.,
     * when comparing distances.
     * \see distanceSquared()
     * \since QGIS 3.0
    */
    double distance( double x, double y ) const SIP_HOLDGIL
    {
      return std::sqrt( ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y ) );
    }

    /**
     * Returns the Cartesian 2D distance between this point and another point. In certain
     * cases it may be more appropriate to call the faster distanceSquared() method, e.g.,
     * when comparing distances.
     * \see distanceSquared()
     * \since QGIS 3.0
    */
    double distance( const QgsPoint &other ) const SIP_HOLDGIL
    {
      return std::sqrt( ( mX - other.x() ) * ( mX - other.x() ) + ( mY - other.y() ) * ( mY - other.y() ) );
    }

    /**
     * Returns the Cartesian 2D squared distance between this point a specified x, y coordinate. Calling
     * this is faster than calling distance(), and may be useful in use cases such as comparing
     * distances where the extra expense of calling distance() is not required.
     * \see distance()
     * \since QGIS 3.0
    */
    double distanceSquared( double x, double y ) const SIP_HOLDGIL
    {
      return ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y );
    }

    /**
     * Returns the Cartesian 2D squared distance between this point another point. Calling
     * this is faster than calling distance(), and may be useful in use cases such as comparing
     * distances where the extra expense of calling distance() is not required.
     * \see distance()
     * \since QGIS 3.0
    */
    double distanceSquared( const QgsPoint &other ) const SIP_HOLDGIL
    {
      return ( mX - other.x() ) * ( mX - other.x() ) + ( mY - other.y() ) * ( mY - other.y() );
    }

    /**
     * Returns the Cartesian 3D distance between this point and a specified x, y, z coordinate. In certain
     * cases it may be more appropriate to call the faster distanceSquared3D() method, e.g.,
     * when comparing distances.
     * \see distanceSquared3D()
     * \since QGIS 3.0
    */
    double distance3D( double x, double y, double z ) const SIP_HOLDGIL;

    /**
     * Returns the Cartesian 3D distance between this point and another point. In certain
     * cases it may be more appropriate to call the faster distanceSquared3D() method, e.g.,
     * when comparing distances.
     * \see distanceSquared3D()
     * \since QGIS 3.0
    */
    double distance3D( const QgsPoint &other ) const SIP_HOLDGIL;

    /**
     * Returns the Cartesian 3D squared distance between this point and a specified x, y, z coordinate. Calling
     * this is faster than calling distance3D(), and may be useful in use cases such as comparing
     * distances where the extra expense of calling distance3D() is not required.
     * \see distance3D()
     * \since QGIS 3.0
    */
    double distanceSquared3D( double x, double y, double z ) const SIP_HOLDGIL;

    /**
     * Returns the Cartesian 3D squared distance between this point and another point. Calling
     * this is faster than calling distance3D(), and may be useful in use cases such as comparing
     * distances where the extra expense of calling distance3D() is not required.
     * \see distance3D()
     * \since QGIS 3.0
    */
    double distanceSquared3D( const QgsPoint &other ) const SIP_HOLDGIL;

    /**
     * Calculates Cartesian azimuth between this point and other one (clockwise in degree, starting from north)
     * \since QGIS 3.0
     */
    double azimuth( const QgsPoint &other ) const SIP_HOLDGIL;

    /**
     * Calculates Cartesian inclination between this point and other one (starting from zenith = 0 to nadir = 180. Horizon = 90)
     * Returns 90.0 if the distance between this point and other one is equal to 0 (same point).
     * \since QGIS 3.0
     */
    double inclination( const QgsPoint &other ) const SIP_HOLDGIL;

    /**
     * Returns a new point which corresponds to this point projected by a specified distance
     * with specified angles (azimuth and inclination), using Cartesian mathematics.
     * M value is preserved.
     * \param distance distance to project
     * \param azimuth angle to project in X Y, clockwise in degrees starting from north
     * \param inclination angle to project in Z (3D). If the point is 2D, the Z value is assumed to be 0.
     * \returns The point projected. If a 2D point is projected a 3D point will be returned except if
     *  inclination is 90. A 3D point is always returned if a 3D point is projected.
     *
     * ### Example
     *
     * \code{.py}
     *   p = QgsPoint( 1, 2 ) # 2D point
     *   pr = p.project ( 1, 0 )
     *   # pr is a 2D point: 'Point (1 3)'
     *   pr = p.project ( 1, 0, 90 )
     *   # pr is a 2D point: 'Point (1 3)'
     *   pr = p.project (1, 0, 0 )
     *   # pr is a 3D point: 'PointZ (1 2 nan)'
     *   p = QgsPoint( 1, 2, 2, wkbType=QgsWkbTypes.PointZ ) # 3D point
     *   pr = p.project ( 1, 0 )
     *   # pr is a 3D point: 'PointZ (1 3 2)'
     *   pr = p.project ( 1, 0, 90 )
     *   # pr is a 3D point: 'PointZ (1 3 2)'
     *   pr = p.project (1, 0, 0 )
     *   # pr is a 3D point: 'PointZ (1 2 3)'
     * \endcode
     * \since QGIS 3.0
     */
    QgsPoint project( double distance, double azimuth, double inclination = 90.0 ) const SIP_HOLDGIL;

    /**
     * Calculates the vector obtained by subtracting a point from this point.
     * \since QGIS 3.0
     */
    QgsVector operator-( const QgsPoint &p ) const SIP_HOLDGIL { return QgsVector( mX - p.mX, mY - p.mY ); }

    /**
     * Adds a vector to this point in place.
     * \since QGIS 3.0
     */
    QgsPoint &operator+=( QgsVector v ) SIP_HOLDGIL { mX += v.x(); mY += v.y(); return *this; }

    /**
     * Subtracts a vector from this point in place.
     * \since QGIS 3.0
     */
    QgsPoint &operator-=( QgsVector v ) SIP_HOLDGIL { mX -= v.x(); mY -= v.y(); return *this; }

    /**
     * Adds a vector to this point.
     * \since QGIS 3.0
     */
    QgsPoint operator+( QgsVector v ) const SIP_HOLDGIL { QgsPoint r = *this; r.rx() += v.x(); r.ry() += v.y(); return r; }

    /**
     * Subtracts a vector from this point.
     * \since QGIS 3.0
     */
    QgsPoint operator-( QgsVector v ) const SIP_HOLDGIL { QgsPoint r = *this; r.rx() -= v.x(); r.ry() -= v.y(); return r; }

    //implementation of inherited methods
    void normalize() final SIP_HOLDGIL;
    bool isEmpty() const override SIP_HOLDGIL;
    QgsRectangle boundingBox() const override SIP_HOLDGIL;
    QString geometryType() const override SIP_HOLDGIL;
    int dimension() const override SIP_HOLDGIL;
    QgsPoint *clone() const override SIP_FACTORY;
    QgsPoint *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;
    void clear() override;
    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;
    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags = QgsAbstractGeometry::WkbFlags() ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    QString asKml( int precision = 17 ) const override;
    void draw( QPainter &p ) const override;
    QPainterPath asQPainterPath() const override;
    void transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d = Qgis::TransformDirection::Forward, bool transformZ = false ) override SIP_THROW( QgsCsException );
    void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 ) override;
    QgsCoordinateSequence coordinateSequence() const override;
    int nCoordinates() const override SIP_HOLDGIL;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override SIP_HOLDGIL;

    //low-level editing
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;

    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;
    bool nextVertex( QgsVertexId &id, QgsPoint &vertex SIP_OUT ) const override;
    void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex SIP_OUT, QgsVertexId &nextVertex SIP_OUT ) const override;

    /**
     * Angle undefined. Always returns 0.0
     * \param vertex the vertex id
     * \returns 0.0
    */
    double vertexAngle( QgsVertexId vertex ) const override;

    int vertexCount( int /*part*/ = 0, int /*ring*/ = 0 ) const override;
    int ringCount( int /*part*/ = 0 ) const override;
    int partCount() const override;
    QgsPoint vertexAt( QgsVertexId /*id*/ ) const override;
    QgsPoint *toCurveType() const override SIP_FACTORY;
    double segmentLength( QgsVertexId startVertex ) const override;
    bool boundingBoxIntersects( const QgsRectangle &rectangle ) const override SIP_HOLDGIL;

    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;
    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;
    bool convertTo( QgsWkbTypes::Type type ) override;

    bool transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback = nullptr ) override;

#ifndef SIP_RUN

    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;
    void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform ) override;

    /**
     * Cast the \a geom to a QgsPoint.
     * Should be used by qgsgeometry_cast<QgsPoint *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline static const QgsPoint *cast( const QgsAbstractGeometry *geom )
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::Point )
        return static_cast<const QgsPoint *>( geom );
      return nullptr;
    }
#endif

    QgsPoint *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPoint: %1>" ).arg( sipCpp->asWkt() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:

    int compareToSameClass( const QgsAbstractGeometry *other ) const final;
    int childCount() const override;
    QgsPoint childPoint( int index ) const override;

  private:
    double mX;
    double mY;
    double mZ;
    double mM;
};

// clazy:excludeall=qstring-allocations

#endif // QGSPOINT_H
