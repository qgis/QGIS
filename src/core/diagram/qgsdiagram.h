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
class QgsDiagramSettings;
class QgsDiagramInterpolationSettings;

class QgsRenderContext;



/**Base class for all diagram types*/
class CORE_EXPORT QgsDiagram
{
  public:
    virtual ~QgsDiagram() {}
    /**Draws the diagram at the given position (in pixel coordinates)*/
    virtual void renderDiagram( const QgsAttributes& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position ) = 0;
    virtual QString diagramName() const = 0;
    /**Returns the size in map units the diagram will use to render.*/
    virtual QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s ) = 0;
    /**Returns the size in map units the diagram will use to render. Interpolate size*/
    virtual QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is ) = 0;

  protected:
    /** Changes the pen width to match the current settings and rendering context
     *  @param pen The pen to modify
     *  @param s   The settings that specify the pen width
     *  @param c   The rendering specifying the proper scale units for pixel conversion
     */
    void setPenWidth( QPen& pen, const QgsDiagramSettings& s, const QgsRenderContext& c );

    /** Calculates a size to match the current settings and rendering context
     *  @param size The size to convert
     *  @param s    The settings that specify the size type
     *  @param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  @return The converted size for rendering
     */
    QSizeF sizePainterUnits( const QSizeF& size, const QgsDiagramSettings& s, const QgsRenderContext& c );

    /** Calculates a length to match the current settings and rendering context
     *  @param l    The length to convert
     *  @param s    Unused
     *  @param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  @return The converted length for rendering
     */
    float sizePainterUnits( float l, const QgsDiagramSettings& s, const QgsRenderContext& c );

    /** Calculates a size to match the current settings and rendering context
     *  @param s    The settings that contain the font size and size type
     *  @param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  @return The properly scaled font for rendering
     */
    QFont scaledFont( const QgsDiagramSettings& s, const QgsRenderContext& c );
};

#endif // QGSDIAGRAM_H
