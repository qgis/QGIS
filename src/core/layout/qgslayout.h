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
#include "qgslayoutsnapper.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgslayoutgridsettings.h"
#include "qgslayoutguidecollection.h"
#include "qgslayoutexporter.h"
#include "qgsmasterlayoutinterface.h"
#include "qgssettingsentry.h"

class QgsLayoutItemMap;
class QgsLayoutModel;
class QgsLayoutMultiFrame;
class QgsLayoutPageCollection;
class QgsLayoutUndoStack;
class QgsLayoutRenderContext;
class QgsLayoutReportContext;

/**
 * \ingroup core
 * \class QgsLayout
 * \brief Base class for layouts, which can contain items such as maps, labels, scalebars, etc.
 *
 * While the raw QGraphicsScene API can be used to render the contents of a QgsLayout
 * to a QPainter, it is recommended to instead use a QgsLayoutExporter to handle rendering
 * layouts instead. QgsLayoutExporter automatically takes care of the intracacies of
 * preparing the layout and paint devices for correct exports, respecting various
 * user settings such as the layout context DPI.
 *
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

    //! Layout undo commands, used for collapsing undo commands
    enum UndoCommand
    {
      UndoLayoutDpi, //!< Change layout default DPI
      UndoNone = -1, //!< No command suppression
    };

    /**
     * Construct a new layout linked to the specified \a project.
     *
     * If the layout is a "new" layout (as opposed to a layout which will
     * restore a previous state from XML) then initializeDefaults() should be
     * called on the new layout.
     */
    QgsLayout( QgsProject *project );

    ~QgsLayout() override;

    /**
     * Creates a clone of the layout. Ownership of the return layout
     * is transferred to the caller.
     */
    QgsLayout *clone() const SIP_FACTORY;

    /**
     * Initializes an empty layout, e.g. by adding a default page to the layout. This should be called after creating
     * a new layout.
     */
    void initializeDefaults();

    /**
     * Clears the layout.
     *
     * Calling this method removes all items and pages from the layout.
     */
    void clear();

    /**
     * The project associated with the layout. Used to get access to layers, map themes,
     * relations and various other bits. It is never NULLPTR.
     *
     */
    QgsProject *project() const;

    /**
     * Returns the items model attached to the layout.
     */
    QgsLayoutModel *itemsModel();

    /**
     * Returns a list of layout items of a specific type.
     * \note not available in Python bindings
     */
    template<class T> void layoutItems( QList<T *> &itemList ) const SIP_SKIP
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
     * Returns a list of layout objects (items and multiframes) of a specific type.
     * \note not available in Python bindings
     */
    template<class T> void layoutObjects( QList<T *> &objectList ) const SIP_SKIP
    {
      objectList.clear();
      const QList<QGraphicsItem *> itemList( items() );
      const QList<QgsLayoutMultiFrame *> frameList( multiFrames() );
      for ( const auto &obj :  itemList )
      {
        T *item = dynamic_cast<T *>( obj );
        if ( item )
        {
          objectList.push_back( item );
        }
      }
      for ( const auto &obj :  frameList )
      {
        T *item = dynamic_cast<T *>( obj );
        if ( item )
        {
          objectList.push_back( item );
        }
      }
    }

    /**
     * Returns list of selected layout items.
     *
     * If \a includeLockedItems is set to TRUE, then locked items will also be included
     * in the returned list.
     */
    QList<QgsLayoutItem *> selectedLayoutItems( bool includeLockedItems = true );

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
     * Returns TRUE if the item was successfully raised.
     *
     * If \a deferUpdate is TRUE, the scene will not be visibly updated
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
     * Returns TRUE if the item was successfully lowered.
     *
     * If \a deferUpdate is TRUE, the scene will not be visibly updated
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
     * Returns TRUE if the item was successfully raised.
     *
     * If \a deferUpdate is TRUE, the scene will not be visibly updated
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
     * Returns TRUE if the item was successfully lowered.
     * If \a deferUpdate is TRUE, the scene will not be visibly updated
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
    void updateZValues( bool addUndoCommands = true );

    /**
     * Returns the layout item with matching \a uuid unique identifier, or NULLPTR
     * if a matching item could not be found.
     *
     * If \a includeTemplateUuids is TRUE, then item's template UUID
     * will also be tested when trying to match the uuid. This may differ from the item's UUID
     * for items which have been added to an existing layout from a template. In this case
     * the template UUID returns the original item UUID at the time the template was created,
     * vs the item's uuid() which returns the current instance of the item's unique identifier.
     * Note that template UUIDs are only available while a layout is being restored from XML.
     *
     * \see itemByTemplateUuid()
     * \see multiFrameByUuid()
     * \see itemById()
     */
    QgsLayoutItem *itemByUuid( const QString &uuid, bool includeTemplateUuids = false ) const;

    /**
     * Returns the layout item with matching template \a uuid unique identifier, or NULLPTR
     * if a matching item could not be found. Unlike itemByUuid(), this method ONLY checks
     * template UUIDs for a match.
     *
     * Template UUIDs are valid only for items which have been added to an existing layout from a template. In this case
     * the template UUID is the original item UUID at the time the template was created,
     * vs the item's uuid() which returns the current instance of the item's unique identifier.
     *
     * Note that template UUIDs are only available while a layout is being restored from XML.
     *
     * \see itemByUuid()
     * \see multiFrameByUuid()
     * \see itemById()
     */
    QgsLayoutItem *itemByTemplateUuid( const QString &uuid ) const;

    /**
     * Returns a layout item given its \a id.
     * Since item IDs are not necessarely unique, this function returns the first matching
     * item found.
     * \see itemByUuid()
     */
    QgsLayoutItem *itemById( const QString &id ) const;

    /**
     * Returns the layout multiframe with matching \a uuid unique identifier, or NULLPTR
     * if a matching multiframe could not be found.
     *
     * If \a includeTemplateUuids is TRUE, then the multiframe's QgsLayoutMultiFrame::templateUuid()
     * will also be tested when trying to match the uuid. Template UUIDs are valid only for items
     * which have been added to an existing layout from a template. In this case
     * the template UUID is the original item UUID at the time the template was created,
     * vs the item's uuid() which returns the current instance of the item's unique identifier.
     * Note that template UUIDs are only available while a layout is being restored from XML.
     *
     * \see itemByUuid()
     */
    QgsLayoutMultiFrame *multiFrameByUuid( const QString &uuid, bool includeTemplateUuids = false ) const;

    /**
     * Returns the topmost layout item at a specified \a position. Ignores paper items.
     * If \a ignoreLocked is set to TRUE any locked items will be ignored.
     */
    QgsLayoutItem *layoutItemAt( QPointF position, bool ignoreLocked = false ) const;

    /**
     * Returns the topmost layout item at a specified \a position which is below a specified \a item. Ignores paper items.
     * If \a ignoreLocked is set to TRUE any locked items will be ignored.
     */
    QgsLayoutItem *layoutItemAt( QPointF position, const QgsLayoutItem *belowItem, bool ignoreLocked = false ) const;

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
    double convertToLayoutUnits( QgsLayoutMeasurement measurement ) const;

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
    QgsLayoutMeasurement convertFromLayoutUnits( double length, QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Converts a \a size from the layout's native units to a specified target \a unit.
     * \returns size of measurement in specified units
     * \see convertToLayoutUnits()
     * \see units()
    */
    QgsLayoutSize convertFromLayoutUnits( QSizeF size, QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Converts a \a point from the layout's native units to a specified target \a unit.
     * \returns point in specified units
     * \see convertToLayoutUnits()
     * \see units()
    */
    QgsLayoutPoint convertFromLayoutUnits( QPointF point, QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Returns a reference to the layout's render context, which stores information relating to the
     * current rendering settings for the layout.
     */
    QgsLayoutRenderContext &renderContext();

    /**
     * Returns a reference to the layout's render context, which stores information relating to the
     * current rendering settings for the layout.
     */
    SIP_SKIP const QgsLayoutRenderContext &renderContext() const;

    /**
     * Returns a reference to the layout's report context, which stores information relating to the
     * current reporting context for the layout.
     */
    QgsLayoutReportContext &reportContext();

    /**
     * Returns a reference to the layout's report context, which stores information relating to the
     * current reporting context for the layout.
     */
    SIP_SKIP const QgsLayoutReportContext &reportContext() const;

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
     * Refreshes the layout when global layout related options change.
     */
    void reloadSettings();

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
     * Returns list of keys stored in custom properties for the layout.
     * \see setCustomProperty()
     * \see customProperty()
     * \see removeCustomProperty()
     */
    QStringList customProperties() const;

    /**
     * Returns the map item which will be used to generate corresponding world files when the
     * layout is exported. If no map was explicitly set via setReferenceMap(), the largest
     * map in the layout will be returned (or NULLPTR if there are no maps in the layout).
     * \see setReferenceMap()
     */
    QgsLayoutItemMap *referenceMap() const;

    /**
     * Sets the \a map item which will be used to generate corresponding world files when the
     * layout is exported.
     * \see referenceMap()
     */
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
     * \param ignorePages set to TRUE to ignore page items
     * \param margin optional marginal (in percent, e.g., 0.05 = 5% ) to add around items
     * \returns layout bounds, in layout units.
     *
     * \see pageItemBounds()
     */
    QRectF layoutBounds( bool ignorePages = false, double margin = 0.0 ) const;

    /**
     * Returns the bounding box of the items contained on a specified \a page.
     * A page number of 0 represents the first page in the layout.
     *
     * Set \a visibleOnly to TRUE to only include visible items.
     *
     * The returned bounds are in layout units.
     *
     * \see layoutBounds()
     */
    QRectF pageItemBounds( int page, bool visibleOnly = false ) const;

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
     * Adds a \a multiFrame to the layout. The object is owned by the layout until removeMultiFrame() is called.
     * \see removeMultiFrame()
     * \see multiFrames()
     */
    void addMultiFrame( QgsLayoutMultiFrame *multiFrame SIP_TRANSFER );

    /**
     * Removes a \a multiFrame from the layout (but does not delete it).
     * \see addMultiFrame()
     * \see multiFrames()
     */
    void removeMultiFrame( QgsLayoutMultiFrame *multiFrame );

    /**
     * Returns a list of multi frames contained in the layout.
     * \see addMultiFrame()
     * \see removeMultiFrame()
     */
    QList< QgsLayoutMultiFrame * > multiFrames() const;

    /**
     * Saves the layout as a template at the given file \a path.
     * Returns TRUE if save was successful.
     * \see loadFromTemplate()
     */
    bool saveAsTemplate( const QString &path, const QgsReadWriteContext &context ) const;

    /**
     * Load a layout template \a document.
     *
     * By default this method will clear all items from the existing layout and real all layout
     * settings from the template. Setting \a clearExisting to FALSE will only add new items
     * from the template, without overwriting the existing items or layout settings.
     *
     * If \a ok is specified, it will be set to TRUE if the load was successful.
     *
     * Returns a list of loaded items.
     */
    QList< QgsLayoutItem * > loadFromTemplate( const QDomDocument &document, const QgsReadWriteContext &context, bool clearExisting = true, bool *ok SIP_OUT = nullptr );

    /**
     * Returns the layout's state encapsulated in a DOM element.
     * \see readXml()
     */
    virtual QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets the collection's state from a DOM element. \a layoutElement is the DOM node corresponding to the layout.
     * \see writeXml()
     */
    virtual bool readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context );

    /**
     * Add items from an XML representation to the layout. Used for project file reading and pasting items from clipboard.
     *
     * The \a position argument is optional, and if it is not specified the items will be restored to their
     * original position from the XML serialization. If specified, the items will be positioned such that the top-left
     * bounds of all added items is located at this \a position.
     *
     * The \a pasteInPlace argument determines whether the serialized position should be respected, but remapped to the
     * origin of the page corresponding to the page at \a position.
     *
     * A list of the newly added items is returned.
     */
    QList< QgsLayoutItem * > addItemsFromXml( const QDomElement &parentElement, const QDomDocument &document,
        const QgsReadWriteContext &context,
        QPointF *position = nullptr, bool pasteInPlace = false );

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
     * If grouping was not possible, NULLPTR will be returned.
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

    /**
     * Accepts the specified style entity \a visitor, causing it to visit all style entities associated
     * with the layout.
     *
     * Returns TRUE if the visitor should continue visiting other objects, or FALSE if visiting
     * should be canceled.
     *
     * \since QGIS 3.10
     */
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const;

#ifndef SIP_RUN
    //! Settings entry search path for templates
    static const inline QgsSettingsEntryStringList settingsSearchPathForTemplates = QgsSettingsEntryStringList( QStringLiteral( "searchPathsForTemplates" ), QgsSettings::Prefix::CORE_LAYOUT, QStringList(), QObject::tr( "Search path for templates" ) );
#endif

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
     * Emitted when properties of the layout change. This signal is only
     * emitted for settings directly managed by the layout, and is not emitted
     * when child items change.
     */
    void changed();

    /**
     * Emitted whenever the expression variables stored in the layout have been changed.
     */
    void variablesChanged();

    /**
     * Emitted whenever the selected item changes.
     * If NULLPTR, no item is selected.
     */
    void selectedItemChanged( QgsLayoutItem *selected );

    /**
     * Emitted when the layout has been refreshed and items should also be refreshed
     * and updated.
     */
    void refreshed();

    /**
     * Emitted whenever the \a total number of background tasks running in items from the layout changes.
     *
     * \since QGIS 3.10
     */
    void backgroundTaskCountChanged( int total );

    /**
     * Emitted when an \a item was added to the layout.
     *
     * \since QGIS 3.20
     */
    void itemAdded( QgsLayoutItem *item );

  private slots:
    void itemBackgroundTaskCountChanged( int count );

  private:

    QgsProject *mProject = nullptr;
    std::unique_ptr< QgsLayoutModel > mItemsModel;

    QgsObjectCustomProperties mCustomProperties;

    QgsUnitTypes::LayoutUnit mUnits = QgsUnitTypes::LayoutMillimeters;
    QgsLayoutRenderContext *mRenderContext = nullptr;
    QgsLayoutReportContext *mReportContext = nullptr;
    QgsLayoutSnapper mSnapper;
    QgsLayoutGridSettings mGridSettings;

    std::unique_ptr< QgsLayoutPageCollection > mPageCollection;
    std::unique_ptr< QgsLayoutUndoStack > mUndoStack;

    //! List of multiframe objects
    QList<QgsLayoutMultiFrame *> mMultiFrames;

    //! Item ID for layout map to use for the world file generation
    QString mWorldFileMapId;

    QHash< QgsLayoutItem *, int > mBackgroundTaskCount;

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

    void deleteAndRemoveMultiFrames();

    //! Calculates the item minimum position from an XML element
    QPointF minPointFromXml( const QDomElement &elem ) const;

    QgsLayout( const QgsLayout & ) = delete;
    QgsLayout &operator=( const QgsLayout & ) = delete;

    friend class QgsLayoutItemAddItemCommand;
    friend class QgsLayoutItemDeleteUndoCommand;
    friend class QgsLayoutItemUndoCommand;
    friend class QgsLayoutUndoCommand;
    friend class QgsLayoutItemGroupUndoCommand;
    friend class QgsLayoutModel;
    friend class QgsLayoutMultiFrame;
    friend class QgsCompositionConverter;
};

#endif //QGSLAYOUT_H
