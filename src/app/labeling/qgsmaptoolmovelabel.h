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
#include "qgis_app.h"

//! A map tool for dragging label positions
class APP_EXPORT QgsMapToolMoveLabel: public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolMoveLabel( QgsMapCanvas *canvas );

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

  protected:

    //! Start point of the move in map coordinates
    QgsPointXY mStartPointMapCoords;

    double mClickOffsetX = 0;
    double mClickOffsetY = 0;
};

#endif // QGSMAPTOOLMOVELABEL_H
