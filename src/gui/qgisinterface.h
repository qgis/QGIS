/***************************************************************************
                          qgisinterface.h
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
#ifndef QGISINTERFACE_H
#define QGISINTERFACE_H

class QAction;
class QMenu;
class QToolBar;
class QDockWidget;
class QMainWindow;
class QWidget;

class QgsAdvancedDigitizingDockWidget;
class QgsAttributeDialog;
class QgsComposerInterface;
class QgsCustomDropHandler;
class QgsFeature;
class QgsLayerTreeMapCanvasBridge;
class QgsLayerTreeView;
class QgsLayout;
class QgsLayoutDesignerInterface;
class QgsMapCanvas;
class QgsMapLayer;
class QgsMapLayerConfigWidgetFactory;
class QgsMessageBar;
class QgsPluginManagerInterface;
class QgsRasterLayer;
class QgsSnappingUtils;
class QgsVectorLayer;
class QgsVectorLayerTools;
class QgsOptionsWidgetFactory;
class QgsLocatorFilter;
class QgsStatusBar;

#include <QObject>
#include <QFont>
#include <QPair>
#include <map>

#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgis_gui.h"


/**
 * \ingroup gui
 * QgisInterface
 * Abstract base class defining interfaces exposed by QgisApp and
 * made available to plugins.
 *
 * Only functionality exposed by QgisInterface can be used in plugins.
 * This interface has to be implemented with application specific details.
 *
 * QGIS implements it in QgisAppInterface class, 3rd party applications
 * could provide their own implementation to be able to use plugins.
 */

Q_NOWARN_DEPRECATED_PUSH
class GUI_EXPORT QgisInterface : public QObject
{
    Q_OBJECT

  public:

    //! Constructor
    QgisInterface() = default;

    virtual QgsPluginManagerInterface *pluginManagerInterface() = 0;

    virtual QgsLayerTreeView *layerTreeView() = 0;

    /**
     * Add action to context menu for layers in the layer tree.
     * If allLayers is true, then the action will be available for all layers of given type,
     * otherwise the action will be available only for specific layers added with addCustomActionForLayer()
     * after this call.
     *
     * If menu argument is not empty, the action will be also added to a menu within the main window,
     * creating menu with the given name if it does not exist yet.
     *
     * \see removeCustomActionForLayerType()
     * \see addCustomActionForLayer()
     */
    virtual void addCustomActionForLayerType( QAction *action, QString menu,
        QgsMapLayer::LayerType type, bool allLayers ) = 0;

    /**
     * Add action to context menu for a specific layer in the layer tree.
     * It is necessary to first call addCustomActionForLayerType() with allLayers=false
     * in order for this method to have any effect.
     * \see addCustomActionForLayerType()
     */
    virtual void addCustomActionForLayer( QAction *action, QgsMapLayer *layer ) = 0;

    /**
     * Remove action for layers in the layer tree previously added with addCustomActionForLayerType()
     * \see addCustomActionForLayerType()
     */
    virtual bool removeCustomActionForLayerType( QAction *action ) = 0;

    /**
     * Returns a list of all map canvases open in the app.
     * \since QGIS 3.0
     */
    virtual QList< QgsMapCanvas * > mapCanvases() = 0;

    /**
     * Create a new map canvas with the specified unique \a name.
     * \since QGIS 3.0
     * \see closeMapCanvas()
     */
    virtual QgsMapCanvas *createNewMapCanvas( const QString &name ) = 0;

    /**
     * Closes the additional map canvas with matching \a name.
     * \since QGIS 3.0
     * \see createNewMapCanvas()
     */
    virtual void closeMapCanvas( const QString &name ) = 0;

    /**
     * Returns the toolbar icon size. If \a dockedToolbar is true, the icon size
     * for toolbars contained within docks is returned.
     */
    virtual QSize iconSize( bool dockedToolbar = false ) const = 0;

    /**
     * Return vector layers in edit mode
     * \param modified whether to return only layers that have been modified
     * \returns list of layers in legend order, or empty list */
    virtual QList<QgsMapLayer *> editableLayers( bool modified = false ) const = 0;

