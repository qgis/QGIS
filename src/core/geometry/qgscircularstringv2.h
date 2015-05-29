/***************************************************************************
                         qgscircularstringv2.h
                         ---------------------
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

#ifndef QGSCIRCULARSTRING_H
#define QGSCIRCULARSTRING_H

#include "qgscurvev2.h"
#include <QVector>

class CORE_EXPORT QgsCircularStringV2: public QgsCurveV2
{
  public:
    QgsCircularStringV2();
    ~QgsCircularStringV2();

    virtual QString geometryType() const override { return "CircularString"; }
    virtual int dimension() const override { return 1; }
    virtual QgsAbstractGeometryV2* clone() const override;
    virtual void clear() override;

    virtual QgsRectangle calculateBoundingBox() const override;

    virtual bool fromWkb( const unsigned char * wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;

    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    int numPoints() const override;
    QgsPointV2 pointN( int i ) const;
    void points( QList<QgsPointV2>& pts ) const override;
    void setPoints( const QList<QgsPointV2>& points );


    //curve interface
    virtual double length() const override;
    virtual QgsPointV2 startPoint() const override;
    virtual QgsPointV2 endPoint() const override;
    virtual QgsLineStringV2* curveToLine() const override;

    void draw( QPainter& p ) const override;
    void transform( const QgsCoordinateTransform& ct ) override;
    void transform( const QTransform& t ) override;
    void clip( const QgsRectangle& rect ) override;
    void addToPainterPath( QPainterPath& path ) const override;
    void drawAsPolygon( QPainter& p ) const override;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( const QgsVertexId& position ) override;

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const override;

    void sumUpArea( double& sum ) const override;

    bool hasCurvedSegments() const override { return true; }

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    //helper methods for curveToLine
    void segmentize( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& p3, QList<QgsPointV2>& points ) const;
    int segmentSide( const QgsPointV2& pt1, const QgsPointV2& pt3, const QgsPointV2& pt2 ) const;
    double interpolateArc( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 ) const;
    static void arcTo( QPainterPath& path, const QPointF& pt1, const QPointF& pt2, const QPointF& pt3 );
    //bounding box of a single segment
    static QgsRectangle segmentBoundingBox( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3 );
    static QList<QgsPointV2> compassPointsOnSegment( double p1Angle, double p2Angle, double p3Angle, double centerX, double centerY, double radius );
    static double closestPointOnArc( double x1, double y1, double x2, double y2, double x3, double y3,
                                     const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon );
    void insertVertexBetween( int after, int before, int pointOnCircle );
    void deleteVertex( int i );
};

#endif // QGSCIRCULARSTRING_H
