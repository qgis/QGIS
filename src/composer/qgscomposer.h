/***************************************************************************
                         qgscomposer.h
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
/* $Id$ */
#ifndef QGSCOMPOSER_H
#define QGSCOMPOSER_H
#include "ui_qgscomposerbase.h"

class QgisApp;
class QgsComposerView;
class QgsComposition;
class QgsMapCanvas;

class QGridLayout;
class QPrinter;
class QDomNode;
class QDomDocument;
class QMoveEvent;
class QResizeEvent;

/* The constructor creates empty composer, without compositions and mFirstTime set to true. 
 * - if signal projectRead() is recieved all old compositions are deleted and
 *     - if the composition exists in project it is created from project settings (mFirstTime set to false)
 *     - if the composition does not exist in project 
 *         - if the composer is visible new default composition is created (mFirstTime set to false)
 *         - if the composer is not visible the composer is left empty (mFirstTime set to true)
 * - if signal newProject() is recieved all old compositions are deleted and 
 *     - if the composer is visible a new default composition is created (mFirstTime set to false)
 *     - if the composer is not visible the composer is left empty (mFirstTime set to true)
 *         
 * If open() is called and mFirstTime == true, a new default composition is created.
 *
 */
class QgsComposer: public Q3MainWindow, private Ui::QgsComposerBase
{
    Q_OBJECT

public:
    QgsComposer(QgisApp *qgis);
    ~QgsComposer();
    
    //! Open and show, set defaults if first time
    void open();

    //! Zoom to full extent of the paper
    void zoomFull();

    //! Select item
    void selectItem();

    //! Return pointer to map canvas
    QgsMapCanvas *mapCanvas( void );
    
    //! Return pointer to composer view
    QgsComposerView *view ( void );

    //! Return current composition
    QgsComposition *composition(void);

    //! Show composition options in widget
    void showCompositionOptions ( QWidget *w );
    
    //! Show item options in widget
    void showItemOptions ( QWidget *w );

    /** \brief stores statei in project */
    bool writeSettings ( void );

    /** \brief read state from project */
    bool readSettings ( void );

    //! Stores state in DOM node
    bool writeXML( QDomNode & node, QDomDocument & doc);

    //! Sets state from DOM document
    bool readXML( QDomNode & node );

    //! Save window state
    void saveWindowState();

    //! Restore the window and toolbar state
    void restoreWindowState();

    //! Move event
    void moveEvent ( QMoveEvent * );
    
    //! Resize event
    void resizeEvent ( QResizeEvent * );

public slots:
    //! Zoom to full extent of the paper
    void on_actionZoomFull_activated(void);

    //! Zoom in
    void on_actionZoomIn_activated(void);

    //! Zoom out 
    void on_actionZoomOut_activated(void);

    //! Refresh view 
    void on_actionRefresh_activated(void);

    //! Print the composition
    void on_actionPrint_activated(void);
    
    //! Print as image
    void on_actionImage_activated(void);
    
    //! Print as SVG
    void on_actionSvg_activated(void);
    
    //! Select item
    void on_actionSelectItem_activated(void);
    
    //! Add new map
    void on_actionAddMap_activated(void);

    //! Add new vector legend
    void on_actionAddVectorLegend_activated(void);
    
    //! Add new label
    void on_actionAddLabel_activated(void);
    
    //! Add new scalebar
    void on_actionAddScalebar_activated(void);
    
    //! Add new picture
    void on_actionAddPicture_activated(void);

    //! read project
    void projectRead();

    //! New project
    void newProject();

private:
    //! remove widget childrens
    void removeWidgetChildren ( QWidget *w );

    //! Set buttons up
    void setToolActionsOff (void);

    //! Pointer to composer view
    QgsComposerView *mView;

    //! Current composition
    QgsComposition *mComposition;
    
    //! Printer
    QPrinter *mPrinter;

    //! Pointer to QGIS application
    QgisApp *mQgis;

    //! The composer was opened first time (-> set defaults)
    bool mFirstTime;

    //! Layout 
    QGridLayout *mCompositionOptionsLayout;

    //! Layout
    QGridLayout *mItemOptionsLayout;
};

#endif
