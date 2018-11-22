/***************************************************************************
    qgsmaptoolselect.h  -  map tool for selecting features
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

#ifndef QGSMAPTOOLSELECT_H
#define QGSMAPTOOLSELECT_H

#include "qgsmaptool.h"
#include "qgis_app.h"
#include "qgsmaptoolselectionhandler.h"

class QgsMapCanvas;
class QMouseEvent;

class APP_EXPORT QgsMapToolSelect : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelect( QgsMapCanvas *canvas );

    void setSelectionMode( QgsMapToolSelectionHandler::SelectionMode selectionMode );

    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;
    void deactivate() override;

  private slots:
    void selectFeatures( Qt::KeyboardModifiers modifiers );

  private:
    std::unique_ptr<QgsMapToolSelectionHandler> mSelectionHandler;
};

#endif
