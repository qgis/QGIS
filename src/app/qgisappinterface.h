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
#include "qgsapppluginmanagerinterface.h"

class QgisApp;


/** \class QgisAppInterface
 * \brief Interface class to provide access to private methods in QgisApp
 * for use by plugins.
 *
 * Only those functions "exposed" by QgisInterface can be called from within a
 * plugin.
 */
Q_NOWARN_DEPRECATED_PUSH
class APP_EXPORT QgisAppInterface : public QgisInterface
{
    Q_OBJECT

  public:
    /**
     * Constructor.
     * @param qgis Pointer to the QgisApp object
     */
    QgisAppInterface( QgisApp *qgisapp );
    ~QgisAppInterface();

    QgsLegendInterface* legendInterface() OVERRIDE;

    QgsPluginManagerInterface* pluginManagerInterface() OVERRIDE;

    QgsLayerTreeView* layerTreeView() OVERRIDE;

    /* Exposed functions */

    //! Zoom map to full extent
    void zoomFull() OVERRIDE;
    //! Zoom map to previous extent
    void zoomToPrevious() OVERRIDE;
    //! Zoom map to next extent
    void zoomToNext() OVERRIDE;
    //! Zoom to active layer
    void zoomToActiveLayer() OVERRIDE;

    //! Add a vector layer
    QgsVectorLayer* addVectorLayer( QString vectorLayerPath, QString baseName, QString providerKey ) OVERRIDE;
    //! Add a raster layer given its file name
    QgsRasterLayer* addRasterLayer( QString rasterLayerPath, QString baseName ) OVERRIDE;
    //! Add a WMS layer
    QgsRasterLayer* addRasterLayer( const QString& url, const QString& baseName, const QString& providerKey ) OVERRIDE;

    //! Add a project
    bool addProject( QString theProjectName ) OVERRIDE;
    //! Start a new blank project
    void newProject( bool thePromptToSaveFlag = false ) OVERRIDE;

    //! Get pointer to the active layer (layer selected in the legend)
    QgsMapLayer *activeLayer() OVERRIDE;

    //! set the active layer (layer selected in the legend)
    bool setActiveLayer( QgsMapLayer *layer ) OVERRIDE;

    //! Add an icon to the plugins toolbar
    int addToolBarIcon( QAction *qAction ) OVERRIDE;
    /**
     * Add a widget to the plugins toolbar.
     * To remove this widget again, call {@link removeToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addToolBarWidget( QWidget* widget ) OVERRIDE;
    //! Remove an icon (action) from the plugin toolbar
    void removeToolBarIcon( QAction *qAction ) OVERRIDE;
    //! Add an icon to the Raster toolbar
    int addRasterToolBarIcon( QAction *qAction ) OVERRIDE;
    /**
     * Add a widget to the raster toolbar.
     * To remove this widget again, call {@link removeRasterToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addRasterToolBarWidget( QWidget* widget ) OVERRIDE;
    //! Remove an icon (action) from the Raster toolbar
    void removeRasterToolBarIcon( QAction *qAction ) OVERRIDE;
    //! Add an icon to the Vector toolbar
    int addVectorToolBarIcon( QAction *qAction ) OVERRIDE;
    /**
     * Add a widget to the vector toolbar.
     * To remove this widget again, call {@link removeVectorToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addVectorToolBarWidget( QWidget* widget ) OVERRIDE;
    //! Remove an icon (action) from the Vector toolbar
    void removeVectorToolBarIcon( QAction *qAction ) OVERRIDE;
    //! Add an icon to the Database toolbar
    int addDatabaseToolBarIcon( QAction *qAction ) OVERRIDE;
    /**
     * Add a widget to the database toolbar.
     * To remove this widget again, call {@link removeDatabaseToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addDatabaseToolBarWidget( QWidget* widget ) OVERRIDE;
    //! Remove an icon (action) from the Database toolbar
    void removeDatabaseToolBarIcon( QAction *qAction ) OVERRIDE;
    //! Add an icon to the Web toolbar
    int addWebToolBarIcon( QAction *qAction ) OVERRIDE;
    /**
     * Add a widget to the web toolbar.
     * To remove this widget again, call {@link removeWebToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addWebToolBarWidget( QWidget* widget ) OVERRIDE;
    //! Remove an icon (action) from the Web toolbar
    void removeWebToolBarIcon( QAction *qAction ) OVERRIDE;

    //! Add toolbar with specified name
    QToolBar* addToolBar( QString name ) OVERRIDE;

    //! Add a toolbar
    //! @note added in 2.3
    void addToolBar( QToolBar* toolbar, Qt::ToolBarArea area = Qt::TopToolBarArea ) OVERRIDE;

    /** Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to false.
     * @param url URL to open
     * @param useQgisDocDirectory If true, the URL will be formed by concatenating
     * url to the QGIS documentation directory path (<prefix>/share/doc)
     */
#ifndef Q_MOC_RUN
    Q_DECL_DEPRECATED
#endif
    void openURL( QString url, bool useQgisDocDirectory = true ) OVERRIDE;

