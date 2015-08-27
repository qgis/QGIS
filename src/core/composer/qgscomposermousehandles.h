/***************************************************************************
                         qgscomposermousehandles.h
                             -------------------
    begin                : September 2013
    copyright            : (C) 2013 by Nyall Dawson, Radim Blazek
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
#ifndef QGSCOMPOSERMOUSEHANDLES_H
#define QGSCOMPOSERMOUSEHANDLES_H

#include <QGraphicsRectItem>
#include <QObject>

class QgsComposition;
class QgsComposerItem;
class QGraphicsView;

/** \ingroup MapComposer
 * Handles drawing of selection outlines and mouse handles. Responsible for mouse
 * interactions such as resizing and moving selected items.
 * */
class CORE_EXPORT QgsComposerMouseHandles: public QObject, public QGraphicsRectItem
{
    Q_OBJECT
  public:

    /** Describes the action (move or resize in different directon) to be done during mouse move*/
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

    QgsComposerMouseHandles( QgsComposition *composition );
    virtual ~QgsComposerMouseHandles();

    void setComposition( QgsComposition* c ) { mComposition = c; }
    QgsComposition* composition() { return mComposition; }

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget ) override;

    /** Finds out which mouse move action to choose depending on the scene cursor position*/
    QgsComposerMouseHandles::MouseAction mouseActionForScenePos( const QPointF& sceneCoordPos );

    /** Returns true is user is currently dragging the handles */
    bool isDragging() { return mIsDragging; }

    /** Returns true is user is currently resizing with the handles */
    bool isResizing() { return mIsResizing; }

  protected:

    void mouseMoveEvent( QGraphicsSceneMouseEvent* event ) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent* event ) override;
    void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event ) override;
    void hoverMoveEvent( QGraphicsSceneHoverEvent * event ) override;
    void hoverLeaveEvent( QGraphicsSceneHoverEvent * event ) override;

  public slots:

    /** Sets up listeners to sizeChanged signal for all selected items*/
    void selectionChanged();

    /** Redraws handles when selected item size changes*/
    void selectedItemSizeChanged();

    /** Redraws handles when selected item rotation changes*/
    void selectedItemRotationChanged();

  private:

    QgsComposition* mComposition; //reference to composition
    QGraphicsView* mGraphicsView; //reference to QGraphicsView

    QgsComposerMouseHandles::MouseAction mCurrentMouseMoveAction;
    /** Start point of the last mouse move action (in scene coordinates)*/
    QPointF mMouseMoveStartPos;
    /** Position of the last mouse move event (in scene coordinates)*/
    QPointF mLastMouseEventPos;
    /** Position of the mouse at beginning of move/resize (in scene coordinates)*/
    QPointF mBeginMouseEventPos;
    /** Position of composer handles at beginning of move/resize (in scene coordinates)*/
    QPointF mBeginHandlePos;
    /** Width and height of composer handles at beginning of resize*/
    double mBeginHandleWidth;
    double mBeginHandleHeight;

    QRectF mResizeRect;
    double mResizeMoveX;
    double mResizeMoveY;

    /** True if user is currently dragging items*/
    bool mIsDragging;
    /** True is user is currently resizing items*/
    bool mIsResizing;

    /** Align snap lines*/
    QGraphicsLineItem* mHAlignSnapItem;
    QGraphicsLineItem* mVAlignSnapItem;

    QSizeF mCursorOffset;

    /** Returns the mouse handle bounds of current selection*/
    QRectF selectionBounds() const;

    /** Returns true if all selected items have same rotation, and if so, updates passed rotation variable*/
    bool selectionRotation( double & rotation ) const;

    /** Redraws or hides the handles based on the current selection*/
    void updateHandles();
    /** Draws the handles*/
    void drawHandles( QPainter* painter, double rectHandlerSize );
    /** Draw outlines for selected items*/
    void drawSelectedItemBounds( QPainter* painter );

    /** Returns the current (zoom level dependent) tolerance to decide if mouse position is close enough to the
    item border for resizing*/
    double rectHandlerBorderTolerance();

    /** Finds out the appropriate cursor for the current mouse position in the widget (e.g. move in the middle, resize at border)*/
    Qt::CursorShape cursorForPosition( const QPointF& itemCoordPos );

    /** Finds out which mouse move action to choose depending on the cursor position inside the widget*/
    QgsComposerMouseHandles::MouseAction mouseActionForPosition( const QPointF& itemCoordPos );

    /** Handles dragging of items during mouse move*/
    void dragMouseMove( const QPointF& currentPosition, bool lockMovement, bool preventSnap );

    /** Calculates the distance of the mouse cursor from thed edge of the mouse handles*/
    QSizeF calcCursorEdgeOffset( const QPointF &cursorPos );

    /** Handles resizing of items during mouse move*/
    void resizeMouseMove( const QPointF& currentPosition, bool lockAspect, bool fromCenter );

    /** Return horizontal align snap item. Creates a new graphics line if 0*/
    QGraphicsLineItem* hAlignSnapItem();
    void deleteHAlignSnapItem();
    /** Return vertical align snap item. Creates a new graphics line if 0*/
    QGraphicsLineItem* vAlignSnapItem();
    void deleteVAlignSnapItem();
    void deleteAlignItems();

    /** Snaps an item or point (depending on mode) originating at originalPoint to the grid or align rulers*/
    QPointF snapPoint( const QPointF& originalPoint, QgsComposerMouseHandles::SnapGuideMode mode );
    /** Snaps an item originating at unalignedX, unalignedY to the grid or align rulers*/
    QPointF alignItem( double& alignX, double& alignY, double unalignedX, double unalignedY );
    /** Snaps a point to to the grid or align rulers*/
    QPointF alignPos( const QPointF& pos, double& alignX, double& alignY );

    //helper functions for item align
    void collectAlignCoordinates( QMap< double, const QgsComposerItem* >& alignCoordsX, QMap< double, const QgsComposerItem* >& alignCoordsY );
    bool nearestItem( const QMap< double, const QgsComposerItem* >& coords, double value, double& nearestValue ) const;
    void checkNearestItem( double checkCoord, const QMap< double, const QgsComposerItem* >& alignCoords, double& smallestDiff, double itemCoordOffset, double& itemCoord, double& alignCoord );

    //tries to return the current QGraphicsView attached to the composition
    QGraphicsView* graphicsView();

    //sets the mouse cursor for the QGraphicsView attached to the composition (workaround qt bug #3732)
    void setViewportCursor( Qt::CursorShape cursor );

    //resets the composer window status bar to the default message
    void resetStatusBar();
};

#endif // QGSCOMPOSERMOUSEHANDLES_H
