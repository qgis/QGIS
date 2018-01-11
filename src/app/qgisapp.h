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
class QMenu;
class QPixmap;
class QProgressBar;
class QPushButton;
class QRect;
class QgsSettings;
class QSpinBox;
class QSplashScreen;
class QStringList;
class QToolButton;
class QTcpSocket;
class QValidator;
class QSystemTrayIcon;

class QgisAppInterface;
class QgisAppStyleSheet;
class QgsAnnotation;
class QgsMapCanvasAnnotationItem;
class QgsAuthManager;
class QgsBookmarks;
class QgsClipboard;
class QgsComposer;
class QgsComposition;
class QgsComposerManager;
class QgsContrastEnhancement;
class QgsCoordinateReferenceSystem;
class QgsCustomDropHandler;
class QgsCustomLayerOrderWidget;
class QgsDockWidget;
class QgsDoubleSpinBox;
class QgsFeature;
class QgsFeatureStore;
class QgsGeometry;
class QgsLayerTreeMapCanvasBridge;
class QgsLayerTreeView;
class QgsLayout;
class QgsMasterLayoutInterface;
class QgsLayoutCustomDropHandler;
class QgsLayoutDesignerDialog;
class QgsLayoutDesignerInterface;
class QgsLayoutManagerDialog;
class QgsMapCanvas;
class QgsMapCanvasDockWidget;
class QgsMapLayer;
class QgsMapLayerConfigWidgetFactory;
class QgsMapOverviewCanvas;
class QgsMapTip;
class QgsMapTool;
class QgsMapToolAddFeature;
class QgsMapToolDigitizeFeature;
class QgsMapToolAdvancedDigitizing;
class QgsMapToolIdentifyAction;
class QgsPluginLayer;
class QgsPluginLayer;
class QgsPluginManager;
class QgsPointXY;
class QgsPrintLayout;
class QgsProviderRegistry;
class QgsPythonUtils;
class QgsRasterLayer;
class QgsRectangle;
class QgsRuntimeProfiler;
class QgsSnappingUtils;
class QgsSnappingWidget;
class QgsStatusBarCoordinatesWidget;
class QgsStatusBarMagnifierWidget;
class QgsStatusBarScaleWidget;
class QgsTaskManagerStatusBarWidget;
class QgsTransactionGroup;
class QgsUndoWidget;
class QgsUserInputDockWidget;
class QgsVectorLayer;
class QgsVectorLayerTools;
class QgsWelcomePage;
class QgsOptionsWidgetFactory;
class QgsStatusBar;

class QgsUserProfileManagerWidgetFactory;

class Qgs3DMapCanvasDockWidget;

class QDomDocument;
class QNetworkReply;
class QNetworkProxy;
class QAuthenticator;

class QgsBrowserDockWidget;
class QgsAdvancedDigitizingDockWidget;
class QgsGPSInformationWidget;
class QgsStatisticalSummaryDockWidget;
class QgsMapCanvasTracer;

class QgsDecorationItem;

class QgsMessageLogViewer;
class QgsMessageBar;

class QgsDataItem;
class QgsTileScaleWidget;

class QgsLabelingWidget;
class QgsLayerStylingWidget;
class QgsDiagramProperties;
class QgsLocatorWidget;
class QgsDataSourceManagerDialog;
class QgsBrowserModel;
class QgsGeoCmsProviderRegistry;
class QgsLayoutQptDropHandler;


#include <QMainWindow>
#include <QToolBar>
#include <QAbstractSocket>
#include <QPointer>
#include <QSslError>
#include <QDateTime>
#include <QStackedWidget>

#include "qgsauthmanager.h"
#include "qgsconfig.h"
#include "qgsfeature.h"
#include "qgspointxy.h"
#include "qgsmessagebar.h"
#include "qgsmimedatautils.h"
#include "qgswelcomepageitemsmodel.h"
#include "qgsraster.h"
#include "qgsrasterminmaxorigin.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsoptionswidgetfactory.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsmasterlayoutinterface.h"
#include "ui_qgisapp.h"
#include "qgis_app.h"

#include <QGestureEvent>
#include <QTapAndHoldGesture>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class QgsLegendFilterButton;

/**
 * \class QgisApp
 * \brief Main window for the Qgis application
 */
