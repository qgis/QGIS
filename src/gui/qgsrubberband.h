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

#include <QWidget>
#include <QPolygon>
class QPaintEvent;

class QgsRubberBand: public QWidget
{
  public:
    QgsRubberBand(QWidget * parent, bool isPolygon = false);
    ~QgsRubberBand();
    
    void reset(bool isPolygon = false);
    void addPoint(const QPoint & p);
    void movePoint(const QPoint & p);

  protected:
    virtual void paintEvent(QPaintEvent * event);

  private:
    QPolygon mPoints;
    bool mIsPolygon;
};

#endif
