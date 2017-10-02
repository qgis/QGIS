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
      ZMapTool,
      ZSnapIndicator,
    };

    QgsLayout( QgsProject *project );
%Docstring
 Construct a new layout linked to the specified ``project``.

 If the layout is a "new" layout (as opposed to a layout which will
 restore a previous state from XML) then initializeDefaults() should be
 called on the new layout.
%End

    void initializeDefaults();
%Docstring
 Initializes an empty layout, e.g. by adding a default page to the layout. This should be called after creating
 a new layout.
%End

    QgsProject *project() const;
%Docstring
 The project associated with the layout. Used to get access to layers, map themes,
 relations and various other bits. It is never null.
 :rtype: QgsProject
%End

    QString name() const;
%Docstring
 Returns the layout's name.
.. seealso:: setName()
 :rtype: str
%End

    void setName( const QString &name );
%Docstring
 Sets the layout's name.
.. seealso:: name()
%End


    QList<QgsLayoutItem *> selectedLayoutItems( const bool includeLockedItems = true );
%Docstring
 Returns list of selected layout items.

 If ``includeLockedItems`` is set to true, then locked items will also be included
 in the returned list.
 :rtype: list of QgsLayoutItem
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

    QgsLayoutItem *itemByUuid( const QString &uuid );
%Docstring
 Returns the layout item with matching ``uuid`` unique identifier, or a None
 if a matching item could not be found.
 :rtype: QgsLayoutItem
%End

    QgsLayoutItem *layoutItemAt( QPointF position, const bool ignoreLocked = false ) const;
%Docstring
 Returns the topmost layout item at a specified ``position``. Ignores paper items.
 If ``ignoreLocked`` is set to true any locked items will be ignored.
 :rtype: QgsLayoutItem
%End

    QgsLayoutItem *layoutItemAt( QPointF position, const QgsLayoutItem *belowItem, const bool ignoreLocked = false ) const;
%Docstring
 Returns the topmost composer item at a specified ``position`` which is below a specified ``item``. Ignores paper items.
 If ``ignoreLocked`` is set to true any locked items will be ignored.
 :rtype: QgsLayoutItem
%End

    void setUnits( QgsUnitTypes::LayoutUnit units );
%Docstring
 Sets the native measurement ``units`` for the layout. These also form the default unit
 for measurements for the layout.
.. seealso:: units()
.. seealso:: convertToLayoutUnits()
%End

    QgsUnitTypes::LayoutUnit units() const;
%Docstring
 Returns the native units for the layout.
.. seealso:: setUnits()
.. seealso:: convertToLayoutUnits()
 :rtype: QgsUnitTypes.LayoutUnit
%End

    double convertToLayoutUnits( const QgsLayoutMeasurement &measurement ) const;
%Docstring
 Converts a measurement into the layout's native units.
 :return: length of measurement in layout units
.. seealso:: convertFromLayoutUnits()
.. seealso:: units()
 :rtype: float
%End

    QSizeF convertToLayoutUnits( const QgsLayoutSize &size ) const;
%Docstring
 Converts a size into the layout's native units.
 :return: size of measurement in layout units
.. seealso:: convertFromLayoutUnits()
.. seealso:: units()
 :rtype: QSizeF
%End

    QPointF convertToLayoutUnits( const QgsLayoutPoint &point ) const;
%Docstring
 Converts a ``point`` into the layout's native units.
 :return: point in layout units
.. seealso:: convertFromLayoutUnits()
.. seealso:: units()
 :rtype: QPointF
