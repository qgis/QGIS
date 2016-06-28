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

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsgeometry.cpp.
 * See details in QEP #17
 ****************************************************************************/

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

    bool operator==( const QgsCurveV2& other ) const override;
    bool operator!=( const QgsCurveV2& other ) const override;

    /** Returns the specified point from inside the line string.
     * @param i index of point, starting at 0 for the first point
     */
    QgsPointV2 pointN( int i ) const;

    /** Returns the x-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns x-coordinate of node, or 0.0 if index is out of bounds
     * @see setXAt()
     */
    double xAt( int index ) const;

    /** Returns the y-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns y-coordinate of node, or 0.0 if index is out of bounds
     * @see setYAt()
     */
    double yAt( int index ) const;

    /** Returns the z-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns z-coordinate of node, or 0.0 if index is out of bounds or the line
     * does not have a z dimension
     * @see setZAt()
     */
    double zAt( int index ) const;

    /** Returns the m value of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns m value of node, or 0.0 if index is out of bounds or the line
     * does not have m values
     * @see setMAt()
     */
    double mAt( int index ) const;

    /** Sets the x-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * @param x x-coordinate of node
     * @see xAt()
     */
    void setXAt( int index, double x );

    /** Sets the y-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * @param y y-coordinate of node
     * @see yAt()
     */
    void setYAt( int index, double y );

    /** Sets the z-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have z-dimension.
     * @param z z-coordinate of node
     * @see zAt()
     */
    void setZAt( int index, double z );

    /** Sets the m value of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have m values.
     * @param m m value of node
     * @see mAt()
     */
    void setMAt( int index, double m );

    /** Resets the line string to match the specified list of points. The line string will
     * inherit the dimensionality of the first point in the list.
     * @param points new points for line string. If empty, line string will be cleared.
     */
    void setPoints( const QgsPointSequenceV2 &points );

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

    /** Returns the geometry converted to the more generic curve type QgsCompoundCurveV2
        @return the converted geometry. Caller takes ownership*/
    QgsAbstractGeometryV2* toCurveType() const override;

    //reimplemented methods

    virtual QString geometryType() const override { return "LineString"; }
    virtual int dimension() const override { return 1; }
    virtual QgsLineStringV2* clone() const override;
    virtual void clear() override;

    virtual bool fromWkb( QgsConstWkbPtr wkb ) override;
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
    /** Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsLineStringV2* curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override;

    int numPoints() const override;
    void points( QgsPointSequenceV2 &pt ) const override;

    void draw( QPainter& p ) const override;

    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform& t ) override;

    void addToPainterPath( QPainterPath& path ) const override;
    void drawAsPolygon( QPainter& p ) const override;

    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( QgsVertexId position ) override;

    virtual QgsLineStringV2* reversed() const override;

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const override;

    virtual QgsPointV2 centroid() const override;

    void sumUpArea( double& sum ) const override;
    double vertexAngle( QgsVertexId vertex ) const override;

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;

    virtual bool dropZValue() override;
    virtual bool dropMValue() override;

    bool convertTo( QgsWKBTypes::Type type ) override;

  protected:

    virtual QgsRectangle calculateBoundingBox() const override;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    void importVerticesFromWkb( const QgsConstWkbPtr& wkb );

    /** Resets the line string to match the line string in a WKB geometry.
     * @param type WKB type
     * @param wkb WKB representation of line geometry
     */
    void fromWkbPoints( QgsWKBTypes::Type type, const QgsConstWkbPtr& wkb );

    friend class QgsPolygonV2;

};

#endif // QGSLINESTRINGV2_H
