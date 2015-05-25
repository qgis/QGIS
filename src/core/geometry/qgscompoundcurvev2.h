/***************************************************************************
                         qgscompoundcurvev2.h
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

#ifndef QGSCOMPOUNDCURVEV2_H
#define QGSCOMPOUNDCURVEV2_H

#include "qgscurvev2.h"

class CORE_EXPORT QgsCompoundCurveV2: public QgsCurveV2
{
  public:
    QgsCompoundCurveV2();
    QgsCompoundCurveV2( const QgsCompoundCurveV2& curve );
    QgsCompoundCurveV2& operator=( const QgsCompoundCurveV2& curve );
    ~QgsCompoundCurveV2();

    virtual QString geometryType() const { return "CompoundCurve"; }
    virtual int dimension() const { return 1; }
    virtual QgsAbstractGeometryV2* clone() const;
    virtual void clear();

    virtual QgsRectangle calculateBoundingBox() const;

    virtual bool fromWkb( const unsigned char* wkb );
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
    virtual void points( QList<QgsPointV2>& pts ) const;
    virtual int numPoints() const;
    virtual QgsLineStringV2* curveToLine() const;
    int nCurves() const { return mCurves.size(); }
    const QgsCurveV2* curveAt( int i ) const;

    /**Adds curve (takes ownership)*/
    void addCurve( QgsCurveV2* c );
    void removeCurve( int i );
    void addVertex( const QgsPointV2& pt );

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct );
    void transform( const QTransform& t );
    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const;

    void sumUpArea( double& sum ) const;

    /**Appends first point if not already closed*/
    void close();

  private:
    QList< QgsCurveV2* > mCurves;
    /**Turns a vertex id for the compound curve into one or more ids for the subcurves
        @return the index of the subcurve or -1 in case of error*/
    QList< QPair<int, QgsVertexId> > curveVertexId( const QgsVertexId& id ) const;
};

#endif // QGSCOMPOUNDCURVEV2_H