%End

    QgsLayoutMeasurement convertFromLayoutUnits( const double length, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
 Converts a ``length`` measurement from the layout's native units to a specified target ``unit``.
 :return: length of measurement in specified units
.. seealso:: convertToLayoutUnits()
.. seealso:: units()
 :rtype: QgsLayoutMeasurement
%End

    QgsLayoutSize convertFromLayoutUnits( const QSizeF &size, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
 Converts a ``size`` from the layout's native units to a specified target ``unit``.
 :return: size of measurement in specified units
.. seealso:: convertToLayoutUnits()
.. seealso:: units()
 :rtype: QgsLayoutSize
%End

    QgsLayoutPoint convertFromLayoutUnits( const QPointF &point, const QgsUnitTypes::LayoutUnit unit ) const;
%Docstring
 Converts a ``point`` from the layout's native units to a specified target ``unit``.
 :return: point in specified units
.. seealso:: convertToLayoutUnits()
.. seealso:: units()
 :rtype: QgsLayoutPoint
%End

    QgsLayoutContext &context();
%Docstring
 Returns a reference to the layout's context, which stores information relating to the
 current context and rendering settings for the layout.
 :rtype: QgsLayoutContext
%End


    QgsLayoutSnapper &snapper();
%Docstring
 Returns a reference to the layout's snapper, which stores handles layout snap grids and lines
 and snapping points to the nearest matching point.
 :rtype: QgsLayoutSnapper
%End


    QgsLayoutGridSettings &gridSettings();
%Docstring
 Returns a reference to the layout's grid settings, which stores settings relating
 to grid appearance, spacing and offsets.
 :rtype: QgsLayoutGridSettings
%End


    QgsLayoutGuideCollection &guides();
%Docstring
 Returns a reference to the layout's guide collection, which manages page snap guides.
 :rtype: QgsLayoutGuideCollection
%End


    virtual QgsExpressionContext createExpressionContext() const;

%Docstring
 Creates an expression context relating to the layout's current state. The context includes
 scopes for global, project, layout and layout context properties.
 :rtype: QgsExpressionContext
%End

    void setCustomProperty( const QString &key, const QVariant &value );
%Docstring
 Set a custom property for the layout.
 \param key property key. If a property with the same key already exists it will be overwritten.
 \param value property value
.. seealso:: customProperty()
.. seealso:: removeCustomProperty()
.. seealso:: customProperties()
%End

    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;
%Docstring
 Read a custom property from the layout.
 \param key property key
 \param defaultValue default value to return if property with matching key does not exist
 :return: value of matching property
.. seealso:: setCustomProperty()
.. seealso:: removeCustomProperty()
.. seealso:: customProperties()
 :rtype: QVariant
%End

    void removeCustomProperty( const QString &key );
%Docstring
 Remove a custom property from the layout.
 \param key property key
.. seealso:: setCustomProperty()
.. seealso:: customProperty()
.. seealso:: customProperties()
%End

    QStringList customProperties() const;
%Docstring
 Return list of keys stored in custom properties for the layout.
.. seealso:: setCustomProperty()
.. seealso:: customProperty()
.. seealso:: removeCustomProperty()
 :rtype: list of str
%End

    QgsLayoutItemMap *referenceMap() const;
%Docstring
 :rtype: QgsLayoutItemMap
%End

    void setReferenceMap( QgsLayoutItemMap *map );

    QgsLayoutPageCollection *pageCollection();
%Docstring
 Returns a pointer to the layout's page collection, which stores and manages
 page items in the layout.
 :rtype: QgsLayoutPageCollection
%End


    QRectF layoutBounds( bool ignorePages = false, double margin = 0.0 ) const;
%Docstring
 Calculates the bounds of all non-gui items in the layout. Ignores snap lines, mouse handles
 and other cosmetic items.
 \param ignorePages set to true to ignore page items
 \param margin optional marginal (in percent, e.g., 0.05 = 5% ) to add around items
 :return: layout bounds, in layout units.
 :rtype: QRectF
%End

    void addLayoutItem( QgsLayoutItem *item /Transfer/ );
%Docstring
 Adds an ``item`` to the layout. This should be called instead of the base class addItem()
 method. Ownership of the item is transferred to the layout.
%End

    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
 Returns the layout's state encapsulated in a DOM element.
.. seealso:: readXml()
 :rtype: QDomElement
%End

    bool readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
 Sets the collection's state from a DOM element. ``layoutElement`` is the DOM node corresponding to the layout.
.. seealso:: writeXml()
 :rtype: bool
%End

    QgsLayoutUndoStack *undoStack();
%Docstring
 Returns a pointer to the layout's undo stack, which manages undo/redo states for the layout
 and it's associated objects.
 :rtype: QgsLayoutUndoStack
%End


    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id = 0, QUndoCommand *parent = 0 ) /Factory/;



  public slots:

    void updateBounds();
%Docstring
 Updates the scene bounds of the layout.
%End

  signals:

    void variablesChanged();
%Docstring
 Emitted whenever the expression variables stored in the layout have been changed.
%End

    void selectedItemChanged( QgsLayoutItem *selected );
%Docstring
 Emitted whenever the selected item changes.
 If None, no item is selected.
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayout.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
