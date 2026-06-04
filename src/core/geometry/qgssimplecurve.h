/***************************************************************************
                       qgssimplecurve.h
                         ------------
    begin                : May 2026
    copyright            : (C) 2026 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSIMPLECURVE_H
#define QGSSIMPLECURVE_H

#include "qgscurve.h"
#include "qgsvertexid.h"

/**
 * \ingroup core
 * \class QgsSimpleCurve
 * \brief Abstract base class for simple curved geometry type.
 *
 * This class is not part of the SQL/MM standard and only exists for implementation convenience.
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsSimpleCurve : public QgsCurve SIP_ABSTRACT
{
  public:
    QgsSimpleCurve() = default;

#ifndef SIP_RUN
    /**
     * Cast the \a geom to a QgsSimpleCurve.
     * Should be used by qgsgeometry_cast<QgsSimpleCurve *>( geometry ).
     *
     * Objects will be automatically converted to the appropriate target type.
     * \note Not available in Python.
     */
    inline static const QgsSimpleCurve *cast( const QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( geom && ( QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::LineString || QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::CircularString ) )
        return static_cast<const QgsSimpleCurve *>( geom );
      return nullptr;
    }

    /**
     * Cast the \a geom to a QgsSimpleCurve.
     * Should be used by qgsgeometry_cast<QgsSimpleCurve *>( geometry ).
     *
     * Objects will be automatically converted to the appropriate target type.
     * \note Not available in Python.
     */
    inline static QgsSimpleCurve *cast( QgsAbstractGeometry *geom ) // cppcheck-suppress duplInheritedMember
    {
      if ( geom && ( QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::LineString || QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::CircularString ) )
        return static_cast<QgsSimpleCurve *>( geom );
      return nullptr;
    }
#endif

    /**
     * Appends the contents of another simple curve to the end of this simple curve.
     * \param curve curve to append. Ownership is not transferred.
     *
     * \note The curve type to be appended must match the base curve type. That is,
     * Only LinearStrings can be appended to LinearStrings and only CircularStrings
     * can be appended to CircularStrings.
     *
     * \warning It is the caller's responsibility to ensure that the first point in
     * the appended \a curve matches the last point in the existing curve, or the
     * result will be undefined.
     */
    void append( const QgsSimpleCurve *curve );

    /**
     * Resets the simple curve to match the specified list of points. The simple curve will
     * inherit the dimensionality of the first point in the list.
     * \param points new points for simple curve. If empty, simple curve will be cleared.
     */
    void setPoints( const QgsPointSequence &points );

#ifndef SIP_RUN
    /**
   * Returns the specified point from inside the simple curve.
   * \param i index of point, starting at 0 for the first point
   */
    QgsPoint pointN( int i ) const;
#else
    // clang-format off

  /**
   * Returns the point at the specified index.
   *
   * - Indexes can be less than 0, in which case they correspond to positions from the end of the simple curve. E.g. an index of -1
   *
   * corresponds to the last point in the simple curve.
   *
   * - \throws IndexError if no point with the specified index exists.
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
// clang-format on
#endif

    /**
   * Returns a const pointer to the x vertex data.
   * \note Not available in Python bindings
   * \see yData()
   */
    const double *xData() const SIP_SKIP { return mX.constData(); }

    /**
   * Returns a const pointer to the y vertex data.
   * \note Not available in Python bindings
   * \see xData()
   */
    const double *yData() const SIP_SKIP { return mY.constData(); }

    /**
   * Returns a const pointer to the z vertex data, or NULLPTR if the simple curve does
   * not have z values.
   * \note Not available in Python bindings
   * \see xData()
   * \see yData()
   */
    const double *zData() const SIP_SKIP
    {
      if ( mZ.empty() )
        return nullptr;
      else
        return mZ.constData();
    }

    /**
   * Returns a const pointer to the m vertex data, or NULLPTR if the simple curve does
   * not have m values.
   * \note Not available in Python bindings
   * \see xData()
   * \see yData()
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
   */
    QVector< double > xVector() const SIP_SKIP { return mX; }

    /**
   * Returns the y vertex values as a vector.
   * \note Not available in Python bindings
   */
    QVector< double > yVector() const SIP_SKIP { return mY; }

    /**
   * Returns the z vertex values as a vector.
   * \note Not available in Python bindings
   */
    QVector< double > zVector() const SIP_SKIP { return mZ; }

    /**
   * Returns the m vertex values as a vector.
   * \note Not available in Python bindings
   */
    QVector< double > mVector() const SIP_SKIP { return mM; }

    // Overrides

