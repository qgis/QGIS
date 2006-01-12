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

//Added by qt3to4:
#include <QMenu>
#include <QKeyEvent>
#include <QEvent>
#include <QPixmap>
#include <QLabel>
#include <QSplashScreen>
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
class Q3ListViewItem;
class QProgressBar;
class QFileInfo;
class QgsMapLayer;
class QSettings;
class QTcpSocket;
class QgsProviderRegistry;
class QgsHelpViewer;
class QgsMapCanvas;
class QgsMapOverviewCanvas;
class QgsMapLayerRegistry;
class QgsRasterLayer;
class QCheckBox;
class QEvent;
class QgsComposer;
class QPushButton;
class QToolButton;
#include "qgisiface.h"
#include "qgsconfig.h"
#include "qgsvectordataprovider.h"
#include "qgsclipboard.h"

#include <map>

#include <ui_qgisappbase.h>
#include <QMainWindow>


/*! \class QgisApp
 * \brief Main window for the Qgis application
 */
class QgisApp : public QMainWindow, public Ui::QgisAppBase
{
  Q_OBJECT;
  public:
  //! Constructor
  QgisApp(QSplashScreen *splash, QWidget * parent = 0, Qt::WFlags fl = Qt::WType_TopLevel);
  //! Destructor
  ~QgisApp();
  /*
   * Get the plugin interface from the application
   */
  QgisIface *getInterface();
  /** \brief Set the Z order of both mapcanvas and overview
   * canvas. Typically this will be called by projectio when loading a
   * stored project.
   */
  void setZOrder (std::list<QString>);
  /*
   * Add a vector layer to the canvas
   */
  void addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey);
  /** \brief overloaded vesion of the privat addLayer method that takes a list of
   * filenames instead of prompting user with a dialog.
   @param enc encoding type for the layer 
   @returns true if successfully added layer
   @note
   This should be deprecated because it's possible to have a
   heterogeneous set of files; i.e., a mix of raster and vector.
   It's much better to try to just open one file at a time.
   */
  bool addLayer(QStringList const & theLayerQStringList, const QString& enc);

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
  /** Open a raster layer using the Raster Data Provider.
   *  Note this is included to support WMS layers only at this stage,
   *  GDAL layer support via a Provider is not yet implemented.
   */        
  void addRasterLayer(QString const & rasterLayerPath, 
      QString const & baseName, 
      QString const & providerKey,
      QStringList const & layers,
      QStringList const & styles,
      QString const & format);
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
  /** Open a raster or vector file; ignore other files.
    Used to process a commandline argument or OpenDocument AppleEvent.
    @returns true if the file is successfully opened
    */
  bool openLayer(const QString & fileName);
  /** Open the specified project file; prompt to save previous project if necessary.
    Used to process a commandline argument or OpenDocument AppleEvent.
    */
  void openProject(const QString & fileName);
  /** opens a qgis project file
    @returns false if unable to open the project
    */
  bool addProject(QString projectFile);
  //!Overloaded version of the private function with same name that takes the imagename as a parameter
  void saveMapAsImage(QString, QPixmap *);
  /** Get the mapcanvas object from the app */
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
  //! Returns a pointer to the internal clipboard
  QgsClipboard * clipboard();

//private slots:
public slots:
  //! About QGis
  void about();
  //! Add a raster layer to the map (will prompt user for filename using dlg )
  void addRasterLayer();
  //! Get the path to the active theme dir
  QString themePath();
  //#ifdef HAVE_POSTGRESQL
  //! Add a databaselayer to the map
  void addDatabaseLayer();
  //#endif
  //! reimplements widget keyPress event so we can check if cancel was pressed
  void keyPressEvent ( QKeyEvent * e );
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
  //! Disable/enable toolbar buttons as appropriate for selected layer
  void currentLayerChanged(Q3ListViewItem *);
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
  void showPluginManager();
  //! plugin loader
  void loadPlugin(QString name, QString description, QString mFullPath);
  //! Add a plugin menu to the main Plugins menu
  int addPluginMenu(QString menuText, QMenu *menu);
  //! Get the menu that holds teh list of loaded plugins 
  QMenu* getPluginMenu(QString menuName);
  //! Remove an item from the qgis main app menus 
  void removePluginMenuItem(QString name, int menuId);
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
  //! Open the project file corresponding to the
  //! path at the given index in mRecentProjectPaths
  void openProject(int pathIndex);
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
  //!Invoke the custom projection dialog
  void customProjection(); 
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
  //! Open project properties dialog and show the projections tab
  void projectPropertiesProjections();
  /*  void urlData(); */
  //! Show the spatial bookmarks dialog
  void showBookmarks();
  //! Create a new spatial bookmark
  void newBookmark();
  //! activates the capture point tool
  void capturePoint();
  //! activates the capture line tool
  void captureLine();
  //! activates the capture polygon tool
  void capturePolygon();
  //! activates the selection tool
  void select();

