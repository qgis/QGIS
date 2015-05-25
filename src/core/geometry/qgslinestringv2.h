/***************************************************************************
                         qgslinestringv2.h
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

#include "qgscurvev2.h"
#include "qgswkbptr.h"
#include <QPolygonF>

class CORE_EXPORT QgsLineStringV2: public QgsCurveV2
{
  public:
    QgsLineStringV2();
    ~QgsLineStringV2();

    virtual QString geometryType() const { return "LineString"; }
    virtual int dimension() const { return 1; }
    virtual QgsAbstractGeometryV2* clone() const;
    virtual void clear();

    virtual bool fromWkb( const unsigned char* wkb );
    void fromWkbPoints( QgsWKBTypes::Type type, const QgsConstWkbPtr& wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual QgsLineStringV2* curveToLine() const;

    int numPoints() const;
    QgsPointV2 pointN( int i ) const;
    void points( QList<QgsPointV2>& pt ) const;

    void setPoints( const QList<QgsPointV2>& points );
    void append( const QgsLineStringV2* line );

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct );
    void transform( const QTransform& t );

    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

    const QPolygonF& qPolygonF() const { return mCoords; }

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );
    void addVertex( const QgsPointV2& pt );

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const;

    void sumUpArea( double& sum ) const;

    /**Appends first point if not already closed*/
    void close();

  private:
    QPolygonF mCoords;
    QVector<double> mZ;
    QVector<double> mM;

    void importVerticesFromWkb( const QgsConstWkbPtr& wkb );
};

#endif // QGSLINESTRINGV2_H
