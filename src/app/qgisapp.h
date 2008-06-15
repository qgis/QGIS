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

class QActionGroup;
class QCheckBox;
class QCursor;
class QFileInfo;
class QKeyEvent;
class QLabel;
class QLineEdit;
class QMenu;
class QPixmap;
class QProgressBar;
class QPushButton;
class QRect;
class QSettings;
class QSplashScreen;
class QStringList;
class QToolButton;
class QTcpSocket;
class QValidator;

class QgisAppInterface;
class QgsClipboard;
class QgsComposer;
class QgsHelpViewer;
class QgsLegend;
class QgsMapCanvas;
class QgsMapLayer;
class QgsMapTip;
class QgsMapTool;
class QgsPoint;
class QgsProviderRegistry;
class QgsPythonDialog;
class QgsPythonUtils;
class QgsRasterLayer;
class QgsRect;
class QgsVectorLayer;

#include <QMainWindow>
#include <QToolBar>
#include <QAbstractSocket>

#include "qgsconfig.h"
#include <qgspoint.h>

/*! \class QgisApp
 * \brief Main window for the Qgis application
 */
class QgisApp : public QMainWindow
{
  Q_OBJECT;
  public:
  //! Constructor
  QgisApp(QSplashScreen *splash, QWidget * parent = 0, Qt::WFlags fl = Qt::Window);
  //! Destructor
  ~QgisApp();
  /**
   * Add a vector layer to the canvas, returns pointer to it
   */
  QgsVectorLayer* addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey);
  
  /** \brief overloaded vesion of the privat addLayer method that takes a list of
   * filenames instead of prompting user with a dialog.
   @param enc encoding type for the layer 
   @returns true if successfully added layer
   */
  bool addVectorLayers(QStringList const & theLayerQStringList, const QString& enc);

  /** overloaded vesion of the private addRasterLayer()
    Method that takes a list of filenames instead of prompting
    user with a dialog.
    @returns true if successfully added layer(s)
    */
  bool addRasterLayers(QStringList const & theLayerQStringList, bool guiWarning=true);

  /** Open a raster layer using the Raster Data Provider.
   *  Note this is included to support WMS layers only at this stage,
   *  GDAL layer support via a Provider is not yet implemented.
   */
  QgsRasterLayer* addRasterLayer(QString const & rasterLayerPath,
      QString const & baseName,
      QString const & providerKey,
      QStringList const & layers,
      QStringList const & styles,
      QString const & format,
      QString const & crs);

  /** open a raster layer for the given file
    @returns false if unable to open a raster layer for rasterFile
    @note
    This is essentially a simplified version of the above
    */
  QgsRasterLayer* addRasterLayer(QString const & rasterFile, QString const & baseName, bool guiWarning=true);

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
  //! Set theme (icons)
  void setTheme(QString themeName="default");
  //! Setup the toolbar popup menus for a given theme
  void setupToolbarPopups(QString themeName);
  //! Returns a pointer to the internal clipboard
  QgsClipboard * clipboard();

  void dragEnterEvent(QDragEnterEvent *);

  void dropEvent(QDropEvent *);

  /** Setup the proxy settings from the QSettings environment.
    * This is not called by default in the constructor. Rather, 
    * the application must explicitly call setupProx(). If 
    * you write your own application and wish to explicitly 
    * set up your own proxy rather, you should e.g.
    *  QNetworkProxy proxy;
    *  proxy.setType(QNetworkProxy::Socks5Proxy);
    *  proxy.setHostName("proxy.example.com");
    *  proxy.setPort(1080);
    *  proxy.setUser("username");
    *  proxy.setPassword("password");
    *  QNetworkProxy::setApplicationProxy(proxy);
    *  
    *  (as documented in Qt documentation.
  */
  void setupProxy();
