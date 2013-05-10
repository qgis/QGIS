/***************************************************************************
                              qgscomposition.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSITION_H
#define QGSCOMPOSITION_H

#include "qgscomposeritem.h"
#include <memory>

#include <QDomDocument>
#include <QGraphicsScene>
#include <QLinkedList>
#include <QList>
#include <QPair>
#include <QSet>
#include <QUndoStack>
#include <QPrinter>
#include <QPainter>

#include "qgsaddremoveitemcommand.h"
#include "qgscomposeritemcommand.h"
#include "qgsatlascomposition.h"

class QgisApp;
class QgsComposerFrame;
class QgsComposerMap;
class QgsPaperItem;
class QGraphicsRectItem;
class QgsMapRenderer;
class QDomElement;
class QgsComposerArrow;
class QgsComposerHtml;
class QgsComposerItem;
class QgsComposerLabel;
class QgsComposerLegend;
class QgsComposerMap;
class QgsComposerPicture;
class QgsComposerScaleBar;
class QgsComposerShape;
class QgsComposerAttributeTable;
class QgsComposerMultiFrame;
class QgsComposerMultiFrameCommand;
class QgsVectorLayer;
class QgsComposer;

/** \ingroup MapComposer
 * Graphics scene for map printing. The class manages the paper item which always
 * is the item in the back (z-value 0). It maintains the z-Values of the items and stores
 * them in a list in ascending z-Order. This list can be changed to lower/raise items one position
 * or to bring them to front/back.
 * */
class CORE_EXPORT QgsComposition: public QGraphicsScene
{
    Q_OBJECT
  public:

    /** \brief Plot type */
    enum PlotStyle
    {
      Preview = 0, // Use cache etc
      Print,       // Render well
      Postscript   // Fonts need different scaling!
    };

    /**Style to draw the snapping grid*/
    enum GridStyle
    {
      Solid,
      Dots,
      Crosses
    };

    QgsComposition( QgsMapRenderer* mapRenderer );
    ~QgsComposition();

    /**Changes size of paper item*/
    void setPaperSize( double width, double height );

    /**Returns height of paper item*/
    double paperHeight() const;

    /**Returns width of paper item*/
    double paperWidth() const;

    double spaceBetweenPages() const { return mSpaceBetweenPages; }

    /**Note: added in version 1.9*/
    void setNumPages( int pages );
    /**Note: added in version 1.9*/
    int numPages() const;

    void setSnapToGridEnabled( bool b );
    bool snapToGridEnabled() const {return mSnapToGrid;}

    void setSnapGridResolution( double r );
    double snapGridResolution() const {return mSnapGridResolution;}

    void setSnapGridOffsetX( double offset );
    double snapGridOffsetX() const {return mSnapGridOffsetX;}

    void setSnapGridOffsetY( double offset );
    double snapGridOffsetY() const {return mSnapGridOffsetY;}

    void setGridPen( const QPen& p );
    const QPen& gridPen() const {return mGridPen;}

    void setGridStyle( GridStyle s );
    GridStyle gridStyle() const {return mGridStyle;}

    void setAlignmentSnap( bool s ) { mAlignmentSnap = s; }
    bool alignmentSnap() const { return mAlignmentSnap; }

    void setAlignmentSnapTolerance( double t ) { mAlignmentSnapTolerance = t; }
    double alignmentSnapTolerance() const { return mAlignmentSnapTolerance; }

    /**Returns pointer to undo/redo command storage*/
    QUndoStack* undoStack() { return &mUndoStack; }

    /**Returns the topmost composer item. Ignores mPaperItem*/
    QgsComposerItem* composerItemAt( const QPointF & position );

    /** Returns the page number (0-bsaed) given a coordinate */
    int pageNumberAt( const QPointF& position ) const;

    /** Returns on which page number (0-based) is displayed an item */
    int itemPageNumber( const QgsComposerItem* ) const;

    QList<QgsComposerItem*> selectedComposerItems();

    /**Returns pointers to all composer maps in the scene
      @note available in python bindings only with PyQt >= 4.8.4
      */
    QList<const QgsComposerMap*> composerMapItems() const;

    /**Return composer items of a specific type
      @note not available in python bindings
     */
    template<class T> void composerItems( QList<T*>& itemList );

    /**Returns the composer map with specified id
     @return QgsComposerMap or 0 pointer if the composer map item does not exist*/
    const QgsComposerMap* getComposerMapById( int id ) const;

    /*Returns the composer html with specified id (a string as named in the
      composer user interface item properties).
      @note Added in QGIS 2.0
      @param id - A QString representing the id of the item.
      @return QgsComposerHtml pointer or 0 pointer if no such item exists.
    */
    const QgsComposerHtml* getComposerHtmlByItem( QgsComposerItem *item ) const;

