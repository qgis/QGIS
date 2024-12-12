/***************************************************************************
                             qgsmodelgraphicitem.h
                             ----------------------------------
    Date                 : February 2020
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

#ifndef QGSMODELGRAPHICITEM_H
#define QGSMODELGRAPHICITEM_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QGraphicsObject>
#include <QPicture>

class QgsModelGraphicsView;
class QgsModelViewMouseEvent;

///@cond NOT_STABLE


/**
 * \ingroup gui
 * \brief A flat button graphic item for use in the Processing model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelDesignerFlatButtonGraphicItem : public QGraphicsObject
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsModelDesignerFlatButtonGraphicItem, with the specified \a parent item.
     *
     * The \a picture argument specifies a QPicture object containing the graphic to render
     * for the button. The button will be rendered at the specified \a position and \a size.
     */
    QgsModelDesignerFlatButtonGraphicItem( QGraphicsItem *parent SIP_TRANSFERTHIS, const QPicture &picture, const QPointF &position, const QSizeF &size = QSizeF( 16, 16 ) );

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;
    QRectF boundingRect() const override;
    void hoverEnterEvent( QGraphicsSceneHoverEvent *event ) override;
    void hoverLeaveEvent( QGraphicsSceneHoverEvent *event ) override;
    void mousePressEvent( QGraphicsSceneMouseEvent *event ) override;

#ifndef SIP_RUN

    /**
     * Handles a model hover enter \a event.
     */
    virtual void modelHoverEnterEvent( QgsModelViewMouseEvent *event );

    /**
     * Handles a model hover leave \a event.
     */
    virtual void modelHoverLeaveEvent( QgsModelViewMouseEvent *event );

    /**
     * Handles a model mouse press \a event.
     */
    virtual void modelPressEvent( QgsModelViewMouseEvent *event );
#endif

    /**
     * Sets the button's \a position.
     */
    void setPosition( const QPointF &position );

    /**
     * Returns the associated model view.
     */
    QgsModelGraphicsView *view();

  signals:

    /**
     * Emitted when the button is clicked.
     */
    void clicked();

  protected:
    /**
     * Sets the \a picture to render for the button graphics.
     */
    void setPicture( const QPicture &picture );

  private:
    QPicture mPicture;
    QPointF mPosition;
    QSizeF mSize;
    bool mHoverState = false;
};


/**
 * \ingroup gui
 * \brief A button allowing folding or expanding component graphics in the Processing model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelDesignerFoldButtonGraphicItem : public QgsModelDesignerFlatButtonGraphicItem
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsModelDesignerFoldButtonGraphicItem, with the specified \a parent item.
     *
     * The \a folded argument specifies whether the button should initially indicate the folded (collapsed)
     * state.
     *
     * The button will be rendered at the specified \a position and \a size.
     */
    QgsModelDesignerFoldButtonGraphicItem( QGraphicsItem *parent SIP_TRANSFERTHIS, bool folded, const QPointF &position, const QSizeF &size = QSizeF( 11, 11 ) );

    void mousePressEvent( QGraphicsSceneMouseEvent *event ) override;
#ifndef SIP_RUN
    void modelPressEvent( QgsModelViewMouseEvent *event ) override;
#endif

  signals:

    /**
     * Emitted when the button \a folded state changes.
     *
     * If \a folded is TRUE, the button represents the collapsed state for the item.
     */
    void folded( bool folded );

  private:
    QPicture mPlusPicture;
    QPicture mMinusPicture;
    bool mFolded = false;
};

///@endcond

#endif // QGSMODELGRAPHICITEM_H
