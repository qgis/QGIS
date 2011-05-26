/***************************************************************************
                          qgisappinterface.h
 Interface class for exposing functions in QgisApp for use by plugins
                             -------------------
  begin                : 2004-02-11
  copyright            : (C) 2004 by Gary E.Sherman
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
#ifndef QGISIFACE_H
#define QGISIFACE_H

#include "qgisinterface.h"
#include "qgsapplegendinterface.h"

class QgisApp;

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4996 )  // declared deprecated
#endif

/** \class QgisAppInterface
 * \brief Interface class to provide access to private methods in QgisApp
 * for use by plugins.
 *
 * Only those functions "exposed" by QgisInterface can be called from within a
 * plugin.
 */
class QgisAppInterface : public QgisInterface
{
    Q_OBJECT

  public:
    /**
     * Constructor.
     * @param qgis Pointer to the QgisApp object
     */
    QgisAppInterface( QgisApp *qgisapp );
    ~QgisAppInterface();

    QgsLegendInterface* legendInterface();

    /* Exposed functions */
    //! Zoom map to full extent
    void zoomFull();
    //! Zoom map to previous extent
    void zoomToPrevious();
    //! Zoom map to next extent
    void zoomToNext();
    //! Zoom to active layer
    void zoomToActiveLayer();

    //! Add a vector layer
    QgsVectorLayer* addVectorLayer( QString vectorLayerPath, QString baseName, QString providerKey );
    //! Add a raster layer given its file name
    QgsRasterLayer* addRasterLayer( QString rasterLayerPath, QString baseName );
    //! Add a WMS layer
    QgsRasterLayer* addRasterLayer( const QString& url, const QString& baseName, const QString& providerKey,
                                    const QStringList& layers, const QStringList& styles, const QString& format, const QString& crs );

    //! Add a project
    bool addProject( QString theProjectName );
    //! Start a new blank project
    void newProject( bool thePromptToSaveFlag = false );

    //! Get pointer to the active layer (layer selected in the legend)
    QgsMapLayer *activeLayer();

    //! set the active layer (layer selected in the legend)
    bool setActiveLayer( QgsMapLayer *layer );

    //! Add an icon to the plugins toolbar
    int addToolBarIcon( QAction *qAction );
    //! Remove an icon (action) from the plugin toolbar
    void removeToolBarIcon( QAction *qAction );
    //! Add toolbar with specified name
    QToolBar* addToolBar( QString name );

    /** Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to false.
     * @param url URL to open
     * @param useQgisDocDirectory If true, the URL will be formed by concatenating
     * url to the QGIS documentation directory path (<prefix>/share/doc)
     */
    void openURL( QString url, bool useQgisDocDirectory = true );

    /** Return a pointer to the map canvas used by qgisapp */
    QgsMapCanvas * mapCanvas();

    /** Gives access to main QgisApp object

        Plugins don't need to know about QgisApp, as we pass it as QWidget,
        it can be used for connecting slots and using as widget's parent
    */
    QWidget * mainWindow();

    QList<QgsComposerView*> activeComposers();

    /** Add action to the plugins menu */
    void addPluginToMenu( QString name, QAction* action );
    /** Remove action from the plugins menu */
    void removePluginMenu( QString name, QAction* action );

    /** Add action to the Database menu */
    void addPluginToDatabaseMenu( QString name, QAction* action );
    /** Remove action from the Database menu */
    void removePluginDatabaseMenu( QString name, QAction* action );

    /** Add "add layer" action to the layer menu */
    void insertAddLayerAction( QAction *action );
    /** remove "add layer" action from the layer menu */
    void removeAddLayerAction( QAction *action );

