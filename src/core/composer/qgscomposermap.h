/***************************************************************************
                         qgscomposermap.h
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
#ifndef QGSCOMPOSERMAP_H
#define QGSCOMPOSERMAP_H

//#include "ui_qgscomposermapbase.h"
#include "qgscomposeritem.h"
#include "qgsrect.h"
#include <QGraphicsRectItem>
#include <QObject>
#include <QPixmap>

class QgsComposition;
class QgsMapRenderer;
class QgsMapToPixel;
class QDomNode;
class QDomDocument;
class QPainter;

/** \ingroup MapComposer
 *  \class QgsComposerMap 
 *  \brief Object representing map window. 
 */
// NOTE: QgsComposerMapBase must be first, otherwise does not compile
class CORE_EXPORT QgsComposerMap : /*public QWidget , private Ui::QgsComposerMapBase,*/ public QObject, public QgsComposerItem
{
  Q_OBJECT

public:
    /** Constructor. */
    QgsComposerMap( QgsComposition *composition, int x, int y, int width, int height );
    /** Constructor. Settings are read from project. */
    QgsComposerMap( QgsComposition *composition);
    ~QgsComposerMap();

    /** \brief Preview style  */
    enum PreviewMode {
	Cache = 0,   // Use raster cache 
	Render,      // Render the map
	Rectangle    // Display only rectangle
    };

    /** \brief Initialise GUI and other settings, shared by constructors */
    void init ( void );
     
    /** \brief Draw to paint device 
	@param extent map extent
	@param size size in scene coordinates
	@param dpi scene dpi*/
    void draw(QPainter *painter, const QgsRect& extent, const QSize& size, int dpi);

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint (QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget);

    /** \brief Recalculate rectangle/extent/scale according to current rule */
    void recalculate ( void );
    
    /** \brief Create cache image */
    void cache ( void );
    
    /** \brief Get identification number*/
    int id() const {return mId;}

    /**True if a draw is already in progress*/
    bool isDrawing() const {return mDrawing;}

    /** resizes an item in x- and y direction (canvas coordinates)*/
    void resize(double dx, double dy);

    /**Move content of map
       @param dx move in x-direction (item and canvas coordinates)
       @param dy move in y-direction (item and canvas coordinates)*/
    void moveContent(double dx, double dy);

    /**Sets new scene rectangle bounds and recalculates hight and extent*/
    void setSceneRect(const QRectF& rectangle);

    /** \brief Scale */
    double scale ( void ) const;

     /**Sets new scale and changes only mExtent*/
    void setNewScale(double scaleDenominator);

    /**Sets new Extent and changes width, height (and implicitely also scale)*/
    void setNewExtent(const QgsRect& extent);

    PreviewMode previewMode() {return mPreviewMode;}
    void setPreviewMode(PreviewMode m) {mPreviewMode = m;}

    // Set cache outdated
    void setCacheUpdated ( bool u = false );

    QgsRect extent() const {return mExtent;}

    const QgsMapRenderer* mapRenderer() const {return mMapRenderer;}

    /**Sets offset values to shift image (useful for live updates when moving item content)*/
    void setOffset(double xOffset, double yOffset);

    /** stores state in DOM node
     * @param elem is DOM element corresponding to 'Composer' tag
     * @param temp write template file
     */
    bool writeXML(QDomElement& elem, QDomDocument & doc);

    /** sets state from DOM document
     * @param itemElem is DOM node corresponding to 'ComposerMap' tag
     */
    bool readXML(const QDomElement& itemElem, const QDomDocument& doc);

public slots:

    // Called if map canvas has changed
    void mapCanvasChanged ( );

 signals:
    /**Is emitted when width/height is changed as a result of user interaction*/
    void extentChanged();

private:

    // Pointer to map renderer of the QGIS main map. Note that QgsComposerMap uses a different map renderer, 
    //it just copies some properties from the main map renderer.
    QgsMapRenderer *mMapRenderer;
    
    /**Unique identifier*/
    int mId;

    // Map region in map units realy used for rendering 
    // It can be the same as mUserExtent, but it can be bigger in on dimension if mCalculate==Scale,
    // so that full rectangle in paper is used.
    QgsRect mExtent;

    // Cache used in composer preview
    // NOTE:  QCanvasView is slow with bigger images but the spped does not decrease with image size.
    //        It is very slow, with zoom in in QCanvasView, it seems, that QCanvas is stored as a big image
    //        with resolution necessary for current zoom and so always a big image mus be redrawn. 
    QPixmap mCachePixmap; 

    // Is cache up to date
    bool mCacheUpdated;
    
    /** \brief Preview style  */
    PreviewMode mPreviewMode;

    /** \brief Number of layers when cache was created  */
    int mNumCachedLayers;

    /** \brief set to true if in state of drawing. Concurrent requests to draw method are returned if set to true */
    bool mDrawing;

    /**Store last scale factor to avoid unnecessary repaints in case preview mode is 'Render'*/
    double mLastScaleFactorX;

    /**Store the last map extent to decide if cache needs to be updatet*/
    QgsRect mCachedMapExtent;

    /**Offset in x direction for showing map cache image*/
    double mXOffset;
    /**Offset in y direction for showing map cache image*/
    double mYOffset;

    /**For the generation of new unique ids*/
    static int mCurrentComposerId;

    /**Etablishes signal/slot connection for update in case of layer change*/
    void connectUpdateSlot();

    /**Returns the zoom factor of the graphics view. If no 
     graphics view exists, the default 1 is returned*/
    double horizontalViewScaleFactor() const;
};

#endif
