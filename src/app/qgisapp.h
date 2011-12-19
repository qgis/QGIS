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

#ifndef QGISAPP_H
#define QGISAPP_H

class QActionGroup;
class QCheckBox;
class QCursor;
class QFileInfo;
class QKeyEvent;
class QLabel;
class QComboBox;
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
class QgsAnnotationItem;
class QgsClipboard;
class QgsComposer;
class QgsComposerView;
class QgsGeometry;
class QgsFeature;

class QgsLegend;
class QgsLayerOrder;
class QgsMapCanvas;
class QgsMapLayer;
class QgsMapTip;
class QgsMapTool;
class QgsPalLabeling;
class QgsPoint;
class QgsProviderRegistry;
class QgsPythonUtils;
class QgsRasterLayer;
class QgsRectangle;
class QgsUndoWidget;
class QgsVectorLayer;
class QgsTileScaleWidget;

class QDomDocument;
class QNetworkReply;
class QNetworkProxy;
class QAuthenticator;

class QgsBrowserDockWidget;
class QgsSnappingDialog;
class QgsGPSInformationWidget;

class QgsDecorationCopyright;
class QgsDecorationNorthArrow;
class QgsDecorationScaleBar;

class QgsMessageLogViewer;

#include <QMainWindow>
#include <QToolBar>
#include <QAbstractSocket>
#include <QPointer>
#include <QSslError>

#include "qgsconfig.h"
#include "qgsfeature.h"
#include "qgspoint.h"
#include "qgssnappingdialog.h"

#include "ui_qgisapp.h"

/*! \class QgisApp
 * \brief Main window for the Qgis application
 */
