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

#ifndef QGSCURVEPOLYGONV2_H
#define QGSCURVEPOLYGONV2_H

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

    bool operator==( const QgsCurvePolygon &other ) const;
    bool operator!=( const QgsCurvePolygon &other ) const;

    ~QgsCurvePolygon();

    QString geometryType() const override;
    int dimension() const override;
    QgsCurvePolygon *clone() const override SIP_FACTORY;
    void clear() override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJson( int precision = 17 ) const override;

    //surface interface
    double area() const override;
    double perimeter() const override;
    QgsPolygon *surfaceToPolygon() const override SIP_FACTORY;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    QgsCurvePolygon *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;

    //curve polygon interface
    int numInteriorRings() const;
    const QgsCurve *exteriorRing() const;
    const QgsCurve *interiorRing( int i ) const;

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

    /**
     * Removes an interior ring from the polygon. The first interior ring has index 0.
     * The corresponding ring is removed from the polygon and deleted. If a ring was successfully removed
     * the function will return true.  It is not possible to remove the exterior ring using this method.
     * \see removeInteriorRings()
     */
    bool removeInteriorRing( int ringIndex );

    /**
     * Removes the interior rings from the polygon. If the minimumAllowedArea
     * parameter is specified then only rings smaller than this minimum
     * area will be removed.
     * \since QGIS 3.0
     * \see removeInteriorRing()
     */
    void removeInteriorRings( double minimumAllowedArea = -1 );

    void draw( QPainter &p ) const override;
    void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform &t ) override;

    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;

    QgsCoordinateSequence coordinateSequence() const override;
    int nCoordinates() const override;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    bool isEmpty() const override;
    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, bool *leftOf SIP_OUT = nullptr, double epsilon = 4 * DBL_EPSILON ) const override;

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
     *  \param vertex the vertex id
     *  \returns rotation in radians, clockwise from north
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

    QgsCurvePolygon *toCurveType() const override SIP_FACTORY;
#ifndef SIP_RUN

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
  protected:
    QgsCurvePolygon *createEmptyWithSameType() const override SIP_FACTORY;
    int childCount() const override;
    QgsAbstractGeometry *childGeometry( int index ) const override;

  protected:

    std::unique_ptr< QgsCurve > mExteriorRing;
    QVector<QgsCurve *> mInteriorRings;

    QgsRectangle calculateBoundingBox() const override;
};

// clazy:excludeall=qstring-allocations

#endif // QGSCURVEPOLYGONV2_H