//private slots:
public slots:
  //! About QGis
  void about();
  //! Add a raster layer to the map (will prompt user for filename using dlg )
  void addRasterLayer();
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
  void inOverview();
  //! Slot to show the map coordinate position of the mouse cursor
  void showMouseCoordinate(QgsPoint &);
  //copy the click coord to clipboard and let the user know its there
  void showCapturePointCoordinate(QgsPoint &);
  //! Slot to show current map scale;
  void showScale(double theScale);
  //! Slot to handle user scale input;
  void userScale();
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
  //! python plugin loader
  void loadPythonPlugin(QString packageName, QString pluginName);
  //! Find the QMenu with the given name (ie the user visible text on the menu item)
  QMenu* getPluginMenu(QString menuName);
  //! Add the action to the submenu with the given name under the plugin menu
  void addPluginMenu(QString name, QAction* action);
  //! Remove the action to the submenu with the given name under the plugin menu
  void removePluginMenu(QString name, QAction* action);
  //! Add an icon to the plugin toolbar
  int addPluginToolBarIcon (QAction * qAction);
  //! Remove an icon from the plugin toolbar
  void removePluginToolBarIcon(QAction *qAction);
  //! Save window state
  void saveWindowState();
  //! Restore the window and toolbar state
  void restoreWindowState();
  //! Save project. Returns true if the user selected a file to save to, false if not.
  bool fileSave();
  //! Save project as
  void fileSaveAs();
  //! Open the project file corresponding to the
  //! text)= of the given action.
  void openProject(QAction *action);
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
  // TODO: remove exportMapServer declaration once the mapserver export plugin is complete
  // and tested
  /*
  //! Export current view as a mapserver map file
  void exportMapServer();
  */
  //! Return pointer to the active layer
  QgsMapLayer *activeLayer();
  //! Open the help contents in a browser
  void helpContents();
  //! Open the QGIS homepage in users browser
  void helpQgisHomePage();
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
  void socketConnected();
  void socketConnectionClosed();
  void socketReadyRead();
  void socketError(QAbstractSocket::SocketError e);
  //! Set project properties, including map untis
  void projectProperties();
  //! Open project properties dialog and show the projections tab
  void projectPropertiesProjections();
  /*  void urlData(); */
  //! Show the spatial bookmarks dialog
  void showBookmarks();
  //! Create a new spatial bookmark
  void newBookmark();
  //! Lets the user show all of the toolbars
  void showAllToolbars();
  //! Lets the user hide all of the toolbars
  void hideAllToolbars();
  //! Sets the visibility of the toolbars
  void setToolbarVisibility(bool visibility);
  //! activates the capture point tool
  void capturePoint();
  //! activates the capture line tool
  void captureLine();
  //! activates the capture polygon tool
  void capturePolygon();
  /**Deletes the selected attributes for the currently selected vector layer*/
  void deleteSelected();
  //! activates the move feature tool
  void moveFeature();
  //! activates the split features tool
  void splitFeatures();
  //! activates the add vertex tool
  void addVertex();
  //! activates the move vertex tool
  void moveVertex();
  //! activates the delete vertex tool
  void deleteVertex();
  //! activates the add ring tool
  void addRing();
  //! activates the add island tool
  void addIsland();

  //! activates the selection tool
  void select();
  //! refresh map canvas
  void refreshMapCanvas();
  //! returns pointer to map legend
  QgsLegend *legend() { return mMapLegend; }
  //! starts/stops editing mode of the current layer
  void toggleEditing();

  /** Activates or deactivates actions depending on the current maplayer type.
  Is called from the legend when the current legend item has changed*/
  void activateDeactivateLayerRelatedActions(QgsMapLayer* layer);

public slots:
  void showProgress(int theProgress, int theTotalSteps);
  void showExtents();
  void showStatusMessage(QString theMessage);
  void updateMouseCoordinatePrecision();
  void projectionsEnabled(bool theFlag);
  void destinationSrsChanged();
  //    void debugHook();
  //! Add a vector layer to the map
  void addVectorLayer();
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
  
  //! show python console
  void showPythonDialog();

  //! cuts selected features on the active layer to the clipboard
  /**
     \param layerContainingSelection  The layer that the selection will be taken from
                                      (defaults to the active layer on the legend)
   */
  void editCut(QgsMapLayer * layerContainingSelection = 0);
  //! copies selected features on the active layer to the clipboard
  /**
     \param layerContainingSelection  The layer that the selection will be taken from
                                      (defaults to the active layer on the legend)
   */
  void editCopy(QgsMapLayer * layerContainingSelection = 0);
  //! copies features on the clipboard to the active layer
  /**
     \param destinationLayer  The layer that the clipboard will be pasted to
                              (defaults to the active layer on the legend)
   */
  void editPaste(QgsMapLayer * destinationLayer = 0);

  //! Shows a warning when an old project file is read.
  void warnOlderProjectVersion(QString);

  //! Toggle map tips on/off
  void toggleMapTips();

  //! Show the map tip
  void showMapTip();

  //! Toggle full screen mode
  void toggleFullScreen();

  //! Stops rendering of the main map
  void stopRendering();

  /** Get a reference to the file toolbar. Mainly intended 
  *   to be used by plugins that want to specifically add 
  *   an icon into the file toolbar for consistency e.g.
  *   addWFS and GPS plugins.
  */
  QToolBar * fileToolBar();
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
   * It won't force a refresh.
   */
  bool addRasterLayer(QgsRasterLayer * theRasterLayer);
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
  //! Read Well Known Binary stream from PostGIS
  //void readWKB(const char *, QStringList tables);
  //! test function
  void testButton();
  //! shows the paste-transformations dialog
  void pasteTransformations();
  //! check to see if file is dirty and if so, prompt the user th save it
  bool saveDirty();
  //! Have some control over closing of the application
  virtual void closeEvent(QCloseEvent* event);

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
  bool createDB();
  void createMapTips();
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
  QAction *mActionAddOgrLayer;
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
  QAction *mActionHelpAbout;
  QAction *mArawAction;
  QAction *mActionToggleEditing;
  QAction *mActionCapturePoint;
  QAction *mActionCaptureLine;
  QAction *mActionCapturePolygon;
  QAction *mActionDeleteSelected;
  QAction *mActionMoveFeature;
  QAction *mActionSplitFeatures;
  QAction *mActionAddVertex;
  QAction *mActionDeleteVertex;
  QAction *mActionMoveVertex;
  QAction *mActionAddRing;
  QAction *mActionAddIsland;
  QAction *mActionEditCut;
  QAction *mActionEditCopy;
  QAction *mActionEditPaste;
  QAction *mActionZoomIn;
  QAction *mActionZoomOut;
  QAction *mActionZoomFullExtent;
  QAction *mActionZoomToSelected;
  QAction *mActionPan;
  QAction *mActionZoomLast;
  QAction *mActionZoomToLayer;
  QAction *mActionIdentify;
  QAction *mActionMapTips;
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
  QAction *mActionShowAllToolbars;
  QAction *mActionHideAllToolbars;
  QAction *mActionToggleFullScreen;
  QAction *mActionShowPythonDialog;
  
  //
  //tool groups -------------------------------------
  QActionGroup *mMapToolGroup;
  //
  //menus   -----------------------------------------
  QMenu *mFileMenu;
  QMenu *mRecentProjectsMenu;
  QMenu *mViewMenu;
  QMenu *mToolbarMenu;
  QMenu *mLayerMenu;
  QMenu *mSettingsMenu;
  QMenu *mHelpMenu;

  QDockWidget *mLegendDock;
  QDockWidget *mOverviewDock;

