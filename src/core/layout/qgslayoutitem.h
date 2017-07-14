/***************************************************************************
                              qgslayoutitem.h
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTITEM_H
#define QGSLAYOUTITEM_H

#include "qgis_core.h"
#include "qgslayoutobject.h"
#include "qgslayoutsize.h"
#include "qgslayoutpoint.h"
#include "qgsrendercontext.h"
#include <QGraphicsRectItem>

class QgsLayout;
class QPainter;

/**
 * \ingroup core
 * \class QgsLayoutItem
 * \brief Base class for graphical items within a QgsLayout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItem : public QgsLayoutObject, public QGraphicsRectItem
{

    Q_OBJECT

  public:

    //! Fixed position reference point
    enum ReferencePoint
    {
      UpperLeft, //!< Upper left corner of item
      UpperMiddle, //!< Upper center of item
      UpperRight, //!< Upper right corner of item
      MiddleLeft, //!< Middle left of item
      Middle, //!< Center of item
      MiddleRight, //!< Middle right of item
      LowerLeft, //!< Lower left corner of item
      LowerMiddle, //!< Lower center of item
      LowerRight, //!< Lower right corner of item
    };

    /**
     * Constructor for QgsLayoutItem, with the specified parent \a layout.
     */
    explicit QgsLayoutItem( QgsLayout *layout );

    /**
     * Handles preparing a paint surface for the layout item and painting the item's
     * content. Derived classes must not override this method, but instead implement
     * the pure virtual method QgsLayoutItem::draw.
     */
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    /**
     * Sets the reference \a point for positioning of the layout item. This point is also
     * fixed during resizing of the item, and any size changes will be performed
     * so that the position of the reference point within the layout remains unchanged.
     * \see referencePoint()
     */
    void setReferencePoint( const ReferencePoint &point );

    /**
     * Returns the reference point for positioning of the layout item. This point is also
     * fixed during resizing of the item, and any size changes will be performed
     * so that the position of the reference point within the layout remains unchanged.
     * \see setReferencePoint()
     */
    ReferencePoint referencePoint() const { return mReferencePoint; }

    /**
     * Returns the fixed size of the item, if applicable, or an empty size if item can be freely
     * resized.
     * \see setFixedSize()
     * \see minimumSize()
    */
    QgsLayoutSize fixedSize() const { return mFixedSize; }

    /**
     * Returns the minimum allowed size of the item, if applicable, or an empty size if item can be freely
     * resized.
     * \see setMinimumSize()
     * \see fixedSize()
    */
    virtual QgsLayoutSize minimumSize() const { return mMinimumSize; }

    /**
     * Attempts to resize the item to a specified target \a size. Note that the final size of the
     * item may not match the specified target size, as items with a fixed or minimum
     * size will place restrictions on the allowed item size. Data defined item size overrides
     * will also override the specified target size.
     * \see minimumSize()
     * \see fixedSize()
     * \see attemptMove()
     * \see sizeWithUnits()
    */
    virtual void attemptResize( const QgsLayoutSize &size );

    /**
     * Attempts to move the item to a specified \a point. This method respects the item's
     * reference point, in that the item will be moved so that its current reference
     * point is placed at the specified target point.
     * Note that the final position of the item may not match the specified target position,
     * as data defined item position may override the specified value.
     * \see attemptResize()
     * \see referencePoint()
     * \see positionWithUnits()
    */
    virtual void attemptMove( const QgsLayoutPoint &point );

    /**
     * Returns the item's current position, including units. The position returned
     * is the position of the item's reference point, which may not necessarily be the top
     * left corner of the item.
     * \see attemptMove()
     * \see referencePoint()
     * \see sizeWithUnits()
    */
    QgsLayoutPoint positionWithUnits() const { return mItemPosition; }

    /**
     * Returns the item's current size, including units.
     * \see attemptResize()
     * \see positionWithUnits()
     */
    QgsLayoutSize sizeWithUnits() const { return mItemSize; }

    /**
     * Returns the current rotation for the item, in degrees clockwise.
     * \see setItemRotation()
     */
    //TODO
    double itemRotation() const;

  public slots:

    /**
     * Refreshes the item, causing a recalculation of any property overrides and
     * recalculation of its position and size.
     */
    void refresh() override;

    /**
     * Refreshes a data defined \a property for the item by reevaluating the property's value
     * and redrawing the item with this new value. If \a property is set to
     * QgsLayoutObject::AllProperties then all data defined properties for the item will be
     * refreshed.
    */
    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );

    /**
     * Sets the layout item's \a rotation, in degrees clockwise. This rotation occurs around the center of the item.
     * \see itemRotation()
     * \see rotateItem()
    */
    virtual void setItemRotation( const double rotation );

    /**
     * Rotates the item by a specified \a angle in degrees clockwise around a specified reference point.
     * \see setItemRotation()
     * \see itemRotation()
    */
    virtual void rotateItem( const double angle, const QPointF &transformOrigin );

  protected:

    /** Draws a debugging rectangle of the item's current bounds within the specified
     * painter.
     * @param painter destination QPainter
     */
    virtual void drawDebugRect( QPainter *painter );

    /**
     * Draws the item's contents using the specified render \a context.
     * Note that the context's painter has been scaled so that painter units are pixels.
     * Use the QgsRenderContext methods to convert from millimeters or other units to the painter's units.
     */
    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) = 0;

    /**
     * Sets a fixed \a size for the layout item, which prevents it from being freely
     * resized. Set an empty size if item can be freely resized.
     * \see fixedSize()
     * \see setMinimumSize()
    */
    virtual void setFixedSize( const QgsLayoutSize &size );

    /**
     * Sets the minimum allowed \a size for the layout item. Set an empty size if item can be freely
     * resized.
     * \see minimumSize()
     * \see setFixedSize()
    */
    virtual void setMinimumSize( const QgsLayoutSize &size );

    /**
     * Refreshes an item's size by rechecking it against any possible item fixed
     * or minimum sizes.
     * \see setFixedSize()
     * \see setMinimumSize()
     * \see refreshItemPosition()
     */
    void refreshItemSize();

    /**
     * Refreshes an item's position by rechecking it against any possible overrides
     * such as data defined positioning.
     * \see refreshItemSize()
    */
    void refreshItemPosition();

    /**
     * Refreshes an item's rotation by rechecking it against any possible overrides
     * such as data defined rotation.
     * \see refreshItemSize()
     * \see refreshItemPosition()
     */
    void refreshItemRotation();

    /**
     * Adjusts the specified \a point at which a \a reference position of the item
     * sits and returns the top left corner of the item, if reference point where placed at the specified position.
     */
    QPointF adjustPointForReferencePosition( const QPointF &point, const QSizeF &size, const ReferencePoint &reference ) const;

    /**
     * Returns the current position (in layout units) of a \a reference point for the item.
    */
    QPointF positionAtReferencePoint( const ReferencePoint &reference ) const;

  private:

    ReferencePoint mReferencePoint = UpperLeft;
    QgsLayoutSize mFixedSize;
    QgsLayoutSize mMinimumSize;

    QgsLayoutSize mItemSize;
    QgsLayoutPoint mItemPosition;
    double mItemRotation = 0.0;

    QImage mItemCachedImage;
    double mItemCacheDpi = -1;

    void initConnectionsToLayout();

    //! Prepares a painter by setting rendering flags
    void preparePainter( QPainter *painter );
    bool shouldDrawAntialiased() const;
    bool shouldDrawDebugRect() const;

    QSizeF applyMinimumSize( const QSizeF &targetSize );
    QSizeF applyFixedSize( const QSizeF &targetSize );
    QgsLayoutPoint applyDataDefinedPosition( const QgsLayoutPoint &position );
    QgsLayoutSize applyDataDefinedSize( const QgsLayoutSize &size );
    double applyDataDefinedRotation( const double rotation );
    void updateStoredItemPosition();
    QPointF itemPositionAtReferencePoint( const ReferencePoint reference, const QSizeF &size ) const;
    void setScenePos( const QPointF &destinationPos );

    friend class TestQgsLayoutItem;
};

#endif //QGSLAYOUTITEM_H



