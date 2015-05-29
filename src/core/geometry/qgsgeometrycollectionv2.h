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

    virtual QgsAbstractGeometryV2* clone() const override;

    int numGeometries() const;
    const QgsAbstractGeometryV2* geometryN( int n ) const;
    QgsAbstractGeometryV2* geometryN( int n );

    //methods inherited from QgsAbstractGeometry
    virtual int dimension() const override;
    virtual QString geometryType() const override { return "GeometryCollection"; }
    virtual void clear() override;

    /**Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometryV2* g );
    virtual bool removeGeometry( int nr );

    virtual void transform( const QgsCoordinateTransform& ct ) override;
    void transform( const QTransform& t ) override;
    virtual void clip( const QgsRectangle& rect ) override;
    virtual void draw( QPainter& p ) const override;

    bool fromWkb( const unsigned char * wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;
    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    virtual QgsRectangle calculateBoundingBox() const override;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const override;
    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const override;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( const QgsVertexId& position ) override;

    virtual double length() const override;
    virtual double area() const override;

    bool hasCurvedSegments() const override;

  protected:
    QVector< QgsAbstractGeometryV2* > mGeometries;
    void removeGeometries();

    bool fromCollectionWkt( const QString &wkt, const QList<QgsAbstractGeometryV2*>& subtypes, const QString& defaultChildWkbType = QString() );

};

#endif // QGSGEOMETRYCOLLECTIONV2_H
