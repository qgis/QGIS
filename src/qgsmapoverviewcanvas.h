/***************************************************************************
                           qgsmapoverviewcanvas.h
                      Map canvas subclassed for overview
                              -------------------
    begin                : 09/14/2005
    copyright            : (C) 2005 by Martin Dobias
    email                : won.der at centrum.sk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSMAPOVERVIEWCANVAS_H
#define QGSMAPOVERVIEWCANVAS_H

#include "qgsmapcanvas.h"

class QgsPanningWidget; // defined in .cpp

class QgsMapOverviewCanvas : public QgsMapCanvas
{
  Q_OBJECT;
  
  public:
    QgsMapOverviewCanvas(QWidget * parent = 0, QgsMapCanvas* mapCanvas = NULL);
  
    //! used for overview canvas to reflect changed extent in main map canvas
    void reflectChangedExtent();

  public slots:

    /** possibly add or remove the given layer from the overview map canvas
  
      @param maplayer is layer to be possibly added or removed from overview canvas
      @param b is true if visible in over view
    */
    void showInOverview( QgsMapLayer * maplayer, bool visible );

    //! reimplemented from QgsMapCanvas
    void addLayer(QgsMapLayer * lyr);

    //! renders overview (using QgsMapCanvas) and updates panning widget
    void render(QPaintDevice * theQPaintDevice=0);
  
  protected:
  
    //! Overridden mouse move event
    void mouseMoveEvent(QMouseEvent * e);

    //! Overridden mouse press event
    void mousePressEvent(QMouseEvent * e);

    //! Overridden mouse release event
    void mouseReleaseEvent(QMouseEvent * e);
    
    //! called when panning to reflect mouse movement
    void updatePanningWidget(const QPoint& pos);
    
    //! wheel event - does nothing in overview
    void wheelEvent(QWheelEvent * e);
        
    //! widget for panning map in overview
    QgsPanningWidget* mPanningWidget;
    
    //! position of cursor inside panning widget
    QPoint mPanningCursorOffset;
  
    //! main map canvas - used to get/set extent
    QgsMapCanvas* mMapCanvas;
        
};

#endif