class QgisApp : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
  public:
    //! Constructor
    QgisApp( QSplashScreen *splash, bool restorePlugins = true, QWidget * parent = 0, Qt::WFlags fl = Qt::Window );
    //! Destructor
    ~QgisApp();
    /**
     * Add a vector layer to the canvas, returns pointer to it
     */
    QgsVectorLayer* addVectorLayer( QString vectorLayerPath, QString baseName, QString providerKey );

    /** \brief overloaded vesion of the privat addLayer method that takes a list of
     * file names instead of prompting user with a dialog.
     @param enc encoding type for the layer
    @param dataSourceType type of ogr datasource
     @returns true if successfully added layer
     */
    bool addVectorLayers( QStringList const & theLayerQStringList, const QString& enc, const QString dataSourceType );

    /** overloaded vesion of the private addRasterLayer()
      Method that takes a list of file names instead of prompting
      user with a dialog.
      @returns true if successfully added layer(s)
      */
    bool addRasterLayers( QStringList const & theLayerQStringList, bool guiWarning = true );

    /** open a raster layer for the given file
      @returns false if unable to open a raster layer for rasterFile
      @note
      This is essentially a simplified version of the above
      */
    QgsRasterLayer* addRasterLayer( QString const & rasterFile, QString const & baseName, bool guiWarning = true );

    /** Add a 'pre-made' map layer to the project */
    void addMapLayer( QgsMapLayer *theMapLayer );

    /** Set the extents of the map canvas */
    void setExtent( QgsRectangle theRect );
    //! Remove all layers from the map and legend - reimplements same method from qgisappbase
    void removeAllLayers();
    /** Open a raster or vector file; ignore other files.
      Used to process a commandline argument, FileOpen or Drop event.
      Set interactive to true if it is ok to ask the user for information (mostly for
      when a vector layer has sublayers and we want to ask which sublayers to use).
      @returns true if the file is successfully opened
      */
    bool openLayer( const QString & fileName, bool allowInteractive = false );
    /** Open the specified project file; prompt to save previous project if necessary.
      Used to process a commandline argument, FileOpen or Drop event.
      */
    void openProject( const QString & fileName );
    /** opens a qgis project file
      @returns false if unable to open the project
      */
    bool addProject( QString projectFile );
    /** Convenience function to open either a project or a layer file.
      */
    void openFile( const QString & fileName );
    //!Overloaded version of the private function with same name that takes the imagename as a parameter
    void saveMapAsImage( QString, QPixmap * );
    /** Get the mapcanvas object from the app */
    QgsMapCanvas * mapCanvas();

    //! Set theme (icons)
    void setTheme( QString themeName = "default" );

    void setIconSizes( int size );

    //! Setup the toolbar popup menus for a given theme
    void setupToolbarPopups( QString themeName );
    //! Returns a pointer to the internal clipboard
    QgsClipboard * clipboard();

    static QgisApp *instance() { return smInstance; }

    //! initialize network manager
    void namSetup();

    //! update proxy settings
    void namUpdate();

    //! Helper to get a theme icon. It will fall back to the
    //default theme if the active theme does not have the required
    //icon.
    static QIcon getThemeIcon( const QString theName );
    //! Helper to get a theme icon as a pixmap. It will fall back to the
    //default theme if the active theme does not have the required
    //icon.
    static QPixmap getThemePixmap( const QString theName );

    /** Add a dock widget to the main window. Overloaded from QMainWindow.
     * After adding the dock widget to the ui (by delegating to the QMainWindow
     * parent class, it will also add it to the View menu list of docks.*/
    void addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget );
    /** Add a toolbar to the main window. Overloaded from QMainWindow.
     * After adding the toolbar to the ui (by delegating to the QMainWindow
     * parent class, it will also add it to the View menu list of toolbars.*/
    QToolBar *addToolBar( QString name );

    /** Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window. */
    void addWindow( QAction *action );
    /** Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed. */
    void removeWindow( QAction *action );

    /**Returns the print composers*/
    QSet<QgsComposer*> printComposers() const {return mPrintComposers;}
    /**Creates a new composer and returns a pointer to it*/
    QgsComposer* createNewComposer();
    /**Deletes a composer and removes entry from Set*/
    void deleteComposer( QgsComposer* c );


    //! Actions to be inserted in menus and toolbars
    QAction *actionNewProject() { return mActionNewProject; }
    QAction *actionOpenProject() { return mActionOpenProject; }
    QAction *actionSaveProject() { return mActionSaveProject; }
    QAction *actionSaveProjectAs() { return mActionSaveProjectAs; }
    QAction *actionSaveMapAsImage() { return mActionSaveMapAsImage; }
    QAction *actionProjectProperties() { return mActionProjectProperties; }
    QAction *actionNewPrintComposer() { return mActionNewPrintComposer; }
    QAction *actionExit() { return mActionExit; }

    QAction *actionCutFeatures() { return mActionCutFeatures; }
    QAction *actionCopyFeatures() { return mActionCopyFeatures; }
    QAction *actionPasteFeatures() { return mActionPasteFeatures; }
    QAction *actionDeleteSelected() { return mActionDeleteSelected; }
    QAction *actionAddFeature() { return mActionAddFeature; }
    QAction *actionMoveFeature() { return mActionMoveFeature; }
    QAction *actionSplitFeatures() { return mActionSplitFeatures; }
    QAction *actionAddRing() { return mActionAddRing; }
    QAction *actionAddPart() { return mActionAddPart; }
    QAction *actionSimplifyFeature() { return mActionSimplifyFeature; }
    QAction *actionDeleteRing() { return mActionDeleteRing; }
    QAction *actionDeletePart() { return mActionDeletePart; }
    QAction *actionNodeTool() { return mActionNodeTool; }
    QAction *actionSnappingOptions() { return mActionSnappingOptions; }

    QAction *actionPan() { return mActionPan; }
    QAction *actionZoomIn() { return mActionZoomIn; }
    QAction *actionZoomOut() { return mActionZoomOut; }
    QAction *actionSelect() { return mActionSelect; }
    QAction *actionSelectRectangle() { return mActionSelectRectangle; }
    QAction *actionSelectPolygon() { return mActionSelectPolygon; }
    QAction *actionSelectFreehand() { return mActionSelectFreehand; }
    QAction *actionSelectRadius() { return mActionSelectRadius; }
    QAction *actionIdentify() { return mActionIdentify; }
    QAction *actionMeasure() { return mActionMeasure; }
    QAction *actionMeasureArea() { return mActionMeasureArea; }
    QAction *actionZoomFullExtent() { return mActionZoomFullExtent; }
    QAction *actionZoomToLayer() { return mActionZoomToLayer; }
    QAction *actionZoomToSelected() { return mActionZoomToSelected; }
    QAction *actionZoomLast() { return mActionZoomLast; }
    QAction *actionZoomNext() { return mActionZoomNext; }
    QAction *actionZoomActualSize() { return mActionZoomActualSize; }
    QAction *actionMapTips() { return mActionMapTips; }
    QAction *actionNewBookmark() { return mActionNewBookmark; }
    QAction *actionShowBookmarks() { return mActionShowBookmarks; }
    QAction *actionDraw() { return mActionDraw; }

    QAction *actionNewVectorLayer() { return mActionNewVectorLayer; }
    QAction *actionNewSpatialLiteLayer() { return mActionNewSpatialiteLayer; }
    QAction *actionEmbedLayers() { return mActionEmbedLayers; }
    QAction *actionAddOgrLayer() { return mActionAddOgrLayer; }
    QAction *actionAddRasterLayer() { return mActionAddRasterLayer; }
    QAction *actionAddPgLayer() { return mActionAddPgLayer; }
    QAction *actionAddSpatiaLiteLayer() { return mActionAddSpatiaLiteLayer; };
    QAction *actionAddWmsLayer() { return mActionAddWmsLayer; }
    QAction *actionAddWfsLayer() { return mActionAddWfsLayer; }
    QAction *actionOpenTable() { return mActionOpenTable; }
    QAction *actionToggleEditing() { return mActionToggleEditing; }
    QAction *actionSaveEdits() { return mActionSaveEdits; }
    QAction *actionLayerSaveAs() { return mActionLayerSaveAs; }
    QAction *actionLayerSelectionSaveAs() { return mActionLayerSelectionSaveAs; }
    QAction *actionRemoveLayer() { return mActionRemoveLayer; }
    QAction *actionSetLayerCRS() { return mActionSetLayerCRS; }
    QAction *actionSetProjectCRSFromLayer() { return mActionSetProjectCRSFromLayer; }
    QAction *actionTileScale() { return mActionTileScale; }
    QAction *actionLayerProperties() { return mActionLayerProperties; }
    QAction *actionLayerSubsetString() { return mActionLayerSubsetString; }
    QAction *actionAddToOverview() { return mActionAddToOverview; }
    QAction *actionAddAllToOverview() { return mActionAddAllToOverview; }
    QAction *actionRemoveAllFromOverview() { return mActionRemoveAllFromOverview; }
    QAction *actionHideAllLayers() { return mActionHideAllLayers; }
    QAction *actionShowAllLayers() { return mActionShowAllLayers; }

    QAction *actionManagePlugins() { return mActionManagePlugins; }
    QAction *actionPluginListSeparator() { return mActionPluginSeparator1; }
    QAction *actionPluginPythonSeparator() { return mActionPluginSeparator2; }
    QAction *actionShowPythonDialog() { return mActionShowPythonDialog; }

    QAction *actionToggleFullScreen() { return mActionToggleFullScreen; }
    QAction *actionOptions() { return mActionOptions; }
    QAction *actionCustomProjection() { return mActionCustomProjection; }
    QAction *actionConfigureShortcuts() { return mActionConfigureShortcuts; }

