/***************************************************************************
                          qgsplottool.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPLOTTOOL_H
#define QGSPLOTTOOL_H

#include "qgsconfig.h"
#include "qgis.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QPointer>
#include <QCursor>

class QgsPlotCanvas;
class QgsPlotMouseEvent;
class QgsPoint;
class QgsPointXY;

class QWheelEvent;
class QKeyEvent;
class QGestureEvent;
class QHelpEvent;
class QMenu;
class QAction;

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsplottool.h"
#include "qgsplottoolpan.h"
#include "qgsplottoolzoom.h"
% End
#endif


/**
 * \ingroup gui
 * \brief Abstract base class for all interactive plot tools.
 *
 * Plot tools are user tools for manipulating and interacting with a QgsPlotCanvas.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsPlotTool : public QObject
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsPlotToolPan *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPlotToolPan;
    else if ( qobject_cast<QgsPlotToolZoom *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPlotToolZoom;
    else if ( qobject_cast<QgsPlotTool *>( sipCpp ) != nullptr )
      sipType = sipType_QgsPlotTool;
    else
      sipType = nullptr;
    SIP_END
#endif

    Q_OBJECT

  public:

    ~QgsPlotTool() override;

    /**
     * Returns the flags for the plot tool.
     */
    virtual Qgis::PlotToolFlags flags() const;

    /**
     * Returns a user-visible, translated name for the tool.
     */
    QString toolName() const { return mToolName; }

    /**
     * Mouse move \a event for overriding.
     *
     * The default implementation does nothing. When subclasses implement this method
     * and have acted on the event, they must explicitly call event->accept() to prevent
     * the event from being passed on to other widgets.
     */
    virtual void plotMoveEvent( QgsPlotMouseEvent *event );

    /**
     * Mouse double-click \a event for overriding.
     *
     * The default implementation does nothing. When subclasses implement this method
     * and have acted on the event, they must explicitly call event->accept() to prevent
     * the event from being passed on to other widgets.
     */
    virtual void plotDoubleClickEvent( QgsPlotMouseEvent *event );

    /**
     * Mouse press \a event for overriding.
     *
     * The default implementation does nothing. When subclasses implement this method
     * and have acted on the event, they must explicitly call event->accept() to prevent
     * the event from being passed on to other widgets.
     */
    virtual void plotPressEvent( QgsPlotMouseEvent *event );

    /**
     * Mouse release \a event for overriding.
     *
     * The default implementation does nothing. When subclasses implement this method
     * and have acted on the event, they must explicitly call event->accept() to prevent
     * the event from being passed on to other widgets.
     */
    virtual void plotReleaseEvent( QgsPlotMouseEvent *event );

    /**
     * Mouse wheel \a event for overriding.
     *
     * The default implementation does nothing. When subclasses implement this method
     * and have acted on the event, they must explicitly call event->accept() to prevent
     * the event from being passed on to other widgets.
     */
    virtual void wheelEvent( QWheelEvent *event );

    /**
     * Key press \a event for overriding.
     *
     * The default implementation does nothing. When subclasses implement this method
     * and have acted on the event, they must explicitly call event->accept() to prevent
     * the event from being passed on to other widgets.
     */
    virtual void keyPressEvent( QKeyEvent *event );

    /**
     * Key release \a event for overriding.
     *
     * The default implementation does nothing. When subclasses implement this method
     * and have acted on the event, they must explicitly call event->accept() to prevent
     * the event from being passed on to other widgets.
     */
    virtual void keyReleaseEvent( QKeyEvent *event );

    /**
     * Gesture \a event for overriding.
     *
     * Returns TRUE if the event was handled by the tool and should not be propagated further.
     */
    virtual bool gestureEvent( QGestureEvent *event );

    /**
     * Tooltip \a event for overriding.
     *
     * Returns TRUE if the event was handled by the tool and should not be propagated further.
     */
    virtual bool canvasToolTipEvent( QHelpEvent *event );

    /**
     * Returns TRUE if this tool is the current tool active on the plot canvas.
     */
    bool isActive() const;

    /**
     * Called when the tool is set as the currently active plot tool.
     */
    virtual void activate();

    /**
     * Called when the tool is being deactivated.
     */
    virtual void deactivate();

    /**
     * Returns the tool's plot canvas.
     */
    QgsPlotCanvas *canvas() const;

    /**
     * Associates an \a action with this tool. When the setTool
     * method of QgsPlotCanvas is called the action's state will be set to on.
     * Usually this will cause a toolbutton to appear pressed in and
     * the previously used toolbutton to pop out.
     *
     * \see action()
    */
    void setAction( QAction *action );

    /**
     * Returns the action associated with the tool or NULLPTR if no action is associated.
     *
     * \see setAction()
     */
    QAction *action();

    /**
     * Sets a user defined \a cursor for use when the tool is active.
     */
    void setCursor( const QCursor &cursor );

    /**
     * Allows the tool to populate and customize the given \a menu,
     * prior to showing it in response to a right-mouse button click.
     *
     * \a menu will be initially populated with a set of default, generic actions.
     * Any new actions added to the menu should be correctly parented to \a menu.
     *
     * A pointer to the plot mouse \a event can be provided to allow particular behavior depending on the plot tool.
     *
     * This method can return TRUE to inform the caller that the menu was effectively populated.
     *
     * The default implementation does nothing and returns FALSE.
     *
     * \note The context menu is only shown when the ShowContextMenu flag
     * is present in flags().
     */
    virtual bool populateContextMenuWithEvent( QMenu *menu, QgsPlotMouseEvent *event );

  signals:

    //! Emitted when the tool is activated.
    void activated();

    //! Emitted when the tool is deactivated
    void deactivated();

  protected:

    //! Constructor takes a plot canvas as a parameter.
    QgsPlotTool( QgsPlotCanvas *canvas SIP_TRANSFERTHIS, const QString &name );

    /**
     * Converts a \a point on the canvas to the associated map coordinate.
     *
     * May return an empty point if the canvas point cannot be converted to a map point.
     */
    QgsPoint toMapCoordinates( const QgsPointXY &point ) const;

    /**
     * Converts a \a point in map coordinates to the associated canvas point.
     *
     * May return an empty point if the map point cannot be converted to a canvas point.
     */
    QgsPointXY toCanvasCoordinates( const QgsPoint &point ) const;

    /**
     * Returns TRUE if a mouse press/release operation which started at
     * \a startViewPoint and ended at \a endViewPoint should be considered
     * a "click and drag". If FALSE is returned, the operation should be
     * instead treated as just a click on \a startViewPoint.
     */
    bool isClickAndDrag( QPoint startViewPoint, QPoint endViewPoint ) const;

    //! The pointer to the canvas
    QgsPlotCanvas *mCanvas = nullptr;

    //! Translated name of the map tool
    QString mToolName;

    //! Optional action associated with tool
    QPointer< QAction > mAction;

    //! Cursor used by tool
    QCursor mCursor = Qt::ArrowCursor;
};

#endif // QGSPLOTTOOL_H
