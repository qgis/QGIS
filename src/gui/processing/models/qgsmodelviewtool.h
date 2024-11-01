/***************************************************************************
                             qgsmodelviewtool.h
                             -------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELVIEWTOOL_H
#define QGSMODELVIEWTOOL_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include <QCursor>
#include <QAction>
#include <QPointer>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class QgsModelGraphicsView;
class QgsModelViewMouseEvent;
class QgsModelComponentGraphicItem;
class QgsModelGraphicsScene;

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Abstract base class for all model designer view tools.
 *
 * Model designer view tools are user interactive tools for manipulating and adding items
 * within the model designer.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelViewTool : public QObject
{
    Q_OBJECT

  public:
    //! Flags for controlling how a tool behaves
    enum Flag
    {
      FlagSnaps = 1 << 1, //!< Tool utilizes snapped coordinates.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    ~QgsModelViewTool() override;

    /**
     * Returns the current combination of flags set for the tool.
     * \see setFlags()
     */
    QgsModelViewTool::Flags flags() const;

    /**
     * Mouse move event for overriding. Default implementation does nothing.
     */
    virtual void modelMoveEvent( QgsModelViewMouseEvent *event );

    /**
     * Mouse double-click event for overriding. Default implementation does nothing.
     */
    virtual void modelDoubleClickEvent( QgsModelViewMouseEvent *event );

    /**
     * Mouse press event for overriding. Default implementation does nothing.
     * Note that subclasses must ensure that they correctly handle cases
     * when a modelPressEvent is called without a corresponding
     * modelReleaseEvent (e.g. due to tool being changed mid way
     * through a press-release operation).
     */
    virtual void modelPressEvent( QgsModelViewMouseEvent *event );

    /**
     * Mouse release event for overriding. Default implementation does nothing.
     * Note that subclasses must ensure that they correctly handle cases
     * when a modelPressEvent is called without a corresponding
     * modelReleaseEvent (e.g. due to tool being changed mid way
     * through a press-release operation).
     */
    virtual void modelReleaseEvent( QgsModelViewMouseEvent *event );

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
     * Returns TRUE if the tool allows interaction with component graphic items.
     */
    virtual bool allowItemInteraction();

    /**
     * Associates an \a action with this tool. When the setModelTool
     * method of QgsModelGraphicsView is called the action's state will be set to on.
     * Usually this will cause a toolbutton to appear pressed in and
     * the previously used toolbutton to pop out.
     * \see action()
    */
    void setAction( QAction *action );

    /**
     * Returns the action associated with the tool or NULLPTR if no action is associated.
     * \see setAction()
     */
    QAction *action();

    /**
     * Sets a user defined \a cursor for use when the tool is active.
     */
    void setCursor( const QCursor &cursor );

    /**
     * Called when tool is set as the currently active model tool.
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

    /**
     * Returns the view associated with the tool.
     * \see scene()
     */
    QgsModelGraphicsView *view() const;

    /**
     * Returns the scene associated with the tool.
     * \see view()
     */
    QgsModelGraphicsScene *scene() const;

  signals:

    /**
     * Emitted when the tool is activated.
     */
    void activated();

    /**
     * Emitted when the tool is deactivated.
     */
    void deactivated();

    /**
     * Emitted when an \a item is "focused" by the tool, i.e. it should become the active
     * item and should have its properties displayed in any designer windows.
     */
    void itemFocused( QgsModelComponentGraphicItem *item );

  protected:
    /**
     * Sets the combination of \a flags that will be used for the tool.
     * \see flags()
     */
    void setFlags( QgsModelViewTool::Flags flags );

    /**
     * Constructor for QgsModelViewTool, taking a model \a view and
     * tool \a name as parameters.
     */
    QgsModelViewTool( QgsModelGraphicsView *view SIP_TRANSFERTHIS, const QString &name );

    /**
     * Returns TRUE if a mouse press/release operation which started at
     * \a startViewPoint and ended at \a endViewPoint should be considered
     * a "click and drag". If FALSE is returned, the operation should be
     * instead treated as just a click on \a startViewPoint.
     */
    bool isClickAndDrag( QPoint startViewPoint, QPoint endViewPoint ) const;

  private:
    //! Pointer to model view.
    QgsModelGraphicsView *mView = nullptr;

    QgsModelViewTool::Flags mFlags = QgsModelViewTool::Flags();

    //! Cursor used by tool
    QCursor mCursor = Qt::ArrowCursor;

    //! Optional action associated with tool
    QPointer<QAction> mAction;

    //! Translated name of the map tool
    QString mToolName;
};

#endif // QGSMODELVIEWTOOL_H
