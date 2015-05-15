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

class CORE_EXPORT QgsCurveV2: public QgsAbstractGeometryV2
{
  public:
    QgsCurveV2();
    virtual ~QgsCurveV2();
    virtual QgsPointV2 startPoint() const = 0;
    virtual QgsPointV2 endPoint() const = 0;
    virtual bool isClosed() const;
    virtual bool isRing() const;
    virtual QgsLineStringV2* curveToLine() const = 0;

    virtual void addToPainterPath( QPainterPath& path ) const = 0;
    virtual void drawAsPolygon( QPainter& p ) const = 0;
    virtual void points( QList<QgsPointV2>& pt ) const = 0;
    virtual int numPoints() const = 0;

    virtual double area() const;
    virtual void sumUpArea( double& sum ) const = 0;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const;
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const;
    virtual bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const = 0;
};

#endif // QGSCURVEV2_H