class APP_EXPORT QgisApp : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
  public:
    //! Constructor
    QgisApp( QSplashScreen *splash, bool restorePlugins = true,
             bool skipVersionCheck = false, const QString &rootProfileLocation = QString(),
             const QString &activeProfile = QString(),
             QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Window );
    //! Constructor for unit tests
    QgisApp();

    ~QgisApp() override;

    QgisApp( QgisApp const & ) = delete;
    QgisApp &operator=( QgisApp const & ) = delete;

    /**
     * Open a raster layer for the given file
      \returns false if unable to open a raster layer for rasterFile
      \note
      This is essentially a simplified version of the above
      */
    QgsRasterLayer *addRasterLayer( const QString &rasterFile, const QString &baseName, bool guiWarning = true );

    /**
     * Returns and adjusted uri for the layer based on current and available CRS as well as the last selected image format
     * \since QGIS 2.8
     */
    QString crsAndFormatAdjustedLayerUri( const QString &uri, const QStringList &supportedCrs, const QStringList &supportedFormats ) const;

    //! Add a 'pre-made' map layer to the project
    void addMapLayer( QgsMapLayer *mapLayer );

    //! Set the extents of the map canvas
    void setExtent( const QgsRectangle &rect );

    /**
     * Open a raster or vector file; ignore other files.
      Used to process a commandline argument, FileOpen or Drop event.
      Set interactive to true if it is OK to ask the user for information (mostly for
      when a vector layer has sublayers and we want to ask which sublayers to use).
      \returns true if the file is successfully opened
      */
    bool openLayer( const QString &fileName, bool allowInteractive = false );

    /**
     * Open the specified project file; prompt to save previous project if necessary.
      Used to process a commandline argument, FileOpen or Drop event.
      */
    void openProject( const QString &fileName );

    void openLayerDefinition( const QString &filename );

    /**
     * Opens a layout template file and creates a new layout designer window for it.
     */
    void openTemplate( const QString &fileName );

    /**
     * Attempts to run a Python script
     * \param filePath full path to Python script
     * \since QGIS 2.7
     */
    void runScript( const QString &filePath );

    /**
     * Opens a qgis project file
      \returns false if unable to open the project
      */
    bool addProject( const QString &projectFile );

    //!Overloaded version of the private function with same name that takes the imagename as a parameter
    void saveMapAsImage( const QString &, QPixmap * );

    //! Get the mapcanvas object from the app
    QgsMapCanvas *mapCanvas();

    /**
     * Returns a list of all map canvases open in the app.
     */
    QList< QgsMapCanvas * > mapCanvases();

    /**
     * Create a new map canvas with the specified unique \a name.
     */
    QgsMapCanvas *createNewMapCanvas( const QString &name );

    /**
     * Create a new map canvas dock widget with the specified unique \a name.
     * \note the mapCanvas() inside the dock widget will initially be frozen to avoid multiple
     * unwanted map redraws. Callers must manually unfreeze the map canvas when they have finished
     * setting the initial state of the canvas and are ready for it to begin rendering.
     */
    QgsMapCanvasDockWidget *createNewMapCanvasDock( const QString &name );

    /**
     * Closes the additional map canvas with matching \a name.
     */
    void closeMapCanvas( const QString &name );

    /**
     * Closes any additional map canvases. The main map canvas will not
     * be affected.
     */
    void closeAdditionalMapCanvases();

    /**
     * Freezes all map canvases (or thaws them if the \a frozen argument is false).
     */
    void freezeCanvases( bool frozen = true );

    //! Return the messageBar object which allows displaying unobtrusive messages to the user.
    QgsMessageBar *messageBar();

    //! Open the message log dock widget *
    void openMessageLog();

    //! Adds a widget to the user input tool bar.
    void addUserInputWidget( QWidget *widget );

    //! Set theme (icons)
    void setTheme( const QString &themeName = "default" );

    void setIconSizes( int size );

    //! Get stylesheet builder object for app and layout designers
    QgisAppStyleSheet *styleSheetBuilder();

    //! Populates a menu with actions for opening layout designers
    void populateLayoutsMenu( QMenu *menu );

    //! Setup the toolbar popup menus for a given theme
    void setupToolbarPopups( QString themeName );
    //! Returns a pointer to the internal clipboard
    QgsClipboard *clipboard();

    static QgisApp *instance() { return sInstance; }

    //! initialize network manager
    void namSetup();

    //! update proxy settings
    void namUpdate();

    //! set up master password
    void masterPasswordSetup();

    /**
     * Add a dock widget to the main window. Overloaded from QMainWindow.
     * After adding the dock widget to the ui (by delegating to the QMainWindow
     * parent class, it will also add it to the View menu list of docks.*/
    void addDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget );
    void removeDockWidget( QDockWidget *dockwidget );

    /**
     * Add a toolbar to the main window. Overloaded from QMainWindow.
     * After adding the toolbar to the ui (by delegating to the QMainWindow
     * parent class, it will also add it to the View menu list of toolbars.*/
    QToolBar *addToolBar( const QString &name );

    /**
     * Add a toolbar to the main window. Overloaded from QMainWindow.
     * After adding the toolbar to the ui (by delegating to the QMainWindow
     * parent class, it will also add it to the View menu list of toolbars.
     * \since QGIS 2.3
     */
    void addToolBar( QToolBar *toolBar, Qt::ToolBarArea area = Qt::TopToolBarArea );


    /**
     * Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window. */
    void addWindow( QAction *action );

    /**
     * Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed. */
    void removeWindow( QAction *action );

    /**
     * Returns the active layout designers.
     */
    QSet<QgsLayoutDesignerDialog *> layoutDesigners() const { return mLayoutDesignerDialogs; }

    /**
     * Gets a unique title from user for new and duplicate layouts.
     *
     * The \a title argument will be filled with the new layout title.
     *
     * If \a acceptEmpty is true then empty titles will be acceptable (one will be generated).
     *
     * The \a currentTitle argument specifies a base name for initial title choice.
     *
     * \returns true if user did not cancel the dialog.
     */
    bool uniqueLayoutTitle( QWidget *parent, QString &title, bool acceptEmpty, QgsMasterLayoutInterface::Type type, const QString &currentTitle = QString() );

    /**
     * Creates a new print layout, opens a designer for it and returns a pointer to
     * designer dialog.
     *
     * If \a title is specified it will be used as the name for the new print layout,
     * otherwise the user will be prompted to enter a name for the layout.
     */
    QgsLayoutDesignerDialog *createNewPrintLayout( const QString &title = QString() );

    //! Creates a new report and returns a pointer to it
    QgsLayoutDesignerDialog *createNewReport( QString title = QString() );

    /**
     * Opens a layout designer dialog for an existing \a layout.
     * If a designer already exists for this layout then it will be activated.
     */
    QgsLayoutDesignerDialog *openLayoutDesignerDialog( QgsMasterLayoutInterface *layout );

    /**
     * Duplicates a \a layout and adds it to the current project.
     *
     * If \a title is set, it will be used as the title for the new layout. If it is not set,
     * and auto-generated title will be used instead.
     */
    QgsLayoutDesignerDialog *duplicateLayout( QgsMasterLayoutInterface *layout, const QString &title = QString() );

    //! Overloaded function used to sort menu entries alphabetically
    QMenu *createPopupMenu() override;

    /**
     * Access the vector layer tools. This will be an instance of {\see QgsGuiVectorLayerTools}
     * by default.
     * \returns  The vector layer tools
     */
    QgsVectorLayerTools *vectorLayerTools() { return mVectorLayerTools; }

    /**
     * Notify the user by using the system tray notifications
     *
     * \note usage of the system tray notifications should be limited
     *       to long running tasks and to when the user needs to be notified
     *       about interaction with OS services, like the password manager.
     *
     * \param title
     * \param message
     */
    void showSystemNotification( const QString &title, const QString &message );


    //! Actions to be inserted in menus and toolbars
    QAction *actionNewProject() { return mActionNewProject; }
    QAction *actionOpenProject() { return mActionOpenProject; }
    QAction *actionSaveProject() { return mActionSaveProject; }
    QAction *actionSaveProjectAs() { return mActionSaveProjectAs; }
    QAction *actionSaveMapAsImage() { return mActionSaveMapAsImage; }
    QAction *actionSaveMapAsPdf() { return mActionSaveMapAsPdf; }
    QAction *actionProjectProperties() { return mActionProjectProperties; }
    QAction *actionShowLayoutManager() { return mActionShowLayoutManager; }
    QAction *actionNewPrintLayout() { return mActionNewPrintLayout; }
    QAction *actionExit() { return mActionExit; }

    QAction *actionCutFeatures() { return mActionCutFeatures; }
    QAction *actionCopyFeatures() { return mActionCopyFeatures; }
    QAction *actionPasteFeatures() { return mActionPasteFeatures; }
    QAction *actionPasteAsNewVector() { return mActionPasteAsNewVector; }
    QAction *actionPasteAsNewMemoryVector() { return mActionPasteAsNewMemoryVector; }
    QAction *actionDeleteSelected() { return mActionDeleteSelected; }
    QAction *actionAddFeature() { return mActionAddFeature; }
    QAction *actionMoveFeature() { return mActionMoveFeature; }
    QAction *actionMoveFeatureCopy() { return mActionMoveFeatureCopy; }
    QAction *actionRotateFeature() { return mActionRotateFeature;}
    QAction *actionSplitFeatures() { return mActionSplitFeatures; }
    QAction *actionSplitParts() { return mActionSplitParts; }
    QAction *actionAddRing() { return mActionAddRing; }
    QAction *actionFillRing() { return mActionFillRing; }
    QAction *actionAddPart() { return mActionAddPart; }
    QAction *actionSimplifyFeature() { return mActionSimplifyFeature; }
    QAction *actionDeleteRing() { return mActionDeleteRing; }
    QAction *actionDeletePart() { return mActionDeletePart; }
    QAction *actionNodeTool() { return mActionNodeTool; }
    QAction *actionSnappingOptions() { return mActionSnappingOptions; }
    QAction *actionOffsetCurve() { return mActionOffsetCurve; }
    QAction *actionPan() { return mActionPan; }
    QAction *actionPanToSelected() { return mActionPanToSelected; }
    QAction *actionZoomIn() { return mActionZoomIn; }
    QAction *actionZoomOut() { return mActionZoomOut; }
    QAction *actionSelect() { return mActionSelectFeatures; }
    QAction *actionSelectRectangle() { return mActionSelectFeatures; }
    QAction *actionSelectPolygon() { return mActionSelectPolygon; }
    QAction *actionSelectFreehand() { return mActionSelectFreehand; }
    QAction *actionSelectRadius() { return mActionSelectRadius; }
    QAction *actionIdentify() { return mActionIdentify; }
    QAction *actionFeatureAction() { return mActionFeatureAction; }
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

    QAction *actionDataSourceManager() { return mActionDataSourceManager; }
    QAction *actionNewVectorLayer() { return mActionNewVectorLayer; }
    QAction *actionNewSpatialLiteLayer() { return mActionNewSpatiaLiteLayer; }
    QAction *actionEmbedLayers() { return mActionEmbedLayers; }
    QAction *actionAddOgrLayer() { return mActionAddOgrLayer; }
    QAction *actionAddRasterLayer() { return mActionAddRasterLayer; }
    QAction *actionAddPgLayer() { return mActionAddPgLayer; }
    QAction *actionAddSpatiaLiteLayer() { return mActionAddSpatiaLiteLayer; }
    QAction *actionAddWmsLayer() { return mActionAddWmsLayer; }
    QAction *actionAddWcsLayer() { return mActionAddWcsLayer; }
    QAction *actionAddWfsLayer() { return mActionAddWfsLayer; }
    QAction *actionAddAfsLayer() { return mActionAddAfsLayer; }
    QAction *actionAddAmsLayer() { return mActionAddAmsLayer; }
    QAction *actionCopyLayerStyle() { return mActionCopyStyle; }
    QAction *actionPasteLayerStyle() { return mActionPasteStyle; }
    QAction *actionOpenTable() { return mActionOpenTable; }
    QAction *actionOpenFieldCalculator() { return mActionOpenFieldCalc; }
    QAction *actionToggleEditing() { return mActionToggleEditing; }
    QAction *actionSaveActiveLayerEdits() { return mActionSaveLayerEdits; }
    QAction *actionAllEdits() { return mActionAllEdits; }
    QAction *actionSaveEdits() { return mActionSaveEdits; }
    QAction *actionSaveAllEdits() { return mActionSaveAllEdits; }
    QAction *actionRollbackEdits() { return mActionRollbackEdits; }
    QAction *actionRollbackAllEdits() { return mActionRollbackAllEdits; }
    QAction *actionCancelEdits() { return mActionCancelEdits; }
    QAction *actionCancelAllEdits() { return mActionCancelAllEdits; }
    QAction *actionLayerSaveAs() { return mActionLayerSaveAs; }
    QAction *actionRemoveLayer() { return mActionRemoveLayer; }
    QAction *actionDuplicateLayer() { return mActionDuplicateLayer; }
    //! \since QGIS 2.4
    QAction *actionSetLayerScaleVisibility() { return mActionSetLayerScaleVisibility; }
    QAction *actionSetLayerCrs() { return mActionSetLayerCRS; }
    QAction *actionSetProjectCrsFromLayer() { return mActionSetProjectCRSFromLayer; }
    QAction *actionLayerProperties() { return mActionLayerProperties; }
    QAction *actionLayerSubsetString() { return mActionLayerSubsetString; }
    QAction *actionAddToOverview() { return mActionAddToOverview; }
    QAction *actionAddAllToOverview() { return mActionAddAllToOverview; }
    QAction *actionRemoveAllFromOverview() { return mActionRemoveAllFromOverview; }
    QAction *actionHideAllLayers() { return mActionHideAllLayers; }
    QAction *actionShowAllLayers() { return mActionShowAllLayers; }
    QAction *actionHideSelectedLayers() { return mActionHideSelectedLayers; }
    QAction *actionHideDeselectedLayers() { return mActionHideDeselectedLayers; }
    QAction *actionShowSelectedLayers() { return mActionShowSelectedLayers; }

    QAction *actionManagePlugins() { return mActionManagePlugins; }
    QAction *actionPluginListSeparator() { return mActionPluginSeparator1; }
    QAction *actionPluginPythonSeparator() { return mActionPluginSeparator2; }
    QAction *actionShowPythonDialog() { return mActionShowPythonDialog; }

    QAction *actionToggleFullScreen() { return mActionToggleFullScreen; }
    QAction *actionTogglePanelsVisibility() { return mActionTogglePanelsVisibility; }
    QAction *actionOptions() { return mActionOptions; }
    QAction *actionCustomProjection() { return mActionCustomProjection; }
    QAction *actionConfigureShortcuts() { return mActionConfigureShortcuts; }

