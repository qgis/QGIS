/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermapitem.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerMapItem : QgsComposerObject
{
%Docstring
 An item which is drawn inside a QgsComposerMap, e.g., a grid or map overview.
%End

%TypeHeaderCode
#include "qgscomposermapitem.h"
%End
  public:

    QgsComposerMapItem( const QString &name, QgsComposerMap *map );
%Docstring
Constructor for QgsComposerMapItem.

:param name: friendly display name for item
:param map: QgsComposerMap the item is attached to
%End

    virtual void draw( QPainter *painter ) = 0;
%Docstring
Draws the item on to a painter

:param painter: destination QPainter
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores map item state in DOM element

:param elem: is DOM element corresponding to a 'ComposerMap' tag
:param doc: DOM document

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets map item state from a DOM document

:param itemElem: is DOM node corresponding to a 'ComposerMapGrid' tag
:param doc: is DOM document

.. seealso:: :py:func:`writeXml`
%End

    virtual void setComposerMap( QgsComposerMap *map );
%Docstring
Sets composer map for the item

:param map: composer map

.. seealso:: :py:func:`composerMap`
%End

    virtual const QgsComposerMap *composerMap() const;
%Docstring
Get composer map for the item

:return: composer map

.. seealso:: :py:func:`setComposerMap`
%End

    QString id() const;
%Docstring
Get the unique id for the map item

:return: unique id
%End

    virtual void setName( const QString &name );
%Docstring
Sets the friendly display name for the item

:param name: display name

.. seealso:: :py:func:`name`
%End

    virtual QString name() const;
%Docstring
Get friendly display name for the item

:return: display name

.. seealso:: :py:func:`setName`
%End

    virtual void setEnabled( const bool enabled );
%Docstring
Controls whether the item will be drawn

:param enabled: set to true to enable drawing of the item

.. seealso:: :py:func:`enabled`
%End

    virtual bool enabled() const;
%Docstring
Returns whether the item will be drawn

:return: true if item will be drawn on the map

.. seealso:: :py:func:`setEnabled`
%End

    virtual bool usesAdvancedEffects() const;
%Docstring
Returns true if the item is drawn using advanced effects, such as blend modes.

:return: true if item uses advanced effects
%End

  protected:





};



class QgsComposerMapItemStack
{
%Docstring
 A collection of map items which are drawn above the map content in a
:py:class:`QgsComposerMap`. The item stack controls which items are drawn and the
order they are drawn in.

.. versionadded:: 2.5

.. seealso:: :py:class:`QgsComposerMapItem`
%End

%TypeHeaderCode
#include "qgscomposermapitem.h"
%End
  public:

    QgsComposerMapItemStack( QgsComposerMap *map );
%Docstring
Constructor for QgsComposerMapItemStack.

:param map: QgsComposerMap the item stack is attached to
%End

    virtual ~QgsComposerMapItemStack();

    int size() const;
%Docstring
Returns the number of items in the stack

:return: number of items in the stack
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;
%Docstring
Stores the state of the item stack in a DOM node

:param elem: is DOM element corresponding to a 'ComposerMap' tag
:param doc: DOM document

:return: true if write was successful

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &elem, const QDomDocument &doc ) = 0;
%Docstring
Sets the item stack's state from a DOM document

:param elem: is DOM node corresponding to 'a ComposerMap' tag
:param doc: DOM document

:return: true if read was successful

.. seealso:: :py:func:`writeXml`
%End

    void drawItems( QPainter *painter );
%Docstring
Draws the items from the stack on a specified painter

:param painter: destination QPainter
%End

    bool containsAdvancedEffects() const;
%Docstring
Returns whether any items within the stack contain advanced effects,
such as blending modes

:return: true if item stack contains advanced effects
%End

  protected:

    void addItem( QgsComposerMapItem *item /Transfer/ );
%Docstring
Adds a new map item to the stack and takes ownership of the item.
The item will be added to the end of the stack, and rendered
above any existing map items already present in the stack.

:param item: QgsComposerMapItem to add to the stack

.. note::

   after adding an item to the stack update()
should be called for the QgsComposerMap to prevent rendering artifacts

.. seealso:: :py:func:`removeItem`
%End

    void removeItem( const QString &itemId );
%Docstring
Removes an item from the stack and deletes the corresponding :py:class:`QgsComposerMapItem`

:param itemId: id for the QgsComposerMapItem to remove

.. note::

   after removing an item from the stack, update()
should be called for the QgsComposerMap to prevent rendering artifacts

.. seealso:: :py:func:`addItem`
%End

    void moveItemUp( const QString &itemId );
%Docstring
Moves an item up the stack, causing it to be rendered above other items

:param itemId: id for the QgsComposerMapItem to move up

.. note::

   after moving an item within the stack, update() should be
called for the QgsComposerMap to redraw the map with the new item stack order

.. seealso:: :py:func:`moveItemDown`
%End

    void moveItemDown( const QString &itemId );
%Docstring
Moves an item up the stack, causing it to be rendered above other items

:param itemId: id for the QgsComposerMapItem to move down

.. note::

   after moving an item within the stack, update() should be
called for the QgsComposerMap to redraw the map with the new item stack order

.. seealso:: :py:func:`moveItemUp`
%End

    const QgsComposerMapItem *constItem( const QString &itemId ) const;
%Docstring
Returns a const reference to an item within the stack

:param itemId: id for the QgsComposerMapItem to find

:return: const reference to item, if found

.. seealso:: :py:func:`item`
%End

    QgsComposerMapItem *item( const QString &itemId ) const;
%Docstring
Returns a reference to an item within the stack

:param itemId: id for the QgsComposerMapItem to find

:return: reference to item if found

.. seealso:: :py:func:`constItem`
%End

    QgsComposerMapItem *item( const int index ) const;
%Docstring
Returns a reference to an item within the stack

:param index: item position in the stack

:return: reference to item if found

.. seealso:: :py:func:`constItem`
%End


    QList< QgsComposerMapItem * > asList() const;
%Docstring
Returns a list of QgsComposerMapItems contained by the stack

:return: list of items
%End

  protected:



    void removeItems();
%Docstring
Clears the item stack and deletes all QgsComposerMapItems contained
by the stack
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermapitem.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
