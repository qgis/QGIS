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

/**\ingroup core
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
    virtual QgsAbstractGeometryV2* clone() const override;
    void clear() override;


    virtual QgsRectangle calculateBoundingBox() const override;
    virtual bool fromWkb( const unsigned char* wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;

    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    //surface interface
    virtual double area() const override;
    virtual double length() const override;
    QgsPointV2 centroid() const override;
    QgsPointV2 pointOnSurface() const override;
    QgsPolygonV2* surfaceToPolygon() const override;

    //curve polygon interface
    int numInteriorRings() const;
    const QgsCurveV2* exteriorRing() const;
    const QgsCurveV2* interiorRing( int i ) const;
    virtual QgsPolygonV2* toPolygon() const;

    /**Sets exterior ring (takes ownership)*/
    void setExteriorRing( QgsCurveV2* ring );
    /**Sets all interior rings (takes ownership)*/
    void setInteriorRings( QList<QgsCurveV2*> rings );
    /**Adds an interior ring to the geometry (takes ownership)*/
    void addInteriorRing( QgsCurveV2* ring );
    /**Removes ring. Exterior ring is 0, first interior ring 1, ...*/
    bool removeInteriorRing( int nr );

    virtual void draw( QPainter& p ) const override;
    void transform( const QgsCoordinateTransform& ct ) override;
    void transform( const QTransform& t ) override;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( const QgsVertexId& position ) override;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const override;
    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const override;

    bool hasCurvedSegments() const override;
    QgsAbstractGeometryV2* segmentize() const override;

  protected:

    QgsCurveV2* mExteriorRing;
    QList<QgsCurveV2*> mInteriorRings;
};

#endif // QGSCURVEPOLYGONV2_H
