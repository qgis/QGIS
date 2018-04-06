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
#include "qgis_app.h"

class QgsMapCanvas;
class QgsRubberBand;
class QgsMapToolSelectionHandler;


class APP_EXPORT QgsMapToolSelectFreehand : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectFreehand( QgsMapCanvas *canvas );

    ~QgsMapToolSelectFreehand() override;

    //! Overridden mouse move event
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Overridden mouse release event
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    //! catch escape when active to cancel selection
    void keyReleaseEvent( QKeyEvent *e ) override;

  private:
    QgsMapToolSelectionHandler *mSelectionHandler;

    void selectFeatures(Qt::KeyboardModifiers modifiers);
};

#endif
