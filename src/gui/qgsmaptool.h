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

class QgsMapLayer;
class QgsMapCanvas;
class QMouseEvent;
class QgsPoint;
class QPoint;
class QAction;
class QAbstractButton;

class QgsMapTool
{
  public:
    
    //! virtual destructor
    virtual ~QgsMapTool();
    
    //! Mouse move event for overriding
    virtual void canvasMoveEvent(QMouseEvent * e) { }

    //! Mouse press event for overriding
    virtual void canvasPressEvent(QMouseEvent * e) { }

    //! Mouse release event for overriding
    virtual void canvasReleaseEvent(QMouseEvent * e) { }
    
    //! Called when rendering has finished
    virtual void renderComplete() {}
    
    
    /** Use this to associate a QAction to this maptool. Then when the setMapTool
     * method of mapcanvas is called the action state will be set to on.
     * Usually this will cause e.g. a toolbutton to appear pressed in and
     * the previously used toolbutton to pop out. */
    void setAction(QAction* action) { mAction = action; }
    
    /** Return associated action with map tool or NULL if no action is associated */
    QAction* action() { return mAction; }
    
    /** Use this to associate a button to this maptool. It has the same meaning
     * as setAction() function except it works with a button instead of an QAction. */
    void setButton(QAbstractButton* button);
    
    /** Return associated button with map tool or NULL if no button is associated */
    QAbstractButton* button();
    
    
    /** Check whether this MapTool performs a zoom or pan operation.
     * If it does, we will be able to perform the zoom  and then 
     * resume operations with the original / previously used tool.*/
    virtual bool isZoomTool() { return false;}
    
    //! called when set as currently active map tool
    virtual void activate();
    
    //! called when map tool is being deactivated
    virtual void deactivate();
    
  protected:

    //! constructor takes map canvas as a parameter
    QgsMapTool(QgsMapCanvas* canvas);
        
    //! transformation from screen coordinates to map coordinates
    QgsPoint toMapCoords(const QPoint& point);
    
    //! transformation from screen coordinates to layer's coordinates
    QgsPoint toLayerCoords(QgsMapLayer* layer, const QPoint& point);
    
    //! transformation from map coordinates to screen coordinates
    QPoint toCanvasCoords(const QgsPoint& point);
    
    //! pointer to map canvas
    QgsMapCanvas* mCanvas;
    
    //! cursor used in map tool
    QCursor mCursor;
    
    //! optionally map tool can have pointer to action
    //! which will be used to set that action as active
    QAction* mAction;
    
    //! optionally map tool can have pointer to a button
    //! which will be used to set that action as active
    QAbstractButton* mButton;
    
};

#endif