    /** Return a pointer to the map canvas used by qgisapp */
    QgsMapCanvas * mapCanvas() OVERRIDE;

    /** Gives access to main QgisApp object

        Plugins don't need to know about QgisApp, as we pass it as QWidget,
        it can be used for connecting slots and using as widget's parent
    */
    QWidget * mainWindow() OVERRIDE;

    QgsMessageBar * messageBar() OVERRIDE;

    // ### QGIS 3: return QgsComposer*, not QgsComposerView*
    QList<QgsComposerView*> activeComposers() OVERRIDE;

    // ### QGIS 3: return QgsComposer*, not QgsComposerView*
    /** Create a new composer
     * @param title window title for new composer (one will be generated if empty)
     * @return pointer to composer's view
     * @note new composer window will be shown and activated
     */
    QgsComposerView* createNewComposer( QString title = QString( "" ) ) OVERRIDE;

    // ### QGIS 3: return QgsComposer*, not QgsComposerView*
    /** Duplicate an existing parent composer from composer view
     * @param composerView pointer to existing composer view
     * @param title window title for duplicated composer (one will be generated if empty)
     * @return pointer to duplicate composer's view
     * @note dupicate composer window will be hidden until loaded, then shown and activated
     */
    QgsComposerView* duplicateComposer( QgsComposerView* composerView, QString title = QString( "" ) ) OVERRIDE;

    /** Deletes parent composer of composer view, after closing composer window */
    void deleteComposer( QgsComposerView* composerView ) OVERRIDE;

    /** Return changeable options built from settings and/or defaults */
    QMap<QString, QVariant> defaultStyleSheetOptions() OVERRIDE;

    /** Generate stylesheet
     * @param opts generated default option values, or a changed copy of them */
    void buildStyleSheet( const QMap<QString, QVariant>& opts ) OVERRIDE;

    /** Save changed default option keys/values to user settings */
    void saveStyleSheetOptions( const QMap<QString, QVariant>& opts ) OVERRIDE;

    /** Get reference font for initial qApp (may not be same as QgisApp) */
    QFont defaultStyleSheetFont() OVERRIDE;

    /** Add action to the plugins menu */
    void addPluginToMenu( QString name, QAction* action ) OVERRIDE;
    /** Remove action from the plugins menu */
    void removePluginMenu( QString name, QAction* action ) OVERRIDE;

    /** Add action to the Database menu */
    void addPluginToDatabaseMenu( QString name, QAction* action ) OVERRIDE;
    /** Remove action from the Database menu */
    void removePluginDatabaseMenu( QString name, QAction* action ) OVERRIDE;

    /** Add action to the Raster menu */
    void addPluginToRasterMenu( QString name, QAction* action ) OVERRIDE;
    /** Remove action from the Raster menu */
    void removePluginRasterMenu( QString name, QAction* action ) OVERRIDE;

    /** Add action to the Vector menu */
    void addPluginToVectorMenu( QString name, QAction* action ) OVERRIDE;
    /** Remove action from the Raster menu */
    void removePluginVectorMenu( QString name, QAction* action ) OVERRIDE;

