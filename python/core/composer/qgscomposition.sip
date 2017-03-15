class QgsComposition : QGraphicsScene, QgsExpressionContextGenerator
{
%TypeHeaderCode
#include <qgscomposition.h>
%End

  public:

    enum PlotStyle
    {
      Preview,
      Print,
      Postscript
    };

    enum GridStyle
    {
      Solid,
      Dots,
      Crosses
    };

    enum ZValueDirection
    {
      ZValueBelow,
      ZValueAbove
    };

    enum PaperOrientation
    {
      Portrait,
      Landscape
    };

    explicit QgsComposition( QgsProject *project );

    enum AtlasMode
    {
      AtlasOff,
      PreviewAtlas,
      ExportAtlas
    };

    ~QgsComposition();

    QgsProject *project() const;
    QString name() const;
    void setName( const QString &name );
    void setPaperSize( double width, double height,
                       bool keepRelativeItemPosition = true );
    double paperHeight() const;
    double paperWidth() const;
    void resizePageToContents( double marginTop = 0.0, double marginRight = 0.0,
                               double marginBottom = 0.0, double marginLeft = 0.0 );
    void setResizeToContentsMargins( double marginTop, double marginRight,
                                     double marginBottom, double marginLeft );
    void resizeToContentsMargins( double &marginTop /Out/, double &marginRight /Out/,
                                  double &marginBottom /Out/, double &marginLeft /Out/ ) const;
    double spaceBetweenPages() const;
    void setNumPages( const int pages );
    int numPages() const;
    bool pageIsEmpty( const int page ) const;
    bool shouldExportPage( const int page ) const;
    void setPageStyleSymbol( QgsFillSymbol *symbol );
    QgsFillSymbol *pageStyleSymbol();
    QPointF positionOnPage( QPointF position ) const;
    int pageNumberForPoint( QPointF position ) const;
    void setStatusMessage( const QString &message );
    void updateSettings();

    void setSnapToGridEnabled( const bool b );
    bool snapToGridEnabled() const;

    void setGridVisible( const bool b );
    bool gridVisible() const;

    void setSnapLinesVisible( const bool visible );
    bool snapLinesVisible() const;

    void setAlignmentSnap( const bool s );
    bool alignmentSnap() const;

    void setSmartGuidesEnabled( const bool b );
    bool smartGuidesEnabled() const;

    /** Sets whether the page items should be visible in the composition. Removing
     * them will prevent both display of the page boundaries in composer views and
     * will also prevent them from being rendered in composition exports.
     * @param visible set to true to show pages, false to hide pages
     * @note added in QGIS 2.12
     * @see pagesVisible()
     */
    void setPagesVisible( bool visible );

    /** Returns whether the page items are be visible in the composition. This setting
     * effects both display of the page boundaries in composer views and
     * whether they will be rendered in composition exports.
     * @note added in QGIS 2.12
     * @see setPagesVisible()
     */
    bool pagesVisible() const;

    /** Removes all snap lines*/
    void clearSnapLines();

    void setSnapGridResolution( const double r );
    double snapGridResolution() const;

    void setSnapGridOffsetX( const double offset );
    double snapGridOffsetX() const;

    void setSnapGridOffsetY( const double offset );
    double snapGridOffsetY() const;

    void setGridPen( const QPen& p );
    QPen gridPen() const;

    void setGridStyle( const GridStyle s );
    GridStyle gridStyle() const;

    /** Sets the snap tolerance to use when automatically snapping items during movement and resizing to guides
     * and the edges and centers of other items.
     * @param snapTolerance snap tolerance in pixels
     * @see alignmentSnapTolerance
     * @note Added in QGIS 2.5
     */
    void setSnapTolerance( const int snapTolerance );

    /** Returns the snap tolerance to use when automatically snapping items during movement and resizing to guides
     * and the edges and centers of other items.
     * @returns snap tolerance in pixels
     * @see setAlignmentSnapTolerance
     * @note Added in QGIS 2.5
     */
    int snapTolerance() const;

    /** Sets whether selection bounding boxes should be shown in the composition
     * @param boundsVisible set to true to show selection bounding box
     * @see boundingBoxesVisible
     * @note added in QGIS 2.7
     */
    void setBoundingBoxesVisible( const bool boundsVisible );

    /** Returns whether selection bounding boxes should be shown in the composition
     * @returns true if selection bounding boxes should be shown
     * @see setBoundingBoxesVisible
     * @note added in QGIS 2.7
     */
    bool boundingBoxesVisible() const;

    /** Returns pointer to undo/redo command storage*/
    QUndoStack* undoStack();

    /** Returns the topmost composer item at a specified position. Ignores paper items.
     * @param position point to search for item at
     * @param ignoreLocked set to true to ignore locked items
     * @returns composer item at position
     */
    QgsComposerItem* composerItemAt( QPointF position, const bool ignoreLocked = false ) const;

    /** Returns the topmost composer item at a specified position which is below a specified item. Ignores paper items.
     * @param position point to search for item at
     * @param belowItem item to search below
     * @param ignoreLocked set to true to ignore locked items
     * @returns composer item at position which is below specified item
     */
    QgsComposerItem* composerItemAt( QPointF position, const QgsComposerItem* belowItem, const bool ignoreLocked = false ) const;

    /** Returns the page number (0-based) given a coordinate */
    int pageNumberAt( QPointF position ) const;

    /** Returns on which page number (0-based) is displayed an item */
    int itemPageNumber( const QgsComposerItem* ) const;

    /** Returns list of selected composer items
     * @param includeLockedItems set to true to include locked items in list
     * @returns list of selected items
     */
    QList<QgsComposerItem*> selectedComposerItems( const bool includeLockedItems = true );

    /** Returns pointers to all composer maps in the scene
     * @note available in python bindings only with PyQt >= 4.8.4
     */
    QList<const QgsComposerMap*> composerMapItems() const;

    /** Return composer items of a specific type
     * @param itemList list of item type to store matching items in
     * @note not available in python bindings
     */
    // template<class T> void composerItems( QList<T*>& itemList );

    /** Return composer items of a specific type on a specified page
     * @param itemList list of item type to store matching items in
     * @param pageNumber page number (0 based)
     * @note not available in python bindings
     * @note added in QGIS 2.5
     */
    //template<class T> void composerItemsOnPage( QList<T*>& itemList, const int pageNumber );

    /** Returns the composer map with specified id
     * @return QgsComposerMap or 0 pointer if the composer map item does not exist
     */
    const QgsComposerMap* getComposerMapById( const int id ) const;

    /** Returns a composer item given its text identifier.
     *  Ids are not necessarely unique, but this function returns only one element.
     * @param id - A QString representing the identifier of the item to retrieve.
     * @return QgsComposerItem pointer or 0 pointer if no such item exists.
     */
    const QgsComposerItem* getComposerItemById( const QString& id ) const;

    /** Returns a composer item given its unique identifier.
     * @param uuid A QString representing the UUID of the item to
     */
    const QgsComposerItem* getComposerItemByUuid( const QString& uuid ) const;

    int printResolution() const;
    void setPrintResolution( const int dpi );

    bool printAsRaster() const;
    void setPrintAsRaster( const bool enabled );

    /** Returns true if the composition will generate corresponding world files when pages
     * are exported.
     * @see setGenerateWorldFile()
     * @see worldFileMap()
     */
    bool generateWorldFile() const;

    /** Sets whether the composition will generate corresponding world files when pages
     * are exported.
     * @param enabled set to true to generate world files
     * @see generateWorldFile()
     * @see setWorldFileMap()
     */
    void setGenerateWorldFile( bool enabled );

    QgsComposerMap* referenceMap() const;

    void setReferenceMap( QgsComposerMap* map );

    /** Returns true if a composition should use advanced effects such as blend modes */
    bool useAdvancedEffects() const;
    /** Used to enable or disable advanced effects such as blend modes in a composition */
    void setUseAdvancedEffects( const bool effectsEnabled );

    QgsComposition::PlotStyle plotStyle() const;
    void setPlotStyle( const QgsComposition::PlotStyle style );

    /** Writes settings to xml (paper dimension)*/
    bool writeXml( QDomElement& composerElem, QDomDocument& doc );

    /** Reads settings from xml file*/
    bool readXml( const QDomElement& compositionElem, const QDomDocument& doc );

    /** Load a template document
     * @param doc template document
     * @param substitutionMap map with text to replace. Text needs to be enclosed by brackets (e.g. '[text]' )
     * @param addUndoCommands whether or not to add undo commands
     * @param clearComposition set to true to clear the existing composition and read all composition and
     * atlas properties from the template. Set to false to only add new items from the template, without
     * overwriting the existing items or composition settings.
     */
    bool loadFromTemplate( const QDomDocument& doc, QMap<QString, QString>* substitutionMap = 0,
                           bool addUndoCommands = false, const bool clearComposition = true );

    /** Add items from XML representation to the graphics scene (for project file reading, pasting items from clipboard)
     * @param elem items parent element, e.g. \verbatim <Composer> \endverbatim or \verbatim <ComposerItemClipboard> \endverbatim
     * @param doc xml document
     * @param mapsToRestore for reading from project file: set preview move 'rectangle' to all maps and save the preview states to show composer maps on demand
     * @param addUndoCommands insert AddItem commands if true (e.g. for copy/paste)
     * @param pos item position. Optional, take position from xml if 0
     * @param pasteInPlace whether the position should be kept but mapped to the page origin. (the page is the page under to the mouse cursor)
     * @note parameters mapsToRestore, addUndoCommands pos and pasteInPlace not available in python bindings
     */
    void addItemsFromXml( const QDomElement& elem, const QDomDocument& doc );

    /** Adds item to z list. Usually called from constructor of QgsComposerItem*/
    void addItemToZList( QgsComposerItem* item );
    /** Removes item from z list. Usually called from destructor of QgsComposerItem*/
    void removeItemFromZList( QgsComposerItem* item );

    //functions to move selected items in hierarchy
    void raiseSelectedItems();

    //returns true if successful
    bool raiseItem( QgsComposerItem* item );
    void lowerSelectedItems();
    //returns true if successful
    bool lowerItem( QgsComposerItem* item );
    void moveSelectedItemsToTop();
    //returns true if successful
    bool moveItemToTop( QgsComposerItem* item );
    void moveSelectedItemsToBottom();
    //returns true if successful
    bool moveItemToBottom( QgsComposerItem* item );

    //functions to find items by their position in the z list
    void selectNextByZOrder( const ZValueDirection direction );
    QgsComposerItem* getComposerItemBelow( QgsComposerItem* item ) const;
    QgsComposerItem* getComposerItemAbove( QgsComposerItem* item ) const;

    //functions to align selected items
    void alignSelectedItemsLeft();
    void alignSelectedItemsHCenter();
    void alignSelectedItemsRight();
    void alignSelectedItemsTop();
    void alignSelectedItemsVCenter();
    void alignSelectedItemsBottom();

    //functions to lock and unlock items
    /** Lock the selected items*/
    void lockSelectedItems();
    /** Unlock all items*/
    void unlockAllItems();

    /** Creates a new group from a list of composer items and adds it to the composition.
     * @param items items to include in group
     * @returns QgsComposerItemGroup of grouped items, if grouping was possible
     * @note added in QGIS 2.6
     */
    QgsComposerItemGroup* groupItems( QList<QgsComposerItem*> items );

    /** Ungroups items by removing them from an item group and removing the group from the
     * composition.
     * @param group item group to ungroup
     * @returns list of items removed from the group, or an empty list if ungrouping
     * was not successful
     * @note added in QGIS 2.6
     */
    QList<QgsComposerItem*> ungroupItems( QgsComposerItemGroup* group );

    /** Rebuilds the z order list by adding any item which are present in the composition
     * but missing from the z order list.
     */
    void refreshZList();

    /** Snaps a scene coordinate point to grid*/
    QPointF snapPointToGrid( QPointF scenePoint ) const;

    /** Returns pointer to snap lines collection*/
    QList< QGraphicsLineItem* >* snapLines();

    /** Returns pointer to selection handles
     * @note not available in python bindings
     */
    // QgsComposerMouseHandles* selectionHandles();

    /** Add a custom snap line (can be horizontal or vertical)*/
    QGraphicsLineItem* addSnapLine();
    /** Remove custom snap line (and delete the object)*/
    void removeSnapLine( QGraphicsLineItem* line );
    /** Get nearest snap line
     * @note not available in python bindings
     */
    // QGraphicsLineItem* nearestSnapLine( const bool horizontal, const double x, const double y, const double tolerance, QList< QPair< QgsComposerItem*, QgsComposerItem::ItemPositionMode > >& snappedItems );

    /** Allocates new item command and saves initial state in it
     * @param item target item
     * @param commandText descriptive command text
     * @param c context for merge commands (unknown for non-mergeable commands)
     */
    void beginCommand( QgsComposerItem* item, const QString& commandText, const QgsComposerMergeCommand::Context c = QgsComposerMergeCommand::Unknown );

    /** Saves end state of item and pushes command to the undo history*/
    void endCommand();
    /** Deletes current command*/
    void cancelCommand();

    void beginMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text, const QgsComposerMultiFrameMergeCommand::Context c = QgsComposerMultiFrameMergeCommand::Unknown );
    void endMultiFrameCommand();
    /** Deletes current multi frame command*/
    void cancelMultiFrameCommand();

    /** Adds multiframe. The object is owned by QgsComposition until removeMultiFrame is called*/
    void addMultiFrame( QgsComposerMultiFrame* multiFrame );
    /** Removes multi frame (but does not delete it)*/
    void removeMultiFrame( QgsComposerMultiFrame* multiFrame );
    /** Adds an arrow item to the graphics scene and advises composer to create a widget for it (through signal)
      @note not available in python bindings*/
    void addComposerArrow( QgsComposerArrow* arrow );
    /** Adds label to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerLabel( QgsComposerLabel* label );
    /** Adds map to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerMap( QgsComposerMap* map, const bool setDefaultPreviewStyle = true );
    /** Adds scale bar to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerScaleBar( QgsComposerScaleBar* scaleBar );
    /** Adds legend to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerLegend( QgsComposerLegend* legend );
    /** Adds picture to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerPicture( QgsComposerPicture* picture );
    /** Adds a composer shape to the graphics scene and advises composer to create a widget for it (through signal)*/
    void addComposerShape( QgsComposerShape* shape );
    /** Adds a composer polygon and advises composer to create a widget for it (through signal)*/
    void addComposerPolygon( QgsComposerPolygon* polygon );
    /** Adds a composer polyline and advises composer to create a widget for it (through signal)*/
    void addComposerPolyline( QgsComposerPolyline* polyline );
    /** Adds composer html frame and advises composer to create a widget for it (through signal)*/
    void addComposerHtmlFrame( QgsComposerHtml* html /Transfer/, QgsComposerFrame* frame /Transfer/);
    /** Adds composer tablev2 frame and advises composer to create a widget for it (through signal)*/
    void addComposerTableFrame( QgsComposerAttributeTableV2* table /Transfer/, QgsComposerFrame* frame /Transfer/);

    /** Remove item from the graphics scene. Additionally to QGraphicsScene::removeItem, this function considers undo/redo command*/
    void removeComposerItem( QgsComposerItem* item, const bool createCommand = true, const bool removeGroupItems = true );

    /** Convenience function to create a QgsAddRemoveItemCommand, connect its signals and push it to the undo stack*/
    void pushAddRemoveCommand( QgsComposerItem* item, const QString& text, const QgsAddRemoveItemCommand::State state = QgsAddRemoveItemCommand::Added );

    /** If true, prevents any mouse cursor changes by the composition or by any composer items
     * Used by QgsComposer and QgsComposerView to prevent unwanted cursor changes
     */
    void setPreventCursorChange( const bool preventChange );
    bool preventCursorChange() const;

    //printing

    /** Prepare the printer for printing */
    void beginPrint( QPrinter& printer, const bool evaluateDDPageSize = false );
    /** Prepare the printer for printing in a PDF */
    void beginPrintAsPDF( QPrinter& printer, const QString& file );

    /** Print on a preconfigured printer
     * @param printer QPrinter destination
     * @param painter QPainter source
     * @param startNewPage set to true to begin the print on a new page
     */
    void doPrint( QPrinter& printer, QPainter& painter, bool startNewPage = false );

    /** Convenience function that prepares the printer and prints
     * @returns true if print was successful
     */
    bool print( QPrinter &printer, const bool evaluateDDPageSize = false );

    /** Convenience function that prepares the printer for printing in PDF and prints
     * @returns true if export was successful
     */
    bool exportAsPDF( const QString& file );

    /** Renders a composer page to an image.
     * @param page page number, 0 based such that the first page is page 0
     * @param imageSize optional target image size, in pixels. It is the caller's responsibility
     * to ensure that the ratio of the target image size matches the ratio of the composition
     * page size.
     * @param dpi optional dpi override, or 0 to use default composition print resolution. This
     * parameter has no effect if imageSize is specified.
     * @returns rendered image, or null image if image does not fit into available memory
     * @see renderRectAsRaster()
     * @see renderPage()
     */
    QImage printPageAsRaster( int page, QSize imageSize = QSize(), int dpi = 0 );

    /** Renders a portion of the composition to an image. This method can be used to render
     * sections of pages rather than full pages.
     * @param rect region of composition to render
     * @param imageSize optional target image size, in pixels. It is the caller's responsibility
     * to ensure that the ratio of the target image size matches the ratio of the specified
     * region of the composition.
     * @param dpi optional dpi override, or 0 to use default composition print resolution. This
     * parameter has no effect if imageSize is specified.
     * @returns rendered image, or null image if image does not fit into available memory
     * @note added in QGIS 2.12
     * @see printPageAsRaster()
     * @see renderRect()
     */
    QImage renderRectAsRaster( const QRectF& rect, QSize imageSize = QSize(), int dpi = 0 );

    /** Renders a full page to a paint device.
     * @param p destination painter
     * @param page page number, 0 based such that the first page is page 0
     * @see renderRect()
     * @see printPageAsRaster()
     */
    void renderPage( QPainter* p, int page );

    /** Renders a portion of the composition to a paint device. This method can be used
     * to render sections of pages rather than full pages.
     * @param p destination painter
     * @param rect region of composition to render
     * @note added in QGIS 2.12
     * @see renderPage()
     * @see renderRectAsRaster()
     */
    void renderRect( QPainter* p, const QRectF& rect );

    /** Georeferences a file (image of PDF) exported from the composition.
     * @param file filename of exported file
     * @param referenceMap map item to use for georeferencing, or leave as nullptr to use the
     * currently defined worldFileMap().
     * @param exportRegion set to a valid rectangle to indicate that only part of the composition was
     * exported
     * @param dpi set to DPI of exported file, or leave as -1 to use composition's DPI.
     * @note added in QGIS 2.16
     */
    void georeferenceOutput( const QString& file, QgsComposerMap* referenceMap = nullptr,
                             const QRectF& exportRegion = QRectF(), double dpi = -1 ) const;

    /** Compute world file parameters. Assumes the whole page containing the associated map item
     * will be exported.
     */
    void computeWorldFileParameters( double& a, double& b, double& c, double& d, double& e, double& f ) const;

    /** Computes the world file parameters for a specified region of the composition.
     * @param exportRegion region of the composition which will be associated with world file
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @note added in QGIS 2.12
     */
    void computeWorldFileParameters( const QRectF& exportRegion, double& a, double& b, double& c, double& d, double& e, double& f ) const;

    QgsAtlasComposition& atlasComposition();

    /** Returns the current atlas mode of the composition
     * @returns current atlas mode
     * @see setAtlasMode
     */
    QgsComposition::AtlasMode atlasMode() const;

    /** Sets the current atlas mode of the composition.
     * @param mode atlas mode to switch to
     * @returns false if the mode could not be changed.
     * @see atlasMode
     */
    bool setAtlasMode( const QgsComposition::AtlasMode mode );

    /** Return pages in the correct order
     * @note composerItems(QList< QgsPaperItem* > &) may not return pages in the correct order
     * @note added in version 2.4
     */
    QList< QgsPaperItem* > pages();

    /** Returns the items model attached to the composition
     * @returns QgsComposerModel for composition
     * @note this method was added in version 2.5
     */
    QgsComposerModel * itemsModel();

    /** Set a custom property for the composition.
     * @param key property key. If a property with the same key already exists it will be overwritten.
     * @param value property value
     * @see customProperty()
     * @see removeCustomProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    void setCustomProperty( const QString &key, const QVariant &value );

    /** Read a custom property from the composition.
     * @param key property key
     * @param defaultValue default value to return if property with matching key does not exist
     * @returns value of matching property
     * @see setCustomProperty()
     * @see removeCustomProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;

    /** Remove a custom property from the composition.
     * @param key property key
     * @see setCustomProperty()
     * @see customProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    void removeCustomProperty( const QString &key );

    /** Return list of keys stored in custom properties for composition.
     * @see setCustomProperty()
     * @see customProperty()
     * @see removeCustomProperty()
     * @note added in QGIS 2.12
     */
    QStringList customProperties() const;

    /** Returns the bounding box of the items contained on a specified page.
     * @param pageNumber page number, where 0 is the first page
     * @param visibleOnly set to true to only include visible items
     * @note added in QGIS 2.12
     */
    QRectF pageItemBounds( int pageNumber, bool visibleOnly = false ) const;

    /** Calculates the bounds of all non-gui items in the composition. Ignores snap lines and mouse handles.
     * @param ignorePages set to true to ignore page items
     * @param margin optional marginal (in percent, e.g., 0.05 = 5% ) to add around items
     */
    QRectF compositionBounds( bool ignorePages = false, double margin = 0.0 ) const;

  public slots:
    /** Casts object to the proper subclass type and calls corresponding itemAdded signal*/
    void sendItemAddedSignal( QgsComposerItem* item );

    /** Updates the scene bounds of the composition
    @note added in version 2.2*/
    void updateBounds();

    /** Forces items in the composition to refresh. For instance, this causes maps to redraw
     * and rebuild cached images, html items to reload their source url, and attribute tables
     * to refresh their contents. Calling this also triggers a recalculation of all data defined
     * attributes within the composition.
     * @note added in version 2.3*/
    void refreshItems();

    /** Clears any selected items and sets an item as the current selection.
     * @param item item to set as selected
     * @note added in version 2.3*/
    void setSelectedItem( QgsComposerItem* item );

    /** Clears any selected items in the composition. Call this method rather than
     * QGraphicsScene::clearSelection, as the latter does not correctly emit signals to allow
     * the composition's model to update.
     * @note added in version 2.5*/
    void setAllDeselected();

    /** Refreshes a data defined property for the composition by reevaluating the property's value
     * and redrawing the composition with this new value.
     * @param property data defined property to refresh. If property is set to
     * QgsComposerItem::AllProperties then all data defined properties for the composition will be
     * refreshed.
     * @param context expression context for evaluating data defined expressions
     * @note this method was added in version 2.5
     */
    void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext* context = 0 );

    /** Creates an expression context relating to the compositions's current state. The context includes
     * scopes for global, project, composition and atlas properties.
     * @note added in QGIS 2.12
     */
    QgsExpressionContext createExpressionContext() const;

    /** Returns a reference to the composition's property collection, used for data defined overrides.
     * @note added in QGIS 3.0
     * @see setDataDefinedProperties()
     */
    QgsPropertyCollection& dataDefinedProperties();

    /** Returns a reference to the composition's property collection, used for data defined overrides.
     * @note added in QGIS 3.0
     * @see setDataDefinedProperties()
     */
    //const QgsPropertyCollection& dataDefinedProperties() const;

    /** Sets the composition's property collection, used for data defined overrides.
     * @param collection property collection. Existing properties will be replaced.
     * @note added in QGIS 3.0
     * @see dataDefinedProperties()
     */
    void setDataDefinedProperties( const QgsPropertyCollection& collection );

  protected:
    void init();


  signals:
    void nameChanged( const QString& name );
    void paperSizeChanged();
    void nPagesChanged();
    void printResolutionChanged();

    /** Is emitted when selected item changed. If 0, no item is selected*/
    void selectedItemChanged( QgsComposerItem* selected );
    /** Is emitted when new composer arrow has been added to the view*/
    void composerArrowAdded( QgsComposerArrow* arrow );
    /** Is emitted when new composer polygon has been added to the view*/
    void composerPolygonAdded( QgsComposerPolygon* polygon );
    /** Is emitted when new composer polyline has been added to the view*/
    void composerPolylineAdded( QgsComposerPolyline* polyline );
    /** Is emitted when a new composer html has been added to the view*/
    void composerHtmlFrameAdded( QgsComposerHtml* html, QgsComposerFrame* frame );
    /** Is emitted when a new item group has been added to the view*/
    void composerItemGroupAdded( QgsComposerItemGroup* group );
    /** Is emitted when new composer label has been added to the view*/
    void composerLabelAdded( QgsComposerLabel* label );
    /** Is emitted when new composer map has been added to the view*/
    void composerMapAdded( QgsComposerMap* map );
    /** Is emitted when new composer scale bar has been added*/
    void composerScaleBarAdded( QgsComposerScaleBar* scalebar );
    /** Is emitted when a new composer legend has been added*/
    void composerLegendAdded( QgsComposerLegend* legend );
    /** Is emitted when a new composer picture has been added*/
    void composerPictureAdded( QgsComposerPicture* picture );
    /** Is emitted when a new composer shape has been added*/
    void composerShapeAdded( QgsComposerShape* shape );
    /** Is emitted when a new composer table frame has been added to the view*/
    void composerTableFrameAdded( QgsComposerAttributeTableV2* table, QgsComposerFrame* frame );
    /** Is emitted when a composer item has been removed from the scene*/
    void itemRemoved( QgsComposerItem* );

    /** Is emitted when item in the composition must be refreshed*/
    void refreshItemsTriggered();

    /** Is emitted when the composition has an updated status bar message for the composer window*/
    void statusMsgChanged( QString message );

    /** Emitted whenever the expression variables stored in the composition have been changed.
     * @note added in QGIS 3.0
     */
    void variablesChanged();
};
