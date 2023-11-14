/***************************************************************************
                             qgsgraphicsviewmousehandles.h
                             -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRAPHICSVIEWMOUSEHANDLES_H
#define QGSGRAPHICSVIEWMOUSEHANDLES_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QGraphicsRectItem>
#include <QObject>
#include <QPointer>
#include <memory>

#include "qgis_gui.h"

class QGraphicsView;
class QInputEvent;

///@cond PRIVATE

/**
 * \ingroup gui
 * \brief Base class for an item handling drawing of selection outlines and mouse handles in a QGraphicsView
 *
 * Also is responsible for mouse interactions such as resizing and moving selected items.
 *
 * \note not available in Python bindings
 *
 * \since QGIS 3.14
*/
class GUI_EXPORT QgsGraphicsViewMouseHandles: public QObject, public QGraphicsRectItem
{
    Q_OBJECT
  public:

    //! Describes the action (move or resize in different direction) to be done during mouse move
    enum MouseAction
    {
      MoveItem,
      ResizeUp,
      ResizeDown,
      ResizeLeft,
      ResizeRight,
      ResizeLeftUp,
      ResizeRightUp,
      ResizeLeftDown,
      ResizeRightDown,
      SelectItem,
      NoAction
    };

    enum ItemPositionMode
    {
      UpperLeft,
      UpperMiddle,
      UpperRight,
      MiddleLeft,
      Middle,
      MiddleRight,
      LowerLeft,
      LowerMiddle,
      LowerRight
    };

    enum SnapGuideMode
    {
      Item,
      Point
    };

    QgsGraphicsViewMouseHandles( QGraphicsView *view );

    //! Finds out which mouse move action to choose depending on the scene cursor position
    QgsGraphicsViewMouseHandles::MouseAction mouseActionForScenePos( QPointF sceneCoordPos );

    //! Returns TRUE is user is currently dragging the handles
    bool isDragging() const { return mIsDragging; }

    //! Returns TRUE is user is currently resizing with the handles
    bool isResizing() const { return mIsResizing; }

    bool shouldBlockEvent( QInputEvent *event ) const;

    //! Initializes a drag operation \since QGIS 3.34
    void startMove( QPointF sceneCoordPos );

  public slots:

    //! Redraws handles when selected item size changes
    void selectedItemSizeChanged();

    //! Redraws handles when selected item rotation changes
    void selectedItemRotationChanged();

  protected:

    void paintInternal( QPainter *painter, bool showHandles, bool showStaticBoundingBoxes,
                        bool showTemporaryBoundingBoxes, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr );

    //! Sets the mouse cursor for the QGraphicsView attached to the composition
    virtual void setViewportCursor( Qt::CursorShape cursor ) = 0;

    virtual QList<QGraphicsItem *> sceneItemsAtPoint( QPointF scenePoint ) = 0;
    virtual QList<QGraphicsItem *> selectedSceneItems( bool includeLockedItems = true ) const = 0;
    virtual bool itemIsLocked( QGraphicsItem *item ) { Q_UNUSED( item ); return false; }
    virtual bool itemIsGroupMember( QGraphicsItem *item ) { Q_UNUSED( item ); return false; }
    virtual QRectF itemRect( QGraphicsItem *item ) const = 0;
    virtual QRectF storedItemRect( QGraphicsItem *item ) const;
    virtual void moveItem( QGraphicsItem *item, double deltaX, double deltaY ) = 0;
    virtual void previewItemMove( QGraphicsItem *item, double deltaX, double deltaY );
    virtual void setItemRect( QGraphicsItem *item, QRectF rect ) = 0;

    /**
     * Called when a resize or move action is in progress and the effects can be previewed for the specified \a item. The
     * \a rect argument gives the new "transient" rectangular bounds of \a item (in item coordinates).
     *
     * If implemented, the method should return the item's calculated desired rect given the specified \a rect. This allows
     * an item to override the rect results, e.g. by applying a minimum size constraint. The returned value
     * should be in the item's coordinates.
     */
    virtual QRectF previewSetItemRect( QGraphicsItem *item, QRectF rect );

