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

#include <qwidget.h>
#include <qcanvas.h>
#include <qobject.h>

#include "qgsrect.h"

#include "qgscomposer.h"
#include "qgscomposition.h"
#include "qgscomposeritem.h"

#ifdef WIN32
#include "qgscomposermapbase.h"
#else
#include "qgscomposermapbase.uic.h"
#endif

class QCanvasItem;
class QCanvasRectangle;
class QPainter;
class QWidget;
class QDomNode;
class QDomDocument;
class QPixmap;
class QImage;


class QgsMapCanvas;
class QgsRect;
class QgsMapToPixel;
class QgsComposition;

/** \class QgsComposerMap 
 *  \brief Object representing map window. 
 */
// NOTE: QgsComposerMapBase must be first, otherwise does not compile
class QgsComposerMap : public QgsComposerMapBase, public QCanvasRectangle, public QgsComposerItem
{
    Q_OBJECT

public:
    /** Constructor. */
    QgsComposerMap( QgsComposition *composition, int id, int x, int y, int width, int height );
    /** Constructor. Settings are read from project. */
    QgsComposerMap( QgsComposition *composition, int id );
    ~QgsComposerMap();

    /** \brief Calculate scale/extent.  */
    enum Calculate {
	Scale = 0,   // calculate scale from extent 
	Extent      // calculate map extent from scale
    };

    /** \brief Preview style  */
    enum PreviewMode {
	Cache = 0,   // Use raster cache 
	Render,      // Render the map
	Rectangle    // Display only rectangle
    };

    /** \brief Initialise GUI and other settings, shared by constructors */
    void init ( void );

    // Reimplement QgsComposerItem:
    void setSelected( bool s );
    bool selected( void );
    QWidget *options ( void );
    bool writeSettings ( void );
    bool readSettings ( void );
    bool removeSettings ( void );
    bool writeXML( QDomNode & node, QDomDocument & document, bool temp = false );
    bool readXML( QDomNode & node );
     
    /** \brief Draw to paint device */
    void draw(QPainter *painter, QgsRect *extent, QgsMapToPixel *transform, QPaintDevice *device);

    /** \brief Reimplementation of QCanvasItem::draw - draw on canvas */
    void draw ( QPainter & painter );
    
    /** \brief Set extent requested by user */
    void setUserExtent ( QgsRect const & rect);

    /** \brief Recalculate rectangle/extent/scale according to current rule */
    void recalculate ( void );
    
    /** \brief Create cache image */
    void cache ( void );

    /** \brief Set values in GUI to current values */
    void setOptions ( void );
    
    /** \brief Map name, used in legend combobox etc. */
    QString name ( void );

    /** \brief Width scale */
    double widthScale(void);
    
    /** \brief Symbol scale */
    double symbolScale ( void );
    
    /** \brief Font size scale */
    double fontScale ( void );

    /** \brief Scale */
    double scale ( void );

public slots:
    // Called by GUI if with or height was changed 
    void sizeChanged ( void );
    
    // Set User extent to current map extent
    void setCurrentExtent ( void );

    // Called by GUI if calculate has changed 
    void calculateChanged ( void );
    
    // Called by GUI if map scale has changed 
    void mapScaleChanged ( void );
    
    // Called by GUI if with  scale was changed 
    void scaleChanged ( void );
    
    // Frame settings changed 
    void frameChanged ( void );
    
    // Called by GUI if preview style was changed
    void previewModeChanged ( int i );
    
    // Called if map canvas has changed
    void mapCanvasChanged ( );
    
    // Set cache outdated
    void setCacheUpdated ( bool u = false );

private:
    // Pointer to composition
    QgsComposition *mComposition;
    
    // Pointer to map canvas
    QgsMapCanvas *mMapCanvas;
    
    /** \brief Map name, used in legend combobox etc. */
    QString mName;

    // Map region in map units specified by user 
    QgsRect mUserExtent;

    // Map region in map units realy used for rendering 
    // It can be the same as mUserExtent, but it can be bigger in on dimension if mCalculate==Scale,
    // so that full rectangle in paper is used.
    QgsRect mExtent;

    // Cache extent (it can be bigger for example than mExtent)
    QgsRect mCacheExtent;

    // Size of of the map rectangle in the composition in paper units
    //double mWidth;
    //double mHeight;

    // Number of paper units in map per paper unit on paper, this is the xxx part of 1:xxx 
    double mUserScale;

    // Scale from map (in map units) to paper (in canvas points), i.e. size_on_paper/size_in_map
    double mScale;

    // Cache used in composer preview
    // NOTE:  QCanvasView is slow with bigger images but the spped does not decrease with image size.
    //        It is very slow, with zoom in in QCanvasView, it seems, that QCanvas is stored as a big image
    //        with resolution necessary for current zoom and so always a big image mus be redrawn. 
    QPixmap mCachePixmap; 

    // Is cache up to date
    bool mCacheUpdated;
    
    // Resize schema
    int mCalculate;

    // Line width scale
    double mWidthScale;

    // Symbol scale
    double mSymbolScale;

    // Font size scale from screen pixels to typographic points
    double mFontScale;
    
    /** \brief Preview style  */
    PreviewMode mPreviewMode;

    /** \brief Number of layers when cache was created  */
    int mNumCachedLayers;

    /** \brief Draw frame  */
    bool mFrame;

    /** \brief set to true if in state of drawing, other requests are to draw are returned */
    bool mDrawing;

    /** \brief calculate mScale from mUserScale */
    double scaleFromUserScale ( double us );

    /** \brief calculate mUserScale from mScale */
    double userScaleFromScale ( double s );
};

#endif
