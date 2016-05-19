/***************************************************************************
                         qgscurvev2.h
                         ------------
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

#ifndef QGSCURVEV2_H
#define QGSCURVEV2_H

#include "qgsabstractgeometryv2.h"
#include "qgspointv2.h"

class QgsLineStringV2;
class QPainterPath;

/** \ingroup core
 * \class QgsCurveV2
 * \brief Abstract base class for curved geometry type
 * \note added in QGIS 2.10
 */
class CORE_EXPORT QgsCurveV2: public QgsAbstractGeometryV2
{
  public:
    QgsCurveV2();
    virtual ~QgsCurveV2();

    virtual bool operator==( const QgsCurveV2& other ) const = 0;
    virtual bool operator!=( const QgsCurveV2& other ) const = 0;

    virtual QgsCurveV2* clone() const override = 0;

    /** Returns the starting point of the curve.
     * @see endPoint
     */
    virtual QgsPointV2 startPoint() const = 0;

    /** Returns the end point of the curve.
     * @see startPoint
     */
    virtual QgsPointV2 endPoint() const = 0;

    /** Returns true if the curve is closed.
     */
    virtual bool isClosed() const;

    /** Returns true if the curve is a ring.
     */
    virtual bool isRing() const;

    /** Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsLineStringV2* curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const = 0;

    /** Adds a curve to a painter path.
     */
    virtual void addToPainterPath( QPainterPath& path ) const = 0;

    /** Draws the curve as a polygon on the specified QPainter.
     * @param p destination QPainter
     */
    virtual void drawAsPolygon( QPainter& p ) const = 0;

    /** Returns a list of points within the curve.
     */
    virtual void points( QgsPointSequenceV2 &pt ) const = 0;

    /** Returns the number of points in the curve.
     */
    virtual int numPoints() const = 0;

    /** Calculates the area of the curve. Derived classes should override this
     * to return the correct area of the curve.
     */
    virtual void sumUpArea( double& sum ) const = 0;

    virtual QgsCoordinateSequenceV2 coordinateSequence() const override;
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const override;

    /** Returns the point and vertex id of a point within the curve.
     * @param node node number, where the first node is 0
     * @param point will be set to point at corresponding node in the curve
     * @param type will be set to the vertex type of the node
     * @returns true if node exists within the curve
     */
    virtual bool pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const = 0;

    /** Returns a reversed copy of the curve, where the direction of the curve has been flipped.
     * @note added in QGIS 2.14
     */
    virtual QgsCurveV2* reversed() const = 0;

    /** Returns a geometry without curves. Caller takes ownership
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    QgsCurveV2* segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override;

    virtual int vertexCount( int part = 0, int ring = 0 ) const override { Q_UNUSED( part );  Q_UNUSED( ring ); return numPoints(); }
    virtual int ringCount( int part = 0 ) const override { Q_UNUSED( part ); return numPoints() > 0 ? 1 : 0; }
    virtual int partCount() const override { return numPoints() > 0 ? 1 : 0; }
    virtual QgsPointV2 vertexAt( QgsVertexId id ) const override;

    virtual QgsRectangle boundingBox() const override;

  protected:

    virtual void clearCache() const override { mBoundingBox = QgsRectangle(); mCoordinateSequence.clear(); QgsAbstractGeometryV2::clearCache(); }

  private:

    mutable QgsRectangle mBoundingBox;
    mutable QgsCoordinateSequenceV2 mCoordinateSequence;
};

#endif // QGSCURVEV2_H
