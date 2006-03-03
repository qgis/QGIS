/***************************************************************************
    qgsrubberband.h - Rubberband widget for drawing multilines and polygons
     --------------------------------------
    Date                 : 07-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSRUBBERBAND_H
#define QGSRUBBERBAND_H

#include "qgsmapcanvasitem.h"
#include <deque>
#include <QBrush>
#include <QPen>
#include <QPolygon>
class QPaintEvent;

class QgsRubberBand: public QgsMapCanvasItem
{
  public:
    QgsRubberBand(QgsMapCanvas* mapCanvas, bool isPolygon = false);
    ~QgsRubberBand();

    void setColor(const QColor & color);
    void setWidth(int width);

    void reset(bool isPolygon = false);
    void addPoint(const QgsPoint & p);
    void movePoint(const QgsPoint & p);
    void movePoint(int index, const QgsPoint& p);

  protected:
    virtual void drawShape(QPainter & p);
    
    //! recalculates needed rectangle
    void updateRect();

  private:
    QBrush mBrush;
    QPen mPen;
    std::deque<QgsPoint> mPoints;
    bool mIsPolygon;
};

#endif