#ifndef SIP_RUN
    double xAt( int index ) const override;
#else
    // clang-format off

  /**
   * Returns the x-coordinate of the specified node in the simple curve.
   *
   * - Indexes can be less than 0, in which case they correspond to positions from the end of the curve. E.g. an index of -1
   *
   * corresponds to the last point in the curve.
   *
   * - \throws IndexError if no point with the specified index exists.
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
// clang-format on
#endif

#ifndef SIP_RUN
    double yAt( int index ) const override;
#else
      // clang-format off

      /**
       * Returns the y-coordinate of the specified node in the simple curve.
       *
       * - Indexes can be less than 0, in which case they correspond to positions from the end of the curve. E.g. an index of -1
       *
       * corresponds to the last point in the curve.
       *
       * - \throws IndexError if no point with the specified index exists.
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
// clang-format on
#endif

#ifndef SIP_RUN

    /**
   * Returns the z-coordinate of the specified node in the simple curve.
   * \param index index of node, where the first node in the curve is 0
   * \returns z-coordinate of node, or ``NaN`` if index is out of bounds or the curve
   * does not have a z dimension
   * \see setZAt()
   */
    double zAt( int index ) const override
    {
      if ( index >= 0 && index < mZ.size() )
        return mZ.at( index );
      else
        return std::numeric_limits<double>::quiet_NaN();
    }
#else
      // clang-format off

      /**
       * Returns the z-coordinate of the specified node in the simple curve.
       *
       * - If the SimpleCurve does not have a z-dimension then ``NaN`` will be returned.
       * - Indexes can be less than 0, in which case they correspond to positions from the end of the curve. E.g. an index of -1
       *
       * corresponds to the last point in the curve.
       *
       * - \throws IndexError if no point with the specified index exists.
       */
      double zAt( int index ) const override;
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
// clang-format on
#endif

#ifndef SIP_RUN

    /**
   * Returns the m value of the specified node in the simple curve.
   * \param index index of node, where the first node in the curve is 0
   * \returns m value of node, or ``NaN`` if index is out of bounds or the curve
   * does not have m values
   * \see setMAt()
   */
    double mAt( int index ) const override
    {
      if ( index >= 0 && index < mM.size() )
        return mM.at( index );
      else
        return std::numeric_limits<double>::quiet_NaN();
    }
#else
      // clang-format off

      /**
       * Returns the m value of the specified node in the simple curve.
       *
       * - If the SimpleCurve does not have m values then ``NaN`` will be returned.
       * - Indexes can be less than 0, in which case they correspond to positions from the end of the curve. E.g. an index of -1
       *
       * corresponds to the last point in the curve.
       *
       * - \throws IndexError if no point with the specified index exists.
       */
      double mAt( int index ) const override;
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
// clang-format on
#endif

#ifndef SIP_RUN

    /**
   * Sets the x-coordinate of the specified node in the simple curve.
   * \param index index of node, where the first node in the curve is 0. Corresponding
   * node must already exist in the simple curve.
   * \param x x-coordinate of node
   * \see xAt()
   */
    void setXAt( int index, double x );
#else
      // clang-format off

      /**
       * Sets the x-coordinate of the specified node in the simple curve.
       * The corresponding node must already exist in simple curve.
       *
       * - Indexes can be less than 0, in which case they correspond to positions from the end of the curve. E.g. an index of -1
       *
       * corresponds to the last point in the curve.
       *
       * - \throws IndexError if no point with the specified index exists.
       * - \see xAt()
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
// clang-format on
#endif

#ifndef SIP_RUN

    /**
   * Sets the y-coordinate of the specified node in the simple curve.
   * \param index index of node, where the first node in the curve is 0. Corresponding
   * node must already exist in the simple curve.
   * \param y y-coordinate of node
   * \see yAt()
   */
    void setYAt( int index, double y );