#ifdef Q_WS_MAC
    QAction *actionWindowMinimize() { return mActionWindowMinimize; }
    QAction *actionWindowZoom() { return mActionWindowZoom; }
    QAction *actionWindowAllToFront() { return mActionWindowAllToFront; }
#endif

    QAction *actionHelpContents() { return mActionHelpContents; }
    QAction *actionHelpAPI() { return mActionHelpAPI; }
    QAction *actionQgisHomePage() { return mActionQgisHomePage; }
    QAction *actionCheckQgisVersion() { return mActionCheckQgisVersion; }
    QAction *actionAbout() { return mActionAbout; }
    QAction *actionSponsors() { return mActionSponsors; }

    //! Menus
    QMenu *fileMenu() { return mFileMenu; }
    QMenu *editMenu() { return mEditMenu; }
    QMenu *viewMenu() { return mViewMenu; }
    QMenu *layerMenu() { return mLayerMenu; }
    QMenu *settingsMenu() { return mSettingsMenu; }
    QMenu *pluginMenu() { return mPluginMenu; }
    QMenu *databaseMenu() { return mDatabaseMenu; }
    QMenu *rasterMenu() { return mRasterMenu; }
#ifdef Q_WS_MAC
    QMenu *firstRightStandardMenu() { return mWindowMenu; }
    QMenu *windowMenu() { return mWindowMenu; }
#else
    QMenu *firstRightStandardMenu() { return mHelpMenu; }
    QMenu *windowMenu() { return NULL; }
