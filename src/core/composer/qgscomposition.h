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

#include <QDomDocument>
#include <QGraphicsScene>
#include <QLinkedList>
#include <QSet>
#include <QUndoStack>

#include "qgsaddremoveitemcommand.h"
#include "qgscomposeritemcommand.h"

class QgsComposerFrame;
class QgsComposerItem;
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

    /**Returns pointer to undo/redo command storage*/
    QUndoStack* undoStack() { return &mUndoStack; }

    /**Returns the topmost composer item. Ignores mPaperItem*/
    QgsComposerItem* composerItemAt( const QPointF & position );

    QList<QgsComposerItem*> selectedComposerItems();

    /**Returns pointers to all composer maps in the scene*/
    QList<const QgsComposerMap*> composerMapItems() const;

    /**Returns the composer map with specified id
     @return id or 0 pointer if the composer map item does not exist*/
    const QgsComposerMap* getComposerMapById( int id ) const;

    int printResolution() const {return mPrintResolution;}
    void setPrintResolution( int dpi ) {mPrintResolution = dpi;}

    bool printAsRaster() const {return mPrintAsRaster;}
    void setPrintAsRaster( bool enabled ) { mPrintAsRaster = enabled; }

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

    /**Add items from XML representation to the graphics scene (for project file reading, pasting items from clipboard)
      @param elem items parent element, e.g. \verbatim <Composer> \endverbatim or \verbatim <ComposerItemClipboard> \endverbatim
      @param doc xml document
      @param mapsToRestore for reading from project file: set preview move 'rectangle' to all maps and save the preview states to show composer maps on demand
      @param addUndoCommands insert AddItem commands if true (e.g. for copy/paste)
      @param pos item position. Optional, take position from xml if 0*/
    void addItemsFromXML( const QDomElement& elem, const QDomDocument& doc, QMap< QgsComposerMap*, int >* mapsToRestore = 0,
                          bool addUndoCommands = false, QPointF* pos = 0 );

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
    /**Adds an arrow item to the graphics scene and advices composer to create a widget for it (through signal)*/
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

    void exportAsPDF( const QString& file );

    void print( QPrinter &printer );

    //! print composer page to image
    //! If the image does not fit into memory, a null image is returned
    QImage printPageAsRaster( int page );

    /**Render a page to a paint device
        @note added in version 1.9*/
    void renderPage( QPainter* p, int page );

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

    /**Distance tolerance for item selection (in mm)*/
    double mSelectionTolerance;

    /**Parameters for snap to grid function*/
    bool mSnapToGrid;
    double mSnapGridResolution;
    double mSnapGridOffsetX;
    double mSnapGridOffsetY;
    QPen mGridPen;
    GridStyle mGridStyle;

    QUndoStack mUndoStack;

    QgsComposerItemCommand* mActiveItemCommand;
    QgsComposerMultiFrameCommand* mActiveMultiFrameCommand;

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

#endif