    //! Get pointer to the active layer (layer selected in the legend)
    virtual QgsMapLayer *activeLayer() = 0;

    //! Return a pointer to the map canvas
    virtual QgsMapCanvas *mapCanvas() = 0;

    /**
     * Returns a pointer to the layer tree canvas bridge
     *
     * \since QGIS 2.12
     */
    virtual QgsLayerTreeMapCanvasBridge *layerTreeCanvasBridge() = 0;

    //! Return a pointer to the main window (instance of QgisApp in case of QGIS)
    virtual QWidget *mainWindow() = 0;

    //! Return the message bar of the main app
    virtual QgsMessageBar *messageBar() = 0;

    /**
     * Returns all currently open composer windows.
     * \since QGIS 3.0
     */
    virtual QList<QgsComposerInterface *> openComposers() = 0;

    /**
     * Returns all currently open layout designers.
     * \since QGIS 3.0
     */
    virtual QList<QgsLayoutDesignerInterface *> openLayoutDesigners() = 0;


    //! Return changeable options built from settings and/or defaults
    virtual QMap<QString, QVariant> defaultStyleSheetOptions() = 0;

    //! Get reference font for initial qApp (may not be same as QgisApp)
    virtual QFont defaultStyleSheetFont() = 0;

    /**
     * Advanced digitizing dock widget
     *  \since QGIS 2.12
     */
    virtual QgsAdvancedDigitizingDockWidget *cadDockWidget() = 0;

    /**
     * Accessors for inserting items into menus and toolbars.
     * An item can be inserted before any existing action.
     */

    // Menus
    virtual QMenu *projectMenu() = 0;
    virtual QMenu *editMenu() = 0;
    virtual QMenu *viewMenu() = 0;
    virtual QMenu *layerMenu() = 0;
    virtual QMenu *newLayerMenu() = 0;
    //! \since QGIS 2.5
    virtual QMenu *addLayerMenu() = 0;
    virtual QMenu *settingsMenu() = 0;
    virtual QMenu *pluginMenu() = 0;
    virtual QMenu *rasterMenu() = 0;
    virtual QMenu *databaseMenu() = 0;
    virtual QMenu *vectorMenu() = 0;
    virtual QMenu *webMenu() = 0;
    virtual QMenu *firstRightStandardMenu() = 0;
    virtual QMenu *windowMenu() = 0;
    virtual QMenu *helpMenu() = 0;

    // ToolBars
    virtual QToolBar *fileToolBar() = 0;
    virtual QToolBar *layerToolBar() = 0;
    virtual QToolBar *mapNavToolToolBar() = 0;
    virtual QToolBar *digitizeToolBar() = 0;
    virtual QToolBar *advancedDigitizeToolBar() = 0;
    virtual QToolBar *attributesToolBar() = 0;
    virtual QToolBar *pluginToolBar() = 0;
    virtual QToolBar *helpToolBar() = 0;
    virtual QToolBar *rasterToolBar() = 0;
    virtual QToolBar *vectorToolBar() = 0;
    virtual QToolBar *databaseToolBar() = 0;
    virtual QToolBar *webToolBar() = 0;

    // Project menu actions
    virtual QAction *actionNewProject() = 0;
    virtual QAction *actionOpenProject() = 0;
    virtual QAction *actionSaveProject() = 0;
    virtual QAction *actionSaveProjectAs() = 0;
    virtual QAction *actionSaveMapAsImage() = 0;
    virtual QAction *actionProjectProperties() = 0;
    virtual QAction *actionPrintComposer() = 0;
    virtual QAction *actionShowComposerManager() = 0;
    virtual QAction *actionExit() = 0;

    // Edit menu actions
    virtual QAction *actionCutFeatures() = 0;
    virtual QAction *actionCopyFeatures() = 0;
    virtual QAction *actionPasteFeatures() = 0;
    virtual QAction *actionAddFeature() = 0;
    virtual QAction *actionDeleteSelected() = 0;
    virtual QAction *actionMoveFeature() = 0;
    virtual QAction *actionSplitFeatures() = 0;
    virtual QAction *actionSplitParts() = 0;
    virtual QAction *actionAddRing() = 0;
    virtual QAction *actionAddPart() = 0;
    virtual QAction *actionSimplifyFeature() = 0;
    virtual QAction *actionDeleteRing() = 0;
    virtual QAction *actionDeletePart() = 0;
    virtual QAction *actionNodeTool() = 0;

