/***************************************************************************
                         qgslinestring.h
                         -----------------
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

#ifndef QGSLINESTRING_H
#define QGSLINESTRING_H


#include <QPolygonF>

#include "qgis_core.h"
#include "qgis.h"
#include "qgscurve.h"
#include "qgscompoundcurve.h"

class QgsLineSegment2D;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsgeometry.cpp.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \ingroup core
 * \class QgsLineString
 * \brief Line string geometry type, with support for z-dimension and m-values.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsLineString: public QgsCurve
{
  public:
    QgsLineString();

    /**
     * Construct a linestring from a vector of points.
     * Z and M type will be set based on the type of the first point
     * in the vector.
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<QgsPoint> &points );

    /**
     * Construct a linestring from arrays of coordinates. If the z or m
     * arrays are non-empty then the resultant linestring will have
     * z and m types accordingly.
     * This constructor is more efficient then calling setPoints()
     * or repeatedly calling addVertex()
     *
     * If the \a z vector is filled, then the geometry type will either
     * be a LineStringZ(M) or LineString25D depending on the \a is25DType
     * argument. If \a is25DType is true (and the \a m vector is unfilled) then
     * the created Linestring will be a LineString25D type. Otherwise, the
     * LineString will be LineStringZ (or LineStringZM) type.
     *
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<double> &x, const QVector<double> &y,
                   const QVector<double> &z = QVector<double>(),
                   const QVector<double> &m = QVector<double>(), bool is25DType = false );

    /**
     * Constructs a linestring with a single segment from \a p1 to \a p2.
     * \since QGIS 3.2
     */
    QgsLineString( const QgsPoint &p1, const QgsPoint &p2 );

    /**
     * Construct a linestring from list of points.
     * This constructor is more efficient then calling setPoints()
     * or repeatedly calling addVertex()
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<QgsPointXY> &points );

    /**
     * Construct a linestring from a single 2d line segment.
     * \since QGIS 3.2
     */
    explicit QgsLineString( const QgsLineSegment2D &segment );

    bool equals( const QgsCurve &other ) const override;

#ifndef SIP_RUN

    /**
     * Returns the specified point from inside the line string.
     * \param i index of point, starting at 0 for the first point
     */
    QgsPoint pointN( int i ) const;
#else

    /**
     * Returns the point at the specified index. An IndexError will be raised if no point with the specified index exists.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     */
    SIP_PYOBJECT pointN( int i ) const SIP_TYPEHINT( QgsPoint );
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      std::unique_ptr< QgsPoint > p;
      if ( a0 >= 0 )
        p = qgis::make_unique< QgsPoint >( sipCpp->pointN( a0 ) );
      else // negative index, count backwards from end
        p = qgis::make_unique< QgsPoint >( sipCpp->pointN( count + a0 ) );
      sipRes = sipConvertFromType( p.release(), sipType_QgsPoint, Py_None );
    }
    % End
#endif

#ifndef SIP_RUN
    double xAt( int index ) const override;
#else

    /**
     * Returns the x-coordinate of the specified node in the line string.
     *
     * An IndexError will be raised if no point with the specified index exists.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
    */
    double xAt( int index ) const override;
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        return PyFloat_FromDouble( sipCpp->xAt( a0 ) );
      else
        return PyFloat_FromDouble( sipCpp->xAt( count + a0 ) );
    }
    % End
#endif

#ifndef SIP_RUN
    double yAt( int index ) const override;
#else

    /**
     * Returns the y-coordinate of the specified node in the line string.
     *
     * An IndexError will be raised if no point with the specified index exists.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
    */
    double yAt( int index ) const override;
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        return PyFloat_FromDouble( sipCpp->yAt( a0 ) );
      else
        return PyFloat_FromDouble( sipCpp->yAt( count + a0 ) );
    }
    % End
#endif

    /**
     * Returns a const pointer to the x vertex data.
     * \note Not available in Python bindings
     * \see yData()
     * \since QGIS 3.2
     */
    const double *xData() const SIP_SKIP
    {
      return mX.constData();
    }

    /**
     * Returns a const pointer to the y vertex data.
     * \note Not available in Python bindings
     * \see xData()
     * \since QGIS 3.2
     */
    const double *yData() const SIP_SKIP
    {
      return mY.constData();
    }

    /**
     * Returns a const pointer to the z vertex data, or a nullptr if the linestring does
     * not have z values.
     * \note Not available in Python bindings
     * \see xData()
     * \see yData()
     * \since QGIS 3.2
     */
    const double *zData() const SIP_SKIP
    {
      if ( mZ.empty() )
        return nullptr;
      else
        return mZ.constData();
    }

    /**
     * Returns a const pointer to the m vertex data, or a nullptr if the linestring does
     * not have m values.
     * \note Not available in Python bindings
     * \see xData()
     * \see yData()
     * \since QGIS 3.2
     */
    const double *mData() const SIP_SKIP
    {
      if ( mM.empty() )
        return nullptr;
      else
        return mM.constData();
    }

