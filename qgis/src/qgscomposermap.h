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
//class QgsComposerMap : public QCanvasSprite, public QgsComposerItem
{
    Q_OBJECT

public:
    QgsComposerMap( QgsComposition *composition, int id, int x, int y, int width, int height );
    ~QgsComposerMap();

    /** \brief Preview style  */
    enum PreviewMode {
	Cache = 0,   // Use raster cache 
	Render,      // Render the map
	Rectangle    // Display only rectangle
    };

    // Reimplement QgsComposerItem:
    void setSelected( bool s );
    bool selected( void );
    bool writeSettings ( void );
    bool readSettings ( void );
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

public slots:
    // Called by GUI if with or height was changed 
    void sizeChanged ( void );
    
    // Set User extent to current map extent
    void setCurrentExtent ( void );
    
    // Called by GUI if with  scale was changed 
    void widthScaleChanged ( void );
    
    // Called by GUI if preview style was changed
    void previewModeChanged ( int i );

private:
    // Pointer to composition
    QgsComposition *mComposition;
    
    // Pointer to map canvas
    QgsMapCanvas *mMapCanvas;

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

    // Scale from map to paper, i.e. size_on_paper/size_in_map
    double mScale;

    // Cache used in composer preview
    // NOTE:  QCanvasView is slow with bigger images but the spped does not decrease with image size.
    //        It is very slow, with zoom in in QCanvasView, it seems, that QCanvas is stored as a big image
    //        with resolution necessary for current zoom and so always a big image mus be redrawn. 
    QPixmap *mCachePixmap; 
    
    // Resize schema
    QgsComposition::Calculate mCalculate;

    // Line width scale
    double mWidthScale;
    
    /** \brief Preview style  */
    PreviewMode mPreviewMode;

    /** \brief Number of layers when cache was created  */
    int mNumCachedLayers;
};

#endif