public slots:
  void showProgress(int theProgress, int theTotalSteps);
  void showExtents(QgsRect theExtents);
  void showStatusMessage(QString theMessage);
  void setLayerOverviewStatus(QString theLayerId, bool theVisibilityFlag);
  void drawExtentRectangle(QPainter *);
  void updateMouseCoordinatePrecision();
  void projectionsEnabled(bool theFlag);
  //    void debugHook();
  void stopZoom();
  /** Used to (re)set the zordering of the overview map*/
  void setOverviewZOrder(QgsLegend * );
  //! Add a vector layer to the map
  void addLayer();
  //! Exit Qgis
  void fileExit();
  //! Add a WMS layer to the map
  void addWmsLayer();
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
  //! Measure distance
  void measure();
  //! Measure area
  void measureArea();
  //! show the attribute table for the currently selected layer
  void attributeTable();

signals:
  /** emitted when a key is pressed and we want non widget sublasses to be able
    to pick up on this (e.g. maplayer) */
  void keyPressed (QKeyEvent *e);

  /** emitted when a project file is successfully read
    @note
    This is useful for plug-ins that store properties with project files.  A
    plug-in can connect to this signal.  When it is emitted, the plug-in
    knows to then check the project properties for any relevant state.
    */
  void projectRead();
  /** emitted when starting an entirely new project
    @note
    This is similar to projectRead(); plug-ins might want to be notified
    that they're in a new project.  Yes, projectRead() could have been
    overloaded to be used in the case of new projects instead.  However,
    it's probably more semantically correct to have an entirely separate
    signal for when this happens.
    */
  void newProject();

  //! emitted when a new bookmark is added 
  void bookmarkAdded();

