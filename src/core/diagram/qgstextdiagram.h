/***************************************************************************
    qgstextdiagram.h
    ---------------------
    begin                : March 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTEXTDIAGRAM_H
#define QGSTEXTDIAGRAM_H

#define DIAGRAM_NAME_TEXT "Text"

#include "qgsdiagram.h"
#include "qgsfeature.h"
#include <QPen>
#include <QBrush>

class QPainter;
class QPointF;
struct QgsDiagramSettings;
struct QgsDiagramInterpolationSettings;

class QgsRenderContext;


class CORE_EXPORT QgsTextDiagram: public QgsDiagram
{
  public:
    enum Shape
    {
      Circle = 0,
      Rectangle,
      Triangle
    };

    enum Orientation
    {
      Horizontal = 0,
      Vertical
    };

    QgsTextDiagram();
    ~QgsTextDiagram();
    void renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );
    QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s );
    QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );

    QString diagramName() const { return DIAGRAM_NAME_TEXT; }

  private:
    Orientation mOrientation;
    Shape mShape;
    QBrush mBrush; //transparent brush
    QPen mPen;

    /**Calculates intersection points between a line and an ellipse
      @return intersection points*/
    void lineEllipseIntersection( const QPointF& lineStart, const QPointF& lineEnd, const QPointF& ellipseMid, double r1, double r2, QList<QPointF>& result ) const;
};

#endif // QGSTEXTDIAGRAM_H
