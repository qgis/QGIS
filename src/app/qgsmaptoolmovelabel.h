/***************************************************************************
                          qgsmaptoolmovelabel.h
                          --------------------
    begin                : 2010-11-03
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLMOVELABEL_H
#define QGSMAPTOOLMOVELABEL_H

#include "qgsmaptoollabel.h"

/**A map tool for dragging label positions*/
class QgsMapToolMoveLabel: public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolMoveLabel( QgsMapCanvas* canvas );
    ~QgsMapToolMoveLabel();

    virtual void canvasPressEvent( QMouseEvent * e );

    virtual void canvasMoveEvent( QMouseEvent * e );

    virtual void canvasReleaseEvent( QMouseEvent * e );

  protected:

    /**Start point of the move in map coordinates*/
    QgsPoint mStartPointMapCoords;

    double mClickOffsetX;
    double mClickOffsetY;
};

#endif // QGSMAPTOOLMOVELABEL_H