private:
  /** Add a raster layer to the map (passed in as a ptr). 
   * It won't force a refresh unless you explicitly
   * use the force redraw flag.
   */
  bool addRasterLayer(QgsRasterLayer * theRasterLayer, bool theForceRedrawFlag=false);
  //@todo We should move these next two into vector layer class
  /** This helper checks to see whether the filename appears to be a valid vector file name */
  bool isValidVectorFileName (QString theFileNameQString);
  /** Overloaded version of the above function provided for convenience that takes a qstring pointer */
  bool isValidVectorFileName (QString * theFileNameQString);
  /** add this file to the recently opened/saved projects list
   *  pass settings by reference since creating more than one
   * instance simultaneously results in data loss.
   */
  void saveRecentProjectPath(QString projectPath, QSettings & settings);
  //! Update file menu with the current list of recently accessed projects
  void updateRecentProjectPaths();
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
  //! activates the add vertex tool
  void addVertex();
  //! activates the move vertex tool
  void moveVertex();
  //! activates the delete vertex tool
  void deleteVertex();
  //! cuts selected features on the active layer to the clipboard
  void editCut();
  //! copies selected features on the active layer to the clipboard
  void editCopy();
  //! copies features on the clipboard to the active layer
  void editPaste();
  //! shows the paste-transformations dialog
  void pasteTransformations();
  //! check to see if file is dirty and if so, prompt the user th save it
  int saveDirty();
  //! Set the pointer to the splash screen so status messages can be


  /// QgisApp aren't copyable
  QgisApp( QgisApp const & );
  /// QgisApp aren't copyable
  QgisApp & operator=( QgisApp const & );

  void readSettings();
  void writeSettings();
  void createActions();
  void createActionGroups();
  void createMenus();
  void createToolBars();
  void createStatusBar();
  void setupConnections();
  void createLegend();
  void createOverview();
  void createCanvas();
  void createDB();
  //toolbars ----------------------------------------
  QToolBar *mFileToolBar;
  QToolBar *mLayerToolBar;
  QToolBar *mMapNavToolBar;
  QToolBar *mDigitizeToolBar;
  QToolBar *mAttributesToolBar;
  QToolBar *mPluginToolBar;
  QToolBar *mHelpToolBar;
  //
  //toolbar buttons ---------------------------------
  QAction *mActionFileNew;
  QAction *mActionFileSave;
  QAction *mActionFileSaveAs;
  QAction *mActionFileOpen;
  QAction *mActionFilePrint;
  QAction *mActionSaveMapAsImage;
  QAction *mActionExportMapServer;
  QAction *mActionFileExit;
  QAction *mActionAddNonDbLayer;
  QAction *mActionAddRasterLayer;
  QAction *mActionAddLayer;
  QAction *mActionRemoveLayer;
  QAction *mActionNewVectorLayer;
  QAction *mActionAddAllToOverview;
  QAction *mActionHideAllLayers;
  QAction *mActionShowAllLayers;
  QAction *mActionRemoveAllFromOverview;
  QAction *mActionLayerProperties;
  QAction *mActionProjectProperties;
  QAction *mActionShowPluginManager;
  QAction *mActionCheckQgisVersion;
  QAction *mActionOptions;
  QAction *mActionHelpContents;
  QAction *mActionQgisHomePage;
  QAction *mActionQgisSourceForgePage;
  QAction *mActionHelpAbout;
  QAction *mArawAction;
  QAction *mActionCapturePoint;
  QAction *mActionCaptureLine;
  QAction *mActionCapturePolygon;
  QAction *mActionZoomIn;
  QAction *mActionZoomOut;
  QAction *mActionZoomFullExtent;
  QAction *mActionZoomToSelected;
  QAction *mActionPan;
  QAction *mActionZoomLast;
  QAction *mActionZoomToLayer;
  QAction *mActionIdentify;
  QAction *mActionSelect;
  QAction *mActionOpenTable;
  QAction *mActionMeasure;
  QAction *mActionMeasureArea;
  QAction *mActionShowBookmarks;
  QAction *mActionNewBookmark;
  QAction *mActionCustomProjection;
  QAction *mActionAddWmsLayer;
  QAction *mActionInOverview;
  QAction *mActionDraw;
  //
  //tool groups -------------------------------------
  QActionGroup *mMapToolGroup;
  //
  //menus   -----------------------------------------
  QMenu *mFileMenu;
  QMenu *mRecentProjectsMenu;
  QMenu *mViewMenu;
  QMenu *mLayerMenu;
  QMenu *mSettingsMenu;
  QMenu *mHelpMenu;

  //!The name of the active theme
  QString mThemeName;

  //! A central registry that keeps track of all loaded layers.
  // prefer QgsMapLayerRegistry::instance() to emphasize Singleton
  ///QgsMapLayerRegistry * mMapLayerRegistry;
  //! Widget that will live on the statusbar to display scale
  QLabel * mScaleLabel;
  //! Widget that will live in the statusbar to display coords
  QLabel * mCoordsLabel;
  //! Widget that will live in the statusbar to show progress of operations
  QProgressBar * mProgressBar;
  //! Widget used to suppress rendering
  QCheckBox * mRenderSuppressionCBox;
  //! Widget in status bar used to show status of on the fly projection
  QToolButton * mOnTheFlyProjectionStatusButton;
  //! Popup menu
  QMenu * mPopupMenu;
  //! Top level plugin menu
  QMenu *mPluginMenu;
  //! Popup menu for the map overview tools
  QMenu *toolPopupOverviews;
  //! Popup menu for the display tools
  QMenu *toolPopupDisplay;
  //! Popup menu for the capture tools
  QMenu *toolPopupCapture;
  //! Legend list view control
  //doesnt see to be used...(TS)
  //QgsLegendView *mLegendView;
  //! Map canvas
  QgsMapCanvas *mMapCanvas;
  //! Map layer registry
  // use instance() now QgsMapLayerRegistry *mLayerRegistry;
  //! Overview canvas where the map overview is shown
  QgsMapOverviewCanvas * mOverviewCanvas;
  //! Table of contents (legend) for the map
  QgsLegend *mMapLegend;
  //! Cursor for the map
  QCursor *mMapCursor;
  //! Cursor for the overview map
  QCursor *mOverviewMapCursor;
  //! scale factor
  double mScaleFactor;
  //! Current map window extent in real-world coordinates
  QRect *mMapWindow;
  //! Current map tool
  int mMapTool;
  //! The previously selected non zoom map tool.
  int mPreviousNonZoomMapTool;
  //QCursor *mCursorZoomIn; //doesnt seem to be used anymore (TS)
  QString mStartupPath;
  //! full path name of the current map file (if it has been saved or loaded)
  QString mFullPathName;
  QgisIface *mQgisInterface;
  QTcpSocket *mSocket;
  QString mVersionMessage;
  QSplashScreen *mSplash;
  friend class QgisIface;
  QgsProviderRegistry *mProviderRegistry;
  //! application directory
  QString mAppDir;
  //! help viewer
  QgsHelpViewer *mHelpViewer;
  //! Flag to indicate that newly added layers are not shown on
  //  the map
  bool mAddedLayersHidden;
  //! menu map (key is name, value is menu id)
  std::map<QString, int>mMenuMapByName;
  //! menu map (key is menu id, value is name)
  std::map<int, QString>mMenuMapById;
  //! list of recently opened/saved project files
  QStringList mRecentProjectPaths;
  //! Map composer
  QgsComposer *mComposer;
  //! How to determine the number of decimal places used to
  //! display the mouse position
  bool mMousePrecisionAutomatic;
  //! The number of decimal places to use if not automatic
  unsigned int mMousePrecisionDecimalPlaces;
  /** QGIS-internal vector feature clipboard */
  QgsClipboard mInternalClipboard;
  //! Flag to indicate how the project properties dialog was summoned
  bool mShowProjectionTab;
  /** String containing supporting vector file formats 
    Suitable for a QFileDialog file filter.  Build in ctor.
    */
  QString mVectorFileFilter;
  /** String containing supporting raster file formats 
    Suitable for a QFileDialog file filter.  Build in ctor.
    */
  QString mRasterFileFilter;
};

#endif