class Tools
  {
    public:
      QgsMapTool* mZoomIn;
      QgsMapTool* mZoomOut;
      QgsMapTool* mPan;
      QgsMapTool* mIdentify;
      QgsMapTool* mMeasureDist;
      QgsMapTool* mMeasureArea;
      QgsMapTool* mCapturePoint;
      QgsMapTool* mCaptureLine;
      QgsMapTool* mCapturePolygon;
      QgsMapTool* mMoveFeature;
      QgsMapTool* mSplitFeatures;
      QgsMapTool* mSelect;
      QgsMapTool* mVertexAdd;
      QgsMapTool* mVertexMove;
      QgsMapTool* mVertexDelete;
      QgsMapTool* mAddRing;
      QgsMapTool* mAddIsland;
  } mMapTools;
  
  //!The name of the active theme
  QString mThemeName;

  //! Widget that will live on the statusbar to display "scale 1:"
  QLabel * mScaleLabel;
  //! Widget that will live on the statusbar to display scale value
  QLineEdit * mScaleEdit;
  //! The validator for the mScaleEdit
  QValidator * mScaleEditValidator;
  //! Widget that will live in the statusbar to display coords
  QLabel * mCoordsLabel;
  //! Widget that will live in the statusbar to show progress of operations
  QProgressBar * mProgressBar;
  //! Widget used to suppress rendering
  QCheckBox * mRenderSuppressionCBox;
  //! Button used to stop rendering
  QToolButton* mStopRenderButton;
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
  //! Map canvas
  QgsMapCanvas *mMapCanvas;
  //! Table of contents (legend) for the map
  QgsLegend *mMapLegend;
  //! Cursor for the overview map
  QCursor *mOverviewMapCursor;
  //! scale factor
  double mScaleFactor;
  //! Current map window extent in real-world coordinates
  QRect *mMapWindow;
  //! The previously selected non zoom map tool.
  int mPreviousNonZoomMapTool;
  //QCursor *mCursorZoomIn; //doesnt seem to be used anymore (TS)
  QString mStartupPath;
  //! full path name of the current map file (if it has been saved or loaded)
  QString mFullPathName;

  //! interface to QgisApp for plugins
  QgisAppInterface *mQgisInterface;
  friend class QgisAppInterface;

  QTcpSocket *mSocket;
  QString mVersionMessage;
  QSplashScreen *mSplash;
  //! help viewer
  QgsHelpViewer *mHelpViewer;
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
  QgsClipboard* mInternalClipboard;
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

  /** Timer for map tips
   */
  QTimer *mpMapTipsTimer;

  /** Point of last mouse position in map coordinates (used with MapTips)
   */
  QgsPoint mLastMapPosition;

  /* Maptip object
   */
  QgsMapTip *  mpMaptip;
  
  // Flag to indicate if maptips are on or off
  bool mMapTipsVisible;
  
  //!flag to indicat wehter we are in fullscreen mode or not
  bool mFullScreenMode;
  QgsPythonDialog* mPythonConsole;
  QgsPythonUtils* mPythonUtils;
};

#endif
