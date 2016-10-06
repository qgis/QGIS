/***************************************************************************
    qgshighlight.h - widget to highlight geometries
     --------------------------------------
    Date                 : 02-Mar-2011
    Copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    Email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSHIGHLIGHT_H
#define QGSHIGHLIGHT_H

#include "qgsmapcanvasitem.h"
#include "qgsgeometry.h"
#include "qgssymbol.h"
#include <QBrush>
#include <QColor>
#include <QList>
#include <QPen>
#include <QPainter>
#include <QPainterPath>

class QgsMapLayer;
class QgsVectorLayer;
class QgsSymbol;

/** \ingroup gui
 * A class for highlight features on the map.
 */
class GUI_EXPORT QgsHighlight: public QgsMapCanvasItem
{
  public:

    /** Constructor for QgsHighlight
     * @param mapCanvas associated map canvas
     * @param geom initial geometry of highlight
     * @param layer associated map layer
     */
    QgsHighlight( QgsMapCanvas *mapCanvas, const QgsGeometry& geom, QgsMapLayer *layer );

    /** Constructor for QgsHighlight
     * @param mapCanvas associated map canvas
     * @param geom initial geometry of highlight
     * @param layer associated vector layer
     */
    QgsHighlight( QgsMapCanvas *mapCanvas, const QgsGeometry& geom, QgsVectorLayer *layer );

    /** Constructor for highlighting true feature shape using feature attributes
     * and renderer.
     * @param mapCanvas map canvas
     * @param feature
     * @param layer vector layer
     */
    QgsHighlight( QgsMapCanvas *mapCanvas, const QgsFeature& feature, QgsVectorLayer *layer );
    ~QgsHighlight();

    /** Set line/outline to color, polygon fill to color with alpha = 63.
     *  This is legacy function, use setFillColor() after setColor() if different fill color is required. */
    void setColor( const QColor & color );

    /** Set polygons fill color.
     * @note: added in version 2.3 */
    void setFillColor( const QColor & fillColor );

    /** Set width. Ignored in feature mode. */
    void setWidth( int width );

    /** Set line / outline buffer in millimeters.
     *  @note: added in version 2.3 */
    void setBuffer( double buffer ) { mBuffer = buffer; }

    /** Set minimum line / outline width in millimeters.
     *  @note: added in version 2.3 */
    void setMinWidth( double width ) { mMinWidth = width; }

    const QgsMapLayer *layer() const { return mLayer; }

    virtual void updatePosition() override;

  protected:
    virtual void paint( QPainter* p ) override;

    //! recalculates needed rectangle
    void updateRect();

  private:
    void init();
    void setSymbol( QgsSymbol* symbol, const QgsRenderContext & context, const QColor & color, const QColor & fillColor );
    double getSymbolWidth( const QgsRenderContext & context, double width, QgsUnitTypes::RenderUnit unit );
    /** Get renderer for current color mode and colors. The renderer should be freed by caller. */
    QgsFeatureRenderer * getRenderer( QgsRenderContext &context, const QColor & color, const QColor & fillColor );
    void paintPoint( QPainter *p, const QgsPoint& point );
    void paintLine( QPainter *p, QgsPolyline line );
    void paintPolygon( QPainter *p, QgsPolygon polygon );

    QBrush mBrush;
    QPen mPen;
    QgsGeometry *mGeometry;
    QgsMapLayer *mLayer;
    QgsFeature mFeature;
    double mBuffer; // line / outline buffer in pixels
    double mMinWidth; // line / outline minimum width in pixels
};

#endif
