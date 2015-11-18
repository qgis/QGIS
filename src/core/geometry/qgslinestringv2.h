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
 * \brief Line string geometry type, with support for z-dimension and m-values.
 * \note added in QGIS 2.10
 */
class CORE_EXPORT QgsLineStringV2: public QgsCurveV2
{
  public:
    QgsLineStringV2();
    ~QgsLineStringV2();

    /** Resets the line string to match the line string in a WKB geometry.
     * @param type WKB type
     * @param wkb WKB representation of line geometry
     * @note not available in Python bindings
     */
    void fromWkbPoints( QgsWKBTypes::Type type, const QgsConstWkbPtr& wkb );

    /** Returns the specified point from inside the line string.
     * @param i index of point, starting at 0 for the first point
     */
    QgsPointV2 pointN( int i ) const;

    /** Resets the line string to match the specified list of points.
     * @param points new points for line string. If empty, line string will be cleared.
     */
    void setPoints( const QList<QgsPointV2>& points );

    /** Appends the contents of another line string to the end of this line string.
     * @param line line to append. Ownership is not transferred.
     */
    void append( const QgsLineStringV2* line );

    /** Adds a new vertex to the end of the line string.
     * @param pt vertex to add
     */
    void addVertex( const QgsPointV2& pt );

    /** Closes the line string by appending the first point to the end of the line, if it is not already closed.*/
    void close();

    /** Returns a QPolygonF representing the line string.
     */
    QPolygonF asQPolygonF() const;

    //reimplemented methods

    virtual QString geometryType() const override { return "LineString"; }
    virtual int dimension() const override { return 1; }
    virtual QgsLineStringV2* clone() const override;
    virtual void clear() override;

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
    virtual QgsLineStringV2* curveToLine() const override;

    int numPoints() const override;
    void points( QList<QgsPointV2>& pt ) const override;

    void draw( QPainter& p ) const override;

    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform ) override;
    void transform( const QTransform& t ) override;

    void addToPainterPath( QPainterPath& path ) const override;
    void drawAsPolygon( QPainter& p ) const override;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( const QgsVertexId& position ) override;

    virtual QgsLineStringV2* reversed() const override;

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const override;

    void sumUpArea( double& sum ) const override;
    double vertexAngle( const QgsVertexId& vertex ) const override;

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    void importVerticesFromWkb( const QgsConstWkbPtr& wkb );
};

#endif // QGSLINESTRINGV2_H
