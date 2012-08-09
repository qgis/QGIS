/***************************************************************************
    qgsdiagram.h
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
#ifndef QGSDIAGRAM_H
#define QGSDIAGRAM_H

#include "qgsfeature.h"
#include <QPen>
#include <QBrush>

class QPainter;
class QPointF;
struct QgsDiagramSettings;
struct QgsDiagramInterpolationSettings;

class QgsRenderContext;



/**Base class for all diagram types*/
class CORE_EXPORT QgsDiagram
{
  public:
    virtual ~QgsDiagram() {}
    /**Draws the diagram at the given position (in pixel coordinates)*/
    virtual void renderDiagram( const QgsAttributeMap& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position ) = 0;
    virtual QString diagramName() const = 0;
    /**Returns the size in map units the diagram will use to render.*/
    virtual QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s ) = 0;
    /**Returns the size in map units the diagram will use to render. Interpolat size*/
    virtual QSizeF diagramSize( const QgsAttributeMap& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is ) = 0;

  protected:
    void setPenWidth( QPen& pen, const QgsDiagramSettings& s, const QgsRenderContext& c );
    QSizeF sizePainterUnits( const QSizeF& size, const QgsDiagramSettings& s, const QgsRenderContext& c );
    float sizePainterUnits( float l, const QgsDiagramSettings& s, const QgsRenderContext& c );
    QFont scaledFont( const QgsDiagramSettings& s, const QgsRenderContext& c );
};

#endif // QGSDIAGRAM_H