    /**Returns a composer item given its text identifier.
       Ids are not necessarely unique, but this function returns only one element.
      @note added in 2.0
      @param theId - A QString representing the identifier of the item to
        retrieve.
      @return QgsComposerItem pointer or 0 pointer if no such item exists.
      **/
    const QgsComposerItem* getComposerItemById( QString theId ) const;

    /**Returns a composer item given its unique identifier.
      @note added in 2.0
      @param theUuid A QString representing the UUID of the item to
      **/
    const QgsComposerItem* getComposerItemByUuid( QString theUuid ) const;

    int printResolution() const {return mPrintResolution;}
    void setPrintResolution( int dpi ) {mPrintResolution = dpi;}

    bool printAsRaster() const {return mPrintAsRaster;}
    void setPrintAsRaster( bool enabled ) { mPrintAsRaster = enabled; }

    /**Returns true if a composition should use advanced effects such as blend modes
      @note added in 1.9*/
    bool useAdvancedEffects() const {return mUseAdvancedEffects;}
    /**Used to enable or disable advanced effects such as blend modes in a composition
      @note: added in version 1.9*/
    void setUseAdvancedEffects( bool effectsEnabled );

    double selectionTolerance() const { return mSelectionTolerance; }
    void setSelectionTolerance( double tol );

    /**Returns pointer to map renderer of qgis map canvas*/
    QgsMapRenderer* mapRenderer() {return mMapRenderer;}

    QgsComposition::PlotStyle plotStyle() const {return mPlotStyle;}
    void setPlotStyle( QgsComposition::PlotStyle style ) {mPlotStyle = style;}

    /**Returns the pixel font size for a font that has point size set.
     The result depends on the resolution (dpi) and of the preview mode. Each item that sets
    a font should call this function before drawing text*/
    int pixelFontSize( double pointSize ) const;

    /**Does the inverse calculation and returns points for pixels (equals to mm in QgsComposition)*/
    double pointFontSize( int pixelSize ) const;

    /**Writes settings to xml (paper dimension)*/
    bool writeXML( QDomElement& composerElem, QDomDocument& doc );

    /**Reads settings from xml file*/
    bool readXML( const QDomElement& compositionElem, const QDomDocument& doc );

    /**Load a template document
        @param doc template document
        @param substitutionMap map with text to replace. Text needs to be enclosed by brackets (e.g. '[text]' )
        @param addUndoCommands whether or not to add undo commands
      */
    bool loadFromTemplate( const QDomDocument& doc, QMap<QString, QString>* substitutionMap = 0, bool addUndoCommands = false );

    /**Add items from XML representation to the graphics scene (for project file reading, pasting items from clipboard)
      @param elem items parent element, e.g. \verbatim <Composer> \endverbatim or \verbatim <ComposerItemClipboard> \endverbatim
      @param doc xml document
      @param mapsToRestore for reading from project file: set preview move 'rectangle' to all maps and save the preview states to show composer maps on demand
      @param addUndoCommands insert AddItem commands if true (e.g. for copy/paste)
      @param pos item position. Optional, take position from xml if 0
      @param pasteInPlace whether the position should be kept but mapped to the page origin. (the page is the page under to the mouse cursor)
      @note not available in python bindings
     */
    void addItemsFromXML( const QDomElement& elem, const QDomDocument& doc, QMap< QgsComposerMap*, int >* mapsToRestore = 0,
                          bool addUndoCommands = false, QPointF* pos = 0, bool pasteInPlace = false );

    /**Adds item to z list. Usually called from constructor of QgsComposerItem*/
    void addItemToZList( QgsComposerItem* item );
    /**Removes item from z list. Usually called from destructor of QgsComposerItem*/
    void removeItemFromZList( QgsComposerItem* item );

    //functions to move selected items in hierarchy
    void raiseSelectedItems();
    void raiseItem( QgsComposerItem* item );
    void lowerSelectedItems();
    void lowerItem( QgsComposerItem* item );
    void moveSelectedItemsToTop();
    void moveItemToTop( QgsComposerItem* item );
    void moveSelectedItemsToBottom();
    void moveItemToBottom( QgsComposerItem* item );

    //functions to align selected items
    void alignSelectedItemsLeft();
    void alignSelectedItemsHCenter();
    void alignSelectedItemsRight();
    void alignSelectedItemsTop();
    void alignSelectedItemsVCenter();
    void alignSelectedItemsBottom();

    /**Sorts the zList. The only time where this function needs to be called is from QgsComposer
     after reading all the items from xml file*/
    void sortZList();

    /**Snaps a scene coordinate point to grid*/
    QPointF snapPointToGrid( const QPointF& scenePoint ) const;

