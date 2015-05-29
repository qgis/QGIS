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

    virtual QString geometryType() const override { return "CompoundCurve"; }
    virtual int dimension() const override { return 1; }
    virtual QgsAbstractGeometryV2* clone() const override;
    virtual void clear() override;

    virtual QgsRectangle calculateBoundingBox() const override;

    virtual bool fromWkb( const unsigned char* wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;

    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    //curve interface
    virtual double length() const override;
    virtual QgsPointV2 startPoint() const override;
    virtual QgsPointV2 endPoint() const override;
    virtual void points( QList<QgsPointV2>& pts ) const override;
    virtual int numPoints() const override;
    virtual QgsLineStringV2* curveToLine() const override;
    int nCurves() const { return mCurves.size(); }
    const QgsCurveV2* curveAt( int i ) const;

    /**Adds curve (takes ownership)*/
    void addCurve( QgsCurveV2* c );
    void removeCurve( int i );
    void addVertex( const QgsPointV2& pt );

    void draw( QPainter& p ) const override;
    void transform( const QgsCoordinateTransform& ct ) override;
    void transform( const QTransform& t ) override;
    void addToPainterPath( QPainterPath& path ) const override;
    void drawAsPolygon( QPainter& p ) const override;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( const QgsVertexId& position ) override;

    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const override;

    void sumUpArea( double& sum ) const override;

    /**Appends first point if not already closed*/
    void close();

    bool hasCurvedSegments() const override;

  private:
    QList< QgsCurveV2* > mCurves;
    /**Turns a vertex id for the compound curve into one or more ids for the subcurves
        @return the index of the subcurve or -1 in case of error*/
    QList< QPair<int, QgsVertexId> > curveVertexId( const QgsVertexId& id ) const;
};

#endif // QGSCOMPOUNDCURVEV2_H
