/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposition.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposition : QGraphicsScene, QgsExpressionContextGenerator
{
%Docstring
 Graphics scene for map printing. The class manages the paper item which always
 is the item in the back (z-value 0). It maintains the z-Values of the items and stores
 them in a list in ascending z-Order. This list can be changed to lower/raise items one position
 or to bring them to front/back.
 *
%End

%TypeHeaderCode
#include "qgscomposition.h"
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
%Docstring
 Construct a new composition linked to the specified project.
%End

    enum AtlasMode
    {
      AtlasOff,
      PreviewAtlas,
      ExportAtlas
    };

    ~QgsComposition();

    QgsProject *project() const;
%Docstring
 The project associated with the composition. Used to get access to layers, map themes,
 relations and various other bits. It is never null.

.. versionadded:: 3.0
 :rtype: QgsProject
%End

    QString name() const;
%Docstring
 Returns the composition's name.
.. seealso:: setName()
.. versionadded:: 3.0
 :rtype: str
%End

    void setName( const QString &name );
%Docstring
 Sets the composition's name.
.. seealso:: name()
.. versionadded:: 3.0
%End

    void setPaperSize( double width, double height,
                       bool keepRelativeItemPosition = true );
%Docstring
 Changes size of paper item.
 \param width page width in mm
 \param height page height in mm
 \param keepRelativeItemPosition if true, all items and guides will be moved so that they retain
 their same relative position to the top left corner of their current page.
.. seealso:: paperHeight
.. seealso:: paperWidth
%End

    double paperHeight() const;
%Docstring
 Height of paper item
 :return: height in mm
.. seealso:: paperWidth
.. seealso:: setPaperSize
 :rtype: float
%End

    double paperWidth() const;
%Docstring
 Width of paper item
 :return: width in mm
.. seealso:: paperHeight
.. seealso:: setPaperSize
 :rtype: float
%End

    void resizePageToContents( double marginTop = 0.0, double marginRight = 0.0,
                               double marginBottom = 0.0, double marginLeft = 0.0 );
%Docstring
 Resizes the composition page to fit the current contents of the composition.
 Calling this method resets the number of pages to 1, with the size set to the
 minimum size required to fit all existing composer items. Items will also be
 repositioned so that the new top-left bounds of the composition is at the point
 (marginLeft, marginTop). An optional margin can be specified.
 \param marginTop top margin (millimeters)
 \param marginRight right margin  (millimeters)
 \param marginBottom bottom margin  (millimeters)
 \param marginLeft left margin (millimeters)
.. versionadded:: 2.12
.. seealso:: setResizeToContentsMargins()
.. seealso:: resizeToContentsMargins()
%End

    void setResizeToContentsMargins( double marginTop, double marginRight,
                                     double marginBottom, double marginLeft );
%Docstring
 Sets the resize to contents margins. These margins are saved in the composition
 so that they can be restored with the composer.
 \param marginTop top margin (millimeters)
 \param marginRight right margin  (millimeters)
 \param marginBottom bottom margin  (millimeters)
 \param marginLeft left margin (millimeters)
.. versionadded:: 2.12
.. seealso:: resizePageToContents()
.. seealso:: resizeToContentsMargins()
%End

    void resizeToContentsMargins( double &marginTop /Out/, double &marginRight /Out/,
                                  double &marginBottom /Out/, double &marginLeft /Out/ ) const;
%Docstring
 Returns the resize to contents margins. These margins are saved in the composition
 so that they can be restored with the composer.
 \param marginTop reference for top margin (millimeters)
 \param marginRight reference for right margin  (millimeters)
 \param marginBottom reference for bottom margin  (millimeters)
 \param marginLeft reference for left margin (millimeters)
.. versionadded:: 2.12
.. seealso:: resizePageToContents()
.. seealso:: setResizeToContentsMargins()
%End

    double spaceBetweenPages() const;
%Docstring
 Returns the vertical space between pages in a composer view
 :return: space between pages in mm
 :rtype: float
%End

    void setNumPages( const int pages );
%Docstring
 Sets the number of pages for the composition.
 \param pages number of pages
.. seealso:: numPages
%End

    int numPages() const;
%Docstring
 Returns the number of pages in the composition.
 :return: number of pages
.. seealso:: setNumPages
 :rtype: int
%End

    bool pageIsEmpty( const int page ) const;
%Docstring
 Returns whether a page is empty, ie, it contains no items except for the background
 paper item.
 \param page page number, starting with 1
 :return: true if page is empty
.. versionadded:: 2.5
.. seealso:: numPages
.. seealso:: setNumPages
.. seealso:: shouldExportPage
 :rtype: bool
%End

    bool shouldExportPage( const int page ) const;
%Docstring
 Returns whether a specified page number should be included in exports of the composition.
 \param page page number, starting with 1
 :return: true if page should be exported
.. versionadded:: 2.5
.. seealso:: numPages
.. seealso:: pageIsEmpty
 :rtype: bool
%End

    void setPageStyleSymbol( QgsFillSymbol *symbol );
%Docstring
Note: added in version 2.1
%End
    QgsFillSymbol *pageStyleSymbol();
%Docstring
Note: added in version 2.1
 :rtype: QgsFillSymbol
%End

    QPointF positionOnPage( QPointF position ) const;
%Docstring
 Returns the position within a page of a point in the composition
.. versionadded:: 2.1
 :rtype: QPointF
%End

    int pageNumberForPoint( QPointF position ) const;
%Docstring
 Returns the page number corresponding to a point in the composition
.. versionadded:: 2.1
 :rtype: int
%End

    void setStatusMessage( const QString &message );
%Docstring
 Sets the status bar message for the composer window
.. versionadded:: 2.1
%End

    void updateSettings();
%Docstring
 Refreshes the composition when composer related options change
.. versionadded:: 2.1
%End

    void setSnapToGridEnabled( const bool b );
    bool snapToGridEnabled() const;
%Docstring
 :rtype: bool
%End

    void setGridVisible( const bool b );
    bool gridVisible() const;
%Docstring
 :rtype: bool
%End

    void setSnapLinesVisible( const bool visible );
%Docstring
Hides / shows custom snap lines
%End
    bool snapLinesVisible() const;
%Docstring
 :rtype: bool
%End

    void setAlignmentSnap( const bool s );
    bool alignmentSnap() const;
%Docstring
 :rtype: bool
%End

    void setSmartGuidesEnabled( const bool b );
    bool smartGuidesEnabled() const;
%Docstring
 :rtype: bool
%End

    void setPagesVisible( bool visible );
%Docstring
 Sets whether the page items should be visible in the composition. Removing
 them will prevent both display of the page boundaries in composer views and
 will also prevent them from being rendered in composition exports.
 \param visible set to true to show pages, false to hide pages
.. versionadded:: 2.12
.. seealso:: pagesVisible()
%End

    bool pagesVisible() const;
%Docstring
 Returns whether the page items are be visible in the composition. This setting
 effects both display of the page boundaries in composer views and
 whether they will be rendered in composition exports.
.. versionadded:: 2.12
.. seealso:: setPagesVisible()
 :rtype: bool
%End

    void clearSnapLines();
%Docstring
Removes all snap lines
%End

    void setSnapGridResolution( const double r );
    double snapGridResolution() const;
%Docstring
 :rtype: float
%End

    void setSnapGridOffsetX( const double offset );
    double snapGridOffsetX() const;
%Docstring
 :rtype: float
%End

    void setSnapGridOffsetY( const double offset );
    double snapGridOffsetY() const;
%Docstring
 :rtype: float
%End

    void setGridPen( const QPen &p );
    QPen gridPen() const;
%Docstring
 :rtype: QPen
%End

    void setGridStyle( const GridStyle s );
    GridStyle gridStyle() const;
%Docstring
 :rtype: GridStyle
%End

    void setSnapTolerance( const int snapTolerance );
%Docstring
 Sets the snap tolerance to use when automatically snapping items during movement and resizing to guides
 and the edges and centers of other items.
 \param snapTolerance snap tolerance in pixels
.. seealso:: alignmentSnapTolerance
.. versionadded:: 2.5
%End

    int snapTolerance() const;
%Docstring
 Returns the snap tolerance to use when automatically snapping items during movement and resizing to guides
 and the edges and centers of other items.
 :return: snap tolerance in pixels
.. seealso:: setAlignmentSnapTolerance
.. versionadded:: 2.5
 :rtype: int
%End

    void setBoundingBoxesVisible( const bool boundsVisible );
%Docstring
 Sets whether selection bounding boxes should be shown in the composition
 \param boundsVisible set to true to show selection bounding box
.. seealso:: boundingBoxesVisible
.. versionadded:: 2.7
%End

    bool boundingBoxesVisible() const;
%Docstring
 Returns whether selection bounding boxes should be shown in the composition
 :return: true if selection bounding boxes should be shown
.. seealso:: setBoundingBoxesVisible
.. versionadded:: 2.7
 :rtype: bool
%End

    QUndoStack *undoStack();
%Docstring
Returns pointer to undo/redo command storage
 :rtype: QUndoStack
%End

    QgsComposerItem *composerItemAt( QPointF position, const bool ignoreLocked = false ) const;
%Docstring
 Returns the topmost composer item at a specified position. Ignores paper items.
 \param position point to search for item at
 \param ignoreLocked set to true to ignore locked items
 :return: composer item at position
 :rtype: QgsComposerItem
%End

    QgsComposerItem *composerItemAt( QPointF position, const QgsComposerItem *belowItem, const bool ignoreLocked = false ) const;
%Docstring
 Returns the topmost composer item at a specified position which is below a specified item. Ignores paper items.
 \param position point to search for item at
 \param belowItem item to search below
 \param ignoreLocked set to true to ignore locked items
 :return: composer item at position which is below specified item
 :rtype: QgsComposerItem
%End

    int pageNumberAt( QPointF position ) const;
%Docstring
Returns the page number (0-based) given a coordinate
 :rtype: int
%End

    int itemPageNumber( const QgsComposerItem * ) const;
%Docstring
Returns on which page number (0-based) is displayed an item
 :rtype: int
%End

    QList<QgsComposerItem *> selectedComposerItems( const bool includeLockedItems = true );
%Docstring
 Returns list of selected composer items
 \param includeLockedItems set to true to include locked items in list
 :return: list of selected items
 :rtype: list of QgsComposerItem
%End

    QList<const QgsComposerMap *> composerMapItems() const;
%Docstring
 Returns pointers to all composer maps in the scene
.. note::

   available in Python bindings only with PyQt >= 4.8.4
 :rtype: list of const QgsComposerMap
%End



    const QgsComposerMap *getComposerMapById( const int id ) const;
%Docstring
 Returns the composer map with specified id
 :return: QgsComposerMap or 0 pointer if the composer map item does not exist
 :rtype: QgsComposerMap
%End

    const QgsComposerItem *getComposerItemById( const QString &id ) const;
%Docstring
 Returns a composer item given its text identifier.
  Ids are not necessarely unique, but this function returns only one element.
 \param id - A QString representing the identifier of the item to retrieve.
 :return: QgsComposerItem pointer or 0 pointer if no such item exists.
 :rtype: QgsComposerItem
%End

    const QgsComposerItem *getComposerItemByUuid( const QString &uuid ) const;
%Docstring
 Returns a composer item given its unique identifier.
 \param uuid A QString representing the UUID of the item to
 :rtype: QgsComposerItem
%End

    int printResolution() const;
%Docstring
 :rtype: int
%End
    void setPrintResolution( const int dpi );

    bool printAsRaster() const;
%Docstring
 :rtype: bool
%End
    void setPrintAsRaster( const bool enabled );

    bool generateWorldFile() const;
%Docstring
 Returns true if the composition will generate corresponding world files when pages
 are exported.
.. seealso:: setGenerateWorldFile()
.. seealso:: referenceMap()
 :rtype: bool
%End

    void setGenerateWorldFile( bool enabled );
%Docstring
 Sets whether the composition will generate corresponding world files when pages
 are exported.
 \param enabled set to true to generate world files
.. seealso:: generateWorldFile()
.. seealso:: setReferenceMap()
%End

    QgsComposerMap *referenceMap() const;
%Docstring
 Returns the map item which will be used to generate corresponding world files when the
 composition is exported. If no map was explicitly set via setReferenceMap(), the largest
 map in the composition will be returned (or None if there are no maps in the composition).
.. seealso:: setReferenceMap()
.. seealso:: generateWorldFile()
 :rtype: QgsComposerMap
%End

    void setReferenceMap( QgsComposerMap *map );
%Docstring
 Sets the map item which will be used to generate corresponding world files when the
 composition is exported.
 \param map composer map item
.. seealso:: referenceMap()
.. seealso:: setGenerateWorldFile()
%End

    bool useAdvancedEffects() const;
%Docstring
Returns true if a composition should use advanced effects such as blend modes
 :rtype: bool
%End
    void setUseAdvancedEffects( const bool effectsEnabled );
%Docstring
Used to enable or disable advanced effects such as blend modes in a composition
%End

    QgsComposition::PlotStyle plotStyle() const;
%Docstring
 :rtype: QgsComposition.PlotStyle
%End
    void setPlotStyle( const QgsComposition::PlotStyle style );

    bool writeXml( QDomElement &composerElem, QDomDocument &doc );
%Docstring
Writes settings to xml (paper dimension)
 :rtype: bool
%End

    bool readXml( const QDomElement &compositionElem, const QDomDocument &doc );
%Docstring
Reads settings from xml file
 :rtype: bool
%End

    bool loadFromTemplate( const QDomDocument &doc, QMap<QString, QString> *substitutionMap = 0,
                           bool addUndoCommands = false, const bool clearComposition = true );
%Docstring
 Load a template document
 \param doc template document
 \param substitutionMap map with text to replace. Text needs to be enclosed by brackets (e.g. '[text]' )
 \param addUndoCommands whether or not to add undo commands
 \param clearComposition set to true to clear the existing composition and read all composition and
 atlas properties from the template. Set to false to only add new items from the template, without
 overwriting the existing items or composition settings.
 :rtype: bool
%End

    void addItemsFromXml( const QDomElement &elem, const QDomDocument &doc,
                          bool addUndoCommands = false, QPointF *pos = 0, bool pasteInPlace = false );
%Docstring
 Add items from XML representation to the graphics scene (for project file reading, pasting items from clipboard)
 \param elem items parent element, e.g. \verbatim <Composer> \endverbatim or \verbatim <ComposerItemClipboard> \endverbatim
 \param doc xml document
 \param addUndoCommands insert AddItem commands if true (e.g. for copy/paste)
 \param pos item position. Optional, take position from xml if 0
 \param pasteInPlace whether the position should be kept but mapped to the page origin. (the page is the page under to the mouse cursor)
%End

    void addItemToZList( QgsComposerItem *item );
%Docstring
Adds item to z list. Usually called from constructor of QgsComposerItem
%End
    void removeItemFromZList( QgsComposerItem *item );
%Docstring
Removes item from z list. Usually called from destructor of QgsComposerItem
%End

    void raiseSelectedItems();

    bool raiseItem( QgsComposerItem *item );
%Docstring
 :rtype: bool
%End
    void lowerSelectedItems();
    bool lowerItem( QgsComposerItem *item );
%Docstring
 :rtype: bool
%End
    void moveSelectedItemsToTop();
    bool moveItemToTop( QgsComposerItem *item );
%Docstring
 :rtype: bool
%End
    void moveSelectedItemsToBottom();
    bool moveItemToBottom( QgsComposerItem *item );
%Docstring
 :rtype: bool
%End

    void selectNextByZOrder( const ZValueDirection direction );
    QgsComposerItem *getComposerItemBelow( QgsComposerItem *item ) const;
%Docstring
 :rtype: QgsComposerItem
%End
    QgsComposerItem *getComposerItemAbove( QgsComposerItem *item ) const;
%Docstring
 :rtype: QgsComposerItem
%End

    void alignSelectedItemsLeft();
    void alignSelectedItemsHCenter();
    void alignSelectedItemsRight();
    void alignSelectedItemsTop();
    void alignSelectedItemsVCenter();
    void alignSelectedItemsBottom();

    void lockSelectedItems();
%Docstring
Lock the selected items
%End
    void unlockAllItems();
%Docstring
Unlock all items
%End

    QgsComposerItemGroup *groupItems( QList<QgsComposerItem *> items );
%Docstring
 Creates a new group from a list of composer items and adds it to the composition.
 \param items items to include in group
 :return: QgsComposerItemGroup of grouped items, if grouping was possible
.. versionadded:: 2.6
 :rtype: QgsComposerItemGroup
%End

    QList<QgsComposerItem *> ungroupItems( QgsComposerItemGroup *group );
%Docstring
 Ungroups items by removing them from an item group and removing the group from the
 composition.
 \param group item group to ungroup
 :return: list of items removed from the group, or an empty list if ungrouping
 was not successful
.. versionadded:: 2.6
 :rtype: list of QgsComposerItem
%End

    void refreshZList();
%Docstring
 Rebuilds the z order list by adding any item which are present in the composition
 but missing from the z order list.
%End

    QPointF snapPointToGrid( QPointF scenePoint ) const;
%Docstring
Snaps a scene coordinate point to grid
 :rtype: QPointF
%End

    QList< QGraphicsLineItem * > *snapLines();
%Docstring
Returns pointer to snap lines collection
 :rtype: list of QGraphicsLineItem
%End


    QGraphicsLineItem *addSnapLine();
%Docstring
Add a custom snap line (can be horizontal or vertical)
 :rtype: QGraphicsLineItem
%End
    void removeSnapLine( QGraphicsLineItem *line );
%Docstring
Remove custom snap line (and delete the object)
%End


    void beginCommand( QgsComposerItem *item, const QString &commandText, const QgsComposerMergeCommand::Context c = QgsComposerMergeCommand::Unknown );
%Docstring
 Allocates new item command and saves initial state in it
 \param item target item
 \param commandText descriptive command text
 \param c context for merge commands (unknown for non-mergeable commands)
%End

    void endCommand();
%Docstring
Saves end state of item and pushes command to the undo history
%End
    void cancelCommand();
%Docstring
Deletes current command
%End

    void beginMultiFrameCommand( QgsComposerMultiFrame *multiFrame, const QString &text, const QgsComposerMultiFrameMergeCommand::Context c = QgsComposerMultiFrameMergeCommand::Unknown );
    void endMultiFrameCommand();
    void cancelMultiFrameCommand();
%Docstring
Deletes current multi frame command
%End

    void addMultiFrame( QgsComposerMultiFrame *multiFrame );
%Docstring
Adds multiframe. The object is owned by QgsComposition until removeMultiFrame is called
%End
    void removeMultiFrame( QgsComposerMultiFrame *multiFrame );
%Docstring
Removes multi frame (but does not delete it)
%End

    void addComposerArrow( QgsComposerArrow *arrow );
%Docstring
Adds an arrow item to the graphics scene and advises composer to create a widget for it (through signal)
%End
    void addComposerLabel( QgsComposerLabel *label );
%Docstring
Adds label to the graphics scene and advises composer to create a widget for it (through signal)
%End
    void addComposerMap( QgsComposerMap *map );
%Docstring
Adds map to the graphics scene and advises composer to create a widget for it (through signal)
%End
    void addComposerScaleBar( QgsComposerScaleBar *scaleBar );
%Docstring
Adds scale bar to the graphics scene and advises composer to create a widget for it (through signal)
%End
    void addComposerLegend( QgsComposerLegend *legend );
%Docstring
Adds legend to the graphics scene and advises composer to create a widget for it (through signal)
%End
    void addComposerPicture( QgsComposerPicture *picture );
%Docstring
Adds picture to the graphics scene and advises composer to create a widget for it (through signal)
%End
    void addComposerShape( QgsComposerShape *shape );
%Docstring
Adds a composer shape to the graphics scene and advises composer to create a widget for it (through signal)
%End
    void addComposerPolygon( QgsComposerPolygon *polygon );
%Docstring
Adds a composer polygon and advises composer to create a widget for it (through signal)
%End
    void addComposerPolyline( QgsComposerPolyline *polyline );
%Docstring
Adds a composer polyline and advises composer to create a widget for it (through signal)
%End
    void addComposerHtmlFrame( QgsComposerHtml *html /Transfer/, QgsComposerFrame *frame /Transfer/ );
%Docstring
Adds composer html frame and advises composer to create a widget for it (through signal)
%End
    void addComposerTableFrame( QgsComposerAttributeTableV2 *table /Transfer/, QgsComposerFrame *frame /Transfer/ );
%Docstring
Adds composer tablev2 frame and advises composer to create a widget for it (through signal)
%End

    void removeComposerItem( QgsComposerItem *item, const bool createCommand = true, const bool removeGroupItems = true );
%Docstring
Remove item from the graphics scene. Additionally to QGraphicsScene.removeItem, this function considers undo/redo command
%End

    void pushAddRemoveCommand( QgsComposerItem *item, const QString &text, const QgsAddRemoveItemCommand::State state = QgsAddRemoveItemCommand::Added );
%Docstring
Convenience function to create a QgsAddRemoveItemCommand, connect its signals and push it to the undo stack
%End

    void setPreventCursorChange( const bool preventChange );
%Docstring
 If true, prevents any mouse cursor changes by the composition or by any composer items
 Used by QgsComposer and QgsComposerView to prevent unwanted cursor changes
%End
    bool preventCursorChange() const;
%Docstring
 :rtype: bool
%End

    void beginPrint( QPrinter &printer, const bool evaluateDDPageSize = false );
%Docstring
Prepare the printer for printing
%End
    void beginPrintAsPDF( QPrinter &printer, const QString &file );
%Docstring
Prepare the printer for printing in a PDF
%End

    void doPrint( QPrinter &printer, QPainter &painter, bool startNewPage = false );
%Docstring
 Print on a preconfigured printer
 \param printer QPrinter destination
 \param painter QPainter source
 \param startNewPage set to true to begin the print on a new page
%End

    bool print( QPrinter &printer, const bool evaluateDDPageSize = false );
%Docstring
 Convenience function that prepares the printer and prints
 :return: true if print was successful
 :rtype: bool
%End

    bool exportAsPDF( const QString &file );
%Docstring
 Convenience function that prepares the printer for printing in PDF and prints
 :return: true if export was successful
 :rtype: bool
%End

    QImage printPageAsRaster( int page, QSize imageSize = QSize(), int dpi = 0 );
%Docstring
 Renders a composer page to an image.
 \param page page number, 0 based such that the first page is page 0
 \param imageSize optional target image size, in pixels. It is the caller's responsibility
 to ensure that the ratio of the target image size matches the ratio of the composition
 page size.
 \param dpi optional dpi override, or 0 to use default composition print resolution. This
 parameter has no effect if imageSize is specified.
 :return: rendered image, or null image if image does not fit into available memory
.. seealso:: renderRectAsRaster()
.. seealso:: renderPage()
 :rtype: QImage
%End

    QImage renderRectAsRaster( const QRectF &rect, QSize imageSize = QSize(), int dpi = 0 );
%Docstring
 Renders a portion of the composition to an image. This method can be used to render
 sections of pages rather than full pages.
 \param rect region of composition to render
 \param imageSize optional target image size, in pixels. It is the caller's responsibility
 to ensure that the ratio of the target image size matches the ratio of the specified
 region of the composition.
 \param dpi optional dpi override, or 0 to use default composition print resolution. This
 parameter has no effect if imageSize is specified.
 :return: rendered image, or null image if image does not fit into available memory
.. versionadded:: 2.12
.. seealso:: printPageAsRaster()
.. seealso:: renderRect()
 :rtype: QImage
%End

    void renderPage( QPainter *p, int page );
%Docstring
 Renders a full page to a paint device.
 \param p destination painter
 \param page page number, 0 based such that the first page is page 0
.. seealso:: renderRect()
.. seealso:: printPageAsRaster()
%End

    void renderRect( QPainter *p, const QRectF &rect );
%Docstring
 Renders a portion of the composition to a paint device. This method can be used
 to render sections of pages rather than full pages.
 \param p destination painter
 \param rect region of composition to render
.. versionadded:: 2.12
.. seealso:: renderPage()
.. seealso:: renderRectAsRaster()
%End

    void georeferenceOutput( const QString &file, QgsComposerMap *referenceMap = 0,
                             const QRectF &exportRegion = QRectF(), double dpi = -1 ) const;
%Docstring
 Georeferences a file (image of PDF) exported from the composition.
 \param file filename of exported file
 \param referenceMap map item to use for georeferencing, or leave as None to use the
 currently defined referenceMap().
 \param exportRegion set to a valid rectangle to indicate that only part of the composition was
 exported
 \param dpi set to DPI of exported file, or leave as -1 to use composition's DPI.
.. versionadded:: 2.16
%End

    void computeWorldFileParameters( double &a, double &b, double &c, double &d, double &e, double &f ) const;
%Docstring
 Compute world file parameters. Assumes the whole page containing the associated map item
 will be exported.
%End

    void computeWorldFileParameters( const QRectF &exportRegion, double &a, double &b, double &c, double &d, double &e, double &f ) const;
%Docstring
 Computes the world file parameters for a specified region of the composition.
 \param exportRegion region of the composition which will be associated with world file
 \param a
 \param b
 \param c
 \param d
 \param e
 \param f
.. versionadded:: 2.12
%End

    QgsAtlasComposition &atlasComposition();
%Docstring
 :rtype: QgsAtlasComposition
%End

    QgsComposition::AtlasMode atlasMode() const;
%Docstring
 Returns the current atlas mode of the composition
 :return: current atlas mode
.. seealso:: setAtlasMode
 :rtype: QgsComposition.AtlasMode
%End

    bool setAtlasMode( const QgsComposition::AtlasMode mode );
%Docstring
 Sets the current atlas mode of the composition.
 \param mode atlas mode to switch to
 :return: false if the mode could not be changed.
.. seealso:: atlasMode
 :rtype: bool
%End

    QList< QgsPaperItem * > pages();
%Docstring
 Return pages in the correct order
.. note::

   composerItems(QList< QgsPaperItem* > &) may not return pages in the correct order
.. versionadded:: 2.4
 :rtype: list of QgsPaperItem
%End

    QgsComposerModel *itemsModel();
%Docstring
 Returns the items model attached to the composition
 :return: QgsComposerModel for composition
.. versionadded:: 2.5
 :rtype: QgsComposerModel
%End

    void setCustomProperty( const QString &key, const QVariant &value );
%Docstring
 Set a custom property for the composition.
 \param key property key. If a property with the same key already exists it will be overwritten.
 \param value property value
.. seealso:: customProperty()
.. seealso:: removeCustomProperty()
.. seealso:: customProperties()
.. versionadded:: 2.12
%End

    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;
%Docstring
 Read a custom property from the composition.
 \param key property key
 \param defaultValue default value to return if property with matching key does not exist
 :return: value of matching property
.. seealso:: setCustomProperty()
.. seealso:: removeCustomProperty()
.. seealso:: customProperties()
.. versionadded:: 2.12
 :rtype: QVariant
%End

    void removeCustomProperty( const QString &key );
%Docstring
 Remove a custom property from the composition.
 \param key property key
.. seealso:: setCustomProperty()
.. seealso:: customProperty()
.. seealso:: customProperties()
.. versionadded:: 2.12
%End

    QStringList customProperties() const;
%Docstring
 Return list of keys stored in custom properties for composition.
.. seealso:: setCustomProperty()
.. seealso:: customProperty()
.. seealso:: removeCustomProperty()
.. versionadded:: 2.12
 :rtype: list of str
%End

    QRectF pageItemBounds( int pageNumber, bool visibleOnly = false ) const;
%Docstring
 Returns the bounding box of the items contained on a specified page.
 \param pageNumber page number, where 0 is the first page
 \param visibleOnly set to true to only include visible items
.. versionadded:: 2.12
 :rtype: QRectF
%End

    QRectF compositionBounds( bool ignorePages = false, double margin = 0.0 ) const;
%Docstring
 Calculates the bounds of all non-gui items in the composition. Ignores snap lines and mouse handles.
 \param ignorePages set to true to ignore page items
 \param margin optional marginal (in percent, e.g., 0.05 = 5% ) to add around items
 :rtype: QRectF
%End

    virtual QgsExpressionContext createExpressionContext() const;

%Docstring
 Creates an expression context relating to the compositions's current state. The context includes
 scopes for global, project, composition and atlas properties.
.. versionadded:: 2.12
 :rtype: QgsExpressionContext
%End

    QgsPropertyCollection &dataDefinedProperties();
%Docstring
 Returns a reference to the composition's property collection, used for data defined overrides.
.. versionadded:: 3.0
.. seealso:: setDataDefinedProperties()
 :rtype: QgsPropertyCollection
%End


  public slots:
    void sendItemAddedSignal( QgsComposerItem *item );
%Docstring
Casts object to the proper subclass type and calls corresponding itemAdded signal
%End

    void updateBounds();
%Docstring
 Updates the scene bounds of the composition
.. versionadded:: 2.2
%End

    void refreshItems();
%Docstring
 Forces items in the composition to refresh. For instance, this causes maps to redraw
 and rebuild cached images, html items to reload their source url, and attribute tables
 to refresh their contents. Calling this also triggers a recalculation of all data defined
 attributes within the composition.
.. versionadded:: 2.3
%End

    void setSelectedItem( QgsComposerItem *item );
%Docstring
 Clears any selected items and sets an item as the current selection.
 \param item item to set as selected
.. versionadded:: 2.3
%End

    void setAllDeselected();
%Docstring
 Clears any selected items in the composition. Call this method rather than
 QGraphicsScene.clearSelection, as the latter does not correctly emit signals to allow
 the composition's model to update.
.. versionadded:: 2.5
%End

    void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = 0 );