    /** Add action to the Web menu */
    void addPluginToWebMenu( QString name, QAction* action ) OVERRIDE;
    /** Remove action from the Web menu */
    void removePluginWebMenu( QString name, QAction* action ) OVERRIDE;

    /** Add "add layer" action to the layer menu */
    void insertAddLayerAction( QAction *action ) OVERRIDE;
    /** remove "add layer" action from the layer menu */
    void removeAddLayerAction( QAction *action ) OVERRIDE;

    /** Add a dock widget to the main window */
    void addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget ) OVERRIDE;

    /** Remove specified dock widget from main window (doesn't delete it). */
    void removeDockWidget( QDockWidget * dockwidget ) OVERRIDE;

    /** show layer properties dialog for layer
     * @param l layer to show properties table for
     */
    virtual void showLayerProperties( QgsMapLayer *l ) OVERRIDE;

    /** show layer attribute dialog for layer
     * @param l layer to show attribute table for
     */
    virtual void showAttributeTable( QgsVectorLayer *l ) OVERRIDE;

    /** Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window. */
    virtual void addWindow( QAction *action ) OVERRIDE;
    /** Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed. */
    virtual void removeWindow( QAction *action ) OVERRIDE;

    /** Register action to the shortcuts manager so its shortcut can be changed in GUI. */
    virtual bool registerMainWindowAction( QAction* action, QString defaultShortcut ) OVERRIDE;

    /** Unregister a previously registered action. (e.g. when plugin is going to be unloaded. */
    virtual bool unregisterMainWindowAction( QAction* action ) OVERRIDE;

    /** Accessors for inserting items into menus and toolbars.
     * An item can be inserted before any existing action.
     */

    //! Menus
    Q_DECL_DEPRECATED virtual QMenu *fileMenu() OVERRIDE;
    virtual QMenu *projectMenu() OVERRIDE;
    virtual QMenu *editMenu() OVERRIDE;
    virtual QMenu *viewMenu() OVERRIDE;
    virtual QMenu *layerMenu() OVERRIDE;
    virtual QMenu *newLayerMenu() OVERRIDE;
    //! @note added in 2.5
    virtual QMenu *addLayerMenu() OVERRIDE;
    virtual QMenu *settingsMenu() OVERRIDE;
    virtual QMenu *pluginMenu() OVERRIDE;
    virtual QMenu *rasterMenu() OVERRIDE;
    virtual QMenu *vectorMenu() OVERRIDE;
    virtual QMenu *databaseMenu() OVERRIDE;
    virtual QMenu *webMenu() OVERRIDE;
    virtual QMenu *firstRightStandardMenu() OVERRIDE;
    virtual QMenu *windowMenu() OVERRIDE;
    virtual QMenu *helpMenu() OVERRIDE;

    //! ToolBars
    virtual QToolBar *fileToolBar() OVERRIDE;
    virtual QToolBar *layerToolBar() OVERRIDE;
    virtual QToolBar *mapNavToolToolBar() OVERRIDE;
    virtual QToolBar *digitizeToolBar() OVERRIDE;
    virtual QToolBar *advancedDigitizeToolBar() OVERRIDE;
    virtual QToolBar *attributesToolBar() OVERRIDE;
    virtual QToolBar *pluginToolBar() OVERRIDE;
    virtual QToolBar *helpToolBar() OVERRIDE;
    virtual QToolBar *rasterToolBar() OVERRIDE;
    virtual QToolBar *vectorToolBar() OVERRIDE;
    virtual QToolBar *databaseToolBar() OVERRIDE;
    virtual QToolBar *webToolBar() OVERRIDE;

    //! Project menu actions
    virtual QAction *actionNewProject() OVERRIDE;
    virtual QAction *actionOpenProject() OVERRIDE;
    virtual QAction *actionSaveProject() OVERRIDE;
    virtual QAction *actionSaveProjectAs() OVERRIDE;
    virtual QAction *actionSaveMapAsImage() OVERRIDE;
    virtual QAction *actionProjectProperties() OVERRIDE;
    virtual QAction *actionPrintComposer() OVERRIDE;
    virtual QAction *actionShowComposerManager() OVERRIDE;
    virtual QAction *actionExit() OVERRIDE;

    //! Edit menu actions
    virtual QAction *actionCutFeatures() OVERRIDE;
    virtual QAction *actionCopyFeatures() OVERRIDE;
    virtual QAction *actionPasteFeatures() OVERRIDE;
    virtual QAction *actionAddFeature() OVERRIDE;
    virtual QAction *actionDeleteSelected() OVERRIDE;
    virtual QAction *actionMoveFeature() OVERRIDE;
    virtual QAction *actionSplitFeatures() OVERRIDE;
    virtual QAction *actionSplitParts() OVERRIDE;
    virtual QAction *actionAddRing() OVERRIDE;
    virtual QAction *actionAddPart() OVERRIDE;
    virtual QAction *actionSimplifyFeature() OVERRIDE;
    virtual QAction *actionDeleteRing() OVERRIDE;
    virtual QAction *actionDeletePart() OVERRIDE;
    virtual QAction *actionNodeTool() OVERRIDE;

    //! View menu actions
    virtual QAction *actionPan() OVERRIDE;
    virtual QAction *actionTouch() OVERRIDE;
    virtual QAction *actionPanToSelected() OVERRIDE;
    virtual QAction *actionZoomIn() OVERRIDE;
    virtual QAction *actionZoomOut() OVERRIDE;
    virtual QAction *actionSelect() OVERRIDE;
    virtual QAction *actionSelectRectangle() OVERRIDE;
    virtual QAction *actionSelectPolygon() OVERRIDE;
    virtual QAction *actionSelectFreehand() OVERRIDE;
    virtual QAction *actionSelectRadius() OVERRIDE;
    virtual QAction *actionIdentify() OVERRIDE;
    virtual QAction *actionFeatureAction() OVERRIDE;
    virtual QAction *actionMeasure() OVERRIDE;
    virtual QAction *actionMeasureArea() OVERRIDE;
    virtual QAction *actionZoomFullExtent() OVERRIDE;
    virtual QAction *actionZoomToLayer() OVERRIDE;
    virtual QAction *actionZoomToSelected() OVERRIDE;
    virtual QAction *actionZoomLast() OVERRIDE;
    virtual QAction *actionZoomNext() OVERRIDE;
    virtual QAction *actionZoomActualSize() OVERRIDE;
    virtual QAction *actionMapTips() OVERRIDE;
    virtual QAction *actionNewBookmark() OVERRIDE;
    virtual QAction *actionShowBookmarks() OVERRIDE;
    virtual QAction *actionDraw() OVERRIDE;

    //! Layer menu actions
    virtual QAction *actionNewVectorLayer() OVERRIDE;
    virtual QAction *actionAddOgrLayer() OVERRIDE;
    virtual QAction *actionAddRasterLayer() OVERRIDE;
    virtual QAction *actionAddPgLayer() OVERRIDE;
    virtual QAction *actionAddWmsLayer() OVERRIDE;
    virtual QAction *actionCopyLayerStyle() OVERRIDE;
    virtual QAction *actionPasteLayerStyle() OVERRIDE;
    virtual QAction *actionOpenTable() OVERRIDE;
    virtual QAction *actionOpenFieldCalculator() OVERRIDE;
    virtual QAction *actionToggleEditing() OVERRIDE;
    virtual QAction *actionSaveActiveLayerEdits() OVERRIDE;
    virtual QAction *actionAllEdits() OVERRIDE;
    virtual QAction *actionSaveEdits() OVERRIDE;
    virtual QAction *actionSaveAllEdits() OVERRIDE;
    virtual QAction *actionRollbackEdits() OVERRIDE;
    virtual QAction *actionRollbackAllEdits() OVERRIDE;
    virtual QAction *actionCancelEdits() OVERRIDE;
    virtual QAction *actionCancelAllEdits() OVERRIDE;
    virtual QAction *actionLayerSaveAs() OVERRIDE;
    virtual QAction *actionLayerSelectionSaveAs() OVERRIDE;
    virtual QAction *actionRemoveLayer() OVERRIDE;
    virtual QAction *actionDuplicateLayer() OVERRIDE;
    virtual QAction *actionLayerProperties() OVERRIDE;
    virtual QAction *actionAddToOverview() OVERRIDE;
    virtual QAction *actionAddAllToOverview() OVERRIDE;
    virtual QAction *actionRemoveAllFromOverview() OVERRIDE;
    virtual QAction *actionHideAllLayers() OVERRIDE;
    virtual QAction *actionShowAllLayers() OVERRIDE;
    virtual QAction *actionHideSelectedLayers() OVERRIDE;
    virtual QAction *actionShowSelectedLayers() OVERRIDE;

    //! Plugin menu actions
    virtual QAction *actionManagePlugins() OVERRIDE;
    virtual QAction *actionPluginListSeparator() OVERRIDE;
    virtual QAction *actionShowPythonDialog() OVERRIDE;

    //! Settings menu actions
    virtual QAction *actionToggleFullScreen() OVERRIDE;
    virtual QAction *actionOptions() OVERRIDE;
    virtual QAction *actionCustomProjection() OVERRIDE;

    //! Help menu actions
    virtual QAction *actionHelpContents() OVERRIDE;
    virtual QAction *actionQgisHomePage() OVERRIDE;
    virtual QAction *actionCheckQgisVersion() OVERRIDE;
    virtual QAction *actionAbout() OVERRIDE;

    /**
     * Open feature form
     * returns true when dialog was accepted (if shown modal, true otherwise)
     * @param l vector layer
     * @param f feature to show/modify
     * @param updateFeatureOnly only update the feature update (don't change any attributes of the layer) [UNUSED]
     * @param showModal if true, will wait for the dialog to be executed (only shown otherwise)
     */
    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false, bool showModal = true ) OVERRIDE;

    /**
     * Returns a feature form for a given feature
     *
     * @param layer   The layer for which the dialog will be created
     * @param feature The feature for which the dialog will be created
     *
     * @return A feature form
     */
    virtual QgsAttributeDialog* getFeatureForm( QgsVectorLayer *layer, QgsFeature &feature ) OVERRIDE;

    /**
     * Access the vector layer tools instance.
     * With the help of this you can access methods like addFeature, startEditing
     * or stopEditing while giving the user the appropriate dialogs.
     *
     * @return An instance of the vector layer tools
     */
    virtual QgsVectorLayerTools* vectorLayerTools() OVERRIDE;

    /** This method is only needed when using a UI form with a custom widget plugin and calling
     * openFeatureForm or getFeatureForm from Python (PyQt4) and you havn't used the info tool first.
     * Python will crash bringing QGIS wtih it
     * if the custom form is not loaded from a C++ method call.
     *
     * This method uses a QTimer to call QUiLoader in order to load the form via C++
     * you only need to call this once after that you can call openFeatureForm/getFeatureForm
     * like normal
     *
     * More information here: http://qt-project.org/forums/viewthread/27098/
     */
    virtual void preloadForm( QString uifile ) OVERRIDE;

    /** Return vector layers in edit mode
     * @param modified whether to return only layers that have been modified
     * @returns list of layers in legend order, or empty list
     */
    virtual QList<QgsMapLayer *> editableLayers( bool modified = false ) const OVERRIDE;

    /** Get timeout for timed messages: default of 5 seconds */
    virtual int messageTimeout() OVERRIDE;

  signals:
    void currentThemeChanged( QString );

  private slots:

    void cacheloadForm( QString uifile );

  private:

    /// QgisInterface aren't copied
    QgisAppInterface( QgisAppInterface const & );

    /// QgisInterface aren't copied
    QgisAppInterface & operator=( QgisAppInterface const & );

    //! Pointer to the QgisApp object
    QgisApp *qgis;

    QTimer *mTimer;

    //! Pointer to the LegendInterface object
    QgsAppLegendInterface legendIface;

    //! Pointer to the PluginManagerInterface object
    QgsAppPluginManagerInterface pluginManagerIface;
};
Q_NOWARN_DEPRECATED_POP

#endif //#define QGISAPPINTERFACE_H
