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
#include <list>
#include <vector>

#include <qprinter.h>
#include <qpoint.h>

#ifdef WIN32
#include "qgscompositionbase.h"
#else
#include "qgscompositionbase.uic.h"
#endif

#include "qgsrect.h"

class QWidget;
class QCanvas;
class QCanvasItem;
class QCanvasRectangle;
class QMouseEvent;
class QDomNode;
class QDomDocument;

class QgsComposerView;
class QgsComposer;
class QgsComposerItem;
class QgsComposerMap;
class QgsMapCanvas;

class QgsCompositionPaper
{
public:
    QString mName;
    int mWidth;
    int mHeight;
    int mCustom;

    QgsCompositionPaper( QString name, int w, int h, bool c = false);
    ~QgsCompositionPaper();
};

/** \class QgsComposition
 * \brief This class can store, write as XML and read from XML the description
 *        map composition for printing.
 */
class QgsComposition: public QgsCompositionBase
{
    Q_OBJECT
public:
    /** \brief Constructor. */
    QgsComposition ( QgsComposer *c, int id ); 

    /** \brief The destuctor.  */
    ~QgsComposition();

    /** \brief Orientation  */
    enum Orientation {
	Portrait = 0,
	Landscape
    };

    /** \brief Current tool  */
    enum Tool {
	Select = 0,      // Select/Move item
	AddMap,          // add new map 
	AddVectorLegend, // add vector legend
	AddLabel,        // add label
	AddScalebar,     // add scalebar
	AddPicture       // add raster/vector picture
    };

    /** \brief Scaling mode, defines which parameters are fixed and which are changing  */
    enum Calculate {
	Scale = 0, // Extent and paper are defined by user, scale is calculated
	Paper,     // Extent and scale are defined by user, paper is calculated
	Extent     // Paper and scale are defined by user, extent is calculated
    };

    /** \brief Plot type */
    enum PlotStyle {
	Preview = 0, // Use cache etc
	Print,       // Render well
	Postscript   // Fonts need different scaling!
    };

    /** \brief Composition ID */
    int id ( void );

    /** \brief Get paper width */
    double paperWidth ( void );

    /** \brief Get paper height */
    double paperHeight ( void );
    
    /** \brief Get paper orientation */
    int paperOrientation ( void );

    /** \brief Get resolutin */
    int resolution ( void );

    /** \brief Create default composition */
    void createDefault ( void );

    /** \brief Remove all items */
    void clear ( void );

    /** \brief Recalculate page size according to mUserPaperWidth/Height and mPaperOrientation,
     *         resize canvas and zoomFull */
    void recalculate(void);

    /** \brief pointer to map canvas */
    QgsMapCanvas *mapCanvas(void);

    /** \brief pointer to composer */
    QgsComposer *composer(void);

    // Return pointer to widget with composition options 
    //QWidget *options ( void );

    /** \brief Set this composition as active, show its canvas with items and show general options */
    void setActive ( bool active );

    /** \brief returns pointer to canvas */
    QCanvas *canvas(void);
    
    /** \brief recieves contentsMousePressEvent from view */
    void contentsMousePressEvent(QMouseEvent*);
    
    /** \brief recieves contentsMouseReleaseEvent from view */
    void contentsMouseReleaseEvent(QMouseEvent*);
    
    /** \brief recieves contentsMouseMoveEvent from view */
    void contentsMouseMoveEvent(QMouseEvent*);
    
    /** \brief recieves keyPressEvent from view */
    void keyPressEvent ( QKeyEvent * e );

    /** \brief set plot style, use before print */
    void setPlotStyle ( PlotStyle p );

    /**  \brief Set composition optiones in GUI */
    void setOptions ( void );
    
    /**  \brief Set tool */
    void setTool ( Tool tool );

    /** Refresh. Refresh objects which are not updated automaticaly, e.g. map object does not know
     * if a layer was switched on/off. Later should be substituted by appropriate signals 
     * se by map canvas */
    void refresh();

