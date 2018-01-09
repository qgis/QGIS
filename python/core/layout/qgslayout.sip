/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayout.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayout : QGraphicsScene, QgsExpressionContextGenerator, QgsLayoutUndoObjectInterface
{
%Docstring
 Base class for layouts, which can contain items such as maps, labels, scalebars, etc.

While the raw QGraphicsScene API can be used to render the contents of a QgsLayout
to a QPainter, it is recommended to instead use a QgsLayoutExporter to handle rendering
layouts instead. QgsLayoutExporter automatically takes care of the intracacies of
preparing the layout and paint devices for correct exports, respecting various
user settings such as the layout context DPI.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayout.h"
%End
  public:

    enum ZValues
    {
      ZPage,
      ZItem,
      ZGrid,
      ZGuide,
      ZSmartGuide,
      ZMouseHandles,
      ZViewTool,
      ZSnapIndicator,
    };

    enum UndoCommand
    {
      UndoLayoutDpi,
      UndoNone,
    };

    QgsLayout( QgsProject *project );
%Docstring
Construct a new layout linked to the specified ``project``.

If the layout is a "new" layout (as opposed to a layout which will
restore a previous state from XML) then initializeDefaults() should be
called on the new layout.
%End

    ~QgsLayout();

    QgsLayout *clone() const /Factory/;
%Docstring
Creates a clone of the layout. Ownership of the return layout
is transferred to the caller.
%End

    void initializeDefaults();
%Docstring
Initializes an empty layout, e.g. by adding a default page to the layout. This should be called after creating
a new layout.
%End

    void clear();
%Docstring
Clears the layout.

Calling this method removes all items and pages from the layout.
%End

    QgsProject *project() const;
%Docstring
The project associated with the layout. Used to get access to layers, map themes,
relations and various other bits. It is never null.
%End

    QgsLayoutModel *itemsModel();
%Docstring
Returns the items model attached to the layout.
%End



    QList<QgsLayoutItem *> selectedLayoutItems( const bool includeLockedItems = true );
%Docstring
Returns list of selected layout items.

If ``includeLockedItems`` is set to true, then locked items will also be included
in the returned list.
%End

    void setSelectedItem( QgsLayoutItem *item );
%Docstring
Clears any selected items and sets ``item`` as the current selection.
%End

    void deselectAll();
%Docstring
Clears any selected items in the layout.

Call this method rather than QGraphicsScene.clearSelection, as the latter does
not correctly emit signals to allow the layout's model to update.
%End

    bool raiseItem( QgsLayoutItem *item, bool deferUpdate = false );
%Docstring
Raises an ``item`` up the z-order.
Returns true if the item was successfully raised.

If ``deferUpdate`` is true, the scene will not be visibly updated
to reflect the new stacking order. This allows multiple
raiseItem() calls to be made in sequence without the cost of
updating the scene for each one.

.. seealso:: :py:func:`lowerItem`

.. seealso:: :py:func:`updateZValues`
%End

    bool lowerItem( QgsLayoutItem *item, bool deferUpdate = false );
%Docstring
Lowers an ``item`` down the z-order.
Returns true if the item was successfully lowered.

If ``deferUpdate`` is true, the scene will not be visibly updated
to reflect the new stacking order. This allows multiple
raiseItem() calls to be made in sequence without the cost of
updating the scene for each one.

.. seealso:: :py:func:`raiseItem`

.. seealso:: :py:func:`updateZValues`
%End

    bool moveItemToTop( QgsLayoutItem *item, bool deferUpdate = false );
%Docstring
Raises an ``item`` up to the top of the z-order.
Returns true if the item was successfully raised.

If ``deferUpdate`` is true, the scene will not be visibly updated
to reflect the new stacking order. This allows multiple
raiseItem() calls to be made in sequence without the cost of
updating the scene for each one.

.. seealso:: :py:func:`moveItemToBottom`

.. seealso:: :py:func:`updateZValues`
%End

    bool moveItemToBottom( QgsLayoutItem *item, bool deferUpdate = false );
%Docstring
Lowers an ``item`` down to the bottom of the z-order.
Returns true if the item was successfully lowered.
If ``deferUpdate`` is true, the scene will not be visibly updated
to reflect the new stacking order. This allows multiple
raiseItem() calls to be made in sequence without the cost of
updating the scene for each one.

.. seealso:: :py:func:`moveItemToTop`

.. seealso:: :py:func:`updateZValues`
%End

    void updateZValues( const bool addUndoCommands = true );
%Docstring
Resets the z-values of items based on their position in the internal
z order list. This should be called after any stacking changes
which deferred z-order updates.
%End

    QgsLayoutItem *itemByUuid( const QString &uuid, bool includeTemplateUuids = false ) const;
%Docstring
Returns the layout item with matching ``uuid`` unique identifier, or a None
if a matching item could not be found.

If ``includeTemplateUuids`` is true, then item's :py:func:`QgsLayoutItem.templateUuid()`
will also be tested when trying to match the uuid.

.. seealso:: :py:func:`multiFrameByUuid`

.. seealso:: :py:func:`itemById`
%End

    QgsLayoutItem *itemById( const QString &id ) const;
%Docstring
Returns a layout item given its ``id``.
Since item IDs are not necessarely unique, this function returns the first matching
item found.

.. seealso:: :py:func:`itemByUuid`
%End

    QgsLayoutMultiFrame *multiFrameByUuid( const QString &uuid ) const;
%Docstring
Returns the layout multiframe with matching ``uuid`` unique identifier, or a None
if a matching multiframe could not be found.

.. seealso:: :py:func:`itemByUuid`
%End

    QgsLayoutItem *layoutItemAt( QPointF position, const bool ignoreLocked = false ) const;
%Docstring
Returns the topmost layout item at a specified ``position``. Ignores paper items.
If ``ignoreLocked`` is set to true any locked items will be ignored.
%End

    QgsLayoutItem *layoutItemAt( QPointF position, const QgsLayoutItem *belowItem, const bool ignoreLocked = false ) const;
%Docstring
Returns the topmost composer item at a specified ``position`` which is below a specified ``item``. Ignores paper items.
If ``ignoreLocked`` is set to true any locked items will be ignored.
%End

    void setUnits( QgsUnitTypes::LayoutUnit units );
%Docstring
Sets the native measurement ``units`` for the layout. These also form the default unit
for measurements for the layout.

.. seealso:: :py:func:`units`

.. seealso:: :py:func:`convertToLayoutUnits`
%End

    QgsUnitTypes::LayoutUnit units() const;
%Docstring
Returns the native units for the layout.

.. seealso:: :py:func:`setUnits`

.. seealso:: :py:func:`convertToLayoutUnits`
%End

    double convertToLayoutUnits( const QgsLayoutMeasurement &measurement ) const;
%Docstring
Converts a measurement into the layout's native units.

:return: length of measurement in layout units

.. seealso:: :py:func:`convertFromLayoutUnits`

.. seealso:: :py:func:`units`
%End

    QSizeF convertToLayoutUnits( const QgsLayoutSize &size ) const;
%Docstring
Converts a size into the layout's native units.

:return: size of measurement in layout units

.. seealso:: :py:func:`convertFromLayoutUnits`

.. seealso:: :py:func:`units`
%End

    QPointF convertToLayoutUnits( const QgsLayoutPoint &point ) const;
%Docstring
Converts a ``point`` into the layout's native units.

:return: point in layout units

.. seealso:: :py:func:`convertFromLayoutUnits`

.. seealso:: :py:func:`units`
%End

    QgsLayoutMeasurement convertFromLayoutUnits( const double length, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
Converts a ``length`` measurement from the layout's native units to a specified target ``unit``.

:return: length of measurement in specified units

.. seealso:: :py:func:`convertToLayoutUnits`

.. seealso:: :py:func:`units`
%End

    QgsLayoutSize convertFromLayoutUnits( const QSizeF &size, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
Converts a ``size`` from the layout's native units to a specified target ``unit``.

:return: size of measurement in specified units

.. seealso:: :py:func:`convertToLayoutUnits`

.. seealso:: :py:func:`units`
%End

    QgsLayoutPoint convertFromLayoutUnits( const QPointF &point, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
Converts a ``point`` from the layout's native units to a specified target ``unit``.

:return: point in specified units

.. seealso:: :py:func:`convertToLayoutUnits`

.. seealso:: :py:func:`units`
%End

    QgsLayoutRenderContext &renderContext();
%Docstring
Returns a reference to the layout's render context, which stores information relating to the
current rendering settings for the layout.
%End


    QgsLayoutReportContext &reportContext();
%Docstring
Returns a reference to the layout's report context, which stores information relating to the
current reporting context for the layout.
%End


    QgsLayoutSnapper &snapper();
%Docstring
Returns a reference to the layout's snapper, which stores handles layout snap grids and lines
and snapping points to the nearest matching point.
%End


    QgsLayoutGridSettings &gridSettings();
%Docstring
Returns a reference to the layout's grid settings, which stores settings relating
to grid appearance, spacing and offsets.
%End


    QgsLayoutGuideCollection &guides();
%Docstring
Returns a reference to the layout's guide collection, which manages page snap guides.
%End


    virtual QgsExpressionContext createExpressionContext() const;

%Docstring
Creates an expression context relating to the layout's current state. The context includes
scopes for global, project, layout and layout context properties.
%End

    void setCustomProperty( const QString &key, const QVariant &value );
%Docstring
Set a custom property for the layout.

:param key: property key. If a property with the same key already exists it will be overwritten.
:param value: property value

.. seealso:: :py:func:`customProperty`

.. seealso:: :py:func:`removeCustomProperty`

.. seealso:: :py:func:`customProperties`
%End

    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;
%Docstring
Read a custom property from the layout.

:param key: property key
:param defaultValue: default value to return if property with matching key does not exist

:return: value of matching property

.. seealso:: :py:func:`setCustomProperty`

.. seealso:: :py:func:`removeCustomProperty`

.. seealso:: :py:func:`customProperties`
%End

    void removeCustomProperty( const QString &key );
%Docstring
Remove a custom property from the layout.

:param key: property key

.. seealso:: :py:func:`setCustomProperty`

.. seealso:: :py:func:`customProperty`

.. seealso:: :py:func:`customProperties`
%End

    QStringList customProperties() const;
%Docstring
Return list of keys stored in custom properties for the layout.

.. seealso:: :py:func:`setCustomProperty`

.. seealso:: :py:func:`customProperty`

.. seealso:: :py:func:`removeCustomProperty`
%End

    QgsLayoutItemMap *referenceMap() const;
%Docstring
Returns the map item which will be used to generate corresponding world files when the
layout is exported. If no map was explicitly set via setReferenceMap(), the largest
map in the layout will be returned (or None if there are no maps in the layout).

.. seealso:: :py:func:`setReferenceMap`

.. seealso:: :py:func:`generateWorldFile`
%End

    void setReferenceMap( QgsLayoutItemMap *map );
%Docstring
Sets the ``map`` item which will be used to generate corresponding world files when the
layout is exported.

.. seealso:: :py:func:`referenceMap`

.. seealso:: :py:func:`setGenerateWorldFile`
%End

    QgsLayoutPageCollection *pageCollection();
%Docstring
Returns a pointer to the layout's page collection, which stores and manages
page items in the layout.
%End


    QRectF layoutBounds( bool ignorePages = false, double margin = 0.0 ) const;
%Docstring
Calculates the bounds of all non-gui items in the layout. Ignores snap lines, mouse handles
and other cosmetic items.

:param ignorePages: set to true to ignore page items
:param margin: optional marginal (in percent, e.g., 0.05 = 5% ) to add around items

:return: layout bounds, in layout units.

.. seealso:: :py:func:`pageItemBounds`
%End

    QRectF pageItemBounds( int page, bool visibleOnly = false ) const;
%Docstring
Returns the bounding box of the items contained on a specified ``page``.
A page number of 0 represents the first page in the layout.

Set ``visibleOnly`` to true to only include visible items.

The returned bounds are in layout units.

.. seealso:: :py:func:`layoutBounds`
%End

    void addLayoutItem( QgsLayoutItem *item /Transfer/ );
%Docstring
Adds an ``item`` to the layout. This should be called instead of the base class addItem()
method. Ownership of the item is transferred to the layout.
%End

    void removeLayoutItem( QgsLayoutItem *item );
%Docstring
Removes an ``item`` from the layout. This should be called instead of the base class removeItem()
method.
The item will also be deleted.
%End

    void addMultiFrame( QgsLayoutMultiFrame *multiFrame /Transfer/ );
%Docstring
Adds a ``multiFrame`` to the layout. The object is owned by the layout until removeMultiFrame() is called.

.. seealso:: :py:func:`removeMultiFrame`

.. seealso:: :py:func:`multiFrames`
%End

    void removeMultiFrame( QgsLayoutMultiFrame *multiFrame );
%Docstring
Removes a ``multiFrame`` from the layout (but does not delete it).

.. seealso:: :py:func:`addMultiFrame`

.. seealso:: :py:func:`multiFrames`
%End

    QList< QgsLayoutMultiFrame * > multiFrames() const;
%Docstring
Returns a list of multi frames contained in the layout.

.. seealso:: :py:func:`addMultiFrame`

.. seealso:: :py:func:`removeMultiFrame`
%End

    bool saveAsTemplate( const QString &path, const QgsReadWriteContext &context ) const;
%Docstring
Saves the layout as a template at the given file ``path``.
Returns true if save was successful.

.. seealso:: :py:func:`loadFromTemplate`
%End

    QList< QgsLayoutItem * > loadFromTemplate( const QDomDocument &document, const QgsReadWriteContext &context, bool clearExisting = true, bool *ok /Out/ = 0 );
%Docstring
Load a layout template ``document``.

By default this method will clear all items from the existing layout and real all layout
settings from the template. Setting ``clearExisting`` to false will only add new items
from the template, without overwriting the existing items or layout settings.

If ``ok`` is specified, it will be set to true if the load was successful.

Returns a list of loaded items.
%End

    virtual QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Returns the layout's state encapsulated in a DOM element.

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
Sets the collection's state from a DOM element. ``layoutElement`` is the DOM node corresponding to the layout.

.. seealso:: :py:func:`writeXml`
%End

    QList< QgsLayoutItem * > addItemsFromXml( const QDomElement &parentElement, const QDomDocument &document,
        const QgsReadWriteContext &context,
        QPointF *position = 0, bool pasteInPlace = false );
%Docstring
Add items from an XML representation to the layout. Used for project file reading and pasting items from clipboard.

The ``position`` argument is optional, and if it is not specified the items will be restored to their
original position from the XML serialization. If specified, the items will be positioned such that the top-left
bounds of all added items is located at this ``position``.

The ``pasteInPlace`` argument determines whether the serialized position should be respected, but remapped to the
origin of the page corresponding to the page at ``position``.

A list of the newly added items is returned.
%End

    QgsLayoutUndoStack *undoStack();
%Docstring
Returns a pointer to the layout's undo stack, which manages undo/redo states for the layout
and it's associated objects.
%End


    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id = 0, QUndoCommand *parent = 0 ) /Factory/;


    QgsLayoutItemGroup *groupItems( const QList<QgsLayoutItem *> &items );
%Docstring
Creates a new group from a list of layout ``items`` and adds the group to the layout.
If grouping was not possible, a None will be returned.

.. seealso:: :py:func:`ungroupItems`
%End

    QList<QgsLayoutItem *> ungroupItems( QgsLayoutItemGroup *group );
%Docstring
Ungroups items by removing them from an item ``group`` and removing the group from the
layout. Child items will remain in the layout and will not be deleted.

Returns a list of the items removed from the group, or an empty list if ungrouping
was not successful.

.. seealso:: :py:func:`groupItems`
%End

  public slots:

    void refresh();
%Docstring
Forces the layout, and all items contained within it, to refresh. For instance, this causes maps to redraw
and rebuild cached images, html items to reload their source url, and attribute tables
to refresh their contents. Calling this also triggers a recalculation of all data defined
attributes within the layout.

.. seealso:: :py:func:`refreshed`
%End

    void updateBounds();
%Docstring
Updates the scene bounds of the layout.
%End

  signals:

    void changed();
%Docstring
Is emitted when properties of the layout change. This signal is only
emitted for settings directly managed by the layout, and is not emitted
when child items change.
%End

    void variablesChanged();
%Docstring
Emitted whenever the expression variables stored in the layout have been changed.
%End

    void selectedItemChanged( QgsLayoutItem *selected );
%Docstring
Emitted whenever the selected item changes.
If None, no item is selected.
%End

    void refreshed();
%Docstring
Is emitted when the layout has been refreshed and items should also be refreshed
and updated.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayout.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