#ifdef Q_OS_MAC
    QAction *actionWindowMinimize() { return mActionWindowMinimize; }
    QAction *actionWindowZoom() { return mActionWindowZoom; }
    QAction *actionWindowAllToFront() { return mActionWindowAllToFront; }
#endif

    QAction *actionHelpContents() { return mActionHelpContents; }
    QAction *actionHelpAPI() { return mActionHelpAPI; }
    QAction *actionReportaBug() { return mActionReportaBug; }
    QAction *actionQgisHomePage() { return mActionQgisHomePage; }
    QAction *actionCheckQgisVersion() { return mActionCheckQgisVersion; }
    QAction *actionAbout() { return mActionAbout; }
    QAction *actionSponsors() { return mActionSponsors; }

    QAction *actionShowPinnedLabels() { return mActionShowPinnedLabels; }

    //! Menus
    QMenu *projectMenu() { return mProjectMenu; }
    QMenu *editMenu() { return mEditMenu; }
    QMenu *viewMenu() { return mViewMenu; }
    QMenu *layerMenu() { return mLayerMenu; }
    QMenu *newLayerMenu() { return mNewLayerMenu; }
    //! \since QGIS 2.5
    QMenu *addLayerMenu() { return mAddLayerMenu; }
    QMenu *settingsMenu() { return mSettingsMenu; }
    QMenu *pluginMenu() { return mPluginMenu; }
    QMenu *databaseMenu() { return mDatabaseMenu; }
    QMenu *rasterMenu() { return mRasterMenu; }
    QMenu *vectorMenu() { return mVectorMenu; }
    QMenu *webMenu() { return mWebMenu; }
#ifdef Q_OS_MAC
    QMenu *firstRightStandardMenu() { return mWindowMenu; }
    QMenu *windowMenu() { return mWindowMenu; }
#else
    QMenu *firstRightStandardMenu() { return mHelpMenu; }
    QMenu *windowMenu() { return nullptr; }
#endif
    QMenu *helpMenu() { return mHelpMenu; }

    //! Toolbars

    /**
     * Get a reference to a toolbar. Mainly intended
     *   to be used by plugins that want to specifically add
     *   an icon into the file toolbar for consistency e.g.
     *   addWFS and GPS plugins.
     */
    QToolBar *fileToolBar() { return mFileToolBar; }
    QToolBar *layerToolBar() { return mLayerToolBar; }
    QToolBar *dataSourceManagerToolBar() { return mDataSourceManagerToolBar; }
    QToolBar *mapNavToolToolBar() { return mMapNavToolBar; }
    QToolBar *digitizeToolBar() { return mDigitizeToolBar; }
    QToolBar *advancedDigitizeToolBar() { return mAdvancedDigitizeToolBar; }
    QToolBar *attributesToolBar() { return mAttributesToolBar; }
    QToolBar *pluginToolBar() { return mPluginToolBar; }
    QToolBar *helpToolBar() { return mHelpToolBar; }
    QToolBar *rasterToolBar() { return mRasterToolBar; }
    QToolBar *vectorToolBar() { return mVectorToolBar; }
    QToolBar *databaseToolBar() { return mDatabaseToolBar; }
    QToolBar *webToolBar() { return mWebToolBar; }

    QgsStatusBar *statusBarIface() { return mStatusBar; }

    //! return CAD dock widget
    QgsAdvancedDigitizingDockWidget *cadDockWidget() { return mAdvancedDigitizingDockWidget; }

    //! Returns map overview canvas
    QgsMapOverviewCanvas *mapOverviewCanvas() { return mOverviewCanvas; }

    QgsLocatorWidget *locatorWidget() { return mLocatorWidget; }

    //! show layer properties
    void showLayerProperties( QgsMapLayer *ml );

    //! returns pointer to map legend
    QgsLayerTreeView *layerTreeView();

    QgsLayerTreeMapCanvasBridge *layerTreeCanvasBridge() { return mLayerTreeCanvasBridge; }

    //! returns pointer to plugin manager
    QgsPluginManager *pluginManager();

    /**
     * The applications user profile manager.
     */
    QgsUserProfileManager *userProfileManager();

    /**
     * Return vector layers in edit mode
     * \param modified whether to return only layers that have been modified
     * \returns list of layers in legend order, or empty list */
    QList<QgsMapLayer *> editableLayers( bool modified = false ) const;

    //! Get timeout for timed messages: default of 5 seconds
    int messageTimeout();

    //! emit initializationCompleted signal
    void completeInitialization();

    void emitCustomCrsValidation( QgsCoordinateReferenceSystem &crs );

    QList<QgsDecorationItem *> decorationItems() { return mDecorationItems; }
    void addDecorationItem( QgsDecorationItem *item ) { mDecorationItems.append( item ); }

    //! \since QGIS 2.1
    static QString normalizedMenuName( const QString &name ) { return name.normalized( QString::NormalizationForm_KD ).remove( QRegExp( "[^a-zA-Z]" ) ); }

    void parseVersionInfo( QNetworkReply *reply, int &latestVersion, QStringList &versionInfo );

    //! Register a new tab in the layer properties dialog
    void registerMapLayerPropertiesFactory( QgsMapLayerConfigWidgetFactory *factory );

    //! Unregister a previously registered tab in the layer properties dialog
    void unregisterMapLayerPropertiesFactory( QgsMapLayerConfigWidgetFactory *factory );

    //! Register a new tab in the options dialog
    void registerOptionsWidgetFactory( QgsOptionsWidgetFactory *factory );

    //! Unregister a previously registered tab in the options dialog
    void unregisterOptionsWidgetFactory( QgsOptionsWidgetFactory *factory );

    //! Register a new custom drop handler.
    void registerCustomDropHandler( QgsCustomDropHandler *handler );

    //! Unregister a previously registered custom drop handler.
    void unregisterCustomDropHandler( QgsCustomDropHandler *handler );

    //! Register a new custom layout drop handler.
    void registerCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler );

    //! Unregister a previously registered custom layout drop handler.
    void unregisterCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler );

    //! Returns a list of registered custom layout drop handlers.
    QVector<QPointer<QgsLayoutCustomDropHandler >> customLayoutDropHandlers() const;

    //! Returns the active map layer.
    QgsMapLayer *activeLayer();

    /**
     * Returns the toolbar icon size. If \a dockedToolbar is true, the icon size
     * for toolbars contained within docks is returned.
     */
    QSize iconSize( bool dockedToolbar = false ) const;

    /**
      * Checks available datum transforms and ask user if several are available and none
      * is chosen. Dialog is shown only if global option is set accordingly.
      * \returns true if a datum transform has been specifically chosen by user or only one is available.
      * \since 3.0
      */
    bool askUserForDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs );

  public slots:
    //! save current vector layer
    void saveAsFile( QgsMapLayer *layer = nullptr );

    //! Process the list of URIs that have been dropped in QGIS
    void handleDropUriList( const QgsMimeDataUtils::UriList &lst );
    //! Convenience function to open either a project or a layer file.
    void openFile( const QString &fileName );
    void layerTreeViewDoubleClicked( const QModelIndex &index );
    //! Make sure the insertion point for new layers is up-to-date with the current item in layer tree view
    void updateNewLayerInsertionPoint();
    //! connected to layer tree registry bridge, selects first of the newly added map layers
    void autoSelectAddedLayer( QList<QgsMapLayer *> layers );
    void activeLayerChanged( QgsMapLayer *layer );
    //! Zoom to full extent
    void zoomFull();
    //! Zoom to the previous extent
    void zoomToPrevious();
    //! Zoom to the forward extent
    void zoomToNext();
    //! Zoom to selected features
    void zoomToSelected();
    //! Pan map to selected features
    void panToSelected();

    //! open the properties dialog for the currently selected layer
    void layerProperties();

    //! show the attribute table for the currently selected layer
    void attributeTable( QgsAttributeTableFilterModel::FilterMode filter = QgsAttributeTableFilterModel::ShowAll );

    void fieldCalculator();

    //! mark project dirty
    void markDirty();

    /**
     * \brief layersWereAdded is triggered when layers were added
     * This method loops through the list of \a layers and connect all
     * application signals that need to listen to layer events.
     * \param layers list of map layers that have been added
     */
    void layersWereAdded( const QList<QgsMapLayer *> &layers );

    /* layer will be removed - changed from removingLayer to removingLayers
       in 1.8.
     */
    void removingLayers( const QStringList & );

    //! starts/stops editing mode of the current layer
    void toggleEditing();

    //! starts/stops editing mode of a layer
    bool toggleEditing( QgsMapLayer *layer, bool allowCancel = true );

    //! Save edits for active vector layer and start new transactions
    void saveActiveLayerEdits();

    /**
     * Save edits of a layer
     * \param leaveEditable leave the layer in editing mode when done
     * \param triggerRepaint send layer signal to repaint canvas when done
     */
    void saveEdits( QgsMapLayer *layer, bool leaveEditable = true, bool triggerRepaint = true );

    /**
     * Cancel edits for a layer
      * \param leaveEditable leave the layer in editing mode when done
      * \param triggerRepaint send layer signal to repaint canvas when done
      */
    void cancelEdits( QgsMapLayer *layer, bool leaveEditable = true, bool triggerRepaint = true );

    //! Save current edits for selected layer(s) and start new transaction(s)
    void saveEdits();

    //! Save edits for all layers and start new transactions
    void saveAllEdits( bool verifyAction = true );

    //! Rollback current edits for selected layer(s) and start new transaction(s)
    void rollbackEdits();

    //! Rollback edits for all layers and start new transactions
    void rollbackAllEdits( bool verifyAction = true );

    //! Cancel edits for selected layer(s) and toggle off editing
    void cancelEdits();

    //! Cancel all edits for all layers and toggle off editing
    void cancelAllEdits( bool verifyAction = true );

    void updateUndoActions();

    //! cuts selected features on the active layer to the clipboard

    /**
       \param layerContainingSelection  The layer that the selection will be taken from
                                        (defaults to the active layer on the legend)
     */
    void cutSelectionToClipboard( QgsMapLayer *layerContainingSelection = nullptr );
    //! copies selected features on the active layer to the clipboard

    /**
       \param layerContainingSelection  The layer that the selection will be taken from
                                        (defaults to the active layer on the legend)
     */
    void copySelectionToClipboard( QgsMapLayer *layerContainingSelection = nullptr );
    //! copies features on the clipboard to the active layer

    /**
       \param destinationLayer  The layer that the clipboard will be pasted to
                                (defaults to the active layer on the legend)
     */
    void pasteFromClipboard( QgsMapLayer *destinationLayer = nullptr );
    //! copies features on the clipboard to a new vector layer
    void pasteAsNewVector();
    //! copies features on the clipboard to a new memory vector layer
    QgsVectorLayer *pasteAsNewMemoryVector( const QString &layerName = QString() );
    //! copies style of the active layer to the clipboard

    /**
       \param sourceLayer  The layer where the style will be taken from
                                        (defaults to the active layer on the legend)
     */
    void copyStyle( QgsMapLayer *sourceLayer = nullptr );
    //! pastes style on the clipboard to the active layer

    /**
       \param destinationLayer  The layer that the clipboard will be pasted to
                                (defaults to the active layer on the legend)
     */
    void pasteStyle( QgsMapLayer *destinationLayer = nullptr );

    //! copies features to internal clipboard
    void copyFeatures( QgsFeatureStore &featureStore );
    void loadGDALSublayers( const QString &uri, const QStringList &list );

    //! Deletes the selected attributes for the currently selected vector layer
    void deleteSelected( QgsMapLayer *layer = nullptr, QWidget *parent = nullptr, bool promptConfirmation = false );

    //! project was written
    void writeProject( QDomDocument & );

    //! project was read
    void readProject( const QDomDocument & );

    //! Set app stylesheet from settings
    void setAppStyleSheet( const QString &stylesheet );

    //! request credentials for network manager
    void namAuthenticationRequired( QNetworkReply *reply, QAuthenticator *auth );
    void namProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *auth );
