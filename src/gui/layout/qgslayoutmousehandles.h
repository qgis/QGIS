/***************************************************************************
                             qgslayoutmousehandles.h
                             -----------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#ifndef QGSLAYOUTMOUSEHANDLES_H
#define QGSLAYOUTMOUSEHANDLES_H

#define SIP_NO_FILE

#include <QGraphicsRectItem>
#include <QObject>
#include <QPointer>
#include <memory>

#include "qgis_gui.h"

class QgsLayout;
class QGraphicsView;
class QgsLayoutView;
class QgsLayoutItem;
class QInputEvent;

///@cond PRIVATE

/**
 * \ingroup gui
 * Handles drawing of selection outlines and mouse handles in a QgsLayoutView
 *
 * Also is responsible for mouse interactions such as resizing and moving selected items.
 *
 * \note not available in Python bindings
 *
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutMouseHandles: public QObject, public QGraphicsRectItem
{
    Q_OBJECT
  public:

    //! Describes the action (move or resize in different directon) to be done during mouse move
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

    QgsLayoutMouseHandles( QgsLayout *layout, QgsLayoutView *view );

    /**
     * Sets the \a layout for the handles.
     * \see layout()
     */
    void setLayout( QgsLayout *layout ) { mLayout = layout; }

    /**
     * Returns the layout for the handles.
     * \see setLayout()
     */
    QgsLayout *layout() { return mLayout; }

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

    //! Finds out which mouse move action to choose depending on the scene cursor position
    QgsLayoutMouseHandles::MouseAction mouseActionForScenePos( QPointF sceneCoordPos );

    //! Returns true is user is currently dragging the handles
    bool isDragging() const { return mIsDragging; }

    //! Returns true is user is currently resizing with the handles
    bool isResizing() const { return mIsResizing; }

    bool shouldBlockEvent( QInputEvent *event ) const;

  protected:

    void mouseMoveEvent( QGraphicsSceneMouseEvent *event ) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) override;
    void mousePressEvent( QGraphicsSceneMouseEvent *event ) override;
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event ) override;
    void hoverMoveEvent( QGraphicsSceneHoverEvent *event ) override;
    void hoverLeaveEvent( QGraphicsSceneHoverEvent *event ) override;

  public slots:

    //! Sets up listeners to sizeChanged signal for all selected items
    void selectionChanged();

    //! Redraws handles when selected item size changes
    void selectedItemSizeChanged();

    //! Redraws handles when selected item rotation changes
    void selectedItemRotationChanged();

  private:

    QgsLayout *mLayout = nullptr;
    QPointer< QgsLayoutView > mView;

    MouseAction mCurrentMouseMoveAction = NoAction;
    //! Start point of the last mouse move action (in scene coordinates)
    QPointF mMouseMoveStartPos;
    //! Position of the last mouse move event (in scene coordinates)
    QPointF mLastMouseEventPos;
    //! Position of the mouse at beginning of move/resize (in scene coordinates)
    QPointF mBeginMouseEventPos;
    //! Position of layout handles at beginning of move/resize (in scene coordinates)
    QPointF mBeginHandlePos;
    //! Width and height of layout handles at beginning of resize
    double mBeginHandleWidth = 0;
    double mBeginHandleHeight = 0;

    QRectF mResizeRect;
    double mResizeMoveX = 0;
    double mResizeMoveY = 0;

    //! True if user is currently dragging items
    bool mIsDragging = false;
    //! True is user is currently resizing items
    bool mIsResizing = false;

    //! Align snap lines
    QGraphicsLineItem *mHorizontalSnapLine = nullptr;
    QGraphicsLineItem *mVerticalSnapLine = nullptr;

    QSizeF mCursorOffset;

    //! Returns the mouse handle bounds of current selection
    QRectF selectionBounds() const;

    //! Returns true if all selected items have same rotation, and if so, updates passed rotation variable
    bool selectionRotation( double &rotation ) const;

    //! Redraws or hides the handles based on the current selection
    void updateHandles();
    //! Draws the handles
    void drawHandles( QPainter *painter, double rectHandlerSize );
    //! Draw outlines for selected items
    void drawSelectedItemBounds( QPainter *painter );

    /**
     * Returns the current (zoom level dependent) tolerance to decide if mouse position is close enough to the
    item border for resizing*/
    double rectHandlerBorderTolerance();

    //! Finds out the appropriate cursor for the current mouse position in the widget (e.g. move in the middle, resize at border)
    Qt::CursorShape cursorForPosition( QPointF itemCoordPos );

    //! Finds out which mouse move action to choose depending on the cursor position inside the widget
    MouseAction mouseActionForPosition( QPointF itemCoordPos );

    //! Handles dragging of items during mouse move
    void dragMouseMove( QPointF currentPosition, bool lockMovement, bool preventSnap );

    //! Calculates the distance of the mouse cursor from thed edge of the mouse handles
    QSizeF calcCursorEdgeOffset( QPointF cursorPos );

    //! Handles resizing of items during mouse move
    void resizeMouseMove( QPointF currentPosition, bool lockAspect, bool fromCenter );

    //sets the mouse cursor for the QGraphicsView attached to the composition (workaround qt bug #3732)
    void setViewportCursor( Qt::CursorShape cursor );

    //resets the layout designer status bar to the default message
    void resetStatusBar();

    //! Snaps an item or point (depending on mode) originating at originalPoint to the grid or align rulers
    QPointF snapPoint( QPointF originalPoint, SnapGuideMode mode, bool snapHorizontal = true, bool snapVertical = true );

    void hideAlignItems();

    //! Collects all items from a list of \a items, exploring for any group members and adding them too
    void collectItems( QList< QgsLayoutItem * > items, QList< QgsLayoutItem * > &collected );

};

///@endcond PRIVATE

#endif // QGSLAYOUTMOUSEHANDLES_H
