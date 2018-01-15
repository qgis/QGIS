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

#include "qgis_core.h"
#include "qgis.h"
#include "qgsdiagram.h"
#include <QPen>
#include <QBrush>

class QPainter;
class QPointF;
class QgsDiagramSettings;
class QgsDiagramInterpolationSettings;
class QgsFeature;
class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsTextDiagram
 */
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
    QgsTextDiagram *clone() const override SIP_FACTORY;

    void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position ) override;

    QSizeF diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s ) override;
    QSizeF diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) override;
    double legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const override;

    QString diagramName() const override;

  private:
    Orientation mOrientation = Vertical;
    Shape mShape = Circle;
    QBrush mBrush; //transparent brush
    QPen mPen;

    /**
     * Calculates intersection points between a line and an ellipse
      \returns intersection points*/
    void lineEllipseIntersection( QPointF lineStart, QPointF lineEnd, QPointF ellipseMid, double r1, double r2, QList<QPointF> &result ) const;
};

#endif // QGSTEXTDIAGRAM_H
