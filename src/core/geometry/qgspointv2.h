/***************************************************************************
                         qgspointv2.h
                         --------------
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

#ifndef QGSPOINTV2_H
#define QGSPOINTV2_H

#include "qgsabstractgeometryv2.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsgeometry.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * \class QgsPointV2
 * \brief Point geometry type, with support for z-dimension and m-values.
 * \note added in QGIS 2.10
 */
class CORE_EXPORT QgsPointV2: public QgsAbstractGeometryV2
{
  public:

    /** Construct a 2 dimensional point with an initial x and y coordinate.
     * @param x x-coordinate of point
     * @param y y-coordinate of point
     */
    QgsPointV2( double x = 0.0, double y = 0.0 );

    /** Construct a QgsPointV2 from a QgsPoint object
     */
    explicit QgsPointV2( const QgsPoint& p );

    /** Construct a QgsPointV2 from a QPointF
     */
    explicit QgsPointV2( QPointF p );

    /** Construct a point with a specified type (eg PointZ, PointM) and initial x, y, z, and m values.
     * @param type point type
     * @param x x-coordinate of point
     * @param y y-coordinate of point
     * @param z z-coordinate of point, for PointZ or PointZM types
     * @param m m-value of point, for PointM or PointZM types
     */
    QgsPointV2( QgsWKBTypes::Type type, double x = 0.0, double y = 0.0, double z = 0.0, double m = 0.0 );

    bool operator==( const QgsPointV2& pt ) const;
    bool operator!=( const QgsPointV2& pt ) const;

    /** Returns the point's x-coordinate.
     * @see setX()
     * @see rx()
     */
    double x() const { return mX; }

    /** Returns the point's y-coordinate.
     * @see setY()
     * @see ry()
     */
    double y() const { return mY; }

    /** Returns the point's z-coordinate.
     * @see setZ()
     * @see rz()
     */
    double z() const { return mZ; }

    /** Returns the point's m value.
     * @see setM()
     * @see rm()
     */
    double m() const { return mM; }

    /** Returns a reference to the x-coordinate of this point.
     * Using a reference makes it possible to directly manipulate x in place.
     * @see x()
     * @see setX()
     * @note not available in Python bindings
     */
    double &rx() { clearCache(); return mX; }

    /** Returns a reference to the y-coordinate of this point.
     * Using a reference makes it possible to directly manipulate y in place.
     * @see y()
     * @see setY()
     * @note not available in Python bindings
     */
    double &ry() { clearCache(); return mY; }

    /** Returns a reference to the z-coordinate of this point.
     * Using a reference makes it possible to directly manipulate z in place.
     * @see z()
     * @see setZ()
     * @note not available in Python bindings
     */
    double &rz() { clearCache(); return mZ; }

    /** Returns a reference to the m value of this point.
     * Using a reference makes it possible to directly manipulate m in place.
     * @see m()
     * @see setM()
     * @note not available in Python bindings
     */
    double &rm() { clearCache(); return mM; }

    /** Sets the point's x-coordinate.
     * @see x()
     * @see rx()
     */
    void setX( double x ) { clearCache(); mX = x; }

    /** Sets the point's y-coordinate.
     * @see y()
     * @see ry()
     */
    void setY( double y ) { clearCache(); mY = y; }

    /** Sets the point's z-coordinate.
     * @note calling this will have no effect if the point does not contain a z-dimension. Use addZValue() to
     * add a z value and force the point to have a z dimension.
     * @see z()
     * @see rz()
     */
    void setZ( double z ) { clearCache(); mZ = z; }

    /** Sets the point's m-value.
     * @note calling this will have no effect if the point does not contain a m-dimension. Use addMValue() to
     * add a m value and force the point to have an m dimension.
     * @see m()
     * @see rm()
     */
    void setM( double m ) { clearCache(); mM = m; }

    /** Returns the point as a QPointF.
     * @note added in QGIS 2.14
     */
    QPointF toQPointF() const;

    //implementation of inherited methods
    virtual QgsRectangle boundingBox() const override { return QgsRectangle( mX, mY, mX, mY ); }
    virtual QString geometryType() const override { return "Point"; }
    virtual int dimension() const override { return 0; }
    virtual QgsPointV2* clone() const override;
    void clear() override;
    virtual bool fromWkb( QgsConstWkbPtr wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;
    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;
    void draw( QPainter& p ) const override;
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false ) override;
    void transform( const QTransform& t ) override;
    virtual QgsCoordinateSequenceV2 coordinateSequence() const override;

    //low-level editing
    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex ) override { Q_UNUSED( position ); Q_UNUSED( vertex ); return false; }
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( QgsVertexId position ) override { Q_UNUSED( position ); return false; }

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const override;

    /** Angle undefined. Always returns 0.0
        @param vertex the vertex id
        @return 0.0*/
    double vertexAngle( QgsVertexId vertex ) const override { Q_UNUSED( vertex ); return 0.0; }

    virtual int vertexCount( int /*part*/ = 0, int /*ring*/ = 0 ) const override { return 1; }
    virtual int ringCount( int /*part*/ = 0 ) const override { return 1; }
    virtual int partCount() const override { return 1; }
    virtual QgsPointV2 vertexAt( QgsVertexId /*id*/ ) const override { return *this; }

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;
    virtual bool dropZValue() override;
    virtual bool dropMValue() override;
    bool convertTo( QgsWKBTypes::Type type ) override;

  private:
    double mX;
    double mY;
    double mZ;
    double mM;
};

#endif // QGSPOINTV2_H
