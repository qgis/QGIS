/***************************************************************************
     qgsgcpcanvasitem.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGCPCANVASITEM_H
#define QGSGCPCANVASITEM_H

#include "qgsmapcanvas.h"
#include "qgsmapcanvasitem.h"

class QgsGeorefDataPoint;

class QgsGCPCanvasItem : public QgsMapCanvasItem
{
  public:
    QgsGCPCanvasItem( QgsMapCanvas* mapCanvas, const QgsGeorefDataPoint* dataPoint, bool isGCPSource/* = true*/ );

    //! draws point information
    void paint( QPainter* p ) override;

    //! handler for manual updating of position and size
    QRectF boundingRect() const override;

    QPainterPath shape() const override;

    void updatePosition() override;

    /**Calls prepareGeometryChange()*/
    void checkBoundingRectChange();

  private:

    const QgsGeorefDataPoint* mDataPoint;
    QSizeF mTextBounds;
    QBrush mPointBrush;
    QBrush mLabelBrush;
    bool mIsGCPSource;
    QPen mResidualPen;

    //text box rect for bounding rect calculation (updated in the paint event)
    QRectF mTextBoxRect;

    void drawResidualArrow( QPainter* p, const QgsRenderContext& context );
    /**Calculates scale factor for residual display*/
    double residualToScreenFactor() const;
    /**Calculates pixel size for a font point size*/
    double fontSizePainterUnits( double points, const QgsRenderContext& c );
};

#endif // QGSGCPCANVASITEM_H