#endif
    QMenu *printComposersMenu() {return mPrintComposersMenu;}
    QMenu *helpMenu() { return mHelpMenu; }

    //! Toolbars
    /** Get a reference to a toolbar. Mainly intended
    *   to be used by plugins that want to specifically add
    *   an icon into the file toolbar for consistency e.g.
    *   addWFS and GPS plugins.
    */
    QToolBar *fileToolBar() { return mFileToolBar; }
    QToolBar *layerToolBar() { return mLayerToolBar; }
    QToolBar *mapNavToolToolBar() { return mMapNavToolBar; }
    QToolBar *digitizeToolBar() { return mDigitizeToolBar; }
    QToolBar *advancedDigitizeToolBar() { return mAdvancedDigitizeToolBar; }
    QToolBar *attributesToolBar() { return mAttributesToolBar; }
    QToolBar *pluginToolBar() { return mPluginToolBar; }
    QToolBar *helpToolBar() { return mHelpToolBar; }
    QToolBar *rasterToolBar() { return mRasterToolBar; }

    //! show layer properties
    void showLayerProperties( QgsMapLayer *ml );

    //! returns pointer to map legend
    QgsLegend *legend();

#ifdef Q_OS_WIN
    //! ugly hack
    void skipNextContextMenuEvent();
#endif

    //! emit initializationCompleted signal
    //! @note added in 1.6
    void completeInitialization();

  public slots:
    //! Zoom to full extent
    void zoomFull();
    //! Zoom to the previous extent
    void zoomToPrevious();
    //! Zoom to the forward extent
    void zoomToNext();
    //! Zoom to selected features
    void zoomToSelected();

    //! open the properties dialog for the currently selected layer
    void layerProperties();

    //! show the attribute table for the currently selected layer
    void attributeTable();

    //! mark project dirty
    void markDirty();

    //! layer was added
    void layerWasAdded( QgsMapLayer * );

    //! layer will be removed
    void removingLayer( QString );

    void updateUndoActions();

    //! cuts selected features on the active layer to the clipboard
    /**
       \param layerContainingSelection  The layer that the selection will be taken from
                                        (defaults to the active layer on the legend)
     */
    void editCut( QgsMapLayer * layerContainingSelection = 0 );
    //! copies selected features on the active layer to the clipboard
    /**
       \param layerContainingSelection  The layer that the selection will be taken from
                                        (defaults to the active layer on the legend)
     */
    void editCopy( QgsMapLayer * layerContainingSelection = 0 );
    //! copies features on the clipboard to the active layer
    /**
       \param destinationLayer  The layer that the clipboard will be pasted to
                                (defaults to the active layer on the legend)
     */
    void editPaste( QgsMapLayer * destinationLayer = 0 );

    void loadOGRSublayers( QString layertype, QString uri, QStringList list );

    /**Deletes the selected attributes for the currently selected vector layer*/
    void deleteSelected( QgsMapLayer *layer = 0 );

    //! project was written
    void writeProject( QDomDocument & );

    //! project was read
    void readProject( const QDomDocument & );

    //! request credentials for network manager
    void namAuthenticationRequired( QNetworkReply *reply, QAuthenticator *auth );
    void namProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *auth );
#ifndef QT_NO_OPENSSL
    void namSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif

    //! update default action of toolbutton
    void toolButtonActionTriggered( QAction * );

    //! layer selection changed
    void legendLayerSelectionChanged( void );

    //! Watch for QFileOpenEvent.
    virtual bool event( QEvent * event );

    /** Open a raster layer using the Raster Data Provider.
     *  Note this is included to support WMS layers only at this stage,
     *  GDAL layer support via a Provider is not yet implemented.
     */
    QgsRasterLayer* addRasterLayer( QString const & rasterLayerPath,
                                    QString const & baseName,
                                    QString const & providerKey,
                                    QStringList const & layers,
                                    QStringList const & styles,
                                    QString const & format,
                                    QString const & crs );

    void addWfsLayer( QString uri, QString typeName );

    void versionReplyFinished();

    QgsMessageLogViewer *logViewer() { return mLogViewer; }

  protected:

    //! Handle state changes (WindowTitleChange)
    virtual void changeEvent( QEvent * event );
    //! Have some control over closing of the application
    virtual void closeEvent( QCloseEvent * event );

    virtual void dragEnterEvent( QDragEnterEvent * event );
    virtual void dropEvent( QDropEvent * event );

    //! reimplements widget keyPress event so we can check if cancel was pressed
    virtual void keyPressEvent( QKeyEvent * event );

