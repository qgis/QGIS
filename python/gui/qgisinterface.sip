
/** 
 * \class QgisInterface
 * \brief Abstract base class defining interfaces exposed by QgisApp and
 * made available to plugins.
 *
 * Only functionality exposed by QgisInterface can be used in plugins.
 * This interface has to be implemented with application specific details.
 *
 * QGIS implements it in QgisAppInterface class, 3rd party applications
 * could provide their own implementation to be able to use plugins.
 */

class QgisInterface : QObject
{
%TypeHeaderCode
#include <qgisinterface.h>
%End

  public:

    /** Constructor */
    QgisInterface();

    /** Virtual destructor */
    virtual ~QgisInterface();
    
    /** Get pointer to legend interface
      \note added in 1.4
     */
    virtual QgsLegendInterface* legendInterface()=0;

  public slots: // TODO: do these functions really need to be slots?

    //! Zoom to full extent of map layers
    virtual void zoomFull()=0;
    //! Zoom to previous view extent
    virtual void zoomToPrevious()=0;
    //! Zoom to next view extent
    virtual void zoomToNext()=0;
    //! Zoome to extent of the active layer
    virtual void zoomToActiveLayer()=0;

    //! Add a vector layer
    virtual QgsVectorLayer* addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)=0;
    //! Add a raster layer given a raster layer file name
    virtual QgsRasterLayer* addRasterLayer(QString rasterLayerPath, QString baseName = QString())=0;
    //! Add a WMS layer
    virtual QgsRasterLayer* addRasterLayer(const QString& url, const QString& layerName, const QString& providerKey, const QStringList& layers, const QStringList& styles, const QString& format, const QString& crs) = 0;

    //! Add a project
    virtual bool addProject(QString theProject)=0; 
    //! Start a blank project
    virtual void newProject(bool thePromptToSaveFlag=false)=0; 

    //! Get pointer to the active layer (layer selected in the legend)
    virtual QgsMapLayer *activeLayer()=0;

    //! Get pointer to the active layer (layer selected in the legend)
    //! added in 1.4
    virtual bool setActiveLayer( QgsMapLayer * )=0;

    //! Add an icon to the plugins toolbar
    virtual int addToolBarIcon(QAction *qAction) =0;
    //! Remove an action (icon) from the plugin toolbar
    virtual void removeToolBarIcon(QAction *qAction) = 0;
    //! Add toolbar with specified name
    virtual QToolBar* addToolBar(QString name)=0 /Factory/;

    // TODO: is this deprecated in favour of QgsContextHelp?
    /** Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to false.
     * @param url URL to open
     * @param useQgisDocDirectory If true, the URL will be formed by concatenating 
     * url to the QGIS documentation directory path (<prefix>/share/doc)
     */
    virtual void openURL(QString url, bool useQgisDocDirectory=true)=0;

    /** Return a pointer to the map canvas */
    virtual QgsMapCanvas * mapCanvas()=0;

    /** Return a pointer to the main window (instance of QgisApp in case of QGIS) */
    virtual QWidget * mainWindow()=0;

    /** Return pointers to the composer views of the running instance (currently only one)*/
    //virtual QList<QgsComposerView*> composerViews()=0;

    /**Return mainwindows / composer views of running composer instances (currently only one)*/
    virtual QList<QgsComposerView*> activeComposers() = 0;

    /** Add action to the plugins menu */
    virtual void addPluginToMenu(QString name, QAction* action)=0;
    /** Remove action from the plugins menu */
    virtual void removePluginMenu(QString name, QAction* action)=0;

    /** Add action to the Database menu
       @note added in version 1.7 */
    virtual void addPluginToDatabaseMenu(QString name, QAction* action)=0;
    /** Remove action from the Database menu */
    virtual void removePluginDatabaseMenu(QString name, QAction* action)=0;

    /** Add a dock widget to the main window
       @note added in version 1.7 */
    virtual void addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget )=0;

    /** Remove specified dock widget from main window (doesn't delete it). Added in QGIS 1.1. */
    virtual void removeDockWidget ( QDockWidget * dockwidget )=0;

    /** refresh legend of a layer
     \note deprecated - use QgsLegendInterface::refreshLayerSymbology()
     */
    virtual void refreshLegend( QgsMapLayer * layer )=0;

    /** open layer properties
      \note added in 1.5
     */
    virtual void showLayerProperties( QgsMapLayer * layer )=0;

    /** Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window. */
    virtual void addWindow( QAction *action ) = 0;
    /** Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed. */
    virtual void removeWindow( QAction *action ) = 0;

    /** Register action to the shortcuts manager so its shortcut can be changed in GUI
       \note added in 1.2
    */
    virtual bool registerMainWindowAction( QAction* action, QString defaultShortcut ) = 0;

    /** Unregister a previously registered action. (e.g. when plugin is going to be unloaded)
       \note added in 1.2
    */
    virtual bool unregisterMainWindowAction( QAction* action ) = 0;

    /** Accessors for inserting items into menus and toolbars.
     * An item can be inserted before any existing action.
     */

    //! Menus
    virtual QMenu *fileMenu() = 0;
    virtual QMenu *editMenu() = 0;
    virtual QMenu *viewMenu() = 0;
    virtual QMenu *layerMenu() = 0;
    virtual QMenu *settingsMenu() = 0;
    virtual QMenu *pluginMenu() = 0;
    virtual QMenu *firstRightStandardMenu() = 0;
    virtual QMenu *windowMenu() = 0;
    virtual QMenu *helpMenu() = 0;

    //! ToolBars
    virtual QToolBar *fileToolBar() = 0;
    virtual QToolBar *layerToolBar() = 0;
    virtual QToolBar *mapNavToolToolBar() = 0;
    virtual QToolBar *digitizeToolBar() = 0;
    virtual QToolBar *advancedDigitizeToolBar() = 0; // added in v1.5
    virtual QToolBar *attributesToolBar() = 0;
    virtual QToolBar *pluginToolBar() = 0;
    virtual QToolBar *helpToolBar() = 0;

    //! File menu actions
    virtual QAction *actionNewProject() = 0;
    virtual QAction *actionOpenProject() = 0;
    virtual QAction *actionFileSeparator1() = 0;
    virtual QAction *actionSaveProject() = 0;
    virtual QAction *actionSaveProjectAs() = 0;
    virtual QAction *actionSaveMapAsImage() = 0;
    virtual QAction *actionFileSeparator2() = 0;
    virtual QAction *actionProjectProperties() = 0;
    virtual QAction *actionFileSeparator3() = 0;
    virtual QAction *actionPrintComposer() = 0;
    virtual QAction *actionFileSeparator4() = 0;
    virtual QAction *actionExit() = 0;

    //! Edit menu actions
    virtual QAction *actionCutFeatures() = 0;
    virtual QAction *actionCopyFeatures() = 0;
    virtual QAction *actionPasteFeatures() = 0;
    virtual QAction *actionEditSeparator1() = 0;
    virtual QAction *actionCapturePoint() = 0;
    virtual QAction *actionCaptureLine() = 0;
    virtual QAction *actionCapturePolygon() = 0;
    virtual QAction *actionDeleteSelected() = 0;
    virtual QAction *actionMoveFeature() = 0;
    virtual QAction *actionSplitFeatures() = 0;
    virtual QAction *actionAddVertex() = 0;
    virtual QAction *actionDeleteVertex() = 0;
    virtual QAction *actionMoveVertex() = 0;
    virtual QAction *actionAddRing() = 0;
    virtual QAction *actionAddIsland() = 0;
    virtual QAction *actionSimplifyFeature() = 0;
    virtual QAction *actionDeleteRing() = 0;
    virtual QAction *actionDeletePart() = 0;
    virtual QAction *actionNodeTool() = 0;
    virtual QAction *actionEditSeparator2() = 0;

    //! View menu actions
    virtual QAction *actionPan() = 0;
    virtual QAction *actionZoomIn() = 0;
    virtual QAction *actionZoomOut() = 0;
    virtual QAction *actionSelect() = 0;
    virtual QAction *actionSelectRectangle() = 0;
    virtual QAction *actionSelectPolygon() = 0;
    virtual QAction *actionSelectFreehand() = 0;
    virtual QAction *actionSelectRadius() = 0;
    virtual QAction *actionIdentify() = 0;
    virtual QAction *actionMeasure() = 0;
    virtual QAction *actionMeasureArea() = 0;
    virtual QAction *actionViewSeparator1() = 0;
    virtual QAction *actionZoomFullExtent() = 0;
    virtual QAction *actionZoomToLayer() = 0;
    virtual QAction *actionZoomToSelected() = 0;
    virtual QAction *actionZoomLast() = 0;
    virtual QAction *actionZoomActualSize() = 0;
    virtual QAction *actionViewSeparator2() = 0;
    virtual QAction *actionMapTips() = 0;
    virtual QAction *actionNewBookmark() = 0;
    virtual QAction *actionShowBookmarks() = 0;
    virtual QAction *actionDraw() = 0;
    virtual QAction *actionViewSeparator3() = 0;

    //! Layer menu actions
    virtual QAction *actionNewVectorLayer() = 0;
    virtual QAction *actionAddOgrLayer() = 0;
    virtual QAction *actionAddRasterLayer() = 0;
    virtual QAction *actionAddPgLayer() = 0;
    virtual QAction *actionAddWmsLayer() = 0;
    virtual QAction *actionLayerSeparator1() = 0;
    virtual QAction *actionOpenTable() = 0;
    virtual QAction *actionToggleEditing() = 0;
    virtual QAction *actionLayerSaveAs() = 0;
    virtual QAction *actionLayerSelectionSaveAs() = 0;
    virtual QAction *actionRemoveLayer() = 0;
    virtual QAction *actionLayerProperties() = 0;
    virtual QAction *actionLayerSeparator2() = 0;
    virtual QAction *actionAddToOverview() = 0;
    virtual QAction *actionAddAllToOverview() = 0;
    virtual QAction *actionRemoveAllFromOverview() = 0;
    virtual QAction *actionLayerSeparator3() = 0;
    virtual QAction *actionHideAllLayers() = 0;
    virtual QAction *actionShowAllLayers() = 0;

    //! Plugin menu actions
    virtual QAction *actionManagePlugins() = 0;
    virtual QAction *actionPluginSeparator1() = 0;
    virtual QAction *actionPluginListSeparator() = 0;
    virtual QAction *actionPluginSeparator2() = 0;
    virtual QAction *actionPluginPythonSeparator() = 0;
    virtual QAction *actionShowPythonDialog() = 0;

    //! Settings menu actions
    virtual QAction *actionToggleFullScreen() = 0;
    virtual QAction *actionSettingsSeparator1() = 0;
    virtual QAction *actionOptions() = 0;
    virtual QAction *actionCustomProjection() = 0;

    //! Help menu actions
    virtual QAction *actionHelpContents() = 0;
    virtual QAction *actionHelpSeparator1() = 0;
    virtual QAction *actionQgisHomePage() = 0;
    virtual QAction *actionCheckQgisVersion() = 0;
    virtual QAction *actionHelpSeparator2() = 0;
    virtual QAction *actionAbout() = 0;

    //! open feature form
    // @added in 1.6
    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false ) = 0;

  signals:
    /** Emited whenever current (selected) layer changes.
     *  The pointer to layer can be null if no layer is selected
     */
    void currentLayerChanged ( QgsMapLayer * layer );

    /**This signal is emitted when a new composer instance has been created
       @note added in version 1.4*/
    void composerAdded( QgsComposerView* v );

    /**This signal is emitted before a new composer instance is going to be removed
       @note added in version 1.4*/
    void composerWillBeRemoved( QgsComposerView* v );

    /**This signal is emitted when QGIS' initialization is complete 
       @note added in version 1.6*/ 
    void initializationCompleted(); 

    /** emitted when a project file is successfully read
        @note
        This is useful for plug-ins that store properties with project files.  A
        plug-in can connect to this signal.  When it is emitted, the plug-in
        knows to then check the project properties for any relevant state.

        Added in v1.6
     */
    void projectRead();
    /** emitted when starting an entirely new project
        @note
        This is similar to projectRead(); plug-ins might want to be notified
        that they're in a new project.  Yes, projectRead() could have been
        overloaded to be used in the case of new projects instead.  However,
        it's probably more semantically correct to have an entirely separate
        signal for when this happens.

        Added in v1.6
      */
     void newProjectCreated();
};

