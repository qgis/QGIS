/***************************************************************************
                          qgisapp.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#ifndef QGISAPP_H
#define QGISAPP_H

class QCanvas;
class QRect;
class QCanvasView;
class QStringList;
class QScrollView;
class QgsPoint;
class QgsLegend;
class QgsLegendView;
class QVBox;
class QCursor;
class QLabel;
class QListView;
class QListViewItem;
class QProgressBar;
class QFileInfo;
class QgsMapLayer;
class QSocket;
class QgsProviderRegistry;
class QgsHelpViewer;
class QgsMapCanvas;
class QgsMapLayerRegistry;
class QgsRasterLayer;
#ifdef WIN32
#include "qgisappbase.h"
#else
#include "qgisappbase.uic.h"
#endif
#include "qgisiface.h"
#include "splashscreen.h"
#include "qgsconfig.h"

static SplashScreen * gSplashScreen ;


/*! \class QgisApp
 * \brief Main window for the Qgis application
 */
class QgisApp : public QgisAppBase
{
    Q_OBJECT;

public:

    //! Constructor
    QgisApp(QWidget * parent = 0, const char *name = 0, WFlags fl = WType_TopLevel);

    ~QgisApp();

    QgisIface *getInterface();

    /** \brief Set the Z order of both mapcanvas and overview
     * canvas. Typically this will be called by projectio when loading a
     * stored project.
     */
    void setZOrder (std::list<QString>);
    
    void addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey);
    /** \brief overloaded vesion of the privat addLayer method that takes a list of
    * filenames instead of prompting user with a dialog. 

    @returns true if successfully added layer

    @note

    This should be deprecated because it's possible to have a
    heterogeneous set of files; i.e., a mix of raster and vector.
    It's much better to try to just open one file at a time.

    */
    bool addLayer(QStringList const & theLayerQStringList);

    /** open a vector layer for the given file

      @returns false if unable to open a raster layer for rasterFile

      @note

      This is essentially a simplified version of the above
    */
    bool addLayer(QFileInfo const & vectorFile);


    /** overloaded vesion of the private addRasterLayer()

    Method that takes a list of filenames instead of prompting
    user with a dialog.

    @returns true if successfully added layer(s)

    @note

    This should be deprecated because it's possible to have a
    heterogeneous set of files; i.e., a mix of raster and vector.
    It's much better to try to just open one file at a time.

    */
    bool addRasterLayer(QStringList const & theLayerQStringList, bool guiWarning=true);


    /** open a raster layer for the given file

       @returns false if unable to open a raster layer for rasterFile

       @note

       This is essentially a simplified version of the above
    */
    bool addRasterLayer(QFileInfo const & rasterFile, bool guiWarning=true);

    /** Add a 'pre-made' map layer to the project */
    void addMapLayer(QgsMapLayer *theMapLayer);
   
    /** Set the extents of the map canvas */
    void setExtent(QgsRect theRect);
    
    //! Remove all layers from the map and legend - reimplements same method from qgisappbase
    void removeAllLayers();
    /** opens a qgis project file
    @returns false if unable to open the project

    */
    bool addProject(QString projectFile);

    //!Overloaded version of the private function with same name that takes the imagename as a parameter
    void saveMapAsImage(QString, QPixmap *);
    QgsMapCanvas * getMapCanvas() { return mMapCanvas; };

    /** return the layer registry

       @note

       Returns QgsMapLayerRegistry::instance(); i.e., it's a Singleton
    */
    QgsMapLayerRegistry * getLayerRegistry();

    //! Set theme (icons)
    void setTheme(QString themeName="default");
    //! Setup the toolbar popup menus for a given theme
    void setupToolbarPopups(QString themeName);

private:

    //! Add a vector layer to the map
    void addLayer();
    //! Add a raster layer to the map (will prompt user for filename using dlg
    void addRasterLayer();
    //! Add a raster layer to the map (passed in as a ptr). It waont force a refresh unless you explicitly 
    //use the force redraw flag.
    //
    bool addRasterLayer(QgsRasterLayer * theRasterLayer, bool theForceRedrawFlag=false);
    //@todo We should move these next two into vector layer class
    /** This helper checks to see whether the filename appears to be a valid vector file name */
    bool isValidVectorFileName (QString theFileNameQString);
    /** Overloaded version of the above function provided for convenience that takes a qstring pointer */
    bool isValidVectorFileName (QString * theFileNameQString);
