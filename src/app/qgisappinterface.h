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

    QgsLegendInterface* legendInterface() override;

    QgsPluginManagerInterface* pluginManagerInterface() override;

    QgsLayerTreeView* layerTreeView() override;

    /* Exposed functions */

    //! Zoom map to full extent
    void zoomFull() override;
    //! Zoom map to previous extent
    void zoomToPrevious() override;
    //! Zoom map to next extent
    void zoomToNext() override;
    //! Zoom to active layer
    void zoomToActiveLayer() override;

    //! Add a vector layer
    QgsVectorLayer* addVectorLayer( QString vectorLayerPath, QString baseName, QString providerKey ) override;
    //! Add a raster layer given its file name
    QgsRasterLayer* addRasterLayer( QString rasterLayerPath, QString baseName ) override;
    //! Add a WMS layer
    QgsRasterLayer* addRasterLayer( const QString& url, const QString& baseName, const QString& providerKey ) override;

    //! Add a project
    bool addProject( QString theProjectName ) override;
    //! Start a new blank project
    void newProject( bool thePromptToSaveFlag = false ) override;

    //! Get pointer to the active layer (layer selected in the legend)
    QgsMapLayer *activeLayer() override;

    //! set the active layer (layer selected in the legend)
    bool setActiveLayer( QgsMapLayer *layer ) override;

    //! Add an icon to the plugins toolbar
    int addToolBarIcon( QAction *qAction ) override;
    /**
     * Add a widget to the plugins toolbar.
     * To remove this widget again, call {@link removeToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addToolBarWidget( QWidget* widget ) override;
    //! Remove an icon (action) from the plugin toolbar
    void removeToolBarIcon( QAction *qAction ) override;
    //! Add an icon to the Raster toolbar
    int addRasterToolBarIcon( QAction *qAction ) override;
    /**
     * Add a widget to the raster toolbar.
     * To remove this widget again, call {@link removeRasterToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addRasterToolBarWidget( QWidget* widget ) override;
    //! Remove an icon (action) from the Raster toolbar
    void removeRasterToolBarIcon( QAction *qAction ) override;
    //! Add an icon to the Vector toolbar
    int addVectorToolBarIcon( QAction *qAction ) override;
    /**
     * Add a widget to the vector toolbar.
     * To remove this widget again, call {@link removeVectorToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addVectorToolBarWidget( QWidget* widget ) override;
    //! Remove an icon (action) from the Vector toolbar
    void removeVectorToolBarIcon( QAction *qAction ) override;
    //! Add an icon to the Database toolbar
    int addDatabaseToolBarIcon( QAction *qAction ) override;
    /**
     * Add a widget to the database toolbar.
     * To remove this widget again, call {@link removeDatabaseToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addDatabaseToolBarWidget( QWidget* widget ) override;
    //! Remove an icon (action) from the Database toolbar
    void removeDatabaseToolBarIcon( QAction *qAction ) override;
    //! Add an icon to the Web toolbar
    int addWebToolBarIcon( QAction *qAction ) override;
    /**
     * Add a widget to the web toolbar.
     * To remove this widget again, call {@link removeWebToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    QAction* addWebToolBarWidget( QWidget* widget ) override;
    //! Remove an icon (action) from the Web toolbar
    void removeWebToolBarIcon( QAction *qAction ) override;

    //! Add toolbar with specified name
    QToolBar* addToolBar( QString name ) override;

    //! Add a toolbar
    //! @note added in 2.3
    void addToolBar( QToolBar* toolbar, Qt::ToolBarArea area = Qt::TopToolBarArea ) override;

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
    void openURL( QString url, bool useQgisDocDirectory = true ) override;

    /** Return a pointer to the map canvas used by qgisapp */
    QgsMapCanvas * mapCanvas() override;

    /** Gives access to main QgisApp object

        Plugins don't need to know about QgisApp, as we pass it as QWidget,
        it can be used for connecting slots and using as widget's parent
    */
    QWidget * mainWindow() override;

    QgsMessageBar * messageBar() override;

    /** Adds a widget to the user input tool bar.*/
    void addUserInputWidget( QWidget* widget ) override;

    // ### QGIS 3: return QgsComposer*, not QgsComposerView*
    QList<QgsComposerView*> activeComposers() override;

    // ### QGIS 3: return QgsComposer*, not QgsComposerView*
    /** Create a new composer
     * @param title window title for new composer (one will be generated if empty)
     * @return pointer to composer's view
     * @note new composer window will be shown and activated
     */
    QgsComposerView* createNewComposer( QString title = QString( "" ) ) override;

    // ### QGIS 3: return QgsComposer*, not QgsComposerView*
    /** Duplicate an existing parent composer from composer view
     * @param composerView pointer to existing composer view
     * @param title window title for duplicated composer (one will be generated if empty)
     * @return pointer to duplicate composer's view
     * @note dupicate composer window will be hidden until loaded, then shown and activated
     */
    QgsComposerView* duplicateComposer( QgsComposerView* composerView, QString title = QString( "" ) ) override;

    /** Deletes parent composer of composer view, after closing composer window */
    void deleteComposer( QgsComposerView* composerView ) override;

    /** Return changeable options built from settings and/or defaults */
    QMap<QString, QVariant> defaultStyleSheetOptions() override;

    /** Generate stylesheet
     * @param opts generated default option values, or a changed copy of them */
    void buildStyleSheet( const QMap<QString, QVariant>& opts ) override;

    /** Save changed default option keys/values to user settings */
    void saveStyleSheetOptions( const QMap<QString, QVariant>& opts ) override;

    /** Get reference font for initial qApp (may not be same as QgisApp) */
    QFont defaultStyleSheetFont() override;

    /** Add action to the plugins menu */
    void addPluginToMenu( QString name, QAction* action ) override;
    /** Remove action from the plugins menu */
    void removePluginMenu( QString name, QAction* action ) override;

    /** Add action to the Database menu */
    void addPluginToDatabaseMenu( QString name, QAction* action ) override;
    /** Remove action from the Database menu */
    void removePluginDatabaseMenu( QString name, QAction* action ) override;

    /** Add action to the Raster menu */
    void addPluginToRasterMenu( QString name, QAction* action ) override;
    /** Remove action from the Raster menu */
    void removePluginRasterMenu( QString name, QAction* action ) override;

    /** Add action to the Vector menu */
    void addPluginToVectorMenu( QString name, QAction* action ) override;
    /** Remove action from the Raster menu */
    void removePluginVectorMenu( QString name, QAction* action ) override;

    /** Add action to the Web menu */
    void addPluginToWebMenu( QString name, QAction* action ) override;
    /** Remove action from the Web menu */
    void removePluginWebMenu( QString name, QAction* action ) override;

    /** Add "add layer" action to the layer menu */
    void insertAddLayerAction( QAction *action ) override;
    /** remove "add layer" action from the layer menu */
    void removeAddLayerAction( QAction *action ) override;

    /** Add a dock widget to the main window */
    void addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget ) override;

    /** Remove specified dock widget from main window (doesn't delete it). */
    void removeDockWidget( QDockWidget * dockwidget ) override;

    /** show layer properties dialog for layer
     * @param l layer to show properties table for
     */
    virtual void showLayerProperties( QgsMapLayer *l ) override;

    /** show layer attribute dialog for layer
     * @param l layer to show attribute table for
     */
    virtual void showAttributeTable( QgsVectorLayer *l ) override;

    /** Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window. */
    virtual void addWindow( QAction *action ) override;
    /** Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed. */
    virtual void removeWindow( QAction *action ) override;

    /** Register action to the shortcuts manager so its shortcut can be changed in GUI. */
    virtual bool registerMainWindowAction( QAction* action, QString defaultShortcut ) override;

    /** Unregister a previously registered action. (e.g. when plugin is going to be unloaded. */
    virtual bool unregisterMainWindowAction( QAction* action ) override;

    /** Accessors for inserting items into menus and toolbars.
     * An item can be inserted before any existing action.
     */

    //! Menus
    Q_DECL_DEPRECATED virtual QMenu *fileMenu() override;
    virtual QMenu *projectMenu() override;
    virtual QMenu *editMenu() override;
    virtual QMenu *viewMenu() override;
    virtual QMenu *layerMenu() override;
    virtual QMenu *newLayerMenu() override;
    //! @note added in 2.5
    virtual QMenu *addLayerMenu() override;
    virtual QMenu *settingsMenu() override;
    virtual QMenu *pluginMenu() override;
    virtual QMenu *rasterMenu() override;
    virtual QMenu *vectorMenu() override;
    virtual QMenu *databaseMenu() override;
    virtual QMenu *webMenu() override;
    virtual QMenu *firstRightStandardMenu() override;
    virtual QMenu *windowMenu() override;
    virtual QMenu *helpMenu() override;

    //! ToolBars
    virtual QToolBar *fileToolBar() override;
    virtual QToolBar *layerToolBar() override;
    virtual QToolBar *mapNavToolToolBar() override;
    virtual QToolBar *digitizeToolBar() override;
    virtual QToolBar *advancedDigitizeToolBar() override;
    virtual QToolBar *attributesToolBar() override;
    virtual QToolBar *pluginToolBar() override;
    virtual QToolBar *helpToolBar() override;
    virtual QToolBar *rasterToolBar() override;
    virtual QToolBar *vectorToolBar() override;
    virtual QToolBar *databaseToolBar() override;
    virtual QToolBar *webToolBar() override;

    //! Project menu actions
    virtual QAction *actionNewProject() override;
    virtual QAction *actionOpenProject() override;
    virtual QAction *actionSaveProject() override;
    virtual QAction *actionSaveProjectAs() override;
    virtual QAction *actionSaveMapAsImage() override;
    virtual QAction *actionProjectProperties() override;
    virtual QAction *actionPrintComposer() override;
    virtual QAction *actionShowComposerManager() override;
    virtual QAction *actionExit() override;

    //! Edit menu actions
    virtual QAction *actionCutFeatures() override;
    virtual QAction *actionCopyFeatures() override;
    virtual QAction *actionPasteFeatures() override;
    virtual QAction *actionAddFeature() override;
    virtual QAction *actionDeleteSelected() override;
    virtual QAction *actionMoveFeature() override;
    virtual QAction *actionSplitFeatures() override;
    virtual QAction *actionSplitParts() override;
    virtual QAction *actionAddRing() override;
    virtual QAction *actionAddPart() override;
    virtual QAction *actionSimplifyFeature() override;
    virtual QAction *actionDeleteRing() override;
    virtual QAction *actionDeletePart() override;
    virtual QAction *actionNodeTool() override;

    //! View menu actions
    virtual QAction *actionPan() override;
    virtual QAction *actionTouch() override;
    virtual QAction *actionPanToSelected() override;
    virtual QAction *actionZoomIn() override;
    virtual QAction *actionZoomOut() override;
    virtual QAction *actionSelect() override;
    virtual QAction *actionSelectRectangle() override;
    virtual QAction *actionSelectPolygon() override;
    virtual QAction *actionSelectFreehand() override;
    virtual QAction *actionSelectRadius() override;
    virtual QAction *actionIdentify() override;
    virtual QAction *actionFeatureAction() override;
    virtual QAction *actionMeasure() override;
    virtual QAction *actionMeasureArea() override;
    virtual QAction *actionZoomFullExtent() override;
    virtual QAction *actionZoomToLayer() override;
    virtual QAction *actionZoomToSelected() override;
    virtual QAction *actionZoomLast() override;
    virtual QAction *actionZoomNext() override;
    virtual QAction *actionZoomActualSize() override;
    virtual QAction *actionMapTips() override;
    virtual QAction *actionNewBookmark() override;
    virtual QAction *actionShowBookmarks() override;
    virtual QAction *actionDraw() override;

    //! Layer menu actions
    virtual QAction *actionNewVectorLayer() override;
    virtual QAction *actionAddOgrLayer() override;
    virtual QAction *actionAddRasterLayer() override;
    virtual QAction *actionAddPgLayer() override;
    virtual QAction *actionAddWmsLayer() override;
    virtual QAction *actionCopyLayerStyle() override;
    virtual QAction *actionPasteLayerStyle() override;
    virtual QAction *actionOpenTable() override;
    virtual QAction *actionOpenFieldCalculator() override;
    virtual QAction *actionToggleEditing() override;
    virtual QAction *actionSaveActiveLayerEdits() override;
    virtual QAction *actionAllEdits() override;
    virtual QAction *actionSaveEdits() override;
    virtual QAction *actionSaveAllEdits() override;
    virtual QAction *actionRollbackEdits() override;
    virtual QAction *actionRollbackAllEdits() override;
    virtual QAction *actionCancelEdits() override;
    virtual QAction *actionCancelAllEdits() override;
    virtual QAction *actionLayerSaveAs() override;
    virtual QAction *actionLayerSelectionSaveAs() override;
    virtual QAction *actionRemoveLayer() override;
    virtual QAction *actionDuplicateLayer() override;
    virtual QAction *actionLayerProperties() override;
    virtual QAction *actionAddToOverview() override;
    virtual QAction *actionAddAllToOverview() override;
    virtual QAction *actionRemoveAllFromOverview() override;
    virtual QAction *actionHideAllLayers() override;
    virtual QAction *actionShowAllLayers() override;
    virtual QAction *actionHideSelectedLayers() override;
    virtual QAction *actionShowSelectedLayers() override;

    //! Plugin menu actions
    virtual QAction *actionManagePlugins() override;
    virtual QAction *actionPluginListSeparator() override;
    virtual QAction *actionShowPythonDialog() override;

    //! Settings menu actions
    virtual QAction *actionToggleFullScreen() override;
    virtual QAction *actionOptions() override;
    virtual QAction *actionCustomProjection() override;

    //! Help menu actions
    virtual QAction *actionHelpContents() override;
    virtual QAction *actionQgisHomePage() override;
    virtual QAction *actionCheckQgisVersion() override;
    virtual QAction *actionAbout() override;

    /**
     * Open feature form
     * returns true when dialog was accepted (if shown modal, true otherwise)
     * @param l vector layer
     * @param f feature to show/modify
     * @param updateFeatureOnly only update the feature update (don't change any attributes of the layer) [UNUSED]
     * @param showModal if true, will wait for the dialog to be executed (only shown otherwise)
     */
    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false, bool showModal = true ) override;

    /**
     * Returns a feature form for a given feature
     *
     * @param layer   The layer for which the dialog will be created
     * @param feature The feature for which the dialog will be created
     *
     * @return A feature form
     */
    virtual QgsAttributeDialog* getFeatureForm( QgsVectorLayer *layer, QgsFeature &feature ) override;

    /**
     * Access the vector layer tools instance.
     * With the help of this you can access methods like addFeature, startEditing
     * or stopEditing while giving the user the appropriate dialogs.
     *
     * @return An instance of the vector layer tools
     */
    virtual QgsVectorLayerTools* vectorLayerTools() override;

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
    virtual void preloadForm( QString uifile ) override;

    /** Return vector layers in edit mode
     * @param modified whether to return only layers that have been modified
     * @returns list of layers in legend order, or empty list
     */
    virtual QList<QgsMapLayer *> editableLayers( bool modified = false ) const override;

    /** Get timeout for timed messages: default of 5 seconds */
    virtual int messageTimeout() override;

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
