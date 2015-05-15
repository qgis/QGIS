/***************************************************************************
                        qgsgeometrycollectionv2.h
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCOLLECTIONV2_H
#define QGSGEOMETRYCOLLECTIONV2_H

#include "qgsabstractgeometryv2.h"
#include <QVector>

class CORE_EXPORT QgsGeometryCollectionV2: public QgsAbstractGeometryV2
{
  public:
    QgsGeometryCollectionV2();
    QgsGeometryCollectionV2( const QgsGeometryCollectionV2& c );
    QgsGeometryCollectionV2& operator=( const QgsGeometryCollectionV2& c );
    virtual ~QgsGeometryCollectionV2();

    int numGeometries() const;
    const QgsAbstractGeometryV2* geometryN( int n ) const;
    QgsAbstractGeometryV2* geometryN( int n );

    //methods inherited from QgsAbstractGeometry
    virtual int dimension() const;
    virtual QString geometryType() const { return "GeometryCollection"; }
    virtual void clear();

    /**Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometryV2* g );
    virtual bool removeGeometry( int nr );

    virtual void transform( const QgsCoordinateTransform& ct );
    void transform( const QTransform& t );
    virtual void clip( const QgsRectangle& rect );
    virtual void draw( QPainter& p ) const;

    bool fromWkb( const unsigned char * wkb );
    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    virtual QgsRectangle calculateBoundingBox() const;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const;
    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    virtual double length() const;
    virtual double area() const;

  protected:
    QVector< QgsAbstractGeometryV2* > mGeometries;
    void removeGeometries();

    bool fromCollectionWkt( const QString &wkt, const QList<QgsAbstractGeometryV2*>& subtypes, const QString& defaultChildWkbType = QString() );

};

#endif // QGSGEOMETRYCOLLECTIONV2_H
