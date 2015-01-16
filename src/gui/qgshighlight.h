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
#include "qgsfeaturestore.h"
#include "qgsgeometry.h"
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include <QBrush>
#include <QColor>
#include <QList>
#include <QPen>
#include <QPainter>
#include <QPainterPath>

class QgsMapLayer;
class QgsVectorLayer;
class QgsSymbolV2;

/** A class for highlight features on the map.
 */
class GUI_EXPORT QgsHighlight: public QgsMapCanvasItem
{
  public:
    QgsHighlight( QgsMapCanvas *mapCanvas, QgsGeometry *geom, QgsMapLayer *layer );
    QgsHighlight( QgsMapCanvas *mapCanvas, QgsGeometry *geom, QgsVectorLayer *layer );
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
    void setSymbol( QgsSymbolV2* symbol, const QgsRenderContext & context, const QColor & color, const QColor & fillColor );
    double getSymbolWidth( const QgsRenderContext & context, double width, QgsSymbolV2::OutputUnit unit );
    /** Get renderer for current color mode and colors. The renderer should be freed by caller. */
    QgsFeatureRendererV2 * getRenderer( const QgsRenderContext & context, const QColor & color, const QColor & fillColor );
    void paintPoint( QPainter *p, QgsPoint point );
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
