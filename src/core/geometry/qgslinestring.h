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

#ifndef QGSLINESTRINGV2_H
#define QGSLINESTRINGV2_H

#include "qgis_core.h"
#include "qgscurve.h"
#include <QPolygonF>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsgeometry.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * \class QgsLineString
 * \brief Line string geometry type, with support for z-dimension and m-values.
 * \note added in QGIS 2.10
 */
class CORE_EXPORT QgsLineString: public QgsCurve
{
  public:
    QgsLineString();

    bool operator==( const QgsCurve& other ) const override;
    bool operator!=( const QgsCurve& other ) const override;

    /** Returns the specified point from inside the line string.
     * @param i index of point, starting at 0 for the first point
     */
    QgsPointV2 pointN( int i ) const;

    double xAt( int index ) const override;
    double yAt( int index ) const override;

    /** Returns the z-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns z-coordinate of node, or 0.0 if index is out of bounds or the line
     * does not have a z dimension
     * @see setZAt()
     */
    double zAt( int index ) const;

    /** Returns the m value of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns m value of node, or 0.0 if index is out of bounds or the line
     * does not have m values
     * @see setMAt()
     */
    double mAt( int index ) const;

    /** Sets the x-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * @param x x-coordinate of node
     * @see xAt()
     */
    void setXAt( int index, double x );

    /** Sets the y-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * @param y y-coordinate of node
     * @see yAt()
     */
    void setYAt( int index, double y );

    /** Sets the z-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have z-dimension.
     * @param z z-coordinate of node
     * @see zAt()
     */
    void setZAt( int index, double z );

    /** Sets the m value of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have m values.
     * @param m m value of node
     * @see mAt()
     */
    void setMAt( int index, double m );

    /** Resets the line string to match the specified list of points. The line string will
     * inherit the dimensionality of the first point in the list.
     * @param points new points for line string. If empty, line string will be cleared.
     */
    void setPoints( const QgsPointSequence &points );

    /** Appends the contents of another line string to the end of this line string.
     * @param line line to append. Ownership is not transferred.
     */
    void append( const QgsLineString* line );

    /** Adds a new vertex to the end of the line string.
     * @param pt vertex to add
     */
    void addVertex( const QgsPointV2& pt );

    //! Closes the line string by appending the first point to the end of the line, if it is not already closed.
    void close();

    /** Returns the geometry converted to the more generic curve type QgsCompoundCurve
        @return the converted geometry. Caller takes ownership*/
    QgsAbstractGeometry* toCurveType() const override;

    /**
     * Extends the line geometry by extrapolating out the start or end of the line
     * by a specified distance. Lines are extended using the bearing of the first or last
     * segment in the line.
     * @note added in QGIS 3.0
     */
    void extend( double startDistance, double endDistance );

    //reimplemented methods

    virtual QString geometryType() const override { return QStringLiteral( "LineString" ); }
    virtual int dimension() const override { return 1; }
    virtual QgsLineString* clone() const override;
    virtual void clear() override;
    bool isEmpty() const override;

