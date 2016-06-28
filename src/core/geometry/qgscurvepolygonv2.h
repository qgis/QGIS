/***************************************************************************
                         qgscurvepolygonv2.h
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

#include "qgssurfacev2.h"

class QgsPolygonV2;

/** \ingroup core
 * \class QgsCurvePolygonV2
 * \brief Curve polygon geometry type
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsCurvePolygonV2: public QgsSurfaceV2
{
  public:
    QgsCurvePolygonV2();
    QgsCurvePolygonV2( const QgsCurvePolygonV2& p );
    QgsCurvePolygonV2& operator=( const QgsCurvePolygonV2& p );
    ~QgsCurvePolygonV2();

    virtual QString geometryType() const override { return "CurvePolygon"; }
    virtual int dimension() const override { return 2; }
    virtual QgsCurvePolygonV2* clone() const override;
    void clear() override;

    virtual bool fromWkb( QgsConstWkbPtr wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;

    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    //surface interface
    virtual double area() const override;
    virtual double perimeter() const override;
    QgsPolygonV2* surfaceToPolygon() const override;

    //curve polygon interface
    int numInteriorRings() const;
    const QgsCurveV2* exteriorRing() const;
    const QgsCurveV2* interiorRing( int i ) const;
    /** Returns a new polygon geometry corresponding to a segmentized approximation
     * of the curve.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsPolygonV2* toPolygon( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const;

    /** Sets the exterior ring of the polygon. The CurvePolygon type will be updated to match the dimensionality
     * of the exterior ring. For instance, setting a 2D exterior ring on a 3D CurvePolygon will drop the z dimension
     * from the CurvePolygon and all interior rings.
     * @param ring new exterior ring. Ownership is transferred to the CurvePolygon.
     * @see setInteriorRings()
     * @see exteriorRing()
     */
    virtual void setExteriorRing( QgsCurveV2* ring );

    /** Sets all interior rings (takes ownership)*/
    void setInteriorRings( const QList<QgsCurveV2*>& rings );
    /** Adds an interior ring to the geometry (takes ownership)*/
    virtual void addInteriorRing( QgsCurveV2* ring );
    /** Removes ring. Exterior ring is 0, first interior ring 1, ...*/
    bool removeInteriorRing( int nr );

    virtual void draw( QPainter& p ) const override;
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform& t ) override;

    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( QgsVertexId position ) override;

    virtual QgsCoordinateSequenceV2 coordinateSequence() const override;
    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const override;

    bool hasCurvedSegments() const override;
    /** Returns a geometry without curves. Caller takes ownership
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    QgsAbstractGeometryV2* segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override;

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
     *  @param vertex the vertex id
     *  @return rotation in radians, clockwise from north
     */
    double vertexAngle( QgsVertexId vertex ) const override;

    virtual int vertexCount( int /*part*/ = 0, int ring = 0 ) const override;
    virtual int ringCount( int /*part*/ = 0 ) const override { return ( nullptr != mExteriorRing ) + mInteriorRings.size(); }
    virtual int partCount() const override { return ringCount() > 0 ? 1 : 0; }
    virtual QgsPointV2 vertexAt( QgsVertexId id ) const override;

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;
    virtual bool dropZValue() override;
    virtual bool dropMValue() override;

  protected:

    QgsCurveV2* mExteriorRing;
    QList<QgsCurveV2*> mInteriorRings;

    virtual QgsRectangle calculateBoundingBox() const override;
};

#endif // QGSCURVEPOLYGONV2_H
