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
#include "qgis_sip.h"
#include "qgscurve.h"
#include "qgscompoundcurve.h"

class QgsLineSegment2D;
class QgsBox3d;

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

    /**
     * Constructor for an empty linestring geometry.
     */
    QgsLineString() SIP_HOLDGIL;
#ifndef SIP_RUN

    /**
     * Construct a linestring from a vector of points.
     * Z and M type will be set based on the type of the first point
     * in the vector.
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<QgsPoint> &points );

    /**
     * Construct a linestring from list of points.
     * This constructor is more efficient then calling setPoints()
     * or repeatedly calling addVertex()
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<QgsPointXY> &points );
#else

    /**
     * Construct a linestring from a sequence of points (QgsPoint objects, QgsPointXY objects, or sequences of float values).
     *
     * The linestring Z and M type will be set based on the type of the first point in the sequence.
     *
     * \since QGIS 3.20
     */
    QgsLineString( SIP_PYOBJECT points SIP_TYPEHINT( Sequence[Union[QgsPoint, QgsPointXY, Sequence[float]]] ) ) SIP_HOLDGIL [( const QVector<double> &x, const QVector<double> &y, const QVector<double> &z = QVector<double>(), const QVector<double> &m = QVector<double>(), bool is25DType = false )];
    % MethodCode
    if ( !PySequence_Check( a0 ) )
    {
      PyErr_SetString( PyExc_TypeError, QStringLiteral( "A sequence of QgsPoint, QgsPointXY or array of floats is expected" ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      int state;
      const int size = PySequence_Size( a0 );
      QVector< double > xl;
      QVector< double > yl;
      bool hasZ = false;
      QVector< double > zl;
      bool hasM = false;
      QVector< double > ml;
      xl.reserve( size );
      yl.reserve( size );

      bool is25D = false;

      sipIsErr = 0;
      for ( int i = 0; i < size; ++i )
      {
        PyObject *value = PySequence_GetItem( a0, i );
        if ( !value )
        {
          PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1." ).arg( i ) .toUtf8().constData() );
          sipIsErr = 1;
          break;
        }

        if ( PySequence_Check( value ) )
        {
          const int elementSize = PySequence_Size( value );
          if ( elementSize < 2 || elementSize > 4 )
          {
            sipIsErr = 1;
            PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid sequence size at index %1. Expected an array of 2-4 float values, got %2." ).arg( i ).arg( elementSize ).toUtf8().constData() );
            Py_DECREF( value );
            break;
          }
          else
          {
            sipIsErr = 0;
            for ( int j = 0; j < elementSize; ++j )
            {
              PyObject *element = PySequence_GetItem( value, j );
              if ( !element )
              {
                PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1." ).arg( i ) .toUtf8().constData() );
                sipIsErr = 1;
                break;
              }

              PyErr_Clear();
              double d = PyFloat_AsDouble( element );
              if ( PyErr_Occurred() )
              {
                Py_DECREF( value );
                sipIsErr = 1;
                break;
              }
              if ( j == 0 )
                xl.append( d );
              else if ( j == 1 )
                yl.append( d );

              if ( i == 0 && j == 2 )
              {
                hasZ = true;
                zl.reserve( size );
                zl.append( d );
              }
              else if ( i > 0 && j == 2 && hasZ )
              {
                zl.append( d );
              }

              if ( i == 0 && j == 3 )
              {
                hasM = true;
                ml.reserve( size );
                ml.append( d );
              }
              else if ( i > 0 && j == 3 && hasM )
              {
                ml.append( d );
              }

              Py_DECREF( element );
            }

            if ( hasZ && elementSize < 3 )
              zl.append( std::numeric_limits< double >::quiet_NaN() );
            if ( hasM && elementSize < 4 )
              ml.append( std::numeric_limits< double >::quiet_NaN() );

            Py_DECREF( value );
            if ( sipIsErr )
            {
              break;
            }
          }
        }
        else
        {
          if ( sipCanConvertToType( value, sipType_QgsPointXY, SIP_NOT_NONE ) )
          {
            sipIsErr = 0;
            QgsPointXY *p = reinterpret_cast<QgsPointXY *>( sipConvertToType( value, sipType_QgsPointXY, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
            if ( !sipIsErr )
            {
              xl.append( p->x() );
              yl.append( p->y() );
            }
            sipReleaseType( p, sipType_QgsPointXY, state );
          }
          else if ( sipCanConvertToType( value, sipType_QgsPoint, SIP_NOT_NONE ) )
          {
            sipIsErr = 0;
            QgsPoint *p = reinterpret_cast<QgsPoint *>( sipConvertToType( value, sipType_QgsPoint, 0, SIP_NOT_NONE, &state, &sipIsErr ) );
            if ( !sipIsErr )
            {
              xl.append( p->x() );
              yl.append( p->y() );

              if ( i == 0 && p->is3D() )
              {
                hasZ = true;
                zl.reserve( size );
                zl.append( p->z() );
              }
              else if ( i > 0 && hasZ )
              {
                zl.append( p->z() );
              }

              if ( i == 0 && p->isMeasure() )
              {
                hasM = true;
                ml.reserve( size );
                ml.append( p->m() );
              }
              else if ( i > 0 && hasM )
              {
                ml.append( p->m() );
              }

              if ( i == 0 && p->wkbType() == QgsWkbTypes::Point25D )
                is25D = true;
            }
            sipReleaseType( p, sipType_QgsPoint, state );
          }
          else
          {
            sipIsErr = 1;
          }

          Py_DECREF( value );

          if ( sipIsErr )
          {
            // couldn't convert the sequence value to a QgsPoint or QgsPointXY
            PyErr_SetString( PyExc_TypeError, QStringLiteral( "Invalid type at index %1. Expected QgsPoint, QgsPointXY or array of floats." ).arg( i ) .toUtf8().constData() );
            break;
          }
        }
      }
      if ( sipIsErr == 0 )
        sipCpp = new sipQgsLineString( QgsLineString( xl, yl, zl, ml, is25D ) );
    }
    % End
#endif

    /**
     * Construct a linestring from a single 2d line segment.
     * \since QGIS 3.2
     */
    explicit QgsLineString( const QgsLineSegment2D &segment ) SIP_HOLDGIL;

    /**
     * Construct a linestring from arrays of coordinates. If the z or m
     * arrays are non-empty then the resultant linestring will have
     * z and m types accordingly.
     * This constructor is more efficient then calling setPoints()
     * or repeatedly calling addVertex()
     *
     * If the \a z vector is filled, then the geometry type will either
     * be a LineStringZ(M) or LineString25D depending on the \a is25DType
     * argument. If \a is25DType is TRUE (and the \a m vector is unfilled) then
     * the created Linestring will be a LineString25D type. Otherwise, the
     * LineString will be LineStringZ (or LineStringZM) type.
     *
     * If the sizes of \a x and \a y are non-equal then the resultant linestring
     * will be created using the minimum size of these arrays.
     *
     * \since QGIS 3.0
     */
    QgsLineString( const QVector<double> &x, const QVector<double> &y,
                   const QVector<double> &z = QVector<double>(),
                   const QVector<double> &m = QVector<double>(), bool is25DType = false ) SIP_HOLDGIL;

    /**
     * Constructs a linestring with a single segment from \a p1 to \a p2.
     * \since QGIS 3.2
     */
    QgsLineString( const QgsPoint &p1, const QgsPoint &p2 ) SIP_HOLDGIL;

    /**
     * Returns a new linestring created by segmentizing the bezier curve between \a start and \a end, with
     * the specified control points.
     *
     * The \a segments parameter controls how many line segments will be present in the returned linestring.
     *
     * Any z or m values present in the input coordinates will be interpolated along with the x and y values.
     *
     * \since QGIS 3.10
     */
    static QgsLineString *fromBezierCurve( const QgsPoint &start, const QgsPoint &controlPoint1, const QgsPoint &controlPoint2, const QgsPoint &end, int segments = 30 ) SIP_FACTORY;

    /**
     * Returns a new linestring from a QPolygonF \a polygon input.
     *
     * \since QGIS 3.10
     */
    static QgsLineString *fromQPolygonF( const QPolygonF &polygon ) SIP_FACTORY;

    bool equals( const QgsCurve &other ) const override;

#ifndef SIP_RUN

    /**
     * Returns the specified point from inside the line string.
     * \param i index of point, starting at 0 for the first point
     */
    QgsPoint pointN( int i ) const;
#else

    /**
     * Returns the point at the specified index.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
        p = std::make_unique< QgsPoint >( sipCpp->pointN( a0 ) );
      else // negative index, count backwards from end
        p = std::make_unique< QgsPoint >( sipCpp->pointN( count + a0 ) );
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
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
     * Returns a const pointer to the z vertex data, or NULLPTR if the linestring does
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
     * Returns a const pointer to the m vertex data, or NULLPTR if the linestring does
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

    /**
     * Returns the x vertex values as a vector.
     * \note Not available in Python bindings
     * \since QGIS 3.26
    */
    QVector< double > xVector() const SIP_SKIP
    {
      return mX;
    }

    /**
     * Returns the y vertex values as a vector.
     * \note Not available in Python bindings
     * \since QGIS 3.26
    */
    QVector< double > yVector() const SIP_SKIP
    {
      return mY;
    }

    /**
     * Returns the z vertex values as a vector.
     * \note Not available in Python bindings
     * \since QGIS 3.26
    */
    QVector< double > zVector() const SIP_SKIP
    {
      return mZ;
    }

    /**
     * Returns the m vertex values as a vector.
     * \note Not available in Python bindings
     * \since QGIS 3.26
    */
    QVector< double > mVector() const SIP_SKIP
    {
      return mM;
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
     * If the LineString does not have a z-dimension then ``nan`` will be returned.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
     * If the LineString does not have a m-dimension then ``nan`` will be returned.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified index exists.
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
    * Resets the line string to match the specified point data. The line string
    * dimensionality will be based on whether \a z or \a m arrays are specified.
    *
    * \param size point count.
    * \param x array of x data
    * \param y array of y data
    * \param z array of z data, can be NULLPTR
    * \param m array of m data, can be NULLPTR
    *
    * \note Not available from Python bindings
    * \since QGIS 3.26
    */
    void setPoints( size_t size, const double *x, const double *y, const double *z = nullptr, const double *m = nullptr ) SIP_SKIP;

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
     * \returns the converted geometry. Caller takes ownership
    */
    QgsCompoundCurve *toCurveType() const override SIP_FACTORY;

    /**
     * Extends the line geometry by extrapolating out the start or end of the line
     * by a specified distance. Lines are extended using the bearing of the first or last
     * segment in the line.
     * \since QGIS 3.0
     */
    void extend( double startDistance, double endDistance );

#ifndef SIP_RUN

    /**
     * Visits regular points along the linestring, spaced by \a distance.
     *
     * The \a visitPoint function should return FALSE to abort further traversal.
     */
    void visitPointsByRegularDistance( double distance, const std::function< bool( double x, double y, double z, double m,
                                       double startSegmentX, double startSegmentY, double startSegmentZ, double startSegmentM,
                                       double endSegmentX, double endSegmentY, double endSegmentZ, double endSegmentM
                                                                                 ) > &visitPoint ) const;
#endif

    //reimplemented methods
    QString geometryType() const override SIP_HOLDGIL;
    int dimension() const override SIP_HOLDGIL;
    QgsLineString *clone() const override SIP_FACTORY;
    void clear() override;
    bool isEmpty() const override SIP_HOLDGIL;
    int indexOf( const QgsPoint &point ) const final;
    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override;
    QgsLineString *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;
    bool isClosed() const override SIP_HOLDGIL;
    bool isClosed2D() const override SIP_HOLDGIL;
    bool boundingBoxIntersects( const QgsRectangle &rectangle ) const override SIP_HOLDGIL;

    /**
     * Returns a list of any duplicate nodes contained in the geometry, within the specified tolerance.
     *
     * If \a useZValues is TRUE then z values will also be considered when testing for duplicates.
     *
     * \since QGIS 3.16
     */
    QVector< QgsVertexId > collectDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) const;

    QPolygonF asQPolygonF() const override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    QString asKml( int precision = 17 ) const override;

    //curve interface
    double length() const override SIP_HOLDGIL;

#ifndef SIP_RUN
    std::tuple< std::unique_ptr< QgsCurve >, std::unique_ptr< QgsCurve > > splitCurveAtVertex( int index ) const final;
#endif

    /**
     * Returns the length in 3D world of the line string.
     * If it is not a 3D line string, return its 2D length.
     * \see length()
     * \since QGIS 3.10
     */
    double length3D() const SIP_HOLDGIL;
    QgsPoint startPoint() const override SIP_HOLDGIL;
    QgsPoint endPoint() const override SIP_HOLDGIL;

    /**
     * Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve
    */
    QgsLineString *curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override  SIP_FACTORY;

    int numPoints() const override SIP_HOLDGIL;
    int nCoordinates() const override SIP_HOLDGIL;
    void points( QgsPointSequence &pt SIP_OUT ) const override;

    void draw( QPainter &p ) const override;

    void transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d = Qgis::TransformDirection::Forward, bool transformZ = false ) override  SIP_THROW( QgsCsException );
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
    bool pointAt( int node, QgsPoint &point, Qgis::VertexType &type ) const override;

    QgsPoint centroid() const override;

    /**
     * Calculates the shoelace/triangle formula sum for the points in the linestring.
     * If the linestring is closed (i.e. a polygon) then the polygon area is equal to the absolute value of the sum.
     * Please note that the sum will be negative if the points are defined in clockwise order.
     * Therefore, if you want to use the sum as an area (as the method name indicates) then you probably should use the absolute value,
     * since otherwise a bug can be introduced (such as the bug fixed for github issue 49578)
     * \see https://en.wikipedia.org/wiki/Shoelace_formula#Triangle_formula
     */
    void sumUpArea( double &sum SIP_OUT ) const override;

    double vertexAngle( QgsVertexId vertex ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;

    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;

    bool convertTo( QgsWkbTypes::Type type ) override;

    bool transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback = nullptr ) override;
    void scroll( int firstVertexIndex ) final;

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
    inline static const QgsLineString *cast( const QgsAbstractGeometry *geom )
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
    * Returns the point at the specified ``index``.
    *
    * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
    * corresponds to the last point in the line.
    *
    * \throws IndexError if no point with the specified ``index`` exists.
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
        p = std::make_unique< QgsPoint >( sipCpp->pointN( a0 ) );
      else
        p = std::make_unique< QgsPoint >( sipCpp->pointN( count + a0 ) );
      sipRes = sipConvertFromType( p.release(), sipType_QgsPoint, Py_None );
    }
    % End

    /**
    * Sets the point at the specified ``index``.
    *
    * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
    * corresponds to the last point in the line.
    *
    * \throws IndexError if no point with the specified ``index`` exists.
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
     * Deletes the vertex at the specified ``index``.
     *
     * Indexes can be less than 0, in which case they correspond to positions from the end of the line. E.g. an index of -1
     * corresponds to the last point in the line.
     *
     * \throws IndexError if no point with the specified ``index`` exists.
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

    /**
     * Calculates the minimal 3D bounding box for the geometry.
     * \see calculateBoundingBox()
     * \since QGIS 3.26
     */
    QgsBox3d calculateBoundingBox3d() const;

  protected:

    int compareToSameClass( const QgsAbstractGeometry *other ) const final;
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
    friend class TestQgsGeometry;

};

// clazy:excludeall=qstring-allocations

#endif // QGSLINESTRING_H