#ifndef SIP_RUN

    /**
     * Returns the z-coordinate of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0
     * \returns z-coordinate of node, or ``nan`` if index is out of bounds or the line
     * does not have a z dimension
     * \see setZAt()
     */
    double zAt( int index ) const
    {
      if ( index >= 0 && index < mZ.size() )
        return mZ.at( index );
      else
        return std::numeric_limits<double>::quiet_NaN();
    }
#else

    /**
     * Returns the z-coordinate of the specified node in the line string.
     *
     * An IndexError will be raised if no point with the specified index exists.
     *
     * If the LineString does not have a z-dimension then ``nan`` will be returned.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
    */
    double zAt( int index ) const;
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        return PyFloat_FromDouble( sipCpp->zAt( a0 ) );
      else
        return PyFloat_FromDouble( sipCpp->zAt( count + a0 ) );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the m value of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0
     * \returns m value of node, or ``nan`` if index is out of bounds or the line
     * does not have m values
     * \see setMAt()
     */
    double mAt( int index ) const
    {
      if ( index >= 0 && index < mM.size() )
        return mM.at( index );
      else
        return std::numeric_limits<double>::quiet_NaN();
    }
#else

    /**
     * Returns the m-coordinate of the specified node in the line string.
     *
     * An IndexError will be raised if no point with the specified index exists.
     *
     * If the LineString does not have a m-dimension then ``nan`` will be returned.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
    */
    double mAt( int index ) const;
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        return PyFloat_FromDouble( sipCpp->mAt( a0 ) );
      else
        return PyFloat_FromDouble( sipCpp->mAt( count + a0 ) );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Sets the x-coordinate of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * \param x x-coordinate of node
     * \see xAt()
     */
    void setXAt( int index, double x );
#else

    /**
     * Sets the x-coordinate of the specified node in the line string.
     * The corresponding node must already exist in line string.
     *
     * An IndexError will be raised if no point with the specified index exists.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \see xAt()
     */
    void setXAt( int index, double x );
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        sipCpp->setXAt( a0, a1 );
      else
        sipCpp->setXAt( count + a0, a1 );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Sets the y-coordinate of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * \param y y-coordinate of node
     * \see yAt()
     */
    void setYAt( int index, double y );
#else

    /**
     * Sets the y-coordinate of the specified node in the line string.
     * The corresponding node must already exist in line string.
     *
     * An IndexError will be raised if no point with the specified index exists.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \see yAt()
     */
    void setYAt( int index, double y );
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        sipCpp->setYAt( a0, a1 );
      else
        sipCpp->setYAt( count + a0, a1 );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Sets the z-coordinate of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have z-dimension.
     * \param z z-coordinate of node
     * \see zAt()
     */
    void setZAt( int index, double z )
    {
      if ( index >= 0 && index < mZ.size() )
        mZ[ index ] = z;
    }
#else

    /**
     * Sets the z-coordinate of the specified node in the line string.
     * The corresponding node must already exist in line string and the line string must have z-dimension.
     *
     * An IndexError will be raised if no point with the specified index exists.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \see zAt()
     */
    void setZAt( int index, double z );
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        sipCpp->setZAt( a0, a1 );
      else
        sipCpp->setZAt( count + a0, a1 );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Sets the m value of the specified node in the line string.
     * \param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have m values.
     * \param m m value of node
     * \see mAt()
     */
    void setMAt( int index, double m )
    {
      if ( index >= 0 && index < mM.size() )
        mM[ index ] = m;
    }
#else

    /**
     * Sets the m-coordinate of the specified node in the line string.
     * The corresponding node must already exist in line string and the line string must have m-dimension.
     *
     * An IndexError will be raised if no point with the specified index exists.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \see mAt()
     */
    void setMAt( int index, double m );
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 >= 0 )
        sipCpp->setMAt( a0, a1 );
      else
        sipCpp->setMAt( count + a0, a1 );
    }
    % End
