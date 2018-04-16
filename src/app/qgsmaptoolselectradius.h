/***************************************************************************
qgsmaptoolselectradius.h  -  map tool for selecting features by radius
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

#ifndef QGSMAPTOOLSELECTRADIUS_H
#define QGSMAPTOOLSELECTRADIUS_H


#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgis_app.h"

class QHBoxLayout;

class QgsMapCanvas;
class QgsSnapIndicator;
class QgsMapToolSelectionHandler;

class APP_EXPORT QgsMapToolSelectRadius : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSelectRadius( QgsMapCanvas *canvas );

    ~QgsMapToolSelectRadius() override;

    //! Overridden mouse move event
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Overridden mouse release event
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

    //! catch escape when active to cancel selection
    void keyReleaseEvent( QKeyEvent *e ) override;

  private:

    void selectFeatures( Qt::KeyboardModifiers modifiers = Qt::NoModifier );

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    QgsMapToolSelectionHandler *mSelectionHandler;
};

#endif