#ifndef QT_NO_SSL
    void namSslErrors( QNetworkReply *reply, const QList<QSslError> &errors );
#endif
    void namRequestTimedOut( QNetworkReply *reply );

    //! Schedule and erase of the authentication database upon confirmation
    void eraseAuthenticationDatabase();

    //! Push authentication manager output to messagebar
    void authMessageOut( const QString &message, const QString &authtag, QgsAuthManager::MessageLevel level );

    //! update default action of toolbutton
    void toolButtonActionTriggered( QAction * );

    //! layer selection changed
    void legendLayerSelectionChanged();

    //! Watch for QFileOpenEvent.
    bool event( QEvent *event ) override;


    /**
     * \brief dataSourceManager Open the DataSourceManager dialog
     * \param pageName the page name, usually the provider name or "browser" (for the browser panel)
     *        or "ogr" (vector layers) or "raster" (raster layers)
     */
    void dataSourceManager( const QString &pageName = QString() );

    /**
     * Add a raster layer directly without prompting user for location
      The caller must provide information compatible with the provider plugin
      using the uri and baseName. The provider can use these
      parameters in any way necessary to initialize the layer. The baseName
      parameter is used in the Map Legend so it should be formed in a meaningful
      way.
      */
    QgsRasterLayer *addRasterLayer( QString const &uri, QString const &baseName, QString const &providerKey );

    /**
     * Add a vector layer directly without prompting user for location
      The caller must provide information compatible with the provider plugin
      using the vectorLayerPath and baseName. The provider can use these
      parameters in any way necessary to initialize the layer. The baseName
      parameter is used in the Map Legend so it should be formed in a meaningful
      way.
      */
    QgsVectorLayer *addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey );

    /**
     * \brief overloaded version of the private addLayer method that takes a list of
     * file names instead of prompting user with a dialog.
     \param enc encoding type for the layer
    \param dataSourceType type of ogr datasource
     \returns true if successfully added layer
     */
    bool addVectorLayers( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType );

    /**
     * Overloaded version of the private addRasterLayer()
      Method that takes a list of file names instead of prompting
      user with a dialog.
      \returns true if successfully added layer(s)
      */
    bool addRasterLayers( const QStringList &layerQStringList, bool guiWarning = true );

    //! Open a plugin layer using its provider
    QgsPluginLayer *addPluginLayer( const QString &uri, const QString &baseName, const QString &providerKey );

    void versionReplyFinished();

    QgsMessageLogViewer *logViewer() { return mLogViewer; }

    //! Update project menu with the project templates
    void updateProjectFromTemplates();

    //! Opens the options dialog
    void showOptionsDialog( QWidget *parent = nullptr, const QString &currentPage = QString() );

    /**
     * Refreshes the state of the layer actions toolbar action
      * \since QGIS 2.1 */
    void refreshActionFeatureAction();

    QMenu *panelMenu() { return mPanelMenu; }

    void renameView();

    void showProgress( int progress, int totalSteps );
    void showStatusMessage( const QString &message );

    //! set the active layer
    bool setActiveLayer( QgsMapLayer * );

    /**
     * Reload connections emitting the connectionsChanged signal
     * \since QGIS 3.0
     */
    void reloadConnections();

    /**
     * Shows the layout manager dialog.
     * \since QGIS 3.0
     */
    void showLayoutManager();

  protected:

    //! Handle state changes (WindowTitleChange)
    void changeEvent( QEvent *event ) override;
    //! Have some control over closing of the application
    void closeEvent( QCloseEvent *event ) override;

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

    //! reimplements widget keyPress event so we can check if cancel was pressed
    void keyPressEvent( QKeyEvent *event ) override;

#ifdef ANDROID
    //! reimplements widget keyReleaseEvent event so we can check if back was pressed
    virtual void keyReleaseEvent( QKeyEvent *event );