#else
      // clang-format off

      /**
       * Sets the y-coordinate of the specified node in the simple curve.
       * The corresponding node must already exist in the simple curve.
       *
       * - Indexes can be less than 0, in which case they correspond to positions from the end of the curve. E.g. an index of -1
       *
       * corresponds to the last point in the curve.
       *
       * - \throws IndexError if no point with the specified index exists.
       * - \see yAt()
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
// clang-format on
#endif

#ifndef SIP_RUN

    /**
   * Sets the z-coordinate of the specified node in the simple curve.
   * \param index index of node, where the first node in the curve is 0. Corresponding
   * node must already exist in simple curve, and the simple curve must have z-dimension.
   * \param z z-coordinate of node
   * \see zAt()
   */
    void setZAt( int index, double z )
    {
      if ( index >= 0 && index < mZ.size() )
        mZ[index] = z;
    }
#else
      // clang-format off

      /**
       * Sets the z-coordinate of the specified node in the simple curve.
       * The corresponding node must already exist in the simple curve and the simple curve must have z-dimension.
       *
       * - Indexes can be less than 0, in which case they correspond to positions from the end of the curve. E.g. an index of -1
       *
       * corresponds to the last point in the curve.
       *
       * - \throws IndexError if no point with the specified index exists.
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
// clang-format on
#endif

#ifndef SIP_RUN

    /**
   * Sets the m value of the specified node in the simple curve.
   * \param index index of node, where the first node in the curve is 0. Corresponding
   * node must already exist in the simple curve, and the simple curve must have m values.
   * \param m m value of node
   * \see mAt()
   */
    void setMAt( int index, double m )
    {
      if ( index >= 0 && index < mM.size() )
        mM[index] = m;
    }
#else
      // clang-format off

      /**
       * Sets the m-coordinate of the specified node in the simple curve.
       * The corresponding node must already exist in the simple curve and the simple curve must have m-dimension.
       *
       * - Indexes can be less than 0, in which case they correspond to positions from the end of the curve. E.g. an index of -1
       *
       * corresponds to the last point in the curve.
       *
       * - \throws IndexError if no point with the specified index exists.
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
// clang-format on
#endif

    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QString asWkt( int precision = 17 ) const override;
    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    void clear() override;
    int numPoints() const override
    SIP_HOLDGIL;
    int nCoordinates() const override SIP_HOLDGIL;
    int dimension() const override SIP_HOLDGIL;

    bool addMValue( double mValue = 0 ) override;
    bool addZValue( double zValue = 0 ) override;
    bool dropMValue() override;
    bool dropZValue() override;

    QgsPoint startPoint() const override SIP_HOLDGIL;
    QgsPoint endPoint() const override SIP_HOLDGIL;

#ifndef SIP_RUN
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;
    void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform ) override;
#endif
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    QgsSimpleCurve *reversed() const override SIP_FACTORY;

protected:
    int compareToSameClass( const QgsAbstractGeometry *other ) const final;

    /**
     * \brief Returns coordinate vectors for the split curves.
     * \param index Vertex index that will be used to split the curve
     * \param x1 x coordinate vector of the first output curve
     * \param y1 y coordinate vector of the first output curve
     * \param z1 z coordinate vector of the first output curve
     * \param m1 m coordinate vector of the first output curve
     * \param x2 x coordinate vector of the second output curve
     * \param y2 y coordinate vector of the second output curve
     * \param z2 z coordinate vector of the second output curve
     * \param m2 m coordinate vector of the second output curve
     * \note The vertex \a index must correspond to a segment vertex, not a curve vertex.
     */
  void splitCurveAtVertexProtected( int index, QVector< double > &x1, QVector< double > &y1, QVector< double > &z1, QVector< double > &m1, QVector< double > &x2, QVector< double > &y2, QVector< double > &z2, QVector< double > &m2 ) const;

    /**
     * \brief Imports vertices from wkb geometry representation
     * \param wkb const wkb pointer
     */
  void importVerticesFromWkb( const QgsConstWkbPtr &wkb );

  QVector<double> mX;
  QVector<double> mY;
  QVector<double> mZ;
  QVector<double> mM;
};

#endif // QGSSIMPLECURVE_H
