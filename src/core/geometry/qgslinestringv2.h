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

/** \ingroup core
 * \class QgsLineStringV2
 * \brief Line string geometry type.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsLineStringV2: public QgsCurveV2
{
  public:
    QgsLineStringV2();
    ~QgsLineStringV2();

    virtual QString geometryType() const override { return "LineString"; }
    virtual int dimension() const override { return 1; }
    virtual QgsAbstractGeometryV2* clone() const override;
    virtual void clear() override;

    virtual bool fromWkb( const unsigned char* wkb ) override;
    void fromWkbPoints( QgsWKBTypes::Type type, const QgsConstWkbPtr& wkb );
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
    virtual QgsLineStringV2* curveToLine() const override;

    int numPoints() const override;
    QgsPointV2 pointN( int i ) const;
    void points( QList<QgsPointV2>& pt ) const override;

    void setPoints( const QList<QgsPointV2>& points );
    void append( const QgsLineStringV2* line );

    void draw( QPainter& p ) const override;

    /** Transforms the geometry using a coordinate transform
     * @param ct coordinate transform
       @param d transformation direction
     */
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform ) override;
    void transform( const QTransform& t ) override;

    void addToPainterPath( QPainterPath& path ) const override;
    void drawAsPolygon( QPainter& p ) const override;

    const QPolygonF& qPolygonF() const { return mCoords; }

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( const QgsVertexId& position ) override;
    void addVertex( const QgsPointV2& pt );

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const override;

    void sumUpArea( double& sum ) const override;

    /** Appends first point if not already closed*/
    void close();

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
        @param vertex the vertex id
        @return rotation in radians, clockwise from north*/
    double vertexAngle( const QgsVertexId& vertex ) const override;

  private:
    QPolygonF mCoords;
    QVector<double> mZ;
    QVector<double> mM;

    void importVerticesFromWkb( const QgsConstWkbPtr& wkb );
};

#endif // QGSLINESTRINGV2_H
