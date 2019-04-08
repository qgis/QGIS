/***************************************************************************
                             qgslayoutitemmapitem.h
                             -------------------
    begin                : October 2017
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
#ifndef QGSLAYOUTITEMMAPITEM_H
#define QGSLAYOUTITEMMAPITEM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayoutobject.h"
#include "qgsmaplayerref.h"

class QgsLayoutItemMap;

/**
 * \ingroup core
 * \class QgsLayoutItemMapItem
 * \brief An item which is drawn inside a QgsLayoutItemMap, e.g., a grid or map overview.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMapItem : public QgsLayoutObject
{
    Q_OBJECT

  public:

    //! Item stacking position, specifies where the in the map's stack the item should be rendered
    enum StackingPosition
    {
      StackBelowMap, //!< Render below all map layers
      StackBelowMapLayer, //!< Render below a specific map layer (see stackingLayer())
      StackAboveMapLayer, //!< Render above a specific map layer (see stackingLayer())
      StackBelowMapLabels, //!< Render above all map layers, but below map labels
      StackAboveMapLabels, //!< Render above all map layers and labels
    };

    /**
     * Constructor for QgsLayoutItemMapItem, attached to the specified \a map.
     *
     * The \a name argument gives a friendly display name for the item.
     */
    QgsLayoutItemMapItem( const QString &name, QgsLayoutItemMap *map );

    /**
     * Draws the item on to a destination \a painter.
     */
    virtual void draw( QPainter *painter ) = 0;

    /**
     * Stores map item state in a DOM element, where \a element is the DOM element
     * corresponding to a 'LayoutMap' tag.
     * \see readXml()
     */
    virtual bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets the map item state from a DOM document, where \a element is the DOM
     * node corresponding to a 'LayoutMapGrid' tag.
     * \see writeXml()
     */
    virtual bool readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context );

    /**
     * Called after all pending items have been restored from XML. Map items can use
     * this method to run steps which must take place after all items have been restored to the layout,
     * such as connecting to signals emitted by other items, which may not have existed in the layout
     * at the time readXml() was called. E.g. an overview can use this to connect to its linked
     * map item after restoration from XML.
     * \see readXml()
     */
    virtual void finalizeRestoreFromXml();

    /**
     * Sets the corresponding layout \a map for the item.
     * \see map()
     */
    void setMap( QgsLayoutItemMap *map );

    /**
     * Returns the layout item map for the item.
     * \see setMap()
     */
    const QgsLayoutItemMap *map() const;

    /**
     * Returns the unique id for the map item.
     */
    QString id() const { return mUuid; }

    /**
     * Sets the friendly display \a name for the item.
     * \see name()
     */
    void setName( const QString &name );

    /**
     * Returns the friendly display name for the item.
     * \see setName()
     */
    QString name() const;

    /**
     * Controls whether the item will be drawn. Set \a enabled to TRUE to enable drawing of the item.
     * \see enabled()
     */
    void setEnabled( bool enabled );

    /**
     * Returns whether the item will be drawn.
     * \see setEnabled()
     */
    bool enabled() const;

    /**
     * Returns TRUE if the item is drawn using advanced effects, such as blend modes.
     */
    virtual bool usesAdvancedEffects() const;

    /**
     * Returns the item's stacking position, which specifies where the in the map's
     * stack the item should be rendered.
     *
     * \see setStackingPosition()
     * \see stackingLayer()
     *
     * \since QGIS 3.6
     */
    StackingPosition stackingPosition() const { return mStackingPosition; }

    /**
     * Sets the item's stacking \a position, which specifies where the in the map's
     * stack the item should be rendered.
     *
     * \see stackingPosition()
     * \see setStackingLayer()
     *
     * \since QGIS 3.6
     */
    void setStackingPosition( StackingPosition position ) { mStackingPosition = position; }

    /**
     * Returns the item's stacking layer, which specifies where the in the map's
     * stack the item should be rendered.
     *
     * This setting is only used when stackingPosition() is StackBelowMapLayer or
     * StackAboveMapLayer.
     *
     * \see setStackingLayer()
     * \see stackingPosition()
     *
     * \since QGIS 3.6
     */
    QgsMapLayer *stackingLayer() const;

    /**
     * Sets the item's stacking \a layer, which specifies where the in the map's
     * stack the item should be rendered.
     *
     * This setting is only used when stackingPosition() is StackBelowMapLayer or
     * StackAboveMapLayer.
     *
     * \see stackingLayer()
     * \see setStackingPosition
     *
     * \since QGIS 3.6
     */
    void setStackingLayer( QgsMapLayer *layer );

  protected:

    //! Friendly display name
    QString mName;

    //! Associated map
    QgsLayoutItemMap *mMap = nullptr;

    //! Unique id
    QString mUuid;

    //! True if item is to be displayed on map
    bool mEnabled;

    StackingPosition mStackingPosition = StackBelowMapLabels;

    QgsMapLayerRef mStackingLayer;

};

