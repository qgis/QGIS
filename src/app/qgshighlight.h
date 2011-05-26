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
#include <QBrush>
#include <QList>
#include <QPen>
#include <QPainter>

class QgsVectorLayer;

/** A class for highlight features on the map.
 */
class QgsHighlight: public QgsMapCanvasItem
{
  public:
    QgsHighlight( QgsMapCanvas *mapCanvas, QgsGeometry *geom, QgsVectorLayer *layer );
    ~QgsHighlight();

    void setColor( const QColor & color );
    void setWidth( int width );

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();

  private:
    void paintPoint( QPainter *p, QgsPoint point );
    void paintLine( QPainter *p, QgsPolyline line );
    void paintPolygon( QPainter *p, QgsPolygon polygon );

    QgsHighlight();

    QBrush mBrush;
    QPen mPen;
    QgsGeometry *mGeometry;
    QgsVectorLayer *mLayer;
};

#endif