    /** Add a dock widget to the main window */
    void addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget );

    /** Remove specified dock widget from main window (doesn't delete it). Added in QGIS 1.1. */
    void removeDockWidget( QDockWidget * dockwidget );

    virtual void refreshLegend( QgsMapLayer *l );

    /** show layer properties dialog for layer
     * @param l layer to show properties table for
     * @note added in added in 1.5
     */
    virtual void showLayerProperties( QgsMapLayer *l );

    /** show layer attribute dialog for layer
     * @param l layer to show attribute table for
     * @note added in added in 1.7
     */
    virtual void showAttributeTable( QgsVectorLayer *l );

    /** Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window. */
    virtual void addWindow( QAction *action );
    /** Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed. */
    virtual void removeWindow( QAction *action );

    /** Register action to the shortcuts manager so its shortcut can be changed in GUI.
      @note added in version 1.2. */
    virtual bool registerMainWindowAction( QAction* action, QString defaultShortcut );

    /** Unregister a previously registered action. (e.g. when plugin is going to be unloaded.
      @note added in version 1.2. */
    virtual bool unregisterMainWindowAction( QAction* action );

    /** Accessors for inserting items into menus and toolbars.
     * An item can be inserted before any existing action.
     */

    //! Menus
    virtual QMenu *fileMenu();
    virtual QMenu *editMenu();
    virtual QMenu *viewMenu();
    virtual QMenu *layerMenu();
    virtual QMenu *settingsMenu();
    virtual QMenu *pluginMenu();
    virtual QMenu *databaseMenu();
    virtual QMenu *firstRightStandardMenu();
    virtual QMenu *windowMenu();
    virtual QMenu *helpMenu();

    //! ToolBars
    virtual QToolBar *fileToolBar();
    virtual QToolBar *layerToolBar();
    virtual QToolBar *mapNavToolToolBar();
    virtual QToolBar *digitizeToolBar();
    virtual QToolBar *advancedDigitizeToolBar();
    virtual QToolBar *attributesToolBar();
    virtual QToolBar *pluginToolBar();
    virtual QToolBar *helpToolBar();
    virtual QToolBar *rasterToolBar();

    //! File menu actions
    virtual QAction *actionNewProject();
    virtual QAction *actionOpenProject();
    virtual QAction *actionFileSeparator1();
    virtual QAction *actionSaveProject();
    virtual QAction *actionSaveProjectAs();
    virtual QAction *actionSaveMapAsImage();
    virtual QAction *actionFileSeparator2();
    virtual QAction *actionProjectProperties();
    virtual QAction *actionFileSeparator3();
    virtual QAction *actionPrintComposer();
    virtual QAction *actionFileSeparator4();
    virtual QAction *actionExit();

    //! Edit menu actions
    virtual QAction *actionCutFeatures();
    virtual QAction *actionCopyFeatures();
    virtual QAction *actionPasteFeatures();
    virtual QAction *actionEditSeparator1();
    virtual QAction *actionAddFeature();
    Q_DECL_DEPRECATED virtual QAction *actionCapturePoint();
    Q_DECL_DEPRECATED virtual QAction *actionCaptureLine();
    Q_DECL_DEPRECATED virtual QAction *actionCapturePolygon();
    virtual QAction *actionDeleteSelected();
    virtual QAction *actionMoveFeature();
    virtual QAction *actionSplitFeatures();
    virtual QAction *actionAddVertex();
    virtual QAction *actionDeleteVertex();
    virtual QAction *actionMoveVertex();
    virtual QAction *actionAddRing();
    virtual QAction *actionAddPart();
    Q_DECL_DEPRECATED virtual QAction *actionAddIsland();
    virtual QAction *actionSimplifyFeature();
    virtual QAction *actionDeleteRing();
    virtual QAction *actionDeletePart();
    virtual QAction *actionNodeTool();
    virtual QAction *actionEditSeparator2();

    //! View menu actions
    virtual QAction *actionPan();
    virtual QAction *actionZoomIn();
    virtual QAction *actionZoomOut();
    virtual QAction *actionSelect();
    virtual QAction *actionSelectRectangle();
    virtual QAction *actionSelectPolygon();
    virtual QAction *actionSelectFreehand();
    virtual QAction *actionSelectRadius();
    virtual QAction *actionIdentify();
    virtual QAction *actionMeasure();
    virtual QAction *actionMeasureArea();
    virtual QAction *actionViewSeparator1();
    virtual QAction *actionZoomFullExtent();
    virtual QAction *actionZoomToLayer();
    virtual QAction *actionZoomToSelected();
    virtual QAction *actionZoomLast();
    virtual QAction *actionZoomNext();
    virtual QAction *actionZoomActualSize();
    virtual QAction *actionViewSeparator2();
    virtual QAction *actionMapTips();
    virtual QAction *actionNewBookmark();
    virtual QAction *actionShowBookmarks();
    virtual QAction *actionDraw();
    virtual QAction *actionViewSeparator3();

    //! Layer menu actions
    virtual QAction *actionNewVectorLayer();
    virtual QAction *actionAddOgrLayer();
    virtual QAction *actionAddRasterLayer();
    virtual QAction *actionAddPgLayer();
    virtual QAction *actionAddWmsLayer();
    virtual QAction *actionLayerSeparator1();
    virtual QAction *actionOpenTable();
    virtual QAction *actionToggleEditing();
    virtual QAction *actionLayerSaveAs();
    virtual QAction *actionLayerSelectionSaveAs();
    virtual QAction *actionRemoveLayer();
    virtual QAction *actionLayerProperties();
    virtual QAction *actionLayerSeparator2();
    virtual QAction *actionAddToOverview();
    virtual QAction *actionAddAllToOverview();
    virtual QAction *actionRemoveAllFromOverview();
    virtual QAction *actionLayerSeparator3();
    virtual QAction *actionHideAllLayers();
    virtual QAction *actionShowAllLayers();

    //! Plugin menu actions
    virtual QAction *actionManagePlugins();
    virtual QAction *actionPluginSeparator1();
    virtual QAction *actionPluginListSeparator();
    virtual QAction *actionPluginSeparator2();
    virtual QAction *actionPluginPythonSeparator();
    virtual QAction *actionShowPythonDialog();

    //! Settings menu actions
    virtual QAction *actionToggleFullScreen();
    virtual QAction *actionSettingsSeparator1();
    virtual QAction *actionOptions();
    virtual QAction *actionCustomProjection();

    //! Help menu actions
    virtual QAction *actionHelpContents();
    virtual QAction *actionHelpSeparator1();
    virtual QAction *actionQgisHomePage();
    virtual QAction *actionCheckQgisVersion();
    virtual QAction *actionHelpSeparator2();
    virtual QAction *actionAbout();

    //! open feature form
    // returns true when dialog was accepted
    // @param l vector layer
    // @param f feature to show/modify
    // @param updateFeatureOnly only update the feature update (don't change any attributes of the layer)
    // @added in 1.6
    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false );

  signals:
    void currentThemeChanged( QString );

  private:

    /// QgisInterface aren't copied
    QgisAppInterface( QgisAppInterface const & );

    /// QgisInterface aren't copied
    QgisAppInterface & operator=( QgisAppInterface const & );

    //! Pointer to the QgisApp object
    QgisApp *qgis;

    //! Pointer to the LegendInterface object
    QgsAppLegendInterface legendIface;
};

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif //#define QGISAPPINTERFACE_H
