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

#ifndef QGSMAPTOOL_H
#define QGSMAPTOOL_H

#include "qgsconfig.h"
#include "qgsmessagebar.h"

#include <QCursor>
#include <QString>
#include <QObject>

#ifdef HAVE_TOUCH
#include <QGestureEvent>
#endif

class QgsMapLayer;
class QgsMapCanvas;
class QgsRenderContext;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
class QgsPoint;
class QgsRectangle;
class QPoint;
class QAction;
class QAbstractButton;

/** \ingroup gui
 * Abstract base class for all map tools.
 * Map tools are user interactive tools for manipulating the
 * map canvas. For example map pan and zoom features are
 * implemented as map tools.
 */
class GUI_EXPORT QgsMapTool : public QObject
{

    Q_OBJECT

  public:

    //! virtual destructor
    virtual ~QgsMapTool();

    //! Mouse move event for overriding. Default implementation does nothing.
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Mouse double click event for overriding. Default implementation does nothing.
    virtual void canvasDoubleClickEvent( QMouseEvent * e );

    //! Mouse press event for overriding. Default implementation does nothing.
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Mouse release event for overriding. Default implementation does nothing.
    virtual void canvasReleaseEvent( QMouseEvent * e );

    //! Mouse wheel event for overriding. Default implementation does nothing.
    virtual void wheelEvent( QWheelEvent* e );

    //! Key event for overriding. Default implementation does nothing.
    virtual void keyPressEvent( QKeyEvent* e );

    //! Key event for overriding. Default implementation does nothing.
    virtual void keyReleaseEvent( QKeyEvent* e );

#ifdef HAVE_TOUCH
    //! gesture event for overriding. Default implementation does nothing.
    virtual bool gestureEvent( QGestureEvent* e );
#endif

    //! Called when rendering has finished. Default implementation does nothing.
    //! @deprecated since 2.4 - not called anymore - map tools must not directly depend on rendering progress
    Q_DECL_DEPRECATED virtual void renderComplete();


    /** Use this to associate a QAction to this maptool. Then when the setMapTool
     * method of mapcanvas is called the action state will be set to on.
     * Usually this will cause e.g. a toolbutton to appear pressed in and
     * the previously used toolbutton to pop out. */
    void setAction( QAction* action );

    /** Return associated action with map tool or NULL if no action is associated */
    QAction* action();

    /** Use this to associate a button to this maptool. It has the same meaning
     * as setAction() function except it works with a button instead of an QAction. */
    void setButton( QAbstractButton* button );

    /** Return associated button with map tool or NULL if no button is associated */
    QAbstractButton* button();

    /** Set a user defined cursor */
    virtual void setCursor( QCursor cursor );

    /** Check whether this MapTool performs a zoom or pan operation.
     * If it does, we will be able to perform the zoom  and then
     * resume operations with the original / previously used tool.*/
    virtual bool isTransient();

    /** Check whether this MapTool performs an edit operation.
     * If it does, we will deactivate it when editing is turned off
     */
    virtual bool isEditTool();

    //! called when set as currently active map tool
    virtual void activate();

    //! called when map tool is being deactivated
    virtual void deactivate();

    //! returns pointer to the tool's map canvas
    QgsMapCanvas* canvas();

    //! Emit map tool changed with the old tool
    //! @note added in 2.3
    QString toolName() { return mToolName; }

    /** Get search radius in mm. Used by identify, tip etc.
     *  The values is currently set in identify tool options (move somewhere else?)
     *  and defaults to QGis::DEFAULT_SEARCH_RADIUS_MM.
     *  @note added in 2.3 */
    static double searchRadiusMM();

    /** Get search radius in map units for given context. Used by identify, tip etc.
     *  The values is calculated from searchRadiusMM().
     *  @note added in 2.3 */
    static double searchRadiusMU( const QgsRenderContext& context );

    /** Get search radius in map units for given canvas. Used by identify, tip etc.
     *  The values is calculated from searchRadiusMM().
     *  @note added in 2.3 */
    static double searchRadiusMU( QgsMapCanvas * canvas );

  signals:
    //! emit a message
    void messageEmitted( QString message, QgsMessageBar::MessageLevel = QgsMessageBar::INFO );

    //! emit signal to clear previous message
    void messageDiscarded();

    //! signal emitted once the map tool is activated
    void activated();

    //! signal emitted once the map tool is deactivated
    void deactivated();

  private slots:
    //! clear pointer when action is destroyed
    void actionDestroyed();

  protected:

    //! constructor takes map canvas as a parameter
    QgsMapTool( QgsMapCanvas* canvas );

    //! transformation from screen coordinates to map coordinates
    QgsPoint toMapCoordinates( const QPoint& point );

    //! transformation from screen coordinates to layer's coordinates
    QgsPoint toLayerCoordinates( QgsMapLayer* layer, const QPoint& point );

    //! transformation from map coordinates to layer's coordinates
    QgsPoint toLayerCoordinates( QgsMapLayer* layer, const QgsPoint& point );

    //!transformation from layer's coordinates to map coordinates (which is different in case reprojection is used)
    QgsPoint toMapCoordinates( QgsMapLayer* layer, const QgsPoint& point );

    //! trnasformation of the rect from map coordinates to layer's coordinates
    QgsRectangle toLayerCoordinates( QgsMapLayer* layer, const QgsRectangle& rect );

    //! transformation from map coordinates to screen coordinates
    QPoint toCanvasCoordinates( const QgsPoint& point );

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

    //! translated name of the map tool
    QString mToolName;
};

#endif