    /**Snaps item position to align with other items (left / middle / right or top / middle / bottom
    @param item current item
    @param alignX x-coordinate of align or -1 if not aligned to x
    @param alignY y-coordinate of align or -1 if not aligned to y
    @param dx item shift in x direction
    @param dy item shift in y direction
    @return new upper left point after the align*/
    QPointF alignItem( const QgsComposerItem* item, double& alignX, double& alignY, double dx = 0, double dy = 0 );

    /**Snaps position to align with the boundaries of other items
    @param pos position to snap
    @param excludeItem item to exclude
    @param alignX snapped x coordinate or -1 if not snapped
    @param alignY snapped y coordinate or -1 if not snapped
    @return snapped position or original position if no snap*/
    QPointF alignPos( const QPointF& pos, const QgsComposerItem* excludeItem, double& alignX, double& alignY );

    /**Add a custom snap line (can be horizontal or vertical)*/
    QGraphicsLineItem* addSnapLine();
    /**Remove custom snap line (and delete the object)*/
    void removeSnapLine( QGraphicsLineItem* line );
    /**Get nearest snap line*/
    QGraphicsLineItem* nearestSnapLine( bool horizontal, double x, double y, double tolerance, QList< QPair< QgsComposerItem*, QgsComposerItem::ItemPositionMode > >& snappedItems );
    /**Hides / shows custom snap lines*/
    void setSnapLinesVisible( bool visible );

    /**Allocates new item command and saves initial state in it
      @param item target item
      @param commandText descriptive command text
      @param c context for merge commands (unknown for non-mergeable commands)*/
    void beginCommand( QgsComposerItem* item, const QString& commandText, QgsComposerMergeCommand::Context c = QgsComposerMergeCommand::Unknown );

    /**Saves end state of item and pushes command to the undo history*/
    void endCommand();
    /**Deletes current command*/
    void cancelCommand();