%Docstring
 Refreshes a data defined property for the composition by reevaluating the property's value
 and redrawing the composition with this new value.
 \param property data defined property to refresh. If property is set to
 QgsComposerItem.AllProperties then all data defined properties for the composition will be
 refreshed.
 \param context expression context for evaluating data defined expressions
.. versionadded:: 2.5
%End

    void setDataDefinedProperties( const QgsPropertyCollection &collection );
%Docstring
 Sets the composition's property collection, used for data defined overrides.
 \param collection property collection. Existing properties will be replaced.
.. versionadded:: 3.0
.. seealso:: dataDefinedProperties()
%End

  protected:
    void init();


  signals:

    void nameChanged( const QString &name );
%Docstring
 Emitted when the composition's name is changed.
.. versionadded:: 3.0
.. seealso:: setName()
%End

    void paperSizeChanged();
    void nPagesChanged();

    void printResolutionChanged();
%Docstring
Is emitted when the compositions print resolution changes
%End

    void selectedItemChanged( QgsComposerItem *selected );
%Docstring
Is emitted when selected item changed. If 0, no item is selected
%End

    void itemAdded( QgsComposerItem *item );
%Docstring
 Is emitted when a new composer item has been added to the composition.
.. versionadded:: 3.0
%End

    void composerItemGroupAdded( QgsComposerItemGroup *group );
%Docstring
Is emitted when a new item group has been added to the view
%End

    void itemRemoved( QgsComposerItem * );
%Docstring
Is emitted when a composer item has been removed from the scene
%End

    void refreshItemsTriggered();
%Docstring
Is emitted when item in the composition must be refreshed
%End

    void statusMsgChanged( const QString &message );
%Docstring
Is emitted when the composition has an updated status bar message for the composer window
%End

    void variablesChanged();
%Docstring
 Emitted whenever the expression variables stored in the composition have been changed.
.. versionadded:: 3.0
%End

};







/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposition.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