#ifdef Q_OS_WIN
    //! reimplements context menu event
    virtual void contextMenuEvent( QContextMenuEvent *event );
#endif

  private slots:
    //! QGis Sponsors
    void sponsors();
    //! About QGis
    void about();
    //! Add a raster layer to the map (will prompt user for file name using dlg )
    void addRasterLayer();
    //#ifdef HAVE_POSTGRESQL
    //! Add a databaselayer to the map
    void addDatabaseLayer();
    //#endif
    //! Add a list of database layers to the map
    void addDatabaseLayers( QStringList const & layerPathList, QString const & providerKey );
    //#ifdef HAVE_SPATIALITE
    //! Add a SpatiaLite layer to the map
    void addSpatiaLiteLayer();
    //#endif
    /** toggles whether the current selected layer is in overview or not */
    void isInOverview();
    //! Slot to show the map coordinate position of the mouse cursor
    void showMouseCoordinate( const QgsPoint & );
    //! Slot to show current map scale;
    void showScale( double theScale );
    //! Slot to handle user scale input;
    void userScale();
    //! Slot to handle user center input;
    void userCenter();
    //! Remove a layer from the map and legend
    void removeLayer();
    //! Set CRS of a layer
    void setLayerCRS();
    //! Assign layer CRS to project
    void setProjectCRSFromLayer();
    //! Show tile scale slider
    void showTileScale();
    //! zoom to extent of layer
    void zoomToLayerExtent();
    //! zoom to actual size of raster layer
    void zoomActualSize();
    /** Perform a local histogram stretch on the active raster layer
     * (stretch based on pixel values in view extent).
     * Valid for non wms raster layers only.
     * @note Added in QGIS 1.7 */
    void localHistogramStretch();
    /** perform a full histogram stretch on the active raster layer
     * (stretch based on pixels values in full dataset)
     * Valid for non wms raster layers only.
     * @note Added in QGIS 1.7 */
    void fullHistogramStretch();
    //! plugin manager
    void showPluginManager();
    //! load python support if possible
    void loadPythonSupport();
    //! Find the QMenu with the given name within plugin menu (ie the user visible text on the menu item)
    QMenu* getPluginMenu( QString menuName );
    //! Add the action to the submenu with the given name under the plugin menu
    void addPluginToMenu( QString name, QAction* action );
    //! Remove the action to the submenu with the given name under the plugin menu
    void removePluginMenu( QString name, QAction* action );
    //! Find the QMenu with the given name within the Database menu (ie the user visible text on the menu item)
    QMenu* getDatabaseMenu( QString menuName );
    //! Add the action to the submenu with the given name under the Database menu
    void addPluginToDatabaseMenu( QString name, QAction* action );
    //! Remove the action to the submenu with the given name under the Database menu
    void removePluginDatabaseMenu( QString name, QAction* action );
    //! Add "add layer" action to layer menu
    void insertAddLayerAction( QAction* action );
    //! Remove "add layer" action to layer menu
    void removeAddLayerAction( QAction* action );
    //! Add an icon to the plugin toolbar
    int addPluginToolBarIcon( QAction * qAction );
    //! Remove an icon from the plugin toolbar
    void removePluginToolBarIcon( QAction *qAction );
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
    void openProject( QAction *action );
    //! Save the map view as an image - user is prompted for image name using a dialog
    void saveMapAsImage();
    //! Open a project
    void fileOpen();
    //! Create a new project
    void fileNew();
    //! As above but allows forcing without prompt
    void fileNew( bool thePromptToSaveFlag );
    //! Calculate new rasters from existing ones
    void showRasterCalculator();
    void embedLayers();

    //! Create a new empty vector layer
    void newVectorLayer();
#ifdef HAVE_SPATIALITE
    //! Create a new empty spatialite layer
    void newSpatialiteLayer();