    void beginMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text );
    void endMultiFrameCommand();

    /**Adds multiframe. The object is owned by QgsComposition until removeMultiFrame is called*/
    void addMultiFrame( QgsComposerMultiFrame* multiFrame );
    /**Removes multi frame (but does not delete it)*/
    void removeMultiFrame( QgsComposerMultiFrame* multiFrame );
    /**Adds an arrow item to the graphics scene and advices composer to create a widget for it (through signal)
      @note not available in python bindings*/
    void addComposerArrow( QgsComposerArrow* arrow );
    /**Adds label to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerLabel( QgsComposerLabel* label );
    /**Adds map to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerMap( QgsComposerMap* map, bool setDefaultPreviewStyle = true );
    /**Adds scale bar to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerScaleBar( QgsComposerScaleBar* scaleBar );
    /**Adds legend to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerLegend( QgsComposerLegend* legend );
    /**Adds picture to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerPicture( QgsComposerPicture* picture );
    /**Adds a composer shape to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerShape( QgsComposerShape* shape );
    /**Adds a composer table to the graphics scene and advices composer to create a widget for it (through signal)*/
    void addComposerTable( QgsComposerAttributeTable* table );
    /**Adds composer html frame and advices composer to create a widget for it (through signal)*/
    void addComposerHtmlFrame( QgsComposerHtml* html, QgsComposerFrame* frame );

    /**Remove item from the graphics scene. Additionally to QGraphicsScene::removeItem, this function considers undo/redo command*/
    void removeComposerItem( QgsComposerItem* item, bool createCommand = true );

    /**Convenience function to create a QgsAddRemoveItemCommand, connect its signals and push it to the undo stack*/
    void pushAddRemoveCommand( QgsComposerItem* item, const QString& text, QgsAddRemoveItemCommand::State state = QgsAddRemoveItemCommand::Added );


    //printing

    /** Prepare the printer for printing */
    void beginPrint( QPrinter& printer );
    /** Prepare the printer for printing in a PDF */
    void beginPrintAsPDF( QPrinter& printer, const QString& file );
    /** Print on a preconfigured printer */
    void doPrint( QPrinter& printer, QPainter& painter );

    /** Convenience function that prepares the printer and prints */
    void print( QPrinter &printer );

    /** Convenience function that prepares the printer for printing in PDF and prints */
    void exportAsPDF( const QString& file );

    //! print composer page to image
    //! If the image does not fit into memory, a null image is returned
    QImage printPageAsRaster( int page );

    /**Render a page to a paint device
        @note added in version 1.9*/
    void renderPage( QPainter* p, int page );

    QgsAtlasComposition& atlasComposition() { return mAtlasComposition; }

  public slots:
    /**Casts object to the proper subclass type and calls corresponding itemAdded signal*/
    void sendItemAddedSignal( QgsComposerItem* item );

  private:
    /**Pointer to map renderer of QGIS main map*/
    QgsMapRenderer* mMapRenderer;
    QgsComposition::PlotStyle mPlotStyle;
    double mPageWidth;
    double mPageHeight;
    QList< QgsPaperItem* > mPages;
    double mSpaceBetweenPages; //space in preview between pages

    /**Maintains z-Order of items. Starts with item at position 1 (position 0 is always paper item)*/
    QLinkedList<QgsComposerItem*> mItemZList;

    /**List multiframe objects*/
    QSet<QgsComposerMultiFrame*> mMultiFrames;

    /**Dpi for printout*/
    int mPrintResolution;

    /**Flag if map should be printed as a raster (via QImage). False by default*/
    bool mPrintAsRaster;

    /**Flag if advanced visual effects such as blend modes should be used. True by default*/
    bool mUseAdvancedEffects;

    /**Distance tolerance for item selection (in mm)*/
    double mSelectionTolerance;

    /**Parameters for snap to grid function*/
    bool mSnapToGrid;
    double mSnapGridResolution;
    double mSnapGridOffsetX;
    double mSnapGridOffsetY;
    QPen mGridPen;
    GridStyle mGridStyle;

    /**Parameters for alignment snap*/
    bool mAlignmentSnap;
    double mAlignmentSnapTolerance;

    /**Arbitraty snap lines (horizontal and vertical)*/
    QList< QGraphicsLineItem* > mSnapLines;

    QUndoStack mUndoStack;

    QgsComposerItemCommand* mActiveItemCommand;
    QgsComposerMultiFrameCommand* mActiveMultiFrameCommand;

    /** The atlas composition object. It is held by the QgsComposition */
    QgsAtlasComposition mAtlasComposition;

    QgsComposition(); //default constructor is forbidden

    /**Reset z-values of items based on position in z list*/
    void updateZValues();

    /**Returns the bounding rectangle of the selected items in scene coordinates
     @return 0 in case of success*/
    int boundingRectOfSelectedItems( QRectF& bRect );

    void loadSettings();
    void saveSettings();

    void connectAddRemoveCommandSignals( QgsAddRemoveItemCommand* c );

    void updatePaperItems();
    void addPaperItem();
    void removePaperItems();
    void deleteAndRemoveMultiFrames();

    static QString encodeStringForXML( const QString& str );

    //helper functions for item align
    void collectAlignCoordinates( QMap< double, const QgsComposerItem* >& alignCoordsX,
                                  QMap< double, const QgsComposerItem* >& alignCoordsY, const QgsComposerItem* excludeItem );

    void checkNearestItem( double checkCoord, const QMap< double, const QgsComposerItem* >& alignCoords, double& smallestDiff,
                           double itemCoordOffset, double& itemCoord, double& alignCoord ) const;

    /**Find nearest item in coordinate map to a double.
        @return true if item found, false if coords is empty*/
    static bool nearestItem( const QMap< double, const QgsComposerItem* >& coords, double value, double& nearestValue );

  signals:
    void paperSizeChanged();
    void nPagesChanged();

    /**Is emitted when selected item changed. If 0, no item is selected*/
    void selectedItemChanged( QgsComposerItem* selected );
    /**Is emitted when new composer arrow has been added to the view*/
    void composerArrowAdded( QgsComposerArrow* arrow );
    /**Is emitted when a new composer html has been added to the view*/
    void composerHtmlFrameAdded( QgsComposerHtml* html, QgsComposerFrame* frame );
    /**Is emitted when new composer label has been added to the view*/
    void composerLabelAdded( QgsComposerLabel* label );
    /**Is emitted when new composer map has been added to the view*/
    void composerMapAdded( QgsComposerMap* map );
    /**Is emitted when new composer scale bar has been added*/
    void composerScaleBarAdded( QgsComposerScaleBar* scalebar );
    /**Is emitted when a new composer legend has been added*/
    void composerLegendAdded( QgsComposerLegend* legend );
    /**Is emitted when a new composer picture has been added*/
    void composerPictureAdded( QgsComposerPicture* picture );
    /**Is emitted when a new composer shape has been added*/
    void composerShapeAdded( QgsComposerShape* shape );
    /**Is emitted when a new composer table has been added*/
    void composerTableAdded( QgsComposerAttributeTable* table );
    /**Is emitted when a composer item has been removed from the scene*/
    void itemRemoved( QgsComposerItem* );
};

template<class T> void QgsComposition::composerItems( QList<T*>& itemList )
{
  itemList.clear();
  QList<QGraphicsItem *> graphicsItemList = items();
  QList<QGraphicsItem *>::iterator itemIt = graphicsItemList.begin();
  for ( ; itemIt != graphicsItemList.end(); ++itemIt )
  {
    T* item = dynamic_cast<T*>( *itemIt );
    if ( item )
    {
      itemList.push_back( item );
    }
  }
}

#endif



