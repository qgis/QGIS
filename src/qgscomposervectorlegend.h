/***************************************************************************
                         qgscomposervectorlegend.h
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
#ifndef QGSCOMPOSERVECTORLEGEND_H
#define QGSCOMPOSERVECTORLEGEND_H

/*           
 *                    |<>| - mMargin         
 *           
 *                    +----------------------+
 *                    |                      |        
 *                    |    Legend Title      |
 *                    |                      |
 *                    |  Section             |
 *                    |                      |
 *                --  |  +-----+             |  __ 
 *  mSymbolHeight |   |  |     | Item Label  |  __| - mFont->pointSize()
 *                --  |  +-----+             |  --          
 *                    |                      |    | - mSymbolSpace (vertical space between symbo, boxes)
 *                    |  +-----+             |  --
 *                    |  |     | Item Label  |
 *                    |  +-----+             |            
 *                    |                      |
 *                    +----------------------+
 *
 *                             
 *                       |<--->| - mSymbolWidth (lines and areas)
 *                   
 */ 
 
#include <qwidget.h>
#include <qcanvas.h>
#include <qobject.h>
#include <map>
#include "qgsrect.h"

#include "qgscomposer.h"
#include "qgscomposition.h"
#include "qgscomposeritem.h"

#ifdef WIN32
#include "qgscomposervectorlegendbase.h"
#else
#include "qgscomposervectorlegendbase.uic.h"
#endif

class QCanvasItem;
class QCanvasRectangle;
class QPainter;
class QWidget;
class QDomNode;
class QDomDocument;
class QPixmap;
class QImage;
class QFont;
class QPen;
class QRect;
class QPopupMenu;


class QgsMapCanvas;
class QgsRect;
class QgsMapToPixel;
class QgsComposition;
class QgsComposerMap;

/** \class QgsComposerVectorLegend 
 *  \brief Object representing map window. 
 */
// NOTE: QgsComposerVectorLegendBase must be first, otherwise does not compile
class QgsComposerVectorLegend : public QgsComposerVectorLegendBase, public QCanvasRectangle, public QgsComposerItem
{
    Q_OBJECT

public:
    /** \brief Constructor  
     *  \param id object id
     *  \param fontSize font size in typographic points!
     */
    QgsComposerVectorLegend( QgsComposition *composition, int id, int x, int y, int fontSize = 0 );

    /** \brief Constructor. Settings are read from project. 
     *  \param id object id
     */
    QgsComposerVectorLegend( QgsComposition *composition, int id );
    ~QgsComposerVectorLegend();

    /** \brief Preview style  */
    enum PreviewMode {
	Cache = 0,   // Use raster cache 
	Render,      // Render the map
	Rectangle    // Display only rectangle
    };

    /** \brief Initialise GUI etc., share by constructors. */
    void init(void);

    // Reimplement QgsComposerItem:
    void setSelected( bool s );
    bool selected( void );
    QWidget *options ( void );
    bool writeSettings ( void );
    bool readSettings ( void );
    bool removeSettings ( void );
    bool writeXML( QDomNode & node, QDomDocument & document, bool temp = false );
    bool readXML( QDomNode & node );
     
    /** \brief Draw to paint device, internal use 
     *  \param painter painter or 0
     *  \return bounding box 
     */
    QRect render (QPainter *painter);

    /** \brief Reimplementation of QCanvasItem::draw - draw on canvas */
    void draw ( QPainter & painter );
    
    /** \brief Calculate size according to current settings */
    void recalculate ( void );
    
    /** \brief Create cache image */
    void cache ( void );

    /** \brief Set values in GUI to current values */
    void setOptions ( void );

    /** \brief Is layer on/off ? */
    bool layerOn ( QString id );

    /** \brief set layer on/off */
    void setLayerOn ( QString id, bool on );

    /** \brief get layer group, 0 == no group */
    int layerGroup ( QString id );

    /** \brief set layer group, 0 == no group */
    void setLayerGroup ( QString id, int group );

public slots:
    // Open font dialog
    void changeFont ( void );

    // Title changed
    void titleChanged ( void );
    
    // Called by GUI if preview style was changed
    void previewModeChanged ( int i );

    // Called by GUI when map selection changed
    void mapSelectionChanged ( int i );

    // Called when map was changed
    void mapChanged ( int id );

    // Show popup menu
    void showLayersPopupMenu ( QListViewItem * lvi, const QPoint & pt, int );

    // Layer status changed
    void layerChanged ( QListViewItem *lvi );

    // Combine selected layers
    void groupLayers( void );

    // Frame settings changed
    void frameChanged ( void );

private:
    // Pointer to composition
    QgsComposition *mComposition;
    
    // Pointer to map canvas
    QgsMapCanvas *mMapCanvas;
    
    // Composer map id or 0
    int mMap;

    // Vector of map id for maps in combobox
    std::vector<int> mMaps;

    // Title 
    QString mTitle;

    // Font. Font size in typographic points!
    QFont mTitleFont;
    QFont mSectionFont;
    QFont mFont;

    // Pen
    QPen  mPen;

    int mMargin;
    int mSymbolHeight;
    int mSymbolWidth;
    int mSymbolSpace;

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

    /** \brief Keeps info if the layer is on or off */
    std::map<QString,bool> mLayersOn;

    /** \brief layer groups */
    std::map<QString,int> mLayersGroups;

    /** \brief new layer group id */
    int mNextLayerGroup;

    /** \brief Layers list popup menu */
    QPopupMenu *mLayersPopupMenu;

    /** \brief Draw frame  */
    bool mFrame;
};

#endif
