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
#include <QBrush>
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

    void setColor( const QColor & color );

    /** Set width. Ignored in feature mode. */
    void setWidth( int width );

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();

  private:
    void init();
    void setSymbolColor( QgsSymbolV2* symbol, const QColor & color );
    void paintPoint( QPainter *p, QgsPoint point );
    void paintLine( QPainter *p, QgsPolyline line );
    void paintPolygon( QPainter *p, QgsPolygon polygon );

    QgsVectorLayer *vectorLayer();

    QgsHighlight();

    QBrush mBrush;
    QPen mPen;
    QgsGeometry *mGeometry;
    QgsMapLayer *mLayer;
    QgsFeature mFeature;
    QgsFeatureRendererV2 *mRenderer;
};

#endif