    virtual bool fromWkb( QgsConstWkbPtr& wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;

    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    //curve interface
    virtual double length() const override;
    virtual QgsPointV2 startPoint() const override;
    virtual QgsPointV2 endPoint() const override;

    /** Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsLineString* curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override;

    int numPoints() const override;
    virtual int nCoordinates() const override { return mPoints.size(); }
    void points( QgsPointSequence &pt ) const override;

    void draw( QPainter& p ) const override;

    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform& t ) override;

    void addToPainterPath( QPainterPath& path ) const override;
    void drawAsPolygon( QPainter& p ) const override;

    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( QgsVertexId position ) override;

    virtual QgsLineString* reversed() const override;

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const override;

    virtual QgsPointV2 centroid() const override;

    void sumUpArea( double& sum ) const override;
    double vertexAngle( QgsVertexId vertex ) const override;

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;

    virtual bool dropZValue() override;
    virtual bool dropMValue() override;

    bool convertTo( QgsWkbTypes::Type type ) override;

#if 0
    //iterators
    ///@cond PRIVATE

    class const_iterator;

    class iterator
    {
      public:
        int index;
        QgsLineString* line;

        typedef std::random_access_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;

        inline iterator()
            : index( -1 )
            , line( nullptr )
        {}

        inline QgsPointV2& operator*() const { return line->field; }
        inline QgsPointV2* operator->() const { return &d->field; }
        inline QgsPointV2& operator[]( difference_type j ) const { return d[j].field; }
        inline bool operator==( const iterator &o ) const noexcept { return d == o.d; } // clazy:exclude=function-args-by-value
        inline bool operator!=( const iterator &o ) const noexcept { return d != o.d; } // clazy:exclude=function-args-by-value
        inline bool operator<( const iterator& other ) const noexcept { return d < other.d; } // clazy:exclude=function-args-by-value
        inline bool operator<=( const iterator& other ) const noexcept { return d <= other.d; } // clazy:exclude=function-args-by-value
        inline bool operator>( const iterator& other ) const noexcept { return d > other.d; } // clazy:exclude=function-args-by-value
        inline bool operator>=( const iterator& other ) const noexcept { return d >= other.d; } // clazy:exclude=function-args-by-value

        inline iterator& operator++() { ++d; return *this; }
        inline iterator operator++( int ) { QgsFields::Field* n = d; ++d; return n; }
        inline iterator& operator--() { d--; return *this; }
        inline iterator operator--( int ) { QgsFields::Field* n = d; d--; return n; }
        inline iterator& operator+=( difference_type j ) { d += j; return *this; }
        inline iterator& operator-=( difference_type j ) { d -= j; return *this; }
        inline iterator operator+( difference_type j ) const { return iterator( d + j ); }
        inline iterator operator-( difference_type j ) const { return iterator( d -j ); }
        inline int operator-( iterator j ) const { return int( d - j.d ); }
    };
    friend class iterator;

    class const_iterator // clazy:exclude=rule-of-three
    {
      public:
        const QgsFields::Field* d;

        typedef std::random_access_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;

        inline const_iterator()
            : d( nullptr ) {}
        inline const_iterator( const QgsFields::Field* f )
            : d( f ) {}
        inline const_iterator( const const_iterator &o )
            : d( o.d ) {}
        inline explicit const_iterator( const iterator &o ) // clazy:exclude=function-args-by-value
            : d( o.d ) {}
        inline const QgsField& operator*() const { return d->field; }
        inline const QgsField* operator->() const { return &d->field; }
        inline const QgsField& operator[]( difference_type j ) const noexcept { return d[j].field; }
        inline bool operator==( const const_iterator &o ) const noexcept { return d == o.d; }
        inline bool operator!=( const const_iterator &o ) const noexcept { return d != o.d; }
        inline bool operator<( const const_iterator& other ) const noexcept { return d < other.d; }
        inline bool operator<=( const const_iterator& other ) const noexcept { return d <= other.d; }
        inline bool operator>( const const_iterator& other ) const noexcept { return d > other.d; }
        inline bool operator>=( const const_iterator& other ) const noexcept { return d >= other.d; }
        inline const_iterator& operator++() { ++d; return *this; }
        inline const_iterator operator++( int ) { const QgsFields::Field* n = d; ++d; return n; }
        inline const_iterator& operator--() { d--; return *this; }
        inline const_iterator operator--( int ) { const QgsFields::Field* n = d; --d; return n; }
        inline const_iterator& operator+=( difference_type j ) { d += j; return *this; }
        inline const_iterator& operator-=( difference_type j ) { d -= j; return *this; }
        inline const_iterator operator+( difference_type j ) const { return const_iterator( d + j ); }
        inline const_iterator operator-( difference_type j ) const { return const_iterator( d -j ); }
        inline int operator-( const_iterator j ) const { return int( d - j.d ); } // clazy:exclude=function-args-by-ref
    };
    friend class const_iterator;
    ///@endcond


    /**
     * Returns a const STL-style iterator pointing to the first node in the linestring.
     *
     * @note added in 3.0
     * @note not available in Python bindings
     */
    const_iterator constBegin() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the imaginary node after the last node in the linestring.
     *
     * @note added in 3.0
     * @note not available in Python bindings
     */
    const_iterator constEnd() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the first node in the linestring.
     *
     * @note added in 3.0
     * @note not available in Python bindings
     */
    const_iterator begin() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the imaginary node after the last node in the linestring.
     *
     * @note added in 3.0
     * @note not available in Python bindings
     */
    const_iterator end() const noexcept;

    /**
     * Returns an STL-style iterator pointing to the first node in the linestring.
     *
     * @note added in 3.0
     * @note not available in Python bindings
     */
    iterator begin();


    /**
     * Returns an STL-style iterator pointing to the imaginary node after the last node in the linestring.
     *
     * @note added in 3.0
     * @note not available in Python bindings
     */
    iterator end();

#endif

  protected:

    virtual QgsRectangle calculateBoundingBox() const override;

  private:
    QgsPointSequence mPoints;

    void importVerticesFromWkb( const QgsConstWkbPtr& wkb );

    /** Resets the line string to match the line string in a WKB geometry.
     * @param type WKB type
     * @param wkb WKB representation of line geometry
     */
    void fromWkbPoints( QgsWkbTypes::Type type, const QgsConstWkbPtr& wkb );

    friend class QgsPolygonV2;

};

#endif // QGSLINESTRINGV2_H