    /**  \brief Canvas scale */
    int scale (void);

    /**  \brief convert canvas unit to mm */
    double toMM ( int v );
    
    /**  \brief convert mm to canvas unit */
    int fromMM ( double v );

    /** \brief Return number screen pixels / canvas point */
    double viewScale( void );

    /** \brief Selection box size in _canvas_ units */
    int selectionBoxSize ( void );
    
    /** \brief Selection box pen and brush */
    QPen selectionPen ( void );
    QBrush selectionBrush ( void );
    
    /** \brief vector of pointers to maps available in composition */
    std::vector<QgsComposerMap*> maps(void);

    /** \brief Find a map by id 
     *  \param id canvas item id
     *  \return pointer to existing map or 0
     */
    QgsComposerMap * map (int id);

    /** \brief stores statei in project */
    bool writeSettings ( void );

    /** \brief read state from project */
    bool readSettings ( void );

    /** \brief  stores state in DOM node */
    bool writeXML( QDomNode & node, QDomDocument & doc, bool templ = false );

    /** \brief sets state from DOM document */
    bool readXML( QDomNode & node );

public slots:
    /**  \brief Called by GUI if paper size was changed */
    void paperSizeChanged ( void );
    
    /**  \brief Called by GUI if resolution was changed */
    void resolutionChanged ( void );

    /**  \brief Called map objects if changed, so that the composition can emit signal */
    void emitMapChanged ( int id );

signals:
    /**  \brief Emitted when map was changed */    
    void mapChanged ( int id );

private:
    /** \brief composition id */
    int mId;

    /** \brief paper width in mm in GUI */
    double mUserPaperWidth;

    /** \brief paper height in mm in GUI */
    double mUserPaperHeight;

    /** \brief paper width in mm (orientaion applied)  */
    double mPaperWidth;

    /** \brief paper height in mm (orientaion applied) */
    double mPaperHeight;

    /** \brief Papers */
    std::vector<QgsCompositionPaper> mPapers;

    /** \brief Current paper */
    int mPaper;
	
    /** \brief Default paper index */
    int mDefaultPaper;

    /** \brief Custom paper index */
    int mCustomPaper;

    /** \brief Orientation */
    int mPaperOrientation;

    /** \brief pointer to map canvas */
    QgsMapCanvas *mMapCanvas;

    /** \brief pointer to composer */
    QgsComposer *mComposer;

    /** \brief Canvas. One per composition, created by QgsComposition */
    QCanvas *mCanvas;

    /** \brief Pointer to canvas view */
    QgsComposerView *mView;

    /** \brief List of all composer items */
    std::list<QgsComposerItem*> mItems;

    /** \brief Last position of the mouse in mView */
    double mLastX;
    double mLastY;

    /** \brief Selected item, 0 if no item is selected */ 
    QCanvasItem* mSelectedItem; 

    /** \brief Item representing the paper */
    QCanvasRectangle *mPaperItem;

    /** \brief  Plot style */
    PlotStyle mPlotStyle;
    
    /** \brief  Current tool */
    Tool mTool;
    
    /** \brief  Tool step first is 0 */
    int mToolStep;
    
    /** \brief Temporary rectangle item used as rectangle drawn by mouse */
    QCanvasRectangle *mRectangleItem;
    
    /** \brief Temporary item used as pointer to new objecs which must be drawn */ 
    QCanvasItem *mNewCanvasItem; 

    /** \breif Resolution in DPI */
    int mResolution; 

    /** \brief canvas scale */
    int mScale;

    /** \brief id for next new item */
    int mNextItemId;

    /** \brief Create canvas */
    void createCanvas(void);
    
    /** \brief Resize canvas to current paper size */
    void resizeCanvas(void);

    /** \brief first point, set with MousePressEvent */
    QPoint mLastPoint;
};

#endif

