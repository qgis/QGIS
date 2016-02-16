/***************************************************************************
    qgsmaptoolselectfreehand.h  -  map tool for selecting features by freehand
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Jeremy Palmer
    email                : jpalmer at linz dot govt dot nz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSELECTFREEHAND_H
#define QGSMAPTOOLSELECTFREEHAND_H

#include "qgsmaptool.h"

class QgsMapCanvas;
class QgsRubberBand;


class APP_EXPORT QgsMapToolSelectFreehand : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectFreehand( QgsMapCanvas* canvas );

    virtual ~QgsMapToolSelectFreehand();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

  private:

    //! used for storing all of the maps point for the freehand sketch
    QgsRubberBand* mRubberBand;

    bool mDragging;

    QColor mFillColor;
    QColor mBorderColour;
};

#endif