#endif

  private slots:
    void newProfile();

    void onTaskCompleteShowNotify( long taskId, int status );

    void onTransactionGroupsChanged();

    void transactionGroupCommitError( const QString &error );

    void onSnappingConfigChanged();

    //! validate a SRS
    void validateCrs( QgsCoordinateReferenceSystem &crs );

    //! QGIS Sponsors
    void sponsors();
    //! About QGIS
    void about();
    //#ifdef HAVE_POSTGRESQL
    //! Add a databaselayer to the map
    void addDatabaseLayer();
    //#endif
    //! Add a list of database layers to the map
    void addDatabaseLayers( QStringList const &layerPathList, QString const &providerKey );
    //! Add a SpatiaLite layer to the map
    void addSpatiaLiteLayer();
    //! Add a Delimited Text layer to the map
    void addDelimitedTextLayer();
    //! Add a vector layer defined by uri, layer name, data source uri
    void addSelectedVectorLayer( const QString &uri, const QString &layerName, const QString &provider );
    //! Replace the selected layer by a vector layer defined by uri, layer name, data source uri
    void replaceSelectedVectorLayer( const QString &oldId, const QString &uri, const QString &layerName, const QString &provider );
    //! Add a MSSQL layer to the map
    void addMssqlLayer();
    //! Add a DB2 layer to the map
    void addDb2Layer();
    //#ifdef HAVE_ORACLE
    //! Add a Oracle layer to the map
    void addOracleLayer();
    //#endif
    //! Add a virtual layer
    void addVirtualLayer();
    //! toggles whether the current selected layer is in overview or not
    void isInOverview();
    //! Store the position for map tool tip
    void saveLastMousePosition( const QgsPointXY & );
    //! Slot to show current map scale;
    void showScale( double scale );

    /**
     * Slot to handle user rotation input;
     * \since QGIS 2.8
     */
    void userRotation();
    //! Remove a layer from the map and legend
    void removeLayer();
    //! Duplicate map layer(s) in legend
    void duplicateLayers( const QList<QgsMapLayer *> &lyrList = QList<QgsMapLayer *>() );
    //! Set scale visibility of selected layers
    void setLayerScaleVisibility();
    //! Zoom to nearest scale such that current layer is visible
    void zoomToLayerScale();
    //! Set CRS of a layer
    void setLayerCrs();
    //! Assign layer CRS to project
    void setProjectCrsFromLayer();

    /**
     * Zooms so that the pixels of the raster layer occupies exactly one screen pixel.
        Only works on raster layers*/
    void legendLayerZoomNative();

    /**
     * Stretches the raster layer, if stretching is active, based on the min and max of the current extent.
        Only workds on raster layers*/
    void legendLayerStretchUsingCurrentExtent();

    //! Apply the same style to selected layers or to current legend group
    void applyStyleToGroup();

    //! Set the CRS of the current legend group
    void legendGroupSetCrs();
    //! Set the WMS data of the current legend group
    void legendGroupSetWmsData();

    //! zoom to extent of layer
    void zoomToLayerExtent();
    //! zoom to actual size of raster layer
    void zoomActualSize();

    /**
     * Perform a local histogram stretch on the active raster layer
     * (stretch based on pixel values in view extent).
     * Valid for non wms raster layers only. */
    void localHistogramStretch();

    /**
     * Perform a full histogram stretch on the active raster layer
     * (stretch based on pixels values in full dataset)
     * Valid for non wms raster layers only. */
    void fullHistogramStretch();
    //! Perform a local cumulative cut stretch
    void localCumulativeCutStretch();
    //! Perform a full extent cumulative cut stretch
    void fullCumulativeCutStretch();

    /**
     * Increase raster brightness
     * Valid for non wms raster layers only. */
    void increaseBrightness();

    /**
     * Decrease raster brightness
     * Valid for non wms raster layers only. */
    void decreaseBrightness();

    /**
     * Increase raster contrast
     * Valid for non wms raster layers only. */
    void increaseContrast();

    /**
     * Decrease raster contrast
     * Valid for non wms raster layers only. */
    void decreaseContrast();
    //! plugin manager
    void showPluginManager();
    //! load Python support if possible
    void loadPythonSupport();

    //! Find the QMenu with the given name within plugin menu (ie the user visible text on the menu item)
    QMenu *getPluginMenu( const QString &menuName );
    //! Add the action to the submenu with the given name under the plugin menu
    void addPluginToMenu( const QString &name, QAction *action );
    //! Remove the action to the submenu with the given name under the plugin menu
    void removePluginMenu( const QString &name, QAction *action );
    //! Find the QMenu with the given name within the Database menu (ie the user visible text on the menu item)
    QMenu *getDatabaseMenu( const QString &menuName );
    //! Add the action to the submenu with the given name under the Database menu
    void addPluginToDatabaseMenu( const QString &name, QAction *action );
    //! Remove the action to the submenu with the given name under the Database menu
    void removePluginDatabaseMenu( const QString &name, QAction *action );
    //! Find the QMenu with the given name within the Raster menu (ie the user visible text on the menu item)
    QMenu *getRasterMenu( const QString &menuName );
    //! Add the action to the submenu with the given name under the Raster menu
    void addPluginToRasterMenu( const QString &name, QAction *action );
    //! Remove the action to the submenu with the given name under the Raster menu
    void removePluginRasterMenu( const QString &name, QAction *action );
    //! Find the QMenu with the given name within the Vector menu (ie the user visible text on the menu item)
    QMenu *getVectorMenu( const QString &menuName );
    //! Add the action to the submenu with the given name under the Vector menu
    void addPluginToVectorMenu( const QString &name, QAction *action );
    //! Remove the action to the submenu with the given name under the Vector menu
    void removePluginVectorMenu( const QString &name, QAction *action );
    //! Find the QMenu with the given name within the Web menu (ie the user visible text on the menu item)
    QMenu *getWebMenu( const QString &menuName );
    //! Add the action to the submenu with the given name under the Web menu
    void addPluginToWebMenu( const QString &name, QAction *action );
    //! Remove the action to the submenu with the given name under the Web menu
    void removePluginWebMenu( const QString &name, QAction *action );
    //! Add "add layer" action to layer menu
    void insertAddLayerAction( QAction *action );
    //! Remove "add layer" action to layer menu
    void removeAddLayerAction( QAction *action );
    //! Add an icon to the plugin toolbar
    int addPluginToolBarIcon( QAction *qAction );

    /**
     * Add a widget to the plugins toolbar.
     * To remove this widget again, call removeToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    QAction *addPluginToolBarWidget( QWidget *widget );
    //! Remove an icon from the plugin toolbar
    void removePluginToolBarIcon( QAction *qAction );
    //! Add an icon to the Raster toolbar
    int addRasterToolBarIcon( QAction *qAction );

    /**
     * Add a widget to the raster toolbar.
     * To remove this widget again, call removeRasterToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    QAction *addRasterToolBarWidget( QWidget *widget );
    //! Remove an icon from the Raster toolbar
    void removeRasterToolBarIcon( QAction *qAction );
    //! Add an icon to the Vector toolbar
    int addVectorToolBarIcon( QAction *qAction );

    /**
     * Add a widget to the vector toolbar.
     * To remove this widget again, call removeVectorToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    QAction *addVectorToolBarWidget( QWidget *widget );
    //! Remove an icon from the Vector toolbar
    void removeVectorToolBarIcon( QAction *qAction );
    //! Add an icon to the Database toolbar
    int addDatabaseToolBarIcon( QAction *qAction );

    /**
     * Add a widget to the database toolbar.
     * To remove this widget again, call removeDatabaseToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    QAction *addDatabaseToolBarWidget( QWidget *widget );
    //! Remove an icon from the Database toolbar
    void removeDatabaseToolBarIcon( QAction *qAction );
    //! Add an icon to the Web toolbar
    int addWebToolBarIcon( QAction *qAction );

    /**
     * Add a widget to the web toolbar.
     * To remove this widget again, call removeWebToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    QAction *addWebToolBarWidget( QWidget *widget );
    //! Remove an icon from the Web toolbar
    void removeWebToolBarIcon( QAction *qAction );
    //! Save window state
    void saveWindowState();
    //! Restore the window and toolbar state
    void restoreWindowState();
    //! Save project. Returns true if the user selected a file to save to, false if not.
    bool fileSave();
    //! Save project as
    void fileSaveAs();
    //! Export project in dxf format
    void dxfExport();
    //! Import layers in dwg format
    void dwgImport();

    /**
     * Open the project file corresponding to the
     * text)= of the given action.
     */
    void openProject( QAction *action );
    //! Save the map view as an image - user is prompted for image name using a dialog
    void saveMapAsImage();
    //! Save the map view as a pdf - user is prompted for image name using a dialog
    void saveMapAsPdf();
    //! Open a project
    void fileOpen();
    //! Create a new project
    void fileNew();
    //! Create a new blank project (no template)
    void fileNewBlank();
    //! As above but allows forcing without prompt and forcing blank project
    void fileNew( bool promptToSaveFlag, bool forceBlank = false );
    //! What type of project to open after launch
    void fileOpenAfterLaunch();
    //! After project read, set any auto-opened project as successful
    void fileOpenedOKAfterLaunch();
    //! Create a new file from a template project
    bool fileNewFromTemplate( const QString &fileName );
    void fileNewFromTemplateAction( QAction *qAction );
    void fileNewFromDefaultTemplate();
    //! Calculate new rasters from existing ones
    void showRasterCalculator();
    //! Open dialog to align raster layers
    void showAlignRasterTool();
    void embedLayers();

    //! Creates a new map canvas view
    void newMapCanvas();
    //! Creates a new 3D map canvas view
    void new3DMapCanvas();

    //! Create a new empty vector layer
    void newVectorLayer();
    //! Create a new memory layer
    void newMemoryLayer();
    //! Create a new empty SpatiaLite layer
    void newSpatialiteLayer();
    //! Create a new empty GeoPackage layer
    void newGeoPackageLayer();

    //! Create a new print layout
    void newPrintLayout();

    //! Create a new report
    void newReport();

    //! Slot to handle display of layouts menu, e.g. sorting
    void layoutsMenuAboutToShow();

    //! Add all loaded layers into the overview - overrides qgisappbase method
    void addAllToOverview();
    //! Remove all loaded layers from the overview - overrides qgisappbase method
    void removeAllFromOverview();
    //reimplements method from base (gui) class
    void hideAllLayers();
    //reimplements method from base (gui) class
    void showAllLayers();
    //reimplements method from base (gui) class
    void hideSelectedLayers();
    //! Hides any layers which are not selected
    void hideDeselectedLayers();
    //reimplements method from base (gui) class
    void showSelectedLayers();

    //! Open the help contents in a browser
    void helpContents();
    //! Open the API documentation in a browser
    void apiDocumentation();
    //! Open the Bugtracker page in a browser
    void reportaBug();
    //! Open the QGIS support page
    void supportProviders();
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
    //! activates the copy and move feature tool
    void moveFeatureCopy();
    //! activates the offset curve tool
    void offsetCurve();
    //! activates the reshape features tool
    void reshapeFeatures();
    //! activates the split features tool
    void splitFeatures();
    //! activates the split parts tool
    void splitParts();
    //! activates the add ring tool
    void addRing();
    //! activates the fill ring tool
    void fillRing();
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
    //! Modifies the attributes of selected features via feature form
    void modifyAttributesOfSelectedFeatures();
    //! provides operations with nodes
    void nodeTool();
    //! activates the rotate points tool
    void rotatePointSymbols();
    //! activates the offset point symbol tool
    void offsetPointSymbol();
    //! shows the snapping Options
    void snappingOptions();
    //! activates the tool
    void setMapTool( QgsMapTool *tool, bool clean = false );


    //! activates the rectangle selection tool
    void selectFeatures();

    //! activates the polygon selection tool
    void selectByPolygon();

    //! activates the freehand selection tool
    void selectByFreehand();

    //! activates the radius selection tool
    void selectByRadius();

    //! deselect features from all layers
    void deselectAll();

    //! select all features
    void selectAll();

    //! invert the selection
    void invertSelection();

    //! select features by expression
    void selectByExpression();

    //! select features by form
    void selectByForm();

    //! refresh map canvas
    void refreshMapCanvas();

    //! start "busy" progress bar
    void canvasRefreshStarted();
    //! stop "busy" progress bar
    void canvasRefreshFinished();

    //! Dialog for verification of action on many edits
    bool verifyEditsActionDialog( const QString &act, const QString &upon );

    //! Update gui actions/menus when layers are modified
    void updateLayerModifiedActions();

    //! change layer subset of current vector layer
    void layerSubsetString();

    //! map tool changed
    void mapToolChanged( QgsMapTool *newTool, QgsMapTool *oldTool );

    //! map layers changed
    void showMapCanvas();

    //! change log message icon in statusbar
    void toggleLogMessageIcon( bool hasLogMessage );

    //! Called when some layer's editing mode was toggled on/off
    void layerEditStateChanged();

    //! Update the label toolbar buttons
    void updateLabelToolButtons();

    /**
     * Activates or deactivates actions depending on the current maplayer type.
    Is called from the legend when the current legend item has changed*/
    void activateDeactivateLayerRelatedActions( QgsMapLayer *layer );

    /**
     * \brief Open one or more raster layers and add to the map
     *  Will prompt user for file names using a file selection dialog
     */
    void addRasterLayer();

    void selectionChanged( QgsMapLayer *layer );

    void extentChanged();
    void showRotation();

    void displayMapToolMessage( const QString &message, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO );
    void displayMessage( const QString &title, const QString &message, QgsMessageBar::MessageLevel level );
    void removeMapToolMessage();
    void updateMouseCoordinatePrecision();
    //    void debugHook();
    //! Add a Layer Definition file
    void addLayerDefinition();
    //! Exit Qgis
    void fileExit();
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

    //! Run the default feature action on the current layer
    void doFeatureAction();
    //! Set the default feature action for the current layer
    void updateDefaultFeatureAction( QAction *action );
    //! Refresh the list of feature actions of the current layer
    void refreshFeatureActions();

    //annotations
    void addFormAnnotation();
    void addTextAnnotation();
    void addHtmlAnnotation();
    void addSvgAnnotation();
    void modifyAnnotation();
    void reprojectAnnotations();

    //! Alerts user when labeling font for layer has not been found on system
    void labelingFontNotFound( QgsVectorLayer *vlayer, const QString &fontfamily );

    //! Alerts user when commit errors occurred
    void commitError( QgsVectorLayer *vlayer );

    //! Opens the labeling dialog for a layer when called from labelingFontNotFound alert
    void labelingDialogFontNotFound( QAction *act );

    //! shows label settings dialog (for labeling-ng)
    void labeling();

    //! shows the map styling dock
    void mapStyleDock( bool enabled );

    //! diagrams properties
    void diagramProperties();

    //! set the CAD dock widget visible
    void setCadDockVisible( bool visible );

    void saveAsLayerDefinition();

    //! save current raster layer
    void saveAsRasterFile( QgsRasterLayer *layer = nullptr );

    //! show Python console
    void showPythonDialog();

    //! Shows a warning when an old project file is read.
    void oldProjectVersionWarning( const QString & );

    //! Toggle map tips on/off
    void toggleMapTips( bool enabled );

    //! Show the map tip
    void showMapTip();

    //! Toggle full screen mode
    void toggleFullScreen();

    //! Toggle visibility of opened panels
    void togglePanelsVisibility();

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

    void showStyleManager();

    //! Toggles whether to show pinned labels
    void showPinnedLabels( bool show );
    //! Activates pin labels tool
    void pinLabels();
    //! Activates show/hide labels tool
    void showHideLabels();
    //! Activates the move label tool
    void moveLabel();
    //! Activates rotate feature tool
    void rotateFeature();
    //! Activates rotate label tool
    void rotateLabel();
    //! Activates label property tool
    void changeLabelProperties();

    void renderDecorationItems( QPainter *p );
    void projectReadDecorationItems();

    //! clear out any stuff from project
    void closeProject();

    //! trust and load project macros
    void enableProjectMacros();

    void clipboardChanged();

    //! catch MapCanvas keyPress event so we can check if selected feature collection must be deleted
    void mapCanvas_keyPressed( QKeyEvent *e );

    /**
     * Disable any preview modes shown on the map canvas
     * \since QGIS 2.3 */
    void disablePreviewMode();

    /**
     * Enable a grayscale preview mode on the map canvas
     * \since QGIS 2.3 */
    void activateGrayscalePreview();

    /**
     * Enable a monochrome preview mode on the map canvas
     * \since QGIS 2.3 */
    void activateMonoPreview();

    /**
     * Enable a color blindness (protanope) preview mode on the map canvas
     * \since QGIS 2.3 */
    void activateProtanopePreview();

    /**
     * Enable a color blindness (deuteranope) preview mode on the map canvas
     * \since QGIS 2.3 */
    void activateDeuteranopePreview();

    void toggleFilterLegendByExpression( bool );
    void updateFilterLegend();

    /**
     * Shows the statistical summary dock widget and brings it to the foreground
     */
    void showStatisticsDockWidget();

    //! Pushes a layer error to the message bar
    void onLayerError( const QString &msg );

    //! Set the layer for the map style dock. Doesn't show the style dock
    void setMapStyleDockLayer( QgsMapLayer *layer );

    void annotationCreated( QgsAnnotation *annotation );

    void updateCrsStatusBar();

    //! handle project crs changes
    void projectCrsChanged();

  signals:

    /**
     * Emitted when a connection has been added/removed or changed by the provider
     * selection dialogs
     */
    void connectionsChanged();

    /**
     * Emitted when a key is pressed and we want non widget sublasses to be able
      to pick up on this (e.g. maplayer) */
    void keyPressed( QKeyEvent *e );

    /**
     * Emitted when a project file is successfully read
      \note
      This is useful for plug-ins that store properties with project files.  A
      plug-in can connect to this signal.  When it is emitted, the plug-in
      knows to then check the project properties for any relevant state.
      */
    void projectRead();

    /**
     * Emitted when starting an entirely new project
      \note
      This is similar to projectRead(); plug-ins might want to be notified
      that they're in a new project.  Yes, projectRead() could have been
      overloaded to be used in the case of new projects instead.  However,
      it's probably more semantically correct to have an entirely separate
      signal for when this happens.
      */
    void newProject();

    /**
     * Signal emitted when the current theme is changed so plugins
     * can change there tool button icons. */
    void currentThemeChanged( const QString & );

    /**
     * This signal is emitted when a new layout \a designer has been opened.
     * \since QGIS 3.0
     * \see layoutDesignerWillBeClosed()
     */
    void layoutDesignerOpened( QgsLayoutDesignerInterface *designer );

    /**
     * This signal is emitted before a layout \a designer is going to be closed
     * and deleted.
     * \since QGIS 3.0
     * \see layoutDesignerClosed()
     * \see layoutDesignerOpened()
     */
    void layoutDesignerWillBeClosed( QgsLayoutDesignerInterface *designer );

    /**
     * This signal is emitted after a layout designer window is closed.
     * \since QGIS 3.0
     * \see layoutDesignerWillBeClosed()
     * \see layoutDesignerOpened()
     */
    void layoutDesignerClosed();

    //! This signal is emitted when QGIS' initialization is complete
    void initializationCompleted();

    void customCrsValidation( QgsCoordinateReferenceSystem &crs );

    /**
     * This signal is emitted when a layer has been saved using save as
       \since QGIS 2.7
     */
    void layerSavedAs( QgsMapLayer *l, const QString &path );

  private:
    void startProfile( const QString &name );
    void endProfile();
    void functionProfile( void ( QgisApp::*fnc )(), QgisApp *instance, const QString &name );

    /**
     * This method will open a dialog so the user can select GDAL sublayers to load
     * \returns true if any items were loaded
     */
    bool askUserForZipItemLayers( const QString &path );

    /**
     * This method will open a dialog so the user can select GDAL sublayers to load
     */
    void askUserForGDALSublayers( QgsRasterLayer *layer );

    /**
     * This method will verify if a GDAL layer contains sublayers
     */
    bool shouldAskUserForGDALSublayers( QgsRasterLayer *layer );

    /**
     * This method will open a dialog so the user can select OGR sublayers to load
     */
    void askUserForOGRSublayers( QgsVectorLayer *layer );

    /**
     * Add a raster layer to the map (passed in as a ptr).
     * It won't force a refresh.
     */
    bool addRasterLayer( QgsRasterLayer *rasterLayer );

    //! Open a raster layer - this is the generic function which takes all parameters
    QgsRasterLayer *addRasterLayerPrivate( const QString &uri, const QString &baseName,
                                           const QString &providerKey, bool guiWarning,
                                           bool guiUpdate );

    /**
     * Add this file to the recently opened/saved projects list
     *  pass settings by reference since creating more than one
     * instance simultaneously results in data loss.
     *
     * \param savePreviewImage Set to false when the preview image should not be saved. E.g. project load.
     */
    void saveRecentProjectPath( const QString &projectPath, bool savePreviewImage = true );
    //! Save recent projects list to settings
    void saveRecentProjects();
    //! Update project menu with the current list of recently accessed projects
    void updateRecentProjectPaths();
    //! Read Well Known Binary stream from PostGIS
    //void readWKB(const char *, QStringList tables);
    //! shows the paste-transformations dialog
    // void pasteTransformations();
    //! check to see if file is dirty and if so, prompt the user th save it
    bool saveDirty();
    //! Checks for running tasks dependent on the open project
    bool checkTasksDependOnProject();

    /**
     * Helper function to union several geometries together (used in function mergeSelectedFeatures)
      \returns empty geometry in case of error or if canceled */
    QgsGeometry unionGeometries( const QgsVectorLayer *vl, QgsFeatureList &featureList, bool &canceled );

    //! Deletes all the layout designer windows
    void deleteLayoutDesigners();

    void setupLayoutManagerConnections();

    void setupAtlasMapLayerAction( QgsPrintLayout *layout, bool enableAction );

    void setLayoutAtlasFeature( QgsPrintLayout *layout, QgsMapLayer *layer, const QgsFeature &feat );

    void saveAsVectorFileGeneral( QgsVectorLayer *vlayer = nullptr, bool symbologyOption = true );

    /**
     * Paste features from clipboard into a new memory layer.
     *  If no features are in clipboard an empty layer is returned.
     *  \returns pointer to a new layer or 0 if failed
     */
    QgsVectorLayer *pasteToNewMemoryVector();

    //! Returns all annotation items in the canvas
    QList<QgsMapCanvasAnnotationItem *> annotationItems();

    //! Removes annotation items in the canvas
    void removeAnnotationItems();

    //! Configure layer tree view according to the user options from QgsSettings
    void setupLayerTreeViewFromSettings();

    void readSettings();
    void writeSettings();
    void createActions();
    void createActionGroups();
    void createMenus();
    void createProfileMenu();
    void createToolBars();
    void createStatusBar();
    void setupConnections();
    void initLayerTreeView();
    void createOverview();
    void createCanvasTools();
    void createMapTips();
    void createDecorations();
    void init3D();
    void initNativeProcessing();
    void initLayouts();

    //! Creates a new 3D map dock without initializing its position or contents
    Qgs3DMapCanvasDockWidget *createNew3DMapCanvasDock( const QString &name );

    //! Closes any existing 3D map docks
    void closeAdditional3DMapCanvases();

    /**
     * Refresh the user profile menu.
     */
    void refreshProfileMenu();

    //! Do histogram stretch for singleband gray / multiband color rasters
    void histogramStretch( bool visibleAreaOnly = false, QgsRasterMinMaxOrigin::Limits limits = QgsRasterMinMaxOrigin::MinMax );

    //! Apply raster brightness
    void adjustBrightnessContrast( int delta, bool updateBrightness = true );

    //! Copy a vector style from a layer to another one, if they have the same geometry type
    void duplicateVectorStyle( QgsVectorLayer *srcLayer, QgsVectorLayer *destLayer );

    //! Loads the list of recent projects from settings
    void readRecentProjects();

    /**
     * Applies project map canvas settings to the specified canvas
     */
    void applyProjectSettingsToCanvas( QgsMapCanvas *canvas );

    /**
     * Applies global qgis settings to the specified canvas
     */
    void applyDefaultSettingsToCanvas( QgsMapCanvas *canvas );

    /**
     * Configures positioning of a newly created dock widget.
     * The \a isFloating and \a dockGeometry arguments can be used to specify an initial floating state
     * and widget geometry rect for the dock.
     */
    void setupDockWidget( QDockWidget *dockWidget, bool isFloating = false, const QRect &dockGeometry = QRect(),
                          Qt::DockWidgetArea area = Qt::RightDockWidgetArea );

    /**
     * Reads dock widget's position settings from a DOM element and calls setupDockWidget()
     * \sa writeDockWidgetSettings()
     */
    void readDockWidgetSettings( QDockWidget *dockWidget, const QDomElement &elem );

    /**
     * Writes dock widget's position settings to a DOM element
     * \sa readDockWidgetSettings()
     */
    void writeDockWidgetSettings( QDockWidget *dockWidget, QDomElement &elem );

    QgsCoordinateReferenceSystem defaultCrsForNewLayers() const;

    //! Attempts to choose a reasonable default icon size based on the window's screen DPI
    int chooseReasonableDefaultIconSize() const;

    /**
     * Returns the size of docked toolbars for a given standard (non-docked) toolbar icon size.
     */
    int dockedToolbarIconSize( int standardToolbarIconSize ) const;

    QgisAppStyleSheet *mStyleSheetBuilder = nullptr;

    // actions for menus and toolbars -----------------

