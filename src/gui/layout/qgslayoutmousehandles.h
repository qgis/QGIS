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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgsgraphicsviewmousehandles.h"
#include <QPointer>
#include <memory>

#include "qgis_gui.h"

class QgsLayout;
class QGraphicsView;
class QgsLayoutView;
class QgsLayoutItem;
class QInputEvent;
class QgsAbstractLayoutUndoCommand;

///@cond PRIVATE

/**
 * \ingroup gui
 * \brief Handles drawing of selection outlines and mouse handles in a QgsLayoutView
 *
 * Also is responsible for mouse interactions such as resizing and moving selected items.
 *
 * \note not available in Python bindings
 *
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutMouseHandles: public QgsGraphicsViewMouseHandles
{
    Q_OBJECT
  public:

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

  protected:

    void setViewportCursor( Qt::CursorShape cursor ) override;
    QList<QGraphicsItem *> sceneItemsAtPoint( QPointF scenePoint ) override;
    QList<QGraphicsItem *> selectedSceneItems( bool includeLockedItems = true ) const override;
    bool itemIsLocked( QGraphicsItem *item ) override;
    bool itemIsGroupMember( QGraphicsItem *item ) override;
    QRectF itemRect( QGraphicsItem *item ) const override;
    void expandItemList( const QList< QGraphicsItem * > &items, QList< QGraphicsItem * > &collected ) const override;
    void expandItemList( const QList< QgsLayoutItem * > &items, QList< QGraphicsItem * > &collected ) const;
    void moveItem( QGraphicsItem *item, double deltaX, double deltaY ) override;
    void setItemRect( QGraphicsItem *item, QRectF rect ) override;
    void showStatusMessage( const QString &message ) override;
    void hideAlignItems() override;
    QPointF snapPoint( QPointF originalPoint, SnapGuideMode mode, bool snapHorizontal = true, bool snapVertical = true ) override;
    void createItemCommand( QGraphicsItem *item ) override;
    void endItemCommand( QGraphicsItem *item ) override;
    void startMacroCommand( const QString &text ) override;
    void endMacroCommand() override;
    void drawMovePreview( QPainter *painter ) override;
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) override;
  public slots:

    //! Sets up listeners to sizeChanged signal for all selected items
    void selectionChanged();

  private:

    QgsLayout *mLayout = nullptr;
    QPointer< QgsLayoutView > mView;

    //! Align snap lines
    QGraphicsLineItem *mHorizontalSnapLine = nullptr;
    QGraphicsLineItem *mVerticalSnapLine = nullptr;

    std::unique_ptr< QgsAbstractLayoutUndoCommand > mItemCommand;

    QImage mItemCachedImage;
};

///@endcond PRIVATE

#endif // QGSLAYOUTMOUSEHANDLES_H
