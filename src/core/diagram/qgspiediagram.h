/***************************************************************************
    qgspiediagram.h
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
#ifndef QGSPIEDIAGRAM_H
#define QGSPIEDIAGRAM_H

#define DIAGRAM_NAME_PIE "Pie"

#include "qgsdiagram.h"
#include "qgsfeature.h"
#include <QPen>
#include <QBrush>

class QPainter;
class QPointF;
class QgsDiagramSettings;
class QgsDiagramInterpolationSettings;

class QgsRenderContext;

class CORE_EXPORT QgsPieDiagram: public QgsDiagram
{
  public:
    QgsPieDiagram();
    ~QgsPieDiagram();

    virtual QgsDiagram* clone() const;

    void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );

    QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s );
    QSizeF diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );
    QString diagramName() const { return DIAGRAM_NAME_PIE; }

  private:
    QBrush mCategoryBrush;
    QPen mPen;

    static int sCount;
};

#endif // QGSPIEDIAGRAM_H