#endif
    //! Print the current map view frame
    void newPrintComposer();
    void showComposerManager();
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
    //! set the active layer
    bool setActiveLayer( QgsMapLayer * );
    //! Open the help contents in a browser
    void helpContents();
    //! Open the API documentation in a browser
    void apiDocumentation();
    //! Open the QGIS homepage in users browser
    void helpQgisHomePage();
    //! Open a url in the users configured browser
    void openURL( QString url, bool useQgisDocDirectory = true );
    //! Check qgis version against the qgis version server
    void checkQgisVersion();
    //!Invoke the custom projection dialog
    void customProjection();
    //! configure shortcuts
    void configureShortcuts();
    //! show customization dialog
    void customize();
    //! options dialog slot
    void options();
    //! Whats-this help slot
    void whatsThis();
    //! Set project properties, including map untis
    void projectProperties();
    //! Open project properties dialog and show the projections tab
    void projectPropertiesProjections();
    /*  void urlData(); */
    //! Show the spatial bookmarks dialog
    void showBookmarks();
    //! Create a new spatial bookmark
    void newBookmark();
    //! activates the add feature tool
    void addFeature();
    //! activates the move feature tool
    void moveFeature();
    //! activates the reshape features tool
    void reshapeFeatures();
    //! activates the split features tool
    void splitFeatures();
    //! activates the add ring tool
    void addRing();
    //! activates the add part tool
    void addPart();
    //! simplifies feature
    void simplifyFeature();
    //! deletes ring in polygon
    void deleteRing();
    //! deletes part of polygon
    void deletePart();
    //! merges the selected features together
    void mergeSelectedFeatures();
    //! merges the attributes of selected features
    void mergeAttributesOfSelectedFeatures();
    //! provides operations with nodes
    void nodeTool();
    //! activates the rotate points tool
    void rotatePointSymbols();
    //! shows the snapping Options
    void snappingOptions();

    //! activates the selection tool
    void select();

    //! activates the rectangle selection tool
    void selectByRectangle();

    //! activates the polygon selection tool
    void selectByPolygon();

    //! activates the freehand selection tool
    void selectByFreehand();

    //! activates the radius selection tool
    void selectByRadius();

    //! deselect features from all layers
    void deselectAll();

    //! refresh map canvas
    void refreshMapCanvas();

    //! starts/stops editing mode of the current layer
    void toggleEditing();

    //! save current edits and start new transaction
    void saveEdits();

    //! change layer subset of current vector layer
    void layerSubsetString();

    //! map tool changed
    void mapToolChanged( QgsMapTool *tool );

    /** Activates or deactivates actions depending on the current maplayer type.
    Is called from the legend when the current legend item has changed*/
    void activateDeactivateLayerRelatedActions( QgsMapLayer* layer );

    void selectionChanged( QgsMapLayer *layer );

    void showProgress( int theProgress, int theTotalSteps );
    void extentsViewToggled( bool theFlag );
    void showExtents();
    void showStatusMessage( QString theMessage );
    void updateMouseCoordinatePrecision();
    void hasCrsTransformEnabled( bool theFlag );
    void destinationSrsChanged();
    //    void debugHook();
    //! Add a vector layer to the map
    void addVectorLayer();
    //! Exit Qgis
    void fileExit();
    //! Add a WMS layer to the map
    void addWmsLayer();
    //! Add a WFS layer to the map
    void addWfsLayer();
    //! Set map tool to Zoom out
    void zoomOut();
    //! Set map tool to Zoom in
    void zoomIn();
    //! Set map tool to pan
    void pan();
    //! Identify feature(s) on the currently selected layer
    void identify();
    //! Measure distance
    void measure();
    //! Measure area
    void measureArea();
    //! Measure angle
    void measureAngle();

    //annotations
    void addFormAnnotation();
    void addTextAnnotation();
    void modifyAnnotation();

    //! shows label settings dialog (for labeling-ng)
    void labeling();

    //! starts/stops editing mode of a layer
    bool toggleEditing( QgsMapLayer *layer, bool allowCancel = true );

    //! save edits of a layer
    void saveEdits( QgsMapLayer *layer );

    //! save current vector layer
    void saveAsVectorFile();
    void saveSelectionAsVectorFile();

    //! show python console
    void showPythonDialog();

    //! Shows a warning when an old project file is read.
    void oldProjectVersionWarning( QString );

    //! Toggle map tips on/off
    void toggleMapTips();

    //! Show the map tip
    void showMapTip();

    //! Toggle full screen mode
    void toggleFullScreen();

    //! Set minimized mode of active window
    void showActiveWindowMinimized();

    //! Toggle maximized mode of active window
    void toggleActiveWindowMaximized();

    //! Raise, unminimize and activate this window
    void activate();

    //! Bring forward all open windows
    void bringAllToFront();

    //! Stops rendering of the main map
    void stopRendering();

    void showStyleManagerV2();

    void writeAnnotationItemsToProject( QDomDocument& doc );

    /**Creates the composer instances in a project file and adds them to the menu*/
    bool loadComposersFromProject( const QDomDocument& doc );

    bool loadAnnotationItemsFromProject( const QDomDocument& doc );

    //! Activates the move label tool
    void moveLabel();
    //! Activates rotate label tool
    void rotateLabel();
    //! Activates label property tool
    void changeLabelProperties();

  signals:
    /** emitted when a key is pressed and we want non widget sublasses to be able
      to pick up on this (e.g. maplayer) */
    void keyPressed( QKeyEvent *e );

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

    /** Signal emitted when the current theme is changed so plugins
     * can change there tool button icons.
     * @note This was added in QGIS 1.1
     */
    void currentThemeChanged( QString );

    /**This signal is emitted when a new composer instance has been created
       @note added in version 1.4*/
    void composerAdded( QgsComposerView* v );

    /**This signal is emitted before a new composer instance is going to be removed
      @note added in version 1.4*/
    void composerWillBeRemoved( QgsComposerView* v );

    /**This signal is emitted when QGIS' initialization is complete
     @note added in version 1.6*/
    void initializationCompleted();

  private:
    /** This method will open a dialog so the user can select the sublayers to load
    */
    void askUserForOGRSublayers( QgsVectorLayer *layer );
    void askUserForGDALSublayers( QgsRasterLayer *layer );
    /** Add a raster layer to the map (passed in as a ptr).
     * It won't force a refresh.
     */
    bool addRasterLayer( QgsRasterLayer * theRasterLayer );

    /** add this file to the recently opened/saved projects list
     *  pass settings by reference since creating more than one
     * instance simultaneously results in data loss.
     */
    void saveRecentProjectPath( QString projectPath, QSettings & settings );
    //! Update file menu with the current list of recently accessed projects
    void updateRecentProjectPaths();
    //! Read Well Known Binary stream from PostGIS
    //void readWKB(const char *, QStringList tables);
    //! shows the paste-transformations dialog
    void pasteTransformations();
    //! check to see if file is dirty and if so, prompt the user th save it
    bool saveDirty();
    /** Helper function to union several geometries together (used in function mergeSelectedFeatures)
      @return 0 in case of error or if canceled */
    QgsGeometry* unionGeometries( const QgsVectorLayer* vl, QgsFeatureList& featureList, bool &canceled );

    /**Deletes all the composer objects and clears mPrintComposers*/
    void deletePrintComposers();


    void saveAsVectorFileGeneral( bool saveOnlySelection );

    /**Returns all annotation items in the canvas*/
    QList<QgsAnnotationItem*> annotationItems();
    /**Removes annotation items in the canvas*/
    void removeAnnotationItems();

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
    void initLegend();
    void createOverview();
    void createCanvasTools();
    bool createDB();
    void createMapTips();
    void updateCRSStatusBar();
    void createDecorations();

    // actions for menus and toolbars -----------------

