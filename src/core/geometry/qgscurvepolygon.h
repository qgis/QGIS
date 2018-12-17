/***************************************************************************
                         qgscurvepolygon.h
                         -------------------
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

#ifndef QGSCURVEPOLYGON_H
#define QGSCURVEPOLYGON_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssurface.h"
#include <memory>

class QgsPolygon;

/**
 * \ingroup core
 * \class QgsCurvePolygon
 * \brief Curve polygon geometry type
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsCurvePolygon: public QgsSurface
{
  public:
    QgsCurvePolygon();
    QgsCurvePolygon( const QgsCurvePolygon &p );
    QgsCurvePolygon &operator=( const QgsCurvePolygon &p );

    bool operator==( const QgsAbstractGeometry &other ) const override;
    bool operator!=( const QgsAbstractGeometry &other ) const override;

    ~QgsCurvePolygon() override;

    QString geometryType() const override;
    int dimension() const override;
    QgsCurvePolygon *clone() const override SIP_FACTORY;
    void clear() override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QString asJson( int precision = 17 ) const override;

    //surface interface
    double area() const override;
    double perimeter() const override;
    QgsPolygon *surfaceToPolygon() const override SIP_FACTORY;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    QgsCurvePolygon *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;

    //curve polygon interface

    /**
     * Returns the number of interior rings contained with the curve polygon.
     *
     * \see interiorRing()
     */
    int numInteriorRings() const
    {
      return mInteriorRings.size();
    }

    /**
     * Returns the curve polygon's exterior ring.
     *
     * \see interiorRing()
     */
    const QgsCurve *exteriorRing() const
    {
      return mExteriorRing.get();
    }

#ifndef SIP_RUN

    /**
     * Retrieves an interior ring from the curve polygon. The first interior ring has index 0.
     *
     * \see numInteriorRings()
     * \see exteriorRing()
     */
    const QgsCurve *interiorRing( int i ) const
    {
      if ( i < 0 || i >= mInteriorRings.size() )
      {
        return nullptr;
      }
      return mInteriorRings.at( i );
    }
#else

    /**
     * Retrieves an interior ring from the curve polygon. The first interior ring has index 0.
     *
     * An IndexError will be raised if no interior ring with the specified index exists.
     *
     * \see numInteriorRings()
     * \see exteriorRing()
     */
    SIP_PYOBJECT interiorRing( int i ) SIP_TYPEHINT( QgsCurve );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numInteriorRings() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( const_cast< QgsCurve * >( sipCpp->interiorRing( a0 ) ), sipType_QgsCurve, NULL );
    }
    % End
#endif

    /**
     * Returns a new polygon geometry corresponding to a segmentized approximation
     * of the curve.
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsPolygon *toPolygon( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const SIP_FACTORY;

    /**
     * Sets the exterior ring of the polygon. The CurvePolygon type will be updated to match the dimensionality
     * of the exterior ring. For instance, setting a 2D exterior ring on a 3D CurvePolygon will drop the z dimension
     * from the CurvePolygon and all interior rings.
     * \param ring new exterior ring. Ownership is transferred to the CurvePolygon.
     * \see setInteriorRings()
     * \see exteriorRing()
     */
    virtual void setExteriorRing( QgsCurve *ring SIP_TRANSFER );

    //! Sets all interior rings (takes ownership)
    void setInteriorRings( const QVector<QgsCurve *> &rings SIP_TRANSFER );
    //! Adds an interior ring to the geometry (takes ownership)
    virtual void addInteriorRing( QgsCurve *ring SIP_TRANSFER );

#ifndef SIP_RUN

    /**
     * Removes an interior ring from the polygon. The first interior ring has index 0.
     * The corresponding ring is removed from the polygon and deleted. If a ring was successfully removed
     * the function will return true.  It is not possible to remove the exterior ring using this method.
     * \see removeInteriorRings()
     */
    bool removeInteriorRing( int ringIndex );
#else

    /**
     * Removes an interior ring from the polygon. The first interior ring has index 0.
     * The corresponding ring is removed from the polygon and deleted.
     * It is not possible to remove the exterior ring using this method.
     *
     * An IndexError will be raised if no interior ring with the specified index exists.
     *
     * \see removeInteriorRings()
     */
    bool removeInteriorRing( int i );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numInteriorRings() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return PyBool_FromLong( sipCpp->removeInteriorRing( a0 ) );
    }
    % End
#endif

    /**
     * Removes the interior rings from the polygon. If the minimumAllowedArea
     * parameter is specified then only rings smaller than this minimum
     * area will be removed.
     * \see removeInteriorRing()
     * \since QGIS 3.0
     */
    void removeInteriorRings( double minimumAllowedArea = -1 );

    /**
     * Removes any interior rings which are not valid from the polygon.
     *
     * For example, this removes unclosed rings and rings with less than 4 vertices.
     *
     * \since QGIS 3.0
     */
    void removeInvalidRings();

    /**
     * Forces the geometry to respect the Right-Hand-Rule, in which the area that is
     * bounded by the polygon is to the right of the boundary. In particular, the exterior
     * ring is oriented in a clockwise direction and the interior rings in a counter-clockwise
     * direction.
     *
     * \since QGIS 3.6
     */
    void forceRHR();

    void draw( QPainter &p ) const override;
    void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform, bool transformZ = false ) override SIP_THROW( QgsCsException );
    void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 ) override;

    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;

    QgsCoordinateSequence coordinateSequence() const override;
    int nCoordinates() const override;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    bool isEmpty() const override;
    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;

    bool nextVertex( QgsVertexId &id, QgsPoint &vertex SIP_OUT ) const override;
    void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex SIP_OUT, QgsVertexId &nextVertex SIP_OUT ) const override;
    bool hasCurvedSegments() const override;

    /**
     * Returns a geometry without curves. Caller takes ownership
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    QgsAbstractGeometry *segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;

    /**
     * Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
     * \param vertex the vertex id
     * \returns rotation in radians, clockwise from north
     */
    double vertexAngle( QgsVertexId vertex ) const override;

    int vertexCount( int part = 0, int ring = 0 ) const override;
    int ringCount( int part = 0 ) const override;
    int partCount() const override;
    QgsPoint vertexAt( QgsVertexId id ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;

    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;
    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;

    QgsCurvePolygon *toCurveType() const override SIP_FACTORY;

#ifndef SIP_RUN
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;
    void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform ) override;

    /**
     * Cast the \a geom to a QgsCurvePolygon.
     * Should be used by qgsgeometry_cast<QgsCurvePolygon *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsCurvePolygon *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( !geom )
        return nullptr;

      QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( geom->wkbType() );
      if ( flatType == QgsWkbTypes::CurvePolygon
           || flatType == QgsWkbTypes::Polygon
           || flatType == QgsWkbTypes::Triangle )
        return static_cast<const QgsCurvePolygon *>( geom );
      return nullptr;
    }
#endif

    QgsCurvePolygon *createEmptyWithSameType() const override SIP_FACTORY;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString wkt = sipCpp->asWkt();
    if ( wkt.length() > 1000 )
      wkt = wkt.left( 1000 ) + QStringLiteral( "..." );
    QString str = QStringLiteral( "<QgsCurvePolygon: %1>" ).arg( wkt );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:

    int childCount() const override;
    QgsAbstractGeometry *childGeometry( int index ) const override;

  protected:

    std::unique_ptr< QgsCurve > mExteriorRing;
    QVector<QgsCurve *> mInteriorRings;

    QgsRectangle calculateBoundingBox() const override;
};

// clazy:excludeall=qstring-allocations

#endif // QGSCURVEPOLYGON_H
