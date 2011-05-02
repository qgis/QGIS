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
#include <QUndoStack>

#include "qgscomposeritemcommand.h"

class QgsComposerItem;
class QgsComposerMap;
class QgsPaperItem;
class QGraphicsRectItem;
class QgsMapRenderer;

class QDomElement;

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


  private:
    /**Pointer to map renderer of QGIS main map*/
    QgsMapRenderer* mMapRenderer;
    QgsComposition::PlotStyle mPlotStyle;
    QgsPaperItem* mPaperItem;

    /**Maintains z-Order of items. Starts with item at position 1 (position 0 is always paper item)*/
    QLinkedList<QgsComposerItem*> mItemZList;

    /**Dpi for printout*/
    int mPrintResolution;

    /**Flag if map should be printed as a raster (via QImage). False by default*/
    bool mPrintAsRaster;

    /**Parameters for snap to grid function*/
    bool mSnapToGrid;
    double mSnapGridResolution;
    double mSnapGridOffsetX;
    double mSnapGridOffsetY;
    QPen mGridPen;
    GridStyle mGridStyle;

    QUndoStack mUndoStack;

    QgsComposerItemCommand* mActiveCommand;

    QgsComposition(); //default constructor is forbidden

    /**Reset z-values of items based on position in z list*/
    void updateZValues();

    /**Returns the bounding rectangle of the selected items in scene coordinates
     @return 0 in case of success*/
    int boundingRectOfSelectedItems( QRectF& bRect );

    void loadGridAppearanceSettings();
    void saveGridAppearanceSettings();

  signals:
    void paperSizeChanged();
};

#endif