    virtual void startMacroCommand( const QString &text );
    virtual void endMacroCommand();
    virtual void createItemCommand( QGraphicsItem *item );
    virtual void endItemCommand( QGraphicsItem *item );
    virtual void showStatusMessage( const QString &message ) { Q_UNUSED( message ) }
    virtual void hideAlignItems() {}
    //! Snaps an item or point (depending on mode) originating at originalPoint to the grid or align rulers
    virtual QPointF snapPoint( QPointF originalPoint, SnapGuideMode mode, bool snapHorizontal = true, bool snapVertical = true );

    //! Collects all items from a list of \a items, exploring for any group members and adding them too
    virtual void expandItemList( const QList< QGraphicsItem * > &items, QList< QGraphicsItem * > &collected ) const;

    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) override;
    void hoverMoveEvent( QGraphicsSceneHoverEvent *event ) override;
    void hoverLeaveEvent( QGraphicsSceneHoverEvent *event ) override;
    void mousePressEvent( QGraphicsSceneMouseEvent *event ) override;
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event ) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) override;

    //resets the layout designer status bar to the default message
    void resetStatusBar();

    //! Returns TRUE if all selected items have same rotation, and if so, updates passed rotation variable
    bool selectionRotation( double &rotation ) const;

    //! Redraws or hides the handles based on the current selection
    void updateHandles();

    //! Handles dragging of items during mouse move
    void dragMouseMove( QPointF currentPosition, bool lockMovement, bool preventSnap );

    //! Handles resizing of items during mouse move
    void resizeMouseMove( QPointF currentPosition, bool lockAspect, bool fromCenter );

    void setHandleSize( double size );

    //! Finds out which mouse move action to choose depending on the cursor position inside the widget
    MouseAction mouseActionForPosition( QPointF itemCoordPos );

    //! Calculates the distance of the mouse cursor from thed edge of the mouse handles
    QSizeF calcCursorEdgeOffset( QPointF cursorPos );

    //! Returns the mouse handle bounds of current selection
    QRectF selectionBounds() const;

    //! Returns whether items can be copied by Alt+dragging
    virtual bool copyDragEnabled() const { return false; }

    virtual void copyDrag() {}

    /**
     * Resizes a QRectF relative to a resized bounding rectangle.
     * \param rectToResize QRectF to resize, contained within boundsBefore. The
     * rectangle is linearly scaled to retain its relative position and size within
     * boundsAfter.
     * \param boundsBefore QRectF of bounds before resize
     * \param boundsAfter QRectF of bounds after resize
     */
    static void relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter );

    /**
     * Returns a scaled position given a before and after range
     * \param position initial position within before range to scale
     * \param beforeMin minimum value in before range
     * \param beforeMax maximum value in before range
     * \param afterMin minimum value in after range
     * \param afterMax maximum value in after range
     * \returns position scaled to range specified by afterMin and afterMax
     */
    static double relativePosition( double position, double beforeMin, double beforeMax, double afterMin, double afterMax );

  private:

    QGraphicsView *mView = nullptr;

    double mHandleSize = 10;

    QSizeF mCursorOffset;
    double mResizeMoveX = 0;
    double mResizeMoveY = 0;

    //! Width and height of layout handles at beginning of resize
    double mBeginHandleWidth = 0;
    double mBeginHandleHeight = 0;

    QRectF mResizeRect;

    //! Start point of the last mouse move action (in scene coordinates)
    QPointF mMouseMoveStartPos;

    MouseAction mCurrentMouseMoveAction = NoAction;

    //! True if user is currently dragging items
    bool mIsDragging = false;
    //! True is user is currently resizing items
    bool mIsResizing = false;

    //! Position of the mouse at beginning of move/resize (in scene coordinates)
    QPointF mBeginMouseEventPos;

    //! Position of layout handles at beginning of move/resize (in scene coordinates)
    QPointF mBeginHandlePos;

    //! Draws the handles
    void drawHandles( QPainter *painter, double rectHandlerSize );

    //! Draw outlines for selected items
    void drawSelectedItemBounds( QPainter *painter );

    /**
     * Returns the current (zoom level dependent) tolerance to decide if mouse position is close enough to the
     * item border for resizing.
    */
    double rectHandlerBorderTolerance();

    //! Finds out the appropriate cursor for the current mouse position in the widget (e.g. move in the middle, resize at border)
    Qt::CursorShape cursorForPosition( QPointF itemCoordPos );



};

///@endcond PRIVATE

#endif // QGSLAYOUTMOUSEHANDLES_H
