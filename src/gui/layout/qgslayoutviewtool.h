/***************************************************************************
                             qgslayoutviewtool.h
                             -------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTVIEWTOOL_H
#define QGSLAYOUTVIEWTOOL_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QCursor>
#include <QAction>
#include <QPointer>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QgsLayoutView;

/**
 * \ingroup gui
 * Abstract base class for all layout view tools.
 * Layout view tools are user interactive tools for manipulating and adding items
 * to QgsLayoutView widgets.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutViewTool : public QObject
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsMapToolZoom *>( sipCpp ) != NULL )
      sipType = sipType_QgsMapToolZoom;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT

  public:

    virtual ~QgsLayoutViewTool();

    /**
     * Mouse move event for overriding. Default implementation does nothing.
     */
    virtual void layoutMoveEvent( QMouseEvent *event );

    /**
     * Mouse double click event for overriding. Default implementation does nothing.
     */
    virtual void layoutDoubleClickEvent( QMouseEvent *event );

    /**
     * Mouse press event for overriding. Default implementation does nothing.
     */
    virtual void layoutPressEvent( QMouseEvent *event );

    /**
     * Mouse release event for overriding. Default implementation does nothing.
     */
    virtual void layoutReleaseEvent( QMouseEvent *event );

    /**
     * Mouse wheel event for overriding. Default implementation does nothing.
     */
    virtual void wheelEvent( QWheelEvent *event );

    /**
     * Key press event for overriding. Default implementation does nothing.
     */
    virtual void keyPressEvent( QKeyEvent *event );

    /**
     * Key release event for overriding. Default implementation does nothing.
     */
    virtual void keyReleaseEvent( QKeyEvent *event );

    /**
     * Associates an \a action with this tool. When the setLayoutTool
     * method of QgsLayoutView is called the action's state will be set to on.
     * Usually this will cause a toolbutton to appear pressed in and
     * the previously used toolbutton to pop out.
     * \see action()
    */
    void setAction( QAction *action );

    /**
     * Returns the action associated with the tool or nullptr if no action is associated.
     * \see setAction()
     */
    QAction *action();

    /**
     * Sets a user defined \a cursor for use when the tool is active.
     */
    void setCursor( const QCursor &cursor );

    /**
     * Called when tool is set as the currently active layout tool.
     * Overridden implementations must take care to call the base class implementation.
     */
    virtual void activate();

    /**
     * Called when tool is deactivated.
     * Overridden implementations must take care to call the base class implementation.
     */
    virtual void deactivate();

    /**
     * Returns a user-visible, translated name for the tool.
     */
    QString toolName() const { return mToolName; }

  signals:

    /**
     * Emitted when the tool is activated.
     */
    void activated();

    /**
     * Emitted when the tool is deactivated.
     */
    void deactivated();

  protected:

    /**
     * Constructor for QgsLayoutViewTool, taking a layout \a view and
     * tool \a name as parameters.
     */
    QgsLayoutViewTool( QgsLayoutView *view SIP_TRANSFERTHIS, const QString &name );

  private:

    //! Pointer to layout view.
    QgsLayoutView *mView = nullptr;

    //! Cursor used by tool
    QCursor mCursor;

    //! Optional action associated with tool
    QPointer< QAction > mAction;

    //! Translated name of the map tool
    QString mToolName;

    friend class TestQgsLayoutView;

};

#endif // QGSLAYOUTVIEWTOOL_H