#ifdef Q_WS_MAC
    QAction *mActionWindowMinimize;
    QAction *mActionWindowZoom;
    QAction *mActionWindowSeparator1;
    QAction *mActionWindowAllToFront;
    QAction *mActionWindowSeparator2;
    QActionGroup *mWindowActions;
#endif

    QAction* mActionPluginSeparator1;
    QAction* mActionPluginSeparator2;

    // action groups ----------------------------------
    QActionGroup *mMapToolGroup;

    // menus ------------------------------------------

#ifdef Q_WS_MAC
    QMenu *mWindowMenu;
#endif
    QMenu *mPanelMenu;
    QMenu *mToolbarMenu;

    // docks ------------------------------------------
    QDockWidget *mLegendDock;
    QDockWidget *mLayerOrderDock;
    QDockWidget *mOverviewDock;
    QDockWidget *mpTileScaleDock;
    QDockWidget *mpGpsDock;
    QDockWidget *mLogDock;

#ifdef Q_WS_MAC
    //! Window menu action to select this window
    QAction *mWindowAction;
#endif

    class Tools
    {
      public:
        QgsMapTool* mZoomIn;
        QgsMapTool* mZoomOut;
        QgsMapTool* mPan;
        QgsMapTool* mIdentify;
        QgsMapTool* mMeasureDist;
        QgsMapTool* mMeasureArea;
        QgsMapTool* mMeasureAngle;
        QgsMapTool* mAddFeature;
        QgsMapTool* mMoveFeature;
        QgsMapTool* mReshapeFeatures;
        QgsMapTool* mSplitFeatures;
        QgsMapTool* mSelect;
        QgsMapTool* mSelectRectangle;
        QgsMapTool* mSelectPolygon;
        QgsMapTool* mSelectFreehand;
        QgsMapTool* mSelectRadius;
        QgsMapTool* mVertexAdd;
        QgsMapTool* mVertexMove;
        QgsMapTool* mVertexDelete;
        QgsMapTool* mAddRing;
        QgsMapTool* mAddPart;
        QgsMapTool* mSimplifyFeature;
        QgsMapTool* mDeleteRing;
        QgsMapTool* mDeletePart;
        QgsMapTool* mNodeTool;
        QgsMapTool* mRotatePointSymbolsTool;
        QgsMapTool* mAnnotation;
        QgsMapTool* mFormAnnotation;
        QgsMapTool* mTextAnnotation;
        QgsMapTool* mMoveLabel;
        QgsMapTool* mRotateLabel;
        QgsMapTool* mChangeLabelProperties;
    } mMapTools;

    QgsMapTool *mNonEditMapTool;

    //! Widget that will live on the statusbar to display "scale 1:"
    QLabel * mScaleLabel;
    //! Widget that will live on the statusbar to display scale value
    QComboBox * mScaleEdit;
    //! The validator for the mScaleEdit
    QValidator * mScaleEditValidator;
    //! Widget that will live on the statusbar to display "Coordinate / Extent"
    QLabel * mCoordsLabel;
    //! Widget that will live in the statusbar to display and edit coords
    QLineEdit * mCoordsEdit;
    //! The validator for the mCoordsEdit
    QValidator * mCoordsEditValidator;
    //! Widget that will live in the statusbar to show progress of operations
    QProgressBar * mProgressBar;
    //! Widget used to suppress rendering
    QCheckBox * mRenderSuppressionCBox;
    //! A toggle to switch between mouse coords and view extents display
    QToolButton * mToggleExtentsViewButton;
    //! Button used to stop rendering
    QToolButton* mStopRenderButton;
    //! Widget in status bar used to show current project CRS
    QLabel * mOnTheFlyProjectionStatusLabel;
    //! Widget in status bar used to show status of on the fly projection
    QToolButton * mOnTheFlyProjectionStatusButton;
    //! Popup menu
    QMenu * mPopupMenu;
    //! Top level database menu
    QMenu *mDatabaseMenu;
    //! Popup menu for the map overview tools
    QMenu *toolPopupOverviews;
    //! Popup menu for the display tools
    QMenu *toolPopupDisplay;
    //! Map canvas
    QgsMapCanvas *mMapCanvas;
    //! Table of contents (legend) for the map
    QgsLegend *mMapLegend;
    //! Table of contents (legend) to order layers of the map
    QgsLayerOrder *mMapLayerOrder;
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

    QSplashScreen *mSplash;
    //! list of recently opened/saved project files
    QStringList mRecentProjectPaths;
    //! Print composers of this project, accessible by id string
    QSet<QgsComposer*> mPrintComposers;
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
    QgsMapTip *mpMaptip;

    // Flag to indicate if maptips are on or off
    bool mMapTipsVisible;

    //!flag to indicate whether we are in fullscreen mode or not
    bool mFullScreenMode;

    //!flag to indicate that the previous screen mode was 'maximised'
    bool mPrevScreenModeMaximized;

    QgsPythonUtils* mPythonUtils;

    static QgisApp *smInstance;

    QgsUndoWidget* mUndoWidget;

    QgsBrowserDockWidget* mBrowserWidget;

    QgsSnappingDialog* mSnappingDialog;

    //! Persistent tile scale slider
    QgsTileScaleWidget * mpTileScaleWidget;

    QgsDecorationCopyright* mDecorationCopyright;
    QgsDecorationNorthArrow* mDecorationNorthArrow;
    QgsDecorationScaleBar* mDecorationScaleBar;

    int mLastComposerId;

#ifdef Q_OS_WIN
    int mSkipNextContextMenuEvent; // ugly hack
#endif

    //! Persistent GPS toolbox
    QgsGPSInformationWidget * mpGpsWidget;

    QgsMessageLogViewer *mLogViewer;

    QgsPalLabeling* mLBL;

    //! project changed
    void projectChanged( const QDomDocument & );
};

#endif
