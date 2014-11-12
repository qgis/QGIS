/***************************************************************************
                         qgscomposermapitem.h
                             -------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
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
#ifndef QGSCOMPOSERMAPITEM_H
#define QGSCOMPOSERMAPITEM_H

#include "qgscomposerobject.h"

class QgsComposerMap;

/** \ingroup MapComposer
 *  \class QgsComposerMapItem
 *  \brief An item which is drawn inside a QgsComposerMap, eg a grid or map overview.
 */

class CORE_EXPORT QgsComposerMapItem : public QgsComposerObject
{
    Q_OBJECT

  public:

    /**Constructor for QgsComposerMapItem.
     * @param name friendly display name for item
     * @param map QgsComposerMap the item is attached to
    */
    QgsComposerMapItem( const QString& name, QgsComposerMap* map );

    virtual ~QgsComposerMapItem();

    /**Draws the item on to a painter
     * @param painter destination QPainter
     */
    virtual void draw( QPainter* painter ) = 0;

    /**Stores map item state in DOM element
     * @param elem is DOM element corresponding to a 'ComposerMap' tag
     * @param doc DOM document
     * @see readXML
    */
    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /**Sets map item state from a DOM document
     * @param itemElem is DOM node corresponding to a 'ComposerMapGrid' tag
     * @param doc is DOM document
     * @see writeXML
    */
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Sets composer map for the item
     * @param map composer map
     * @see composerMap
    */
    virtual void setComposerMap( QgsComposerMap* map );

    /**Get composer map for the item
     * @returns composer map
     * @see setComposerMap
    */
    virtual const QgsComposerMap* composerMap() const { return mComposerMap; }

    /**Get the unique id for the map item
     * @returns unique id
    */
    QString id() const { return mUuid; }

    /**Sets the friendly display name for the item
     * @param name display name
     * @see name
    */
    virtual void setName( const QString& name ) { mName = name; }

    /**Get friendly display name for the item
     * @returns display name
     * @see setName
    */
    virtual QString name() const { return mName; }

    /**Controls whether the item will be drawn
     * @param enabled set to true to enable drawing of the item
     * @see enabled
    */
    virtual void setEnabled( const bool enabled ) { mEnabled = enabled; }

    /**Returns whether the item will be drawn
     * @returns true if item will be drawn on the map
     * @see setEnabled
    */
    virtual bool enabled() const { return mEnabled; }

    /**Returns true if the item is drawn using advanced effects, such as blend modes.
     * @returns true if item uses advanced effects
    */
    virtual bool usesAdvancedEffects() const { return false; }

  protected:

    /**Friendly display name*/
    QString mName;

    /**Associated composer map*/
    QgsComposerMap* mComposerMap;

    /**Unique id*/
    QString mUuid;

    /**True if item is to be displayed on map*/
    bool mEnabled;

};



/**\ingroup MapComposer
 * \class QgsComposerMapItemStack
 * \brief A collection of map items which are drawn above the map content in a
 * QgsComposerMap. The item stack controls which items are drawn and the
 * order they are drawn in.
 * \note added in QGIS 2.5
 * \see QgsComposerMapItem
 */
class CORE_EXPORT QgsComposerMapItemStack
{
  public:

    /**Constructor for QgsComposerMapItemStack.
     * @param map QgsComposerMap the item stack is attached to
    */
    QgsComposerMapItemStack( QgsComposerMap* map );

    virtual ~QgsComposerMapItemStack();

    /**Returns the number of items in the stack
     * @returns number of items in the stack
    */
    int size() const { return mItems.size(); }

    /**Stores the state of the item stack in a DOM node
     * @param elem is DOM element corresponding to a 'ComposerMap' tag
     * @param doc DOM document
     * @returns true if write was successful
     * @see readXML
     */
    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /**Sets the item stack's state from a DOM document
     * @param elem is DOM node corresponding to 'a ComposerMap' tag
     * @param doc DOM document
     * @returns true if read was successful
     * @see writeXML
     */
    virtual bool readXML( const QDomElement& elem, const QDomDocument& doc ) = 0;

    /**Draws the items from the stack on a specified painter
     * @param painter destination QPainter
     */
    void drawItems( QPainter* painter );

    /**Returns whether any items within the stack contain advanced effects,
     * such as blending modes
     * @returns true if item stack contains advanced effects
     */
    bool containsAdvancedEffects() const;

  protected:

    /**Adds a new map item to the stack and takes ownership of the item.
     * The item will be added to the end of the stack, and rendered
     * above any existing map items already present in the stack.
     * @param item QgsComposerMapItem to add to the stack
     * @note after adding an item to the stack update()
     * should be called for the QgsComposerMap to prevent rendering artifacts
     * @see removeItem
    */
    void addItem( QgsComposerMapItem* item );

    /**Removes an item from the stack and deletes the corresponding QgsComposerMapItem
     * @param itemId id for the QgsComposerMapItem to remove
     * @note after removing an item from the stack, update()
     * should be called for the QgsComposerMap to prevent rendering artifacts
     * @see addItem
    */
    void removeItem( const QString& itemId );

    /**Moves an item up the stack, causing it to be rendered above other items
     * @param itemId id for the QgsComposerMapItem to move up
     * @note after moving an item within the stack, update() should be
     * called for the QgsComposerMap to redraw the map with the new item stack order
     * @see moveItemDown
    */
    void moveItemUp( const QString& itemId );

    /**Moves an item up the stack, causing it to be rendered above other items
     * @param itemId id for the QgsComposerMapItem to move down
     * @note after moving an item within the stack, update() should be
     * called for the QgsComposerMap to redraw the map with the new item stack order
     * @see moveItemUp
    */
    void moveItemDown( const QString& itemId );

    /**Returns a const reference to an item within the stack
     * @param itemId id for the QgsComposerMapItem to find
     * @returns const reference to item, if found
     * @see item
    */
    const QgsComposerMapItem* constItem( const QString& itemId ) const;

    /**Returns a reference to an item within the stack
     * @param itemId id for the QgsComposerMapItem to find
     * @returns reference to item if found
     * @see constItem
    */
    QgsComposerMapItem* item( const QString& itemId ) const;

    /**Returns a reference to an item within the stack
     * @param index item position in the stack
     * @returns reference to item if found
     * @see constItem
    */
    QgsComposerMapItem* item( const int index ) const;

    /**Returns a reference to an item within the stack
     * @param idx item position in the stack
     * @returns reference to item if found
     * @see constItem
     * @see item
     * @note not available in python bindings
    */
    QgsComposerMapItem &operator[]( int idx );

    /**Returns a list of QgsComposerMapItems contained by the stack
     * @returns list of items
    */
    QList< QgsComposerMapItem* > asList() const;

  protected:

    QList< QgsComposerMapItem* > mItems;

    QgsComposerMap* mComposerMap;

    /**Clears the item stack and deletes all QgsComposerMapItems contained
     * by the stack
     */
    void removeItems();
};

#endif //QGSCOMPOSERMAPITEM_H