#endif

    /**
     * Resets the line string to match the specified list of points. The line string will
     * inherit the dimensionality of the first point in the list.
     * \param points new points for line string. If empty, line string will be cleared.
     */
    void setPoints( const QgsPointSequence &points );

    /**
     * Appends the contents of another line string to the end of this line string.
     * \param line line to append. Ownership is not transferred.
     */
    void append( const QgsLineString *line );

    /**
     * Adds a new vertex to the end of the line string.
     * \param pt vertex to add
     */
    void addVertex( const QgsPoint &pt );

    //! Closes the line string by appending the first point to the end of the line, if it is not already closed.
    void close();

    /**
     * Returns the geometry converted to the more generic curve type QgsCompoundCurve
        \returns the converted geometry. Caller takes ownership*/
    QgsCompoundCurve *toCurveType() const override SIP_FACTORY;

    /**
     * Extends the line geometry by extrapolating out the start or end of the line
     * by a specified distance. Lines are extended using the bearing of the first or last
     * segment in the line.
     * \since QGIS 3.0
     */
    void extend( double startDistance, double endDistance );

    //reimplemented methods

    QString geometryType() const override;
    int dimension() const override;
    QgsLineString *clone() const override SIP_FACTORY;
    void clear() override;
    bool isEmpty() const override;
    QgsLineString *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;
    QPolygonF asQPolygonF() const override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QString asJson( int precision = 17 ) const override;

    //curve interface
    double length() const override;
    QgsPoint startPoint() const override;
    QgsPoint endPoint() const override;

    /**
     * Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override  SIP_FACTORY;

    int numPoints() const override;
    int nCoordinates() const override;
    void points( QgsPointSequence &pt SIP_OUT ) const override;

    void draw( QPainter &p ) const override;

    void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform, bool transformZ = false ) override  SIP_THROW( QgsCsException );
    void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 ) override;

    void addToPainterPath( QPainterPath &path ) const override;
    void drawAsPolygon( QPainter &p ) const override;

    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;

    QgsLineString *reversed() const override SIP_FACTORY;
    QgsPoint *interpolatePoint( double distance ) const override SIP_FACTORY;
    QgsLineString *curveSubstring( double startDistance, double endDistance ) const override SIP_FACTORY;

    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;
    bool pointAt( int node, QgsPoint &point, QgsVertexId::VertexType &type ) const override;

    QgsPoint centroid() const override;

    void sumUpArea( double &sum SIP_OUT ) const override;
    double vertexAngle( QgsVertexId vertex ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;

    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;

    bool convertTo( QgsWkbTypes::Type type ) override;

#ifndef SIP_RUN
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;
    void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform ) override;

    /**
     * Cast the \a geom to a QgsLineString.
     * Should be used by qgsgeometry_cast<QgsLineString *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsLineString *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::flatType( geom->wkbType() ) == QgsWkbTypes::LineString )
        return static_cast<const QgsLineString *>( geom );
      return nullptr;
    }
#endif

    QgsLineString *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsLineString: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End

    /**
    * Returns the point at the specified ``index``. An IndexError will be raised if no point with the specified ``index`` exists.
    *
    * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
    * corresponds to the last point in the line.
    *
    * \since QGIS 3.6
    */
    SIP_PYOBJECT __getitem__( int index ) SIP_TYPEHINT( QgsPoint );
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      std::unique_ptr< QgsPoint > p;
      if ( a0 >= 0 )
        p = qgis::make_unique< QgsPoint >( sipCpp->pointN( a0 ) );
      else
        p = qgis::make_unique< QgsPoint >( sipCpp->pointN( count + a0 ) );
      sipRes = sipConvertFromType( p.release(), sipType_QgsPoint, Py_None );
    }
    % End

    /**
    * Sets the point at the specified ``index``. A point at the ``index`` must already exist or an IndexError will be raised.
    *
    * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
    * corresponds to the last point in the line.
    *
    * \since QGIS 3.6
    */
    void __setitem__( int index, const QgsPoint &point );
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      if ( a0 < 0 )
        a0 = count + a0;
      sipCpp->setXAt( a0, a1->x() );
      sipCpp->setYAt( a0, a1->y() );
      if ( sipCpp->isMeasure() )
        sipCpp->setMAt( a0, a1->m() );
      if ( sipCpp->is3D() )
        sipCpp->setZAt( a0, a1->z() );
    }
    % End


    /**
     * Deletes the vertex at the specified ``index``. A point at the ``index`` must already exist or an IndexError will be raised.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \since QGIS 3.6
     */
    void __delitem__( int index );
    % MethodCode
    const int count = sipCpp->numPoints();
    if ( a0 >= 0 && a0 < count )
      sipCpp->deleteVertex( QgsVertexId( -1, -1, a0 ) );
    else if ( a0 < 0 && a0 >= -count )
      sipCpp->deleteVertex( QgsVertexId( -1, -1, count + a0 ) );
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End

#endif

  protected:

    QgsRectangle calculateBoundingBox() const override;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    void importVerticesFromWkb( const QgsConstWkbPtr &wkb );

    /**
     * Resets the line string to match the line string in a WKB geometry.
     * \param type WKB type
     * \param wkb WKB representation of line geometry
     */
    void fromWkbPoints( QgsWkbTypes::Type type, const QgsConstWkbPtr &wkb )
    {
      mWkbType = type;
      importVerticesFromWkb( wkb );
    }

    friend class QgsPolygon;
    friend class QgsTriangle;

};

// clazy:excludeall=qstring-allocations

#endif // QGSLINESTRING_H