    // View menu actions
    //! Get access to the native pan action. Call trigger() on it to set the default pan map tool.
    virtual QAction *actionPan() = 0;
    //! Get access to the native pan to selected action. Call trigger() on it to pan the map canvas to the selection.
    virtual QAction *actionPanToSelected() = 0;
    //! Get access to the native zoom in action. Call trigger() on it to set the default zoom in map tool.
    virtual QAction *actionZoomIn() = 0;
    //! Get access to the native zoom out action. Call trigger() on it to set the default zoom out map tool.
    virtual QAction *actionZoomOut() = 0;
    //! Get access to the native select action. Call trigger() on it to set the default select map tool.
    virtual QAction *actionSelect() = 0;
    //! Get access to the native select rectangle action. Call trigger() on it to set the default select rectangle map tool.
    virtual QAction *actionSelectRectangle() = 0;
    //! Get access to the native select polygon action. Call trigger() on it to set the default select polygon map tool.
    virtual QAction *actionSelectPolygon() = 0;
    //! Get access to the native select freehand action. Call trigger() on it to set the default select freehand map tool.
    virtual QAction *actionSelectFreehand() = 0;
    //! Get access to the native select radius action. Call trigger() on it to set the default select radius map tool.
    virtual QAction *actionSelectRadius() = 0;
    //! Get access to the native identify action. Call trigger() on it to set the default identify map tool.
    virtual QAction *actionIdentify() = 0;
    //! Get access to the native run action feature action. Call trigger() on it to set the default run feature action map tool.
    virtual QAction *actionFeatureAction() = 0;
    //! Get access to the native measure action. Call trigger() on it to set the default measure map tool.
    virtual QAction *actionMeasure() = 0;
    //! Get access to the native measure area action. Call trigger() on it to set the default measure area map tool.
    virtual QAction *actionMeasureArea() = 0;
    //! Get access to the native zoom full extent action. Call trigger() on it to zoom to the full extent.
    virtual QAction *actionZoomFullExtent() = 0;
    //! Get access to the native zoom to layer action. Call trigger() on it to zoom to the active layer.
    virtual QAction *actionZoomToLayer() = 0;
    //! Get access to the native zoom to selected action. Call trigger() on it to zoom to the current selection.
    virtual QAction *actionZoomToSelected() = 0;
    //! Get access to the native zoom last action. Call trigger() on it to zoom to last.
    virtual QAction *actionZoomLast() = 0;
    //! Get access to the native zoom next action. Call trigger() on it to zoom to next.
    virtual QAction *actionZoomNext() = 0;
    //! Get access to the native zoom resolution (100%) action. Call trigger() on it to zoom to actual size.
    virtual QAction *actionZoomActualSize() = 0;
    //! Get access to the native map tips action. Call trigger() on it to toggle map tips.
    virtual QAction *actionMapTips() = 0;
    //! Get access to the native new bookmark action. Call trigger() on it to open the new bookmark dialog.
    virtual QAction *actionNewBookmark() = 0;
    //! Get access to the native show bookmarks action. Call trigger() on it to open the bookmarks dialog.
    virtual QAction *actionShowBookmarks() = 0;
    //! Get access to the native draw action.
    virtual QAction *actionDraw() = 0;

