/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemmapitem.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemMapItem : QgsLayoutObject
{
%Docstring
 An item which is drawn inside a QgsLayoutItemMap, e.g., a grid or map overview.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemmapitem.h"
%End
  public:

    QgsLayoutItemMapItem( const QString &name, QgsLayoutItemMap *map );
%Docstring
Constructor for QgsLayoutItemMapItem, attached to the specified ``map``.

The ``name`` argument gives a friendly display name for the item.
%End

    virtual void draw( QPainter *painter ) = 0;
%Docstring
Draws the item on to a destination ``painter``.
%End

    virtual bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Stores map item state in a DOM element, where ``element`` is the DOM element
corresponding to a 'LayoutMap' tag.

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context );
%Docstring
Sets the map item state from a DOM document, where ``element`` is the DOM
node corresponding to a 'LayoutMapGrid' tag.

.. seealso:: :py:func:`writeXml`
%End

    void setMap( QgsLayoutItemMap *map );
%Docstring
Sets the corresponding layout ``map`` for the item.

.. seealso:: :py:func:`map`
%End

    const QgsLayoutItemMap *map() const;
%Docstring
Returns the layout item map for the item.

.. seealso:: :py:func:`setMap`
%End

    QString id() const;
%Docstring
Returns the unique id for the map item.
%End

    void setName( const QString &name );
%Docstring
Sets the friendly display ``name`` for the item.

.. seealso:: :py:func:`name`
%End

    QString name() const;
%Docstring
Returns the friendly display name for the item.

.. seealso:: :py:func:`setName`
%End

    void setEnabled( bool enabled );
%Docstring
Controls whether the item will be drawn. Set ``enabled`` to true to enable drawing of the item.

.. seealso:: :py:func:`enabled`
%End

    bool enabled() const;
%Docstring
Returns whether the item will be drawn.

.. seealso:: :py:func:`setEnabled`
%End

    virtual bool usesAdvancedEffects() const;
%Docstring
Returns true if the item is drawn using advanced effects, such as blend modes.
%End

  protected:





};



class QgsLayoutItemMapItemStack
{
%Docstring
 A collection of map items which are drawn above the map content in a
:py:class:`QgsLayoutItemMap`. The item stack controls which items are drawn and the
order they are drawn in.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsLayoutItemMapItem`
%End

%TypeHeaderCode
#include "qgslayoutitemmapitem.h"
%End
  public:

    QgsLayoutItemMapItemStack( QgsLayoutItemMap *map );
%Docstring
Constructor for QgsLayoutItemMapItemStack, attached to the
specified ``map``.
%End

    virtual ~QgsLayoutItemMapItemStack();

    int size() const;
%Docstring
Returns the number of items in the stack.
%End

    virtual bool writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
Stores the state of the item stack in a DOM node, where ``element`` is the DOM element corresponding to a 'LayoutMap' tag.
Returns true if write was successful.

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context ) = 0;
%Docstring
Sets the item stack's state from a DOM document, where ``element`` is a DOM node corresponding to a 'LayoutMap' tag.
Returns true if read was successful.

.. seealso:: :py:func:`writeXml`
%End

    void drawItems( QPainter *painter );
%Docstring
Draws the items from the stack on a specified ``painter``.
%End

    bool containsAdvancedEffects() const;
%Docstring
Returns whether any items within the stack contain advanced effects,
such as blending modes.
%End

  protected:

    void addItem( QgsLayoutItemMapItem *item /Transfer/ );
%Docstring
Adds a new map item to the stack and takes ownership of the item.
The item will be added to the end of the stack, and rendered
above any existing map items already present in the stack.

.. note::

   After adding an item to the stack update()
should be called for the QgsLayoutItemMap to prevent rendering artifacts.

.. seealso:: :py:func:`removeItem`
%End

    void removeItem( const QString &itemId );
%Docstring
Removes an item which matching ``itemId`` from the stack and deletes the corresponding :py:class:`QgsLayoutItemMapItem`

.. note::

   After removing an item from the stack, update()
should be called for the QgsLayoutItemMap to prevent rendering artifacts.

.. seealso:: :py:func:`addItem`
%End

    void moveItemUp( const QString &itemId );
%Docstring
Moves an item which matching ``itemId`` up the stack, causing it to be rendered above other items.

.. note::

   After moving an item within the stack, update() should be
called for the QgsLayoutItemMap to redraw the map with the new item stack order.

.. seealso:: :py:func:`moveItemDown`
%End

    void moveItemDown( const QString &itemId );
%Docstring
Moves an item which matching ``itemId`` up the stack, causing it to be rendered above other items.

.. note::

   After moving an item within the stack, update() should be
called for the QgsLayoutItemMap to redraw the map with the new item stack order.

.. seealso:: :py:func:`moveItemUp`
%End

    QgsLayoutItemMapItem *item( const QString &itemId ) const;
%Docstring
Returns a reference to an item which matching ``itemId`` within the stack.

.. seealso:: :py:func:`constItem`
%End

    QgsLayoutItemMapItem *item( int index ) const;
%Docstring
Returns a reference to the item at the specified ``index`` within the stack.

.. seealso:: :py:func:`constItem`
%End


    QList< QgsLayoutItemMapItem * > asList() const;
%Docstring
Returns a list of QgsLayoutItemMapItems contained by the stack.
%End

  protected:



    void removeItems();
%Docstring
Clears the item stack and deletes all QgsLayoutItemMapItems contained
by the stack
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemmapitem.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