/**
 * \ingroup core
 * \class QgsLayoutItemMapItemStack
 * \brief A collection of map items which are drawn above the map content in a
 * QgsLayoutItemMap. The item stack controls which items are drawn and the
 * order they are drawn in.
 * \see QgsLayoutItemMapItem
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMapItemStack
{
  public:

    /**
     * Constructor for QgsLayoutItemMapItemStack, attached to the
     * specified \a map.
     */
    QgsLayoutItemMapItemStack( QgsLayoutItemMap *map );

    virtual ~QgsLayoutItemMapItemStack();

    /**
     * Returns the number of items in the stack.
     */
    int size() const { return mItems.size(); }

    /**
     * Stores the state of the item stack in a DOM node, where \a element is the DOM element corresponding to a 'LayoutMap' tag.
     * Returns TRUE if write was successful.
     * \see readXml()
     */
    virtual bool writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Sets the item stack's state from a DOM document, where \a element is a DOM node corresponding to a 'LayoutMap' tag.
     * Returns TRUE if read was successful.
     * \see writeXml()
     */
    virtual bool readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context ) = 0;

    /**
     * Called after all pending items have been restored from XML. Map item stacks can use
     * this method to run steps which must take place after all items have been restored to the layout,
     * such as connecting to signals emitted by other items, which may not have existed in the layout
     * at the time readXml() was called. E.g. an overview can use this to connect to its linked
     * map item after restoration from XML.
     * \see readXml()
     */
    virtual void finalizeRestoreFromXml();

    /**
     * Draws the items from the stack on a specified \a painter.
     *
     * If \a ignoreStacking is TRUE, then all items will be drawn, regardless
     * of their actual stacking position settings. If it is FALSE, only items
     * which are set to stack above the map item will be drawn.
     */
    void drawItems( QPainter *painter, bool ignoreStacking = true );

    /**
     * Returns whether any items within the stack contain advanced effects,
     * such as blending modes.
     */
    bool containsAdvancedEffects() const;

  protected:

    /**
     * Adds a new map item to the stack and takes ownership of the item.
     * The item will be added to the end of the stack, and rendered
     * above any existing map items already present in the stack.
     * \note After adding an item to the stack update()
     * should be called for the QgsLayoutItemMap to prevent rendering artifacts.
     * \see removeItem()
     */
    void addItem( QgsLayoutItemMapItem *item SIP_TRANSFER );

    /**
     * Removes an item which matching \a itemId from the stack and deletes the corresponding QgsLayoutItemMapItem
     * \note After removing an item from the stack, update()
     * should be called for the QgsLayoutItemMap to prevent rendering artifacts.
     * \see addItem()
     */
    void removeItem( const QString &itemId );

    /**
     * Moves an item which matching \a itemId up the stack, causing it to be rendered above other items.
     * \note After moving an item within the stack, update() should be
     * called for the QgsLayoutItemMap to redraw the map with the new item stack order.
     * \see moveItemDown()
     */
    void moveItemUp( const QString &itemId );

    /**
     * Moves an item which matching \a itemId up the stack, causing it to be rendered above other items.
     * \note After moving an item within the stack, update() should be
     * called for the QgsLayoutItemMap to redraw the map with the new item stack order.
     * \see moveItemUp()
     */
    void moveItemDown( const QString &itemId );

    /**
     * Returns a reference to an item which matching \a itemId within the stack.
     */
    QgsLayoutItemMapItem *item( const QString &itemId ) const;

    /**
     * Returns a reference to the item at the specified \a index within the stack.
     */
    QgsLayoutItemMapItem *item( int index ) const;

    /**
     * Returns a reference to an item at the specified \a index within the stack.
     * \see item()
     * \note not available in Python bindings
     */
    QgsLayoutItemMapItem &operator[]( int index ) SIP_SKIP;

    /**
     * Returns a list of QgsLayoutItemMapItems contained by the stack.
     */
    QList< QgsLayoutItemMapItem * > asList() const;

  protected:

    QList< QgsLayoutItemMapItem * > mItems;

    QgsLayoutItemMap *mMap = nullptr;

    /**
     * Clears the item stack and deletes all QgsLayoutItemMapItems contained
     * by the stack
     */
    void removeItems();
};

#endif //QGSLAYOUTITEMMAPITEM_H