    // Layer menu actions
    virtual QAction *actionNewVectorLayer() = 0;
    virtual QAction *actionAddOgrLayer() = 0;
    virtual QAction *actionAddRasterLayer() = 0;
    virtual QAction *actionAddPgLayer() = 0;
    virtual QAction *actionAddWmsLayer() = 0;
    //! Get access to the native Add ArcGIS FeatureServer action.
    virtual QAction *actionAddAfsLayer() = 0;
    //! Get access to the native Add ArcGIS MapServer action.
    virtual QAction *actionAddAmsLayer() = 0;
    virtual QAction *actionCopyLayerStyle() = 0;
    virtual QAction *actionPasteLayerStyle() = 0;
    virtual QAction *actionOpenTable() = 0;
    virtual QAction *actionOpenFieldCalculator() = 0;
    virtual QAction *actionToggleEditing() = 0;
    virtual QAction *actionSaveActiveLayerEdits() = 0;
    virtual QAction *actionAllEdits() = 0;
    virtual QAction *actionSaveEdits() = 0;
    virtual QAction *actionSaveAllEdits() = 0;
    virtual QAction *actionRollbackEdits() = 0;
    virtual QAction *actionRollbackAllEdits() = 0;
    virtual QAction *actionCancelEdits() = 0;
    virtual QAction *actionCancelAllEdits() = 0;
    virtual QAction *actionLayerSaveAs() = 0;
    virtual QAction *actionDuplicateLayer() = 0;
    virtual QAction *actionLayerProperties() = 0;
    virtual QAction *actionAddToOverview() = 0;
    virtual QAction *actionAddAllToOverview() = 0;
    virtual QAction *actionRemoveAllFromOverview() = 0;
    virtual QAction *actionHideAllLayers() = 0;
    virtual QAction *actionShowAllLayers() = 0;
    virtual QAction *actionHideSelectedLayers() = 0;

    /**
     * Returns the Hide Deselected Layers action.
     * \since QGIS 3.0
     */
    virtual QAction *actionHideDeselectedLayers() = 0;
    virtual QAction *actionShowSelectedLayers() = 0;

    // Plugin menu actions
    virtual QAction *actionManagePlugins() = 0;
    virtual QAction *actionPluginListSeparator() = 0;
    virtual QAction *actionShowPythonDialog() = 0;

    // Settings menu actions
    virtual QAction *actionToggleFullScreen() = 0;
    virtual QAction *actionOptions() = 0;
    virtual QAction *actionCustomProjection() = 0;

    // Help menu actions
    virtual QAction *actionHelpContents() = 0;
    virtual QAction *actionQgisHomePage() = 0;
    virtual QAction *actionCheckQgisVersion() = 0;
    virtual QAction *actionAbout() = 0;

    /**
     * Access the vector layer tools instance.
     * With the help of this you can access methods like addFeature, startEditing
     * or stopEditing while giving the user the appropriate dialogs.
     *
     * \returns An instance of the vector layer tools
     */
    virtual QgsVectorLayerTools *vectorLayerTools() = 0;

    //! Get timeout for timed messages: default of 5 seconds
    virtual int messageTimeout() = 0;

    /**
     * Returns a pointer to the app's status bar interface. This should be
     * used for interacting and adding widgets and messages to the app's
     * status bar (do not use the native Qt statusBar() method).
     * \since QGIS 3.0
     */
    virtual QgsStatusBar *statusBarIface() = 0;

  public slots: // TODO: do these functions really need to be slots?

    /* Exposed functions */

    //! Zoom to full extent of map layers
    virtual void zoomFull() = 0;

    //! Zoom to previous view extent
    virtual void zoomToPrevious() = 0;

    //! Zoom to next view extent
    virtual void zoomToNext() = 0;

    //! Zoom to extent of the active layer
    virtual void zoomToActiveLayer() = 0;

