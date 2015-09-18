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
#include "qgsexpressioncontext.h"
#include <QPen>
#include <QBrush>

class QPainter;
class QPointF;
class QgsDiagramSettings;
class QgsDiagramInterpolationSettings;

class QgsRenderContext;

class QgsExpression;



/** Base class for all diagram types*/
class CORE_EXPORT QgsDiagram
{
  public:
    virtual ~QgsDiagram() { clearCache(); }
    /** Returns an instance that is equivalent to this one
     * @note added in 2.4 */
    virtual QgsDiagram* clone() const = 0;

    void clearCache();

    Q_DECL_DEPRECATED QgsExpression* getExpression( const QString& expression, const QgsFields* fields );

    /** Returns a prepared expression for the specified context.
     * @param expression expression string
     * @param context expression context
     * @note added in QGIS 2.12
     */
    QgsExpression* getExpression( const QString& expression, const QgsExpressionContext& context );

    /** @deprecated `void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )` should be used instead */
    virtual Q_DECL_DEPRECATED void renderDiagram( const QgsAttributes& att, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position );
    /** Draws the diagram at the given position (in pixel coordinates)*/
    virtual void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position ) = 0;
    virtual QString diagramName() const = 0;
    /** Returns the size in map units the diagram will use to render.*/
    virtual QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s ) = 0;
    /** @deprecated `QSizeF diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is )` should be used instead */
    virtual Q_DECL_DEPRECATED QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is );
    /** Returns the size in map units the diagram will use to render. Interpolate size*/
    virtual QSizeF diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is ) = 0;

  protected:
    QgsDiagram();
    QgsDiagram( const QgsDiagram& other );

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

  private:
    QMap<QString, QgsExpression*> mExpressions;
};

#endif // QGSDIAGRAM_H