#ifdef Q_OS_MAC
    QAction *mActionWindowMinimize = nullptr;
    QAction *mActionWindowZoom = nullptr;
    QAction *mActionWindowSeparator1 = nullptr;
    QAction *mActionWindowAllToFront = nullptr;
    QAction *mActionWindowSeparator2 = nullptr;
    QActionGroup *mWindowActions = nullptr;
#endif

    QAction *mActionPluginSeparator1 = nullptr;
    QAction *mActionPluginSeparator2 = nullptr;
    QAction *mActionRasterSeparator = nullptr;

    // action groups ----------------------------------
    QActionGroup *mMapToolGroup = nullptr;
    QActionGroup *mPreviewGroup = nullptr;

    // menus ------------------------------------------

#ifdef Q_OS_MAC
    QMenu *mWindowMenu = nullptr;
#endif
    QMenu *mPanelMenu = nullptr;
    QMenu *mToolbarMenu = nullptr;

    // docks ------------------------------------------
    QgsDockWidget *mLayerTreeDock = nullptr;
    QgsDockWidget *mLayerOrderDock = nullptr;
    QgsDockWidget *mOverviewDock = nullptr;
    QgsDockWidget *mpGpsDock = nullptr;
    QgsDockWidget *mLogDock = nullptr;

#ifdef Q_OS_MAC
    //! Window menu action to select this window
    QAction *mWindowAction = nullptr;