#ifdef HAVE_POSTGRESQL
    //! Add a databaselayer to the map
    void addDatabaseLayer();
#endif

    //! Exit Qgis
    void fileExit();

    //! Set map tool to Zoom out
    void zoomOut();
    //! Set map tool to Zoom in
    void zoomIn();
    //! Zoom to full extent
    void zoomFull();
    //! Zoom to the previous extent
    void zoomPrevious();
    //! Zoom to selected features
    void zoomToSelected();
    //! Set map tool to pan
    void pan();
    //! Identify feature(s) on the currently selected layer
    void identify();
    //! show the attribute table for the currently selected layer
    void attributeTable();
    /**Deletes the selected attributes for the currently selected vector layer*/
    void deleteSelected();
    //! Read Well Known Binary stream from PostGIS
    //void readWKB(const char *, QStringList tables);
    //! Draw a point on the map canvas
    void drawPoint(double x, double y);
    //! draw layers
    void drawLayers();
    //! test function
    void testButton();
    //! About QGis
    void about();
    //! activates the capture point tool
    void capturePoint();
    //! activates the capture line tool
    void captureLine();
    //! activates the capture polygon tool
    void capturePolygon();
    //! activates the selection tool
    void select();
    //! check to see if file is dirty and if so, prompt the user th save it
    int saveDirty();

private slots:


   /** for when a menu bar item is activated
       Used to dynamically update pop-up menu items
   */
    /* virtual */ void menubar_highlighted( int i );

 
    /** toggles whether the current selected layer is in overview or not */
    void inOverview(bool);
    //! Slot to show the map coordinate position of the mouse cursor
    void showMouseCoordinate(QgsPoint &);
    //copy the click coord to clipboard and let the user know its there
    void showCapturePointCoordinate(QgsPoint &);
    //! Slot to show current map scale;
    void showScale(QString theScale);
    //! Show layer properties for the selected layer
    void layerProperties(QListViewItem *);
    //! Show layer properties for selected layer (called by right-click menu)
    void layerProperties();
    //! Show the right-click menu for the legend
    void rightClickLegendMenu(QListViewItem *, const QPoint &, int);
    //! Disable/enable toolbar buttons as appropriate for selected layer
    void currentLayerChanged(QListViewItem *);
    //! Remove a layer from the map and legend
    void removeLayer();
    //! zoom to extent of layer
    void zoomToLayerExtent();
    //! load any plugins used in the last qgis session
    void restoreSessionPlugins(QString thePluginDirString);
    //! test plugin functionality
    void testPluginFunctions();
    //! test maplayer plugins
    void testMapLayerPlugins();
    //! plugin manager
    void actionPluginManager_activated();
    //! plugin loader
    void loadPlugin(QString name, QString description, QString mFullPath);
    //! Add a plugin menu to the main Plugins menu
    int addPluginMenu(QString menuText, QPopupMenu *menu);
    //! Add an icon to the plugin toolbar
    int addPluginToolBarIcon (QAction * qAction);
    //! Remove an icon from the plugin toolbar
    void removePluginToolBarIcon(QAction *qAction);
    //! Save window state
    void saveWindowState();
    //! Restore the window and toolbar state
    void restoreWindowState();
    //! Save project
    void fileSave();
    //! Save project as
    void fileSaveAs();
    //! Save the map view as an image - user is prompted for image name using a dialog
    void saveMapAsImage();
    //! Open a project
    void fileOpen();
    //! Create a new project
    void fileNew();
    //! As above but allows forcing without prompt
    void fileNew(bool thePromptToSaveFlag);
    //! Create a new empty vector layer
    void newVectorLayer();
    //! Print the current map view frame
    void filePrint();
    //! Add all loaded layers into the overview - overides qgisappbase method
    void addAllToOverview();
    //! Remove all loaded layers from the overview - overides qgisappbase method
    void removeAllFromOverview();
    //reimplements method from base (gui) class
    void hideAllLayers();
    //reimplements method from base (gui) class
    void showAllLayers();


    
    //! Export current view as a mapserver map file
    void exportMapServer();
    //! Return pointer to the active layer
    QgsMapLayer *activeLayer();
    //! Return data source of the active layer
    QString activeLayerSource();
    //! Open the help contents in a browser
    void helpContents();
    //! Open the QGIS homepage in users browser
    void helpQgisHomePage();
    //! Open the QGIS Sourceforge page in users browser
    void helpQgisSourceForge();
    //! Open a url in the users configured browser
    void openURL(QString url, bool useQgisDocDirectory=true);
    //! Check qgis version against the qgis version server
    void checkQgisVersion();
    //! options dialog slot
    void options();
    //! Whats-this help slot
    void whatsThis();
    //! Get the Menu map (key is name, value is menu id)
    std::map<QString, int> menuMapByName();
    //! Get the Menu map (key is menu id, value is name)
    std::map<int, QString> menuMapById();
    //! Populate the menu maps
    void populateMenuMaps();
    void socketConnected();
    void socketConnectionClosed();
    void socketReadyRead();
    void socketError(int e);
    //! Set project properties, including map untis
    void projectProperties();
    /*  void urlData(); */
    /** Used to (re)set the zordering of the overview map*/
    void setOverviewZOrder(QgsLegend * );
    //! Kills the splash screen 
    void killSplashScreen();
