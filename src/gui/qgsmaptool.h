/***************************************************************************
    qgsmaptool.h  -  base class for map canvas tools
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSMAPTOOL_H
#define QGSMAPTOOL_H

#include <QCursor>
#include <QString>
#include <QObject>

class QgsMapLayer;
class QgsMapCanvas;
class QMouseEvent;
class QgsPoint;
class QgsRect;
class QPoint;
class QAction;


class GUI_EXPORT QgsMapTool : public QObject
{
  public:
    
    //! virtual destructor
    virtual ~QgsMapTool();
    
    //! Mouse move event for overriding. Default implementation does nothing.
    virtual void canvasMoveEvent(QMouseEvent * e);

    //! Mouse press event for overriding. Default implementation does nothing.
    virtual void canvasPressEvent(QMouseEvent * e);

    //! Mouse release event for overriding. Default implementation does nothing.
    virtual void canvasReleaseEvent(QMouseEvent * e);
    
    //! Called when rendering has finished. Default implementation does nothing.
    virtual void renderComplete();
    
    /** Use this to associate a button, toolbutton, menu entry etc
     * that inherits qaction to this maptool. Then when the setMapTool
     * method of mapcanvas is called the action state will be set to on.
     * Usually this will cause e.g. a toolbutton to appear pressed in and
     * the previously used toolbutton to pop out. */
    void setAction(QAction* action);
    
    QAction* action();
    
    /** Check whether this MapTool performs a zoom or pan operation.
     * If it does, we will be able to perform the zoom  and then 
     * resume operations with the original / previously used tool.*/
    virtual bool isZoomTool();
    
    //! called when set as currently active map tool
    virtual void activate();
    
    //! called when map tool is being deactivated
    virtual void deactivate();
    
    //! returns pointer to the tool's map canvas
    QgsMapCanvas* canvas();
    
  protected:

    //! constructor takes map canvas as a parameter
    QgsMapTool(QgsMapCanvas* canvas);
        
    //! transformation from screen coordinates to map coordinates
    QgsPoint toMapCoords(const QPoint& point);
    
    //! transformation from screen coordinates to layer's coordinates
    QgsPoint toLayerCoords(QgsMapLayer* layer, const QPoint& point);
    
    //! trnasformation from map coordinates to layer's coordinates
    QgsPoint toLayerCoords(QgsMapLayer* layer, const QgsPoint& point);
    
    //! trnasformation of the rect from map coordinates to layer's coordinates
    QgsRect toLayerCoords(QgsMapLayer* layer, const QgsRect& rect);

    //! transformation from map coordinates to screen coordinates
    QPoint toCanvasCoords(const QgsPoint& point);
    
    //! pointer to map canvas
    QgsMapCanvas* mCanvas;
    
    //! cursor used in map tool
    QCursor mCursor;
    
    //! optionally map tool can have pointer to action
    //! which will be used to set that action as active
    QAction* mAction;
    
};

#endif
