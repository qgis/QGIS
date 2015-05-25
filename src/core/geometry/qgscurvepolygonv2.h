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

class CORE_EXPORT QgsCurvePolygonV2: public QgsSurfaceV2
{
  public:
    QgsCurvePolygonV2();
    QgsCurvePolygonV2( const QgsCurvePolygonV2& p );
    QgsCurvePolygonV2& operator=( const QgsCurvePolygonV2& p );
    ~QgsCurvePolygonV2();

    virtual QString geometryType() const { return "CurvePolygon"; }
    virtual int dimension() const { return 2; }
    virtual QgsAbstractGeometryV2* clone() const;
    void clear();


    virtual QgsRectangle calculateBoundingBox() const;
    virtual bool fromWkb( const unsigned char* wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    //surface interface
    virtual double area() const;
    virtual double length() const;
    QgsPointV2 centroid() const;
    QgsPointV2 pointOnSurface() const;
    QgsPolygonV2* surfaceToPolygon() const;

    //curve polygon interface
    int numInteriorRings() const;
    const QgsCurveV2* exteriorRing() const;
    const QgsCurveV2* interiorRing( int i ) const;
    virtual QgsPolygonV2* toPolygon() const;

    /**Sets exterior ring (takes ownership)*/
    void setExteriorRing( QgsCurveV2* ring );
    /**Sets interior rings (takes ownership)*/
    void setInteriorRings( QList<QgsCurveV2*> rings );
    void addInteriorRing( QgsCurveV2* ring );
    /**Removes ring. Exterior ring is 0, first interior ring 1, ...*/
    bool removeInteriorRing( int nr );

    virtual void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct );
    void transform( const QTransform& t );

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const;
    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;

  protected:

    QgsCurveV2* mExteriorRing;
    QList<QgsCurveV2*> mInteriorRings;
};

#endif // QGSCURVEPOLYGONV2_H
