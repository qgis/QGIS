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
     */
    virtual QgsLineStringV2* curveToLine() const = 0;

    /** Adds a curve to a painter path.
     */
    virtual void addToPainterPath( QPainterPath& path ) const = 0;

    /** Draws the curve as a polygon on the specified QPainter.
     * @param p destination QPainter
     */
    virtual void drawAsPolygon( QPainter& p ) const = 0;

    /** Returns a list of points within the curve.
     */
    virtual void points( QList<QgsPointV2>& pt ) const = 0;

    /** Returns the number of points in the curve.
     */
    virtual int numPoints() const = 0;

    virtual double area() const override;

    /** Calculates the area of the curve. Derived classes should override this
     * to return the correct area of the curve.
     */
    virtual void sumUpArea( double& sum ) const  = 0;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const override;
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const override;

    /** Returns the point and vertex id of a point within the curve.
     */
    virtual bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const = 0;

    QgsAbstractGeometryV2* segmentize() const override;
};

#endif // QGSCURVEV2_H