#endif

    class Tools
    {
      public:

        Tools() = default;

        QgsMapTool *mZoomIn = nullptr;
        QgsMapTool *mZoomOut = nullptr;
        QgsMapTool *mPan = nullptr;
        QgsMapToolIdentifyAction *mIdentify = nullptr;
        QgsMapTool *mFeatureAction = nullptr;
        QgsMapTool *mMeasureDist = nullptr;
        QgsMapTool *mMeasureArea = nullptr;
        QgsMapTool *mMeasureAngle = nullptr;
        QgsMapToolAddFeature *mAddFeature = nullptr;
        QgsMapTool *mCircularStringCurvePoint = nullptr;
        QgsMapTool *mCircularStringRadius = nullptr;
        QgsMapTool *mCircle2Points = nullptr;
        QgsMapTool *mCircle3Points = nullptr;
        QgsMapTool *mCircle3Tangents = nullptr;
        QgsMapTool *mCircle2TangentsPoint = nullptr;
        QgsMapTool *mCircleCenterPoint = nullptr;
        QgsMapTool *mEllipseCenter2Points = nullptr;
        QgsMapTool *mEllipseCenterPoint = nullptr;
        QgsMapTool *mEllipseExtent = nullptr;
        QgsMapTool *mEllipseFoci = nullptr;
        QgsMapTool *mRectangleCenterPoint = nullptr;
        QgsMapTool *mRectangleExtent = nullptr;
        QgsMapTool *mRectangle3Points = nullptr;
        QgsMapTool *mRegularPolygon2Points = nullptr;
        QgsMapTool *mRegularPolygonCenterPoint = nullptr;
        QgsMapTool *mRegularPolygonCenterCorner = nullptr;
        QgsMapTool *mMoveFeature = nullptr;
        QgsMapTool *mMoveFeatureCopy = nullptr;
        QgsMapTool *mOffsetCurve = nullptr;
        QgsMapTool *mReshapeFeatures = nullptr;
        QgsMapTool *mSplitFeatures = nullptr;
        QgsMapTool *mSplitParts = nullptr;
        QgsMapTool *mSelect = nullptr;
        QgsMapTool *mSelectFeatures = nullptr;
        QgsMapTool *mSelectPolygon = nullptr;
        QgsMapTool *mSelectFreehand = nullptr;
        QgsMapTool *mSelectRadius = nullptr;
        QgsMapTool *mVertexAdd = nullptr;
        QgsMapTool *mVertexMove = nullptr;
        QgsMapTool *mVertexDelete = nullptr;
        QgsMapTool *mAddRing = nullptr;
        QgsMapTool *mFillRing = nullptr;
        QgsMapTool *mAddPart = nullptr;
        QgsMapTool *mSimplifyFeature = nullptr;
        QgsMapTool *mDeleteRing = nullptr;
        QgsMapTool *mDeletePart = nullptr;
        QgsMapTool *mNodeTool = nullptr;
        QgsMapTool *mRotatePointSymbolsTool = nullptr;
        QgsMapTool *mOffsetPointSymbolTool = nullptr;
        QgsMapTool *mAnnotation = nullptr;
        QgsMapTool *mFormAnnotation = nullptr;
        QgsMapTool *mHtmlAnnotation = nullptr;
        QgsMapTool *mSvgAnnotation = nullptr;
        QgsMapTool *mTextAnnotation = nullptr;
        QgsMapTool *mPinLabels = nullptr;
        QgsMapTool *mShowHideLabels = nullptr;
        QgsMapTool *mMoveLabel = nullptr;
        QgsMapTool *mRotateFeature = nullptr;
        QgsMapTool *mRotateLabel = nullptr;
        QgsMapTool *mChangeLabelProperties = nullptr;
    } mMapTools;

    QgsMapTool *mNonEditMapTool = nullptr;

    QgsTaskManagerStatusBarWidget *mTaskManagerWidget = nullptr;

    QgsStatusBarScaleWidget *mScaleWidget = nullptr;

    //! zoom widget
    QgsStatusBarMagnifierWidget *mMagnifierWidget = nullptr;

    //! Widget that will live in the statusbar to display and edit coords
    QgsStatusBarCoordinatesWidget *mCoordsEdit = nullptr;

    //! Widget that will live on the statusbar to display "Rotation"
    QLabel *mRotationLabel = nullptr;
    //! Widget that will live in the statusbar to display and edit rotation
    QgsDoubleSpinBox *mRotationEdit = nullptr;
    //! The validator for the mCoordsEdit
    QValidator *mRotationEditValidator = nullptr;
    //! Widget that will live in the statusbar to show progress of operations
    QProgressBar *mProgressBar = nullptr;
    //! Widget used to suppress rendering
    QCheckBox *mRenderSuppressionCBox = nullptr;
    //! Widget in status bar used to show current project CRS
    QLabel *mOnTheFlyProjectionStatusLabel = nullptr;
    //! Widget in status bar used to show status of on the fly projection
    QToolButton *mOnTheFlyProjectionStatusButton = nullptr;
    QToolButton *mMessageButton = nullptr;
    //! Menu that contains the list of actions of the selected vector layer
    QMenu *mFeatureActionMenu = nullptr;
    //! Popup menu
    QMenu *mPopupMenu = nullptr;
    //! Top level database menu
    QMenu *mDatabaseMenu = nullptr;
    //! Top level web menu
    QMenu *mWebMenu = nullptr;

    QMenu *mConfigMenu = nullptr;

    //! Popup menu for the map overview tools
    QMenu *mToolPopupOverviews = nullptr;
    //! Popup menu for the display tools
    QMenu *mToolPopupDisplay = nullptr;
    //! Map canvas
    QgsMapCanvas *mMapCanvas = nullptr;
    //! Overview map canvas
    QgsMapOverviewCanvas *mOverviewCanvas = nullptr;
    //! Table of contents (legend) for the map
    QgsLayerTreeView *mLayerTreeView = nullptr;
    //! Helper class that connects layer tree with map canvas
    QgsLayerTreeMapCanvasBridge *mLayerTreeCanvasBridge = nullptr;
    //! Table of contents (legend) to order layers of the map
    QgsCustomLayerOrderWidget *mMapLayerOrder = nullptr;
    //! Cursor for the overview map
    QCursor *mOverviewMapCursor = nullptr;
    //! Current map window extent in real-world coordinates
    QRect *mMapWindow = nullptr;
    QString mStartupPath;
    //! full path name of the current map file (if it has been saved or loaded)
    QString mFullPathName;

    //! interface to QgisApp for plugins
    QgisAppInterface *mQgisInterface = nullptr;
    friend class QgisAppInterface;

    QSplashScreen *mSplash = nullptr;
    //! list of recently opened/saved project files
    QList<QgsWelcomePageItemsModel::RecentProjectData> mRecentProjects;

    //! Currently open layout designer dialogs
    QSet<QgsLayoutDesignerDialog *> mLayoutDesignerDialogs;

    //! QGIS-internal vector feature clipboard
    QgsClipboard *mInternalClipboard = nullptr;
    //! Flag to indicate how the project properties dialog was summoned
    bool mShowProjectionTab = false;

    /**
     * String containing supporting vector file formats
      Suitable for a QFileDialog file filter.  Build in ctor.
      */
    QString mVectorFileFilter;

    /**
     * String containing supporting raster file formats
      Suitable for a QFileDialog file filter.  Build in ctor.
      */
    QString mRasterFileFilter;

    //! Timer for map tips
    QTimer *mpMapTipsTimer = nullptr;

    //! Point of last mouse position in map coordinates (used with MapTips)
    QgsPointXY mLastMapPosition;

    //! Maptip object
    QgsMapTip *mpMaptip = nullptr;

    //! Flag to indicate if maptips are on or off
    bool mMapTipsVisible = false;

    //! Flag to indicate whether we are in fullscreen mode or not
    bool mFullScreenMode = false;

    //! Flag to indicate that the previous screen mode was 'maximised'
    bool mPrevScreenModeMaximized = false;

    //! Flag to indicate an edits save/rollback for active layer is in progress
    bool mSaveRollbackInProgress = false;

    QgsPythonUtils *mPythonUtils = nullptr;

    static QgisApp *sInstance;

    QgsUndoWidget *mUndoWidget = nullptr;
    QgsDockWidget *mUndoDock = nullptr;

    QgsBrowserDockWidget *mBrowserWidget = nullptr;
    QgsBrowserDockWidget *mBrowserWidget2 = nullptr;

    QgsAdvancedDigitizingDockWidget *mAdvancedDigitizingDockWidget = nullptr;
    QgsStatisticalSummaryDockWidget *mStatisticalSummaryDockWidget = nullptr;
    QgsBookmarks *mBookMarksDockWidget = nullptr;

    //! Data Source Manager
    QgsDataSourceManagerDialog *mDataSourceManagerDialog = nullptr;

    //! snapping widget
    QgsSnappingWidget *mSnappingWidget = nullptr;
    QWidget *mSnappingDialogContainer = nullptr;
    QgsSnappingWidget *mSnappingDialog = nullptr;

    QgsPluginManager *mPluginManager = nullptr;
    QgsUserProfileManager *mUserProfileManager = nullptr;
    QgsDockWidget *mMapStylingDock = nullptr;
    QgsLayerStylingWidget *mMapStyleWidget = nullptr;

    QPointer< QgsLayoutManagerDialog > mLayoutManagerDialog;

    //! Persistent tile scale slider
    QgsTileScaleWidget *mpTileScaleWidget = nullptr;

    QList<QgsDecorationItem *> mDecorationItems;

    //! Persistent GPS toolbox
    QgsGPSInformationWidget *mpGpsWidget = nullptr;

    QgsMessageBarItem *mLastMapToolMessage = nullptr;

    QgsMessageLogViewer *mLogViewer = nullptr;

    //! project changed
    void projectChanged( const QDomDocument & );

    bool cmpByText( QAction *a, QAction *b );

    //! the user has trusted the project macros
    bool mTrustedMacros = false;

    //! a bar to display warnings in a non-blocker manner
    QgsMessageBar *mInfoBar = nullptr;
    QWidget *mMacrosWarn = nullptr;

    //! A tool bar for user input
    QgsUserInputDockWidget *mUserInputDockWidget = nullptr;

    QgsVectorLayerTools *mVectorLayerTools = nullptr;

    //! A class that facilitates tracing of features
    QgsMapCanvasTracer *mTracer = nullptr;

    QAction *mActionFilterLegend = nullptr;
    QAction *mActionStyleDock = nullptr;

    QgsLegendFilterButton *mLegendExpressionFilterButton = nullptr;

    QgsSnappingUtils *mSnappingUtils = nullptr;

    QList<QgsMapLayerConfigWidgetFactory *> mMapLayerPanelFactories;
    QList<QPointer<QgsOptionsWidgetFactory>> mOptionsWidgetFactories;

    QVector<QPointer<QgsCustomDropHandler>> mCustomDropHandlers;
    QVector<QPointer<QgsLayoutCustomDropHandler>> mCustomLayoutDropHandlers;

    QgsLayoutQptDropHandler *mLayoutQptDropHandler = nullptr;

    QDateTime mProjectLastModified;

    QgsWelcomePage *mWelcomePage = nullptr;

    QStackedWidget *mCentralContainer = nullptr;

    QHash< QgsPrintLayout *, QgsMapLayerAction * > mAtlasFeatureActions;

    std::unique_ptr<QgsMapLayerAction> mDuplicateFeatureAction;
    std::unique_ptr<QgsMapLayerAction> mDuplicateFeatureDigitizeAction;

    int mProjOpen = 0;

    bool gestureEvent( QGestureEvent *event );
    void tapAndHoldTriggered( QTapAndHoldGesture *gesture );

    QSystemTrayIcon *mTray = nullptr;

    QgsLocatorWidget *mLocatorWidget = nullptr;

    QgsStatusBar *mStatusBar = nullptr;

    QTime mLastRenderTime;
    double mLastRenderTimeSeconds = 0;
    QTimer mRenderProgressBarTimer;
    QMetaObject::Connection mRenderProgressBarTimerConnection;

    QgsBrowserModel *mBrowserModel = nullptr;

    void setupDuplicateFeaturesAction();

    QgsFeature duplicateFeatures( QgsMapLayer *mlayer, const QgsFeature &feature );
    QgsFeature duplicateFeatureDigitized( QgsMapLayer *mlayer, const QgsFeature &feature );

    friend class TestQgisAppPython;
};

#ifdef ANDROID
#define QGIS_ICON_SIZE 32
#else
#define QGIS_ICON_SIZE 24
#endif

// clazy:excludeall=qstring-allocations

#endif