public slots:
    void showProgress(int theProgress, int theTotalSteps);
    void showExtents(QgsRect theExtents);
    void showStatusMessage(QString theMessage);
    void setLayerOverviewStatus(QString theLayerId, bool theVisibilityFlag);
    void drawExtentRectangle(QPainter *);
private:

    /// QgisApp aren't copyable
    QgisApp( QgisApp const & );

    /// QgisApp aren't copyable
    QgisApp & operator=( QgisApp const & );

    //! A central registry that keeps track of all loaded layers.
    // prefer QgsMapLayerRegistry::instance() to emphasize Singleton
    ///QgsMapLayerRegistry * mMapLayerRegistry;
    //! Widget that will live on the statusbar to display scale
    QLabel * mScaleLabel;
    //! Widget that will live in the statusbar to display coords
    QLabel * mCoordsLabel;
    //! Widget that will live in the statusbar to show progress of operations
    QProgressBar * mProgressBar;
    //! Popup menu
    QPopupMenu * mPopupMenu;
    //! Top level plugin menu
    QPopupMenu *mPluginMenu;
    //! Popup menu for the map overview tools
    QPopupMenu *toolPopupOverviews;
    //! Popup menu for the display tools
    QPopupMenu *toolPopupDisplay;
    //! Popup menu for the capture tools
    QPopupMenu *toolPopupCapture;
    //! Legend list view control
    //doesnt see to be used...(TS)
    //QgsLegendView *mLegendView;
    //! Map canvas
    QgsMapCanvas *mMapCanvas;
    //! Map layer registry
    // use instance() now QgsMapLayerRegistry *mLayerRegistry;
    //! Overview canvas where the map overview is shown
    QgsMapCanvas * mOverviewCanvas;
    //! Table of contents (legend) for the map
    QgsLegend *mMapLegend;
    QCursor *mMapCursor;
    //! scale factor
    double mScaleFactor;
    //! Current map window extent in real-world coordinates
    QRect *mMapWindow;
    //! Current map tool
    int mMapTool;
    //QCursor *mCursorZoomIn; //doesnt seem to be used anymore (TS)
    QString mStartupPath;
    //! full path name of the current map file (if it has been saved or loaded)
    QString mFullPathName;
    QgisIface *mQgisInterface;
    QSocket *mSocket;
    QString mVersionMessage;
    friend class QgisIface;
    QgsProviderRegistry *mProviderRegistry;
    //! application directory
    QString mAppDir;
    //! help viewer
    QgsHelpViewer *mHelpViewer;

    //! Flag to indicate if the splash screen is shown on startup
    bool myHideSplashFlag;

    //! menu map (key is name, value is menu id)
    std::map<QString, int>mMenuMapByName;
    //! menu map (key is menu id, value is name)
    std::map<int, QString>mMenuMapById;
};

#endif
