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
 
#include "ui_qgscomposervectorlegendbase.h"
#include "qgscomposeritem.h"

#include <QGraphicsRectItem>
#include <QPen>

class QgsComposition;
class QgsMapCanvas;
class QDomNode;
class QDomDocument;
class QTreeWidgetItem;

/** \class QgsComposerVectorLegend 
 *  \brief Object representing map window. 
 */
// NOTE: QgsComposerVectorLegendBase must be first, otherwise does not compile
class QgsComposerVectorLegend : public QWidget,
                                private Ui::QgsComposerVectorLegendBase, 
                                public QGraphicsRectItem, 
                                public QgsComposerItem
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
    QRectF render (QPainter *painter);

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint ( QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
    
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
    void on_mFontButton_clicked ( void );

    // Title changed
    void on_mTitleLineEdit_editingFinished ( void );
    
    // Called by GUI if preview style was changed
    void on_mPreviewModeComboBox_activated ( int i );

    // Called by GUI when map selection changed
    void on_mMapComboBox_activated ( int i );

    // Called when map was changed
    void mapChanged ( int id );

    // Layer status changed
    void layerChanged ( QTreeWidgetItem *lvi );

    // Combine selected layers
    void groupLayers( void );

    // Frame settings changed
    void on_mFrameCheckBox_stateChanged ( int i );

protected:
    // Show popup menu
    void contextMenuEvent ( QContextMenuEvent * event );

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

    double mMargin;
    double mSymbolHeight;
    double mSymbolWidth;
    double mSymbolSpace;

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

    /** \brief Draw frame  */
    bool mFrame;
};

#endif