    //! Add a vector layer
    virtual QgsVectorLayer *addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey ) = 0;

    //! Add a raster layer given a raster layer file name
    virtual QgsRasterLayer *addRasterLayer( const QString &rasterLayerPath, const QString &baseName = QString() ) = 0;

    //! Add a WMS layer
    virtual QgsRasterLayer *addRasterLayer( const QString &url, const QString &layerName, const QString &providerKey ) = 0;

    //! Add a project
    virtual bool addProject( const QString &project ) = 0;
    //! Start a blank project
    virtual void newProject( bool promptToSaveFlag = false ) = 0;

    /**
     * Triggered when connections have changed.
     * This calls reloadConnections in the main application and triggers a signal that is
     * forwarded to the GUI elements that needs to be updated (i.e. the source
     * select dialogs and the browser widgets)
     * \since QGIS 3.0
     */
    virtual void reloadConnections( ) = 0;

    /**
     * Set the active layer (layer gets selected in the legend)
     * returns true if the layer exists, false otherwise
     */
    virtual bool setActiveLayer( QgsMapLayer * ) = 0;

    //! Add an icon to the plugins toolbar
    virtual int addToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the plugins toolbar.
     * To remove this widget again, call removeToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Remove an action (icon) from the plugin toolbar
    virtual void removeToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the raster toolbar.
     * To remove this widget again, call removeRasterToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addRasterToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Add an icon to the Raster toolbar
    virtual int addRasterToolBarIcon( QAction *qAction ) = 0;

    //! Remove an action (icon) from the Raster toolbar
    virtual void removeRasterToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Vector toolbar
    virtual int addVectorToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the vector toolbar.
     * To remove this widget again, call removeVectorToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addVectorToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Remove an action (icon) from the Vector toolbar
    virtual void removeVectorToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Database toolbar
    virtual int addDatabaseToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the database toolbar.
     * To remove this widget again, call removeDatabaseToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addDatabaseToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Remove an action (icon) from the Database toolbar
    virtual void removeDatabaseToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Web toolbar
    virtual int addWebToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the web toolbar.
     * To remove this widget again, call removeWebToolBarIcon()
     * with the returned QAction.
     *
     * \param widget widget to add. The toolbar will take ownership of this widget
     * \returns the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction *addWebToolBarWidget( QWidget *widget SIP_TRANSFER ) = 0;

    //! Remove an action (icon) from the Web toolbar
    virtual void removeWebToolBarIcon( QAction *qAction ) = 0;

    //! Add toolbar with specified name
    virtual QToolBar *addToolBar( const QString &name ) = 0 SIP_FACTORY;

    /**
     * Add a toolbar
     * \since QGIS 2.3
     */
    virtual void addToolBar( QToolBar *toolbar SIP_TRANSFER, Qt::ToolBarArea area = Qt::TopToolBarArea ) = 0;

    //! Open the message log dock widget *
    virtual void openMessageLog() = 0;

    //! Adds a widget to the user input tool bar.
    virtual void addUserInputWidget( QWidget *widget ) = 0;

    /**
     * Opens a new composer window for the specified \a composition, or
     * brings an already open composer window to the foreground if one
     * is already created for the composition.
     * \since QGIS 3.0
     * \see closeComposer()
     */
    virtual QgsComposerInterface *openComposer( QgsComposition *composition ) = 0;

    /**
     * Closes an open composer window showing the specified \a composition.
     * The composition remains unaffected.
     * \since QGIS 3.0
     * \see openComposer()
     */
    virtual void closeComposer( QgsComposition *composition ) = 0;

    /**
     * Opens a new layout designer dialog for the specified \a layout, or
     * brings an already open designer window to the foreground if one
     * is already created for the layout.
     * \since QGIS 3.0
     * \see closeComposer()
     */
    virtual QgsLayoutDesignerInterface *openLayoutDesigner( QgsLayout *layout ) = 0;

    /**
     * Opens the options dialog. The \a currentPage argument can be used to force
     * the dialog to open at a specific page.
     * \since QGIS 3.0
     */
    virtual void showOptionsDialog( QWidget *parent = nullptr, const QString &currentPage = QString() ) = 0;

    /**
     * Generate stylesheet
     * \param opts generated default option values, or a changed copy of them
     */
    virtual void buildStyleSheet( const QMap<QString, QVariant> &opts ) = 0;

    //! Save changed default option keys/values to user settings
    virtual void saveStyleSheetOptions( const QMap<QString, QVariant> &opts ) = 0;

    //! Add action to the plugins menu
    virtual void addPluginToMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the plugins menu
    virtual void removePluginMenu( const QString &name, QAction *action ) = 0;

    //! Add "add layer" action to layer menu
    virtual void insertAddLayerAction( QAction *action ) = 0;

    //! Remove "add layer" action from layer menu
    virtual void removeAddLayerAction( QAction *action ) = 0;

    //! Add action to the Database menu
    virtual void addPluginToDatabaseMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the Database menu
    virtual void removePluginDatabaseMenu( const QString &name, QAction *action ) = 0;

    //! Add action to the Raster menu
    virtual void addPluginToRasterMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the Raster menu
    virtual void removePluginRasterMenu( const QString &name, QAction *action ) = 0;

    //! Add action to the Vector menu
    virtual void addPluginToVectorMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the Vector menu
    virtual void removePluginVectorMenu( const QString &name, QAction *action ) = 0;

    //! Add action to the Web menu
    virtual void addPluginToWebMenu( const QString &name, QAction *action ) = 0;

    //! Remove action from the Web menu
    virtual void removePluginWebMenu( const QString &name, QAction *action ) = 0;

    //! Add a dock widget to the main window
    virtual void addDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget ) = 0;

    //! Remove specified dock widget from main window (doesn't delete it).
    virtual void removeDockWidget( QDockWidget *dockwidget ) = 0;

    //! Open layer properties dialog
    virtual void showLayerProperties( QgsMapLayer *l ) = 0;

    //! Open attribute table dialog
    virtual QDialog *showAttributeTable( QgsVectorLayer *l, const QString &filterExpression = QString() ) = 0;

    /**
     * Add window to Window menu. The action title is the window title
     * and the action should raise, unminimize and activate the window. */
    virtual void addWindow( QAction *action ) = 0;

    /**
     * Remove window from Window menu. Calling this is necessary only for
     * windows which are hidden rather than deleted when closed. */
    virtual void removeWindow( QAction *action ) = 0;

    //! Register action to the shortcuts manager so its shortcut can be changed in GUI
    virtual bool registerMainWindowAction( QAction *action, const QString &defaultShortcut ) = 0;

    //! Unregister a previously registered action. (e.g. when plugin is going to be unloaded)
    virtual bool unregisterMainWindowAction( QAction *action ) = 0;

    /**
     * Register a new tab in the vector layer properties dialog.
     * \since QGIS 2.16
     * \note Ownership of the factory is not transferred, and the factory must
     *       be unregistered when plugin is unloaded.
     * \see unregisterMapLayerPropertiesFactory() */
    virtual void registerMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) = 0;

    /**
     * Unregister a previously registered tab in the vector layer properties dialog.
     * \since QGIS 2.16
     * \see registerMapLayerPropertiesFactory()
    */
    virtual void unregisterMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) = 0;

    /**
     * Register a new tab in the options dialog.
     * \since QGIS 3.0
     * \note Ownership of the factory is not transferred, and the factory must
     *       be unregistered when plugin is unloaded.
     * \see unregisterOptionsWidgetFactory() */
    virtual void registerOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;

    /**
     * Unregister a previously registered tab in the options dialog.
     * \since QGIS 3.0
     * \see registerOptionsWidgetFactory()
    */
    virtual void unregisterOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;

    /**
     * Register a new custom drop handler.
     * \since QGIS 3.0
     * \note Ownership of the factory is not transferred, and the factory must
     *       be unregistered when plugin is unloaded.
     * \see unregisterCustomDropHandler() */
    virtual void registerCustomDropHandler( QgsCustomDropHandler *handler ) = 0;

    /**
     * Unregister a previously registered custom drop handler.
     * \since QGIS 3.0
     * \see registerCustomDropHandler() */
    virtual void unregisterCustomDropHandler( QgsCustomDropHandler *handler ) = 0;

    // @todo is this deprecated in favour of QgsContextHelp?

    /**
     * Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to false.
     * \param url URL to open
     * \param useQgisDocDirectory If true, the URL will be formed by concatenating
     * url to the QGIS documentation directory path (prefix/share/doc)
     * \deprecated
     */
