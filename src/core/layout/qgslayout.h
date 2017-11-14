/***************************************************************************
                              qgslayout.h
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
#ifndef QGSLAYOUT_H
#define QGSLAYOUT_H

#include "qgis_core.h"
#include <QGraphicsScene>
#include "qgslayoutcontext.h"
#include "qgslayoutsnapper.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutgridsettings.h"
#include "qgslayoutguidecollection.h"
#include "qgslayoutundostack.h"
#include "qgslayoutexporter.h"

class QgsLayoutItemMap;
class QgsLayoutModel;

/**
 * \ingroup core
 * \class QgsLayout
 * \brief Base class for layouts, which can contain items such as maps, labels, scalebars, etc.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayout : public QGraphicsScene, public QgsExpressionContextGenerator, public QgsLayoutUndoObjectInterface
{
    Q_OBJECT

  public:

    //! Preset item z-values, to ensure correct stacking
    enum ZValues
    {
      ZPage = 0, //!< Z-value for page (paper) items
      ZItem = 1, //!< Minimum z value for items
      ZGrid = 9997, //!< Z-value for page grids
      ZGuide = 9998, //!< Z-value for page guides
      ZSmartGuide = 9999, //!< Z-value for smart (item bounds based) guides
      ZMouseHandles = 10000, //!< Z-value for mouse handles
      ZViewTool = 10001, //!< Z-value for temporary view tool items
      ZSnapIndicator = 10002, //!< Z-value for snapping indicator
    };

    /**
     * Construct a new layout linked to the specified \a project.
     *
     * If the layout is a "new" layout (as opposed to a layout which will
     * restore a previous state from XML) then initializeDefaults() should be
     * called on the new layout.
     */
    QgsLayout( QgsProject *project );

    ~QgsLayout();

    /**
     * Initializes an empty layout, e.g. by adding a default page to the layout. This should be called after creating
     * a new layout.
     */
    void initializeDefaults();

    /**
     * The project associated with the layout. Used to get access to layers, map themes,
     * relations and various other bits. It is never null.
     *
     */
    QgsProject *project() const;

    /**
     * Returns the items model attached to the layout.
     */
    QgsLayoutModel *itemsModel();

    /**
     * Returns the layout's exporter, which is used for rendering the layout and exporting
     * to various formats.
     */
    QgsLayoutExporter &exporter();

    /**
     * Returns the layout's name.
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the layout's name.
     * \see name()
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns a list of layout items of a specific type.
     * \note not available in Python bindings
     */
    template<class T> void layoutItems( QList<T *> &itemList ) SIP_SKIP
    {
      itemList.clear();
      QList<QGraphicsItem *> graphicsItemList = items();
      QList<QGraphicsItem *>::iterator itemIt = graphicsItemList.begin();
      for ( ; itemIt != graphicsItemList.end(); ++itemIt )
      {
        T *item = dynamic_cast<T *>( *itemIt );
        if ( item )
        {
          itemList.push_back( item );
        }
      }
    }

    /**
     * Returns list of selected layout items.
     *
     * If \a includeLockedItems is set to true, then locked items will also be included
     * in the returned list.
     */
    QList<QgsLayoutItem *> selectedLayoutItems( const bool includeLockedItems = true );

    /**
     * Clears any selected items and sets \a item as the current selection.
    */
    void setSelectedItem( QgsLayoutItem *item );

    /**
     * Clears any selected items in the layout.
     *
     * Call this method rather than QGraphicsScene::clearSelection, as the latter does
     * not correctly emit signals to allow the layout's model to update.
    */
    void deselectAll();

    /**
     * Raises an \a item up the z-order.
     * Returns true if the item was successfully raised.
     *
     * If \a deferUpdate is true, the scene will not be visibly updated
     * to reflect the new stacking order. This allows multiple
     * raiseItem() calls to be made in sequence without the cost of
     * updating the scene for each one.
     *
     * \see lowerItem()
     * \see updateZValues()
     */
    bool raiseItem( QgsLayoutItem *item, bool deferUpdate = false );

    /**
     * Lowers an \a item down the z-order.
     * Returns true if the item was successfully lowered.
     *
     * If \a deferUpdate is true, the scene will not be visibly updated
     * to reflect the new stacking order. This allows multiple
     * raiseItem() calls to be made in sequence without the cost of
     * updating the scene for each one.
     *
     * \see raiseItem()
     * \see updateZValues()
     */
    bool lowerItem( QgsLayoutItem *item, bool deferUpdate = false );

    /**
     * Raises an \a item up to the top of the z-order.
     * Returns true if the item was successfully raised.
     *
     * If \a deferUpdate is true, the scene will not be visibly updated
     * to reflect the new stacking order. This allows multiple
     * raiseItem() calls to be made in sequence without the cost of
     * updating the scene for each one.
     *
     * \see moveItemToBottom()
     * \see updateZValues()
     */
    bool moveItemToTop( QgsLayoutItem *item, bool deferUpdate = false );

    /**
     * Lowers an \a item down to the bottom of the z-order.
     * Returns true if the item was successfully lowered.
     * If \a deferUpdate is true, the scene will not be visibly updated
     * to reflect the new stacking order. This allows multiple
     * raiseItem() calls to be made in sequence without the cost of
     * updating the scene for each one.
     *
     * \see moveItemToTop()
     * \see updateZValues()
     */
    bool moveItemToBottom( QgsLayoutItem *item, bool deferUpdate = false );

    /**
     * Resets the z-values of items based on their position in the internal
     * z order list. This should be called after any stacking changes
     * which deferred z-order updates.
     */
    void updateZValues( const bool addUndoCommands = true );

    /**
     * Returns the layout item with matching \a uuid unique identifier, or a nullptr
     * if a matching item could not be found.
     */
    QgsLayoutItem *itemByUuid( const QString &uuid );

    /**
     * Returns the topmost layout item at a specified \a position. Ignores paper items.
     * If \a ignoreLocked is set to true any locked items will be ignored.
     */
    QgsLayoutItem *layoutItemAt( QPointF position, const bool ignoreLocked = false ) const;

    /**
     * Returns the topmost composer item at a specified \a position which is below a specified \a item. Ignores paper items.
     * If \a ignoreLocked is set to true any locked items will be ignored.
     */
    QgsLayoutItem *layoutItemAt( QPointF position, const QgsLayoutItem *belowItem, const bool ignoreLocked = false ) const;

    /**
     * Sets the native measurement \a units for the layout. These also form the default unit
     * for measurements for the layout.
     * \see units()
     * \see convertToLayoutUnits()
    */
    void setUnits( QgsUnitTypes::LayoutUnit units ) { mUnits = units; }

    /**
     * Returns the native units for the layout.
     * \see setUnits()
     * \see convertToLayoutUnits()
    */
    QgsUnitTypes::LayoutUnit units() const { return mUnits; }

    /**
     * Converts a measurement into the layout's native units.
     * \returns length of measurement in layout units
     * \see convertFromLayoutUnits()
     * \see units()
    */
    double convertToLayoutUnits( const QgsLayoutMeasurement &measurement ) const;

    /**
     * Converts a size into the layout's native units.
     * \returns size of measurement in layout units
     * \see convertFromLayoutUnits()
     * \see units()
    */
    QSizeF convertToLayoutUnits( const QgsLayoutSize &size ) const;

    /**
     * Converts a \a point into the layout's native units.
     * \returns point in layout units
     * \see convertFromLayoutUnits()
     * \see units()
     */
    QPointF convertToLayoutUnits( const QgsLayoutPoint &point ) const;

    /**
     * Converts a \a length measurement from the layout's native units to a specified target \a unit.
     * \returns length of measurement in specified units
     * \see convertToLayoutUnits()
     * \see units()
    */
    QgsLayoutMeasurement convertFromLayoutUnits( const double length, const QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Converts a \a size from the layout's native units to a specified target \a unit.
     * \returns size of measurement in specified units
     * \see convertToLayoutUnits()
     * \see units()
    */
    QgsLayoutSize convertFromLayoutUnits( const QSizeF &size, const QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Converts a \a point from the layout's native units to a specified target \a unit.
     * \returns point in specified units
     * \see convertToLayoutUnits()
     * \see units()
    */
    QgsLayoutPoint convertFromLayoutUnits( const QPointF &point, const QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Returns a reference to the layout's context, which stores information relating to the
     * current context and rendering settings for the layout.
     */
    QgsLayoutContext &context() { return mContext; }

    /**
     * Returns a reference to the layout's context, which stores information relating to the
     * current context and rendering settings for the layout.
     */
    SIP_SKIP const QgsLayoutContext &context() const { return mContext; }

    /**
     * Returns a reference to the layout's snapper, which stores handles layout snap grids and lines
     * and snapping points to the nearest matching point.
     */
    QgsLayoutSnapper &snapper() { return mSnapper; }

    /**
     * Returns a reference to the layout's snapper, which stores handles layout snap grids and lines
     * and snapping points to the nearest matching point.
     */
    SIP_SKIP const QgsLayoutSnapper &snapper() const { return mSnapper; }

    /**
     * Returns a reference to the layout's grid settings, which stores settings relating
     * to grid appearance, spacing and offsets.
     */
    QgsLayoutGridSettings &gridSettings() { return mGridSettings; }

    /**
     * Returns a reference to the layout's grid settings, which stores settings relating
     * to grid appearance, spacing and offsets.
     */
    SIP_SKIP const QgsLayoutGridSettings &gridSettings() const { return mGridSettings; }

    /**
     * Returns a reference to the layout's guide collection, which manages page snap guides.
     */
    QgsLayoutGuideCollection &guides();

    /**
     * Returns a reference to the layout's guide collection, which manages page snap guides.
     */
    SIP_SKIP const QgsLayoutGuideCollection &guides() const;

    /**
     * Creates an expression context relating to the layout's current state. The context includes
     * scopes for global, project, layout and layout context properties.
     */
    QgsExpressionContext createExpressionContext() const override;

    /**
     * Set a custom property for the layout.
     * \param key property key. If a property with the same key already exists it will be overwritten.
     * \param value property value
     * \see customProperty()
     * \see removeCustomProperty()
     * \see customProperties()
     */
    void setCustomProperty( const QString &key, const QVariant &value );

    /**
     * Read a custom property from the layout.
     * \param key property key
     * \param defaultValue default value to return if property with matching key does not exist
     * \returns value of matching property
     * \see setCustomProperty()
     * \see removeCustomProperty()
     * \see customProperties()
     */
    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;

    /**
     * Remove a custom property from the layout.
     * \param key property key
     * \see setCustomProperty()
     * \see customProperty()
     * \see customProperties()
     */
    void removeCustomProperty( const QString &key );

    /**
     * Return list of keys stored in custom properties for the layout.
     * \see setCustomProperty()
     * \see customProperty()
     * \see removeCustomProperty()
     */
    QStringList customProperties() const;

    /**
     * Returns the map item which will be used to generate corresponding world files when the
     * layout is exported. If no map was explicitly set via setReferenceMap(), the largest
     * map in the layout will be returned (or nullptr if there are no maps in the layout).
     * \see setReferenceMap()
     * \see generateWorldFile()
     */
    //TODO
    QgsLayoutItemMap *referenceMap() const;

    /**
     * Sets the \a map item which will be used to generate corresponding world files when the
     * layout is exported.
     * \see referenceMap()
     * \see setGenerateWorldFile()
     */
    //TODO
    void setReferenceMap( QgsLayoutItemMap *map );

    /**
     * Returns a pointer to the layout's page collection, which stores and manages
     * page items in the layout.
     */
    QgsLayoutPageCollection *pageCollection();

    /**
     * Returns a pointer to the layout's page collection, which stores and manages
     * page items in the layout.
     */
    SIP_SKIP const QgsLayoutPageCollection *pageCollection() const;

    /**
     * Calculates the bounds of all non-gui items in the layout. Ignores snap lines, mouse handles
     * and other cosmetic items.
     * \param ignorePages set to true to ignore page items
     * \param margin optional marginal (in percent, e.g., 0.05 = 5% ) to add around items
     * \returns layout bounds, in layout units.
     */
    QRectF layoutBounds( bool ignorePages = false, double margin = 0.0 ) const;

    /**
     * Adds an \a item to the layout. This should be called instead of the base class addItem()
     * method. Ownership of the item is transferred to the layout.
     */
    void addLayoutItem( QgsLayoutItem *item SIP_TRANSFER );

    /**
     * Removes an \a item from the layout. This should be called instead of the base class removeItem()
     * method.
     * The item will also be deleted.
     */
    void removeLayoutItem( QgsLayoutItem *item );

    /**
     * Returns the layout's state encapsulated in a DOM element.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets the collection's state from a DOM element. \a layoutElement is the DOM node corresponding to the layout.
     * \see writeXml()
     */
    bool readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );

    /**
     * Returns a pointer to the layout's undo stack, which manages undo/redo states for the layout
     * and it's associated objects.
     */
    QgsLayoutUndoStack *undoStack();

    /**
     * Returns a pointer to the layout's undo stack, which manages undo/redo states for the layout
     * and it's associated objects.
     */
    SIP_SKIP const QgsLayoutUndoStack *undoStack() const;

    QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id = 0, QUndoCommand *parent = nullptr ) SIP_FACTORY override;

    /**
     * Creates a new group from a list of layout \a items and adds the group to the layout.
     * If grouping was not possible, a nullptr will be returned.
     * \see ungroupItems()
     */
    QgsLayoutItemGroup *groupItems( const QList<QgsLayoutItem *> &items );

    /**
     * Ungroups items by removing them from an item \a group and removing the group from the
     * layout. Child items will remain in the layout and will not be deleted.
     *
     * Returns a list of the items removed from the group, or an empty list if ungrouping
     * was not successful.
     *
     * \see groupItems()
     */
    QList<QgsLayoutItem *> ungroupItems( QgsLayoutItemGroup *group );

  public slots:

    /**
     * Forces the layout, and all items contained within it, to refresh. For instance, this causes maps to redraw
     * and rebuild cached images, html items to reload their source url, and attribute tables
     * to refresh their contents. Calling this also triggers a recalculation of all data defined
     * attributes within the layout.
     *
     * \see refreshed()
     */
    void refresh();

    /**
     * Updates the scene bounds of the layout.
     */
    void updateBounds();

  signals:

    /**
     * Emitted whenever the expression variables stored in the layout have been changed.
     */
    void variablesChanged();

    /**
     * Emitted whenever the selected item changes.
     * If nullptr, no item is selected.
     */
    void selectedItemChanged( QgsLayoutItem *selected );

    /**
     * Is emitted when the layout has been refreshed and items should also be refreshed
     * and updated.
     */
    void refreshed();

  private:

    QgsProject *mProject = nullptr;
    std::unique_ptr< QgsLayoutModel > mItemsModel;

    QString mName;

    QgsObjectCustomProperties mCustomProperties;

    QgsUnitTypes::LayoutUnit mUnits = QgsUnitTypes::LayoutMillimeters;
    QgsLayoutContext mContext;
    QgsLayoutSnapper mSnapper;
    QgsLayoutGridSettings mGridSettings;

    std::unique_ptr< QgsLayoutPageCollection > mPageCollection;
    std::unique_ptr< QgsLayoutUndoStack > mUndoStack;
    QgsLayoutExporter mExporter;

    bool mBlockUndoCommands = false;

    //! Writes only the layout settings (not member settings like grid settings, etc) to XML
    void writeXmlLayoutSettings( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;
    //! Reads only the layout settings (not member settings like grid settings, etc) from XML
    bool readXmlLayoutSettings( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );

    /**
     * Adds a layout item to the layout, without adding the corresponding undo commands.
     */
    void addLayoutItemPrivate( QgsLayoutItem *item );

    /**
     * Removes an item from the layout, without adding the corresponding undo commands.
     */
    void removeLayoutItemPrivate( QgsLayoutItem *item );

    friend class QgsLayoutItemAddItemCommand;
    friend class QgsLayoutItemDeleteUndoCommand;
    friend class QgsLayoutItemUndoCommand;
    friend class QgsLayoutUndoCommand;
    friend class QgsLayoutItemGroupUndoCommand;
    friend class QgsLayoutModel;
};

#endif //QGSLAYOUT_H