#ifndef Q_MOC_RUN
    Q_DECL_DEPRECATED
#endif
    virtual void openURL( const QString &url, bool useQgisDocDirectory = true ) = 0 SIP_DEPRECATED;

    /**
     * Open feature form
     * \param l vector layer
     * \param f feature to show/modify
     * \param updateFeatureOnly only update the feature update (don't change any attributes of the layer) [UNUSED]
     * \param showModal if true, will wait for the dialog to be executed (only shown otherwise)
     */
    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false, bool showModal = true ) = 0;

    /**
     * Returns a feature form for a given feature
     *
     * \param l The layer for which the dialog will be created
     * \param f The feature for which the dialog will be created
     *
     * \returns A feature form
     */
    virtual QgsAttributeDialog *getFeatureForm( QgsVectorLayer *l, QgsFeature &f ) = 0;

    /**
     * This method is only needed when using a UI form with a custom widget plugin and calling
     * openFeatureForm or getFeatureForm from Python (PyQt4) and you haven't used the info tool first.
     * Python will crash bringing QGIS with it
     * if the custom form is not loaded from a C++ method call.
     *
     * This method uses a QTimer to call QUiLoader in order to load the form via C++
     * you only need to call this once after that you can call openFeatureForm/getFeatureForm
     * like normal
     *
     * More information here: http://qt-project.org/forums/viewthread/27098/
     */
    virtual void preloadForm( const QString &uifile ) = 0;

    /**
     * Registers a locator \a filter for the app's locator bar. Ownership of the filter is transferred to the
     * locator.
     * \warning Plugins which register filters to the locator bar must take care to correctly call
     * deregisterLocatorFilter() and deregister their filters upon plugin unload to avoid crashes.
     * \see deregisterLocatorFilter()
     * \since QGIS 3.0
     */
    virtual void registerLocatorFilter( QgsLocatorFilter *filter SIP_TRANSFER ) = 0;

    /**
     * Deregisters a locator \a filter from the app's locator bar and deletes it. Calling this will block whilst
     * any currently running query is terminated.
     *
     * Plugins which register filters to the locator bar must take care to correctly call
     * deregisterLocatorFilter() to deregister their filters upon plugin unload to avoid crashes.
     *
     * \see registerLocatorFilter()
     * \since QGIS 3.0
     */
    virtual void deregisterLocatorFilter( QgsLocatorFilter *filter ) = 0;

  signals:

    /**
     * Emitted whenever current (selected) layer changes.
     *  The pointer to layer can be null if no layer is selected
     */
    void currentLayerChanged( QgsMapLayer *layer );

    /**
     * Signal emitted when the current \a theme is changed so plugins
     * can change their tool button icons.
     * \since QGIS 3.0
    */
    void currentThemeChanged( const QString &theme );

    /**
     * This signal is emitted when a new composer window has been opened.
     * \since QGIS 3.0
     * \see composerWillBeClosed()
     */
    void composerOpened( QgsComposerInterface *composer );

    /**
     * This signal is emitted before a composer window is going to be closed
     * and deleted.
     * \since QGIS 3.0
     * \see composerClosed()
     * \see composerOpened()
     */
    void composerWillBeClosed( QgsComposerInterface *composer );

    /**
     * This signal is emitted after a composer window is closed.
     * \since QGIS 3.0
     * \see composerWillBeClosed()
     * \see composerOpened()
     */
    void composerClosed( QgsComposerInterface *composer );

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

    /**
     * This signal is emitted when the initialization is complete
     */
    void initializationCompleted();

    /**
     * Emitted when a project file is successfully read
     * \note
     * This is useful for plug-ins that store properties with project files.  A
     * plug-in can connect to this signal.  When it is emitted, the plug-in
     * knows to then check the project properties for any relevant state.
     */
    void projectRead();

    /**
     * Emitted when starting an entirely new project
     * \note
     * This is similar to projectRead(); plug-ins might want to be notified
     * that they're in a new project.  Yes, projectRead() could have been
     * overloaded to be used in the case of new projects instead.  However,
     * it's probably more semantically correct to have an entirely separate
     * signal for when this happens.
     */
    void newProjectCreated();

    /**
     * This signal is emitted when a layer has been saved using save as
     * \note
     * added in version 2.7
     */
    void layerSavedAs( QgsMapLayer *l, const QString &path );

};
Q_NOWARN_DEPRECATED_POP

#endif //#ifndef QGISINTERFACE_H
