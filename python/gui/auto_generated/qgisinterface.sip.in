/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgisinterface.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgisInterface : QObject
{
%Docstring
QgisInterface
Abstract base class defining interfaces exposed by QgisApp and
made available to plugins.

Only functionality exposed by QgisInterface can be used in plugins.
This interface has to be implemented with application specific details.

QGIS implements it in QgisAppInterface class, 3rd party applications
could provide their own implementation to be able to use plugins.
%End

%TypeHeaderCode
#include "qgisinterface.h"
%End
  public:

    QgisInterface();
%Docstring
Constructor
%End

    virtual QgsPluginManagerInterface *pluginManagerInterface() = 0;

    virtual QgsLayerTreeView *layerTreeView() = 0;

    virtual void addCustomActionForLayerType( QAction *action, QString menu,
        QgsMapLayerType type, bool allLayers ) = 0;
%Docstring
Add action to context menu for layers in the layer tree.
If allLayers is ``True``, then the action will be available for all layers of given type,
otherwise the action will be available only for specific layers added with addCustomActionForLayer()
after this call.

If menu argument is not empty, the action will be also added to a menu within the main window,
creating menu with the given name if it does not exist yet.

.. seealso:: :py:func:`removeCustomActionForLayerType`

.. seealso:: :py:func:`addCustomActionForLayer`
%End

    virtual void addCustomActionForLayer( QAction *action, QgsMapLayer *layer ) = 0;
%Docstring
Add action to context menu for a specific layer in the layer tree.
It is necessary to first call addCustomActionForLayerType() with allLayers=false
in order for this method to have any effect.

.. seealso:: :py:func:`addCustomActionForLayerType`
%End

    virtual bool removeCustomActionForLayerType( QAction *action ) = 0;
%Docstring
Remove action for layers in the layer tree previously added with addCustomActionForLayerType()

.. seealso:: :py:func:`addCustomActionForLayerType`
%End

    virtual QList< QgsMapCanvas * > mapCanvases() = 0;
%Docstring
Returns a list of all map canvases open in the app.

.. versionadded:: 3.0
%End

    virtual QgsMapCanvas *createNewMapCanvas( const QString &name ) = 0;
%Docstring
Create a new map canvas with the specified unique ``name``.

.. seealso:: :py:func:`closeMapCanvas`

.. versionadded:: 3.0
%End

    virtual void closeMapCanvas( const QString &name ) = 0;
%Docstring
Closes the additional map canvas with matching ``name``.

.. seealso:: :py:func:`createNewMapCanvas`

.. versionadded:: 3.0
%End

    virtual QSize iconSize( bool dockedToolbar = false ) const = 0;
%Docstring
Returns the toolbar icon size. If ``dockedToolbar`` is ``True``, the icon size
for toolbars contained within docks is returned.
%End

    virtual QList<QgsMapLayer *> editableLayers( bool modified = false ) const = 0;
%Docstring
Returns vector layers in edit mode

:param modified: whether to return only layers that have been modified

:return: list of layers in legend order, or empty list
%End

    virtual QgsMapLayer *activeLayer() = 0;
%Docstring
Returns a pointer to the active layer (layer selected in the legend)
%End

    virtual QgsMapCanvas *mapCanvas() = 0;
%Docstring
Returns a pointer to the map canvas
%End

    virtual QgsLayerTreeMapCanvasBridge *layerTreeCanvasBridge() = 0;
%Docstring
Returns a pointer to the layer tree canvas bridge

.. versionadded:: 2.12
%End

    virtual QWidget *mainWindow() = 0;
%Docstring
Returns a pointer to the main window (instance of QgisApp in case of QGIS)
%End

    virtual QgsMessageBar *messageBar() = 0;
%Docstring
Returns the message bar of the main app
%End

    virtual QList<QgsLayoutDesignerInterface *> openLayoutDesigners() = 0;
%Docstring
Returns all currently open layout designers.

.. versionadded:: 3.0
%End


    virtual QMap<QString, QVariant> defaultStyleSheetOptions() = 0;
%Docstring
Returns changeable options built from settings and/or defaults
%End

    virtual QFont defaultStyleSheetFont() = 0;
%Docstring
Returns a reference font for initial qApp (may not be same as QgisApp)
%End

    virtual QgsAdvancedDigitizingDockWidget *cadDockWidget() = 0;
%Docstring
Advanced digitizing dock widget

.. versionadded:: 2.12
%End


    virtual QMenu *projectMenu() = 0;
%Docstring
Returns a reference to the main window "Project" menu.
%End

    virtual QMenu *editMenu() = 0;
%Docstring
Returns a reference to the main window "Edit" menu.
%End

    virtual QMenu *viewMenu() = 0;
%Docstring
Returns a reference to the main window "View" menu.
%End

    virtual QMenu *layerMenu() = 0;
%Docstring
Returns a reference to the main window "Layer" menu.
%End

    virtual QMenu *newLayerMenu() = 0;
%Docstring
Returns a reference to the main window "Create New Layer" menu.
%End

    virtual QMenu *addLayerMenu() = 0;
%Docstring
Returns a reference to the main window "Add Layer" menu.

.. versionadded:: 2.5
%End

    virtual QMenu *settingsMenu() = 0;
%Docstring
Returns a reference to the main window "Settings" menu.
%End

    virtual QMenu *pluginMenu() = 0;
%Docstring
Returns a reference to the main window "Plugin" menu.
%End

    virtual QMenu *rasterMenu() = 0;
%Docstring
Returns a reference to the main window "Raster" menu.
%End

    virtual QMenu *databaseMenu() = 0;
%Docstring
Returns a reference to the main window "Database" menu.
%End

    virtual QMenu *vectorMenu() = 0;
%Docstring
Returns a reference to the main window "Vector" menu.
%End

    virtual QMenu *webMenu() = 0;
%Docstring
Returns a reference to the main window "Web" menu.
%End

    virtual QMenu *firstRightStandardMenu() = 0;
%Docstring
Returns a reference to the right most standard menu, which is
usually the last menu item before the "Help" menu.

This can be used to insert additional top-level menus into
their correct position BEFORE the help menu.
%End

    virtual QMenu *windowMenu() = 0;
%Docstring
Returns a reference to the main window "Window" menu.
%End

    virtual QMenu *helpMenu() = 0;
%Docstring
Returns a reference to the main window "Help" menu.
%End


    virtual QToolBar *fileToolBar() = 0;
%Docstring
Returns a reference to the main window "File" toolbar.
%End

    virtual QToolBar *layerToolBar() = 0;
%Docstring
Returns a reference to the main window "Layer" toolbar.
%End

    virtual QToolBar *dataSourceManagerToolBar() = 0;
%Docstring
Returns a reference to the main window "Data Source Manager" toolbar.

.. versionadded:: 3.4
%End

    virtual QToolBar *mapNavToolToolBar() = 0;
%Docstring
Returns a reference to the main window "Map Navigation" toolbar.
%End

    virtual QToolBar *digitizeToolBar() = 0;
%Docstring
Returns a reference to the main window "Digitize" toolbar.
%End

    virtual QToolBar *advancedDigitizeToolBar() = 0;
%Docstring
Returns a reference to the main window "Advanced Digitizing" toolbar.
%End

    virtual QToolBar *shapeDigitizeToolBar() = 0;
%Docstring
Returns a reference to the main window "Shape Digitizing" toolbar.

.. versionadded:: 3.0
%End

    virtual QToolBar *attributesToolBar() = 0;
%Docstring
Returns a reference to the main window "Attributes" toolbar.
%End

    virtual QToolBar *pluginToolBar() = 0;
%Docstring
Returns a reference to the main window "Plugin" toolbar.
%End

    virtual QToolBar *helpToolBar() = 0;
%Docstring
Returns a reference to the main window "Help" toolbar.
%End

    virtual QToolBar *rasterToolBar() = 0;
%Docstring
Returns a reference to the main window "Raster" toolbar.
%End

    virtual QToolBar *vectorToolBar() = 0;
%Docstring
Returns a reference to the main window "Vector" toolbar.
%End

    virtual QToolBar *databaseToolBar() = 0;
%Docstring
Returns a reference to the main window "Database" toolbar.
%End

    virtual QToolBar *webToolBar() = 0;
%Docstring
Returns a reference to the main window "Web" toolbar.
%End

    virtual QAction *actionNewProject() = 0;
%Docstring
Returns the native New Project action.
%End
    virtual QAction *actionOpenProject() = 0;
%Docstring
Returns the Open Project action.
%End
    virtual QAction *actionSaveProject() = 0;
%Docstring
Returns the native Save Project action.
%End
    virtual QAction *actionSaveProjectAs() = 0;
%Docstring
Returns the native Save Project As action.
%End
    virtual QAction *actionSaveMapAsImage() = 0;
%Docstring
Returns the native Save Map as Image action.
%End
    virtual QAction *actionProjectProperties() = 0;
%Docstring
Returns the native Project Properties action.
%End

    virtual QAction *actionCreatePrintLayout() = 0;
%Docstring
Create new print layout action
%End

    virtual QAction *actionShowLayoutManager() = 0;
%Docstring
Show layout manager action
%End
    virtual QAction *actionExit() = 0;
%Docstring
Returns the Exit QGIS action.
%End


    virtual QAction *actionCutFeatures() = 0;
%Docstring
Returns the native Cut Features action.
%End
    virtual QAction *actionCopyFeatures() = 0;
%Docstring
Returns the native Copy Features action.
%End
    virtual QAction *actionPasteFeatures() = 0;
%Docstring
Returns the native Paste Features action.
%End
    virtual QAction *actionAddFeature() = 0;
%Docstring
Returns the native Add Feature action.
%End
    virtual QAction *actionDeleteSelected() = 0;
%Docstring
Returns the native Delete Selected Features action.
%End
    virtual QAction *actionMoveFeature() = 0;
%Docstring
Returns the native Move Features action.
%End
    virtual QAction *actionSplitFeatures() = 0;
%Docstring
Returns the native Split Features action.
%End
    virtual QAction *actionSplitParts() = 0;
%Docstring
Returns the native Split Parts action.
%End
    virtual QAction *actionAddRing() = 0;
%Docstring
Returns the native Add Ring action.
%End
    virtual QAction *actionAddPart() = 0;
%Docstring
Returns the native Add Part action.
%End
    virtual QAction *actionSimplifyFeature() = 0;
%Docstring
Returns the native Simplify/Smooth Features action.
%End
    virtual QAction *actionDeleteRing() = 0;
%Docstring
Returns the native Delete Ring action.
%End
    virtual QAction *actionDeletePart() = 0;
%Docstring
Returns the native Delete Part action.
%End

    virtual QAction *actionVertexTool() = 0;
%Docstring
Returns the native "Vertex Tool for All Layers" action.

.. seealso:: :py:func:`actionVertexToolActiveLayer`
%End

    virtual QAction *actionVertexToolActiveLayer() = 0;
%Docstring
Returns the native "Vertex Tool for Active Layer" action.

.. seealso:: :py:func:`actionVertexTool`

.. versionadded:: 3.6
%End

    virtual QAction *actionPan() = 0;
%Docstring
Returns the native pan action. Call trigger() on it to set the default pan map tool.
%End
    virtual QAction *actionPanToSelected() = 0;
%Docstring
Returns the native pan to selected action. Call trigger() on it to pan the map canvas to the selection.
%End
    virtual QAction *actionZoomIn() = 0;
%Docstring
Returns the native zoom in action. Call trigger() on it to set the default zoom in map tool.
%End
    virtual QAction *actionZoomOut() = 0;
%Docstring
Returns the native zoom out action. Call trigger() on it to set the default zoom out map tool.
%End
    virtual QAction *actionSelect() = 0;
%Docstring
Returns the native select action. Call trigger() on it to set the default select map tool.
%End
    virtual QAction *actionSelectRectangle() = 0;
%Docstring
Returns the native select rectangle action. Call trigger() on it to set the default select rectangle map tool.
%End
    virtual QAction *actionSelectPolygon() = 0;
%Docstring
Returns the native select polygon action. Call trigger() on it to set the default select polygon map tool.
%End
    virtual QAction *actionSelectFreehand() = 0;
%Docstring
Returns the native select freehand action. Call trigger() on it to set the default select freehand map tool.
%End
    virtual QAction *actionSelectRadius() = 0;
%Docstring
Returns the native select radius action. Call trigger() on it to set the default select radius map tool.
%End
    virtual QAction *actionIdentify() = 0;
%Docstring
Returns the native identify action. Call trigger() on it to set the default identify map tool.
%End
    virtual QAction *actionFeatureAction() = 0;
%Docstring
Returns the native run action feature action. Call trigger() on it to set the default run feature action map tool.
%End
    virtual QAction *actionMeasure() = 0;
%Docstring
Returns the native measure action. Call trigger() on it to set the default measure map tool.
%End
    virtual QAction *actionMeasureArea() = 0;
%Docstring
Returns the native measure area action. Call trigger() on it to set the default measure area map tool.
%End
    virtual QAction *actionZoomFullExtent() = 0;
%Docstring
Returns the native zoom full extent action. Call trigger() on it to zoom to the full extent.
%End
    virtual QAction *actionZoomToLayer() = 0;
%Docstring
Returns the native zoom to layer action. Call trigger() on it to zoom to the active layer.
%End
    virtual QAction *actionZoomToSelected() = 0;
%Docstring
Returns the native zoom to selected action. Call trigger() on it to zoom to the current selection.
%End
    virtual QAction *actionZoomLast() = 0;
%Docstring
Returns the native zoom last action. Call trigger() on it to zoom to last.
%End
    virtual QAction *actionZoomNext() = 0;
%Docstring
Returns the native zoom next action. Call trigger() on it to zoom to next.
%End
    virtual QAction *actionZoomActualSize() = 0;
%Docstring
Returns the native zoom resolution (100%) action. Call trigger() on it to zoom to actual size.
%End
    virtual QAction *actionMapTips() = 0;
%Docstring
Returns the native map tips action. Call trigger() on it to toggle map tips.
%End
    virtual QAction *actionNewBookmark() = 0;
%Docstring
Returns the native new bookmark action. Call trigger() on it to open the new bookmark dialog.
%End
    virtual QAction *actionShowBookmarks() = 0;
%Docstring
Returns the native show bookmarks action. Call trigger() on it to open the bookmarks dialog.
%End
    virtual QAction *actionDraw() = 0;
%Docstring
Returns the native draw action.
%End

    virtual QAction *actionNewVectorLayer() = 0;
    virtual QAction *actionAddOgrLayer() = 0;
    virtual QAction *actionAddRasterLayer() = 0;
    virtual QAction *actionAddPgLayer() = 0;
    virtual QAction *actionAddWmsLayer() = 0;
    virtual QAction *actionAddAfsLayer() = 0;
%Docstring
Returns the native Add ArcGIS FeatureServer action.
%End
    virtual QAction *actionAddAmsLayer() = 0;
%Docstring
Returns the native Add ArcGIS MapServer action.
%End
    virtual QAction *actionCopyLayerStyle() = 0;
    virtual QAction *actionPasteLayerStyle() = 0;
    virtual QAction *actionOpenTable() = 0;
    virtual QAction *actionOpenFieldCalculator() = 0;

    virtual QAction *actionOpenStatisticalSummary() = 0;
%Docstring
Statistical summary action.

.. versionadded:: 3.0
%End

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

    virtual QAction *actionHideDeselectedLayers() = 0;
%Docstring
Returns the Hide Deselected Layers action.

.. versionadded:: 3.0
%End
    virtual QAction *actionShowSelectedLayers() = 0;

    virtual QAction *actionManagePlugins() = 0;
    virtual QAction *actionPluginListSeparator() = 0;
    virtual QAction *actionShowPythonDialog() = 0;

    virtual QAction *actionToggleFullScreen() = 0;
    virtual QAction *actionOptions() = 0;
    virtual QAction *actionCustomProjection() = 0;

    virtual QAction *actionHelpContents() = 0;
    virtual QAction *actionQgisHomePage() = 0;
    virtual QAction *actionCheckQgisVersion() = 0;
    virtual QAction *actionAbout() = 0;

    virtual QgsVectorLayerTools *vectorLayerTools() = 0;
%Docstring
Access the vector layer tools instance.
With the help of this you can access methods like addFeature, startEditing
or stopEditing while giving the user the appropriate dialogs.

:return: An instance of the vector layer tools
%End

    virtual int messageTimeout() = 0;
%Docstring
Returns the timeout for timed messages: default of 5 seconds
%End

    virtual QgsStatusBar *statusBarIface() = 0;
%Docstring
Returns a pointer to the app's status bar interface. This should be
used for interacting and adding widgets and messages to the app's
status bar (do not use the native Qt statusBar() method).

.. versionadded:: 3.0
%End

    virtual void takeAppScreenShots( const QString &saveDirectory, const int categories = 0 );
%Docstring
Take screenshots for user documentation

:param saveDirectory: path where the screenshots will be saved
:param categories: an int as a flag value of QgsAppScreenShots.Categories

.. versionadded:: 3.4
%End

  public slots: // TODO: do these functions really need to be slots?


    virtual void zoomFull() = 0;
%Docstring
Zooms to the full extent of all map layers.
%End

    virtual void zoomToPrevious() = 0;
%Docstring
Zooms to the previous view extent.
%End

    virtual void zoomToNext() = 0;
%Docstring
Zooms to the next view extent.
%End

    virtual void zoomToActiveLayer() = 0;
%Docstring
Zooms to extent of the active layer.
%End

    virtual QgsVectorLayer *addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey ) = 0;
%Docstring
Adds a vector layer to the current project.
%End

    virtual QgsRasterLayer *addRasterLayer( const QString &rasterLayerPath, const QString &baseName = QString() ) = 0;
%Docstring
Adds a raster layer to the current project, given a raster layer file name.
%End

    virtual QgsRasterLayer *addRasterLayer( const QString &url, const QString &layerName, const QString &providerKey ) = 0;
%Docstring
Adds a raster layer to the current project, from the specified raster data provider.
%End

    virtual QgsMeshLayer *addMeshLayer( const QString &url, const QString &baseName, const QString &providerKey ) = 0;
%Docstring
Adds a mesh layer to the current project.
%End

    virtual bool addProject( const QString &project ) = 0;
%Docstring
Adds (opens) a project
%End
    virtual void newProject( bool promptToSaveFlag = false ) = 0;
%Docstring
Starts a new blank project
%End

    virtual void reloadConnections( ) = 0;
%Docstring
Triggered when connections have changed.
This calls reloadConnections in the main application and triggers a signal that is
forwarded to the GUI elements that needs to be updated (i.e. the source
select dialogs and the browser widgets)

.. versionadded:: 3.0
%End

    virtual bool setActiveLayer( QgsMapLayer * ) = 0;
%Docstring
Set the active layer (layer gets selected in the legend)
returns ``True`` if the layer exists, ``False`` otherwise
%End

    virtual void copySelectionToClipboard( QgsMapLayer * ) = 0;
%Docstring
Copy selected features from the layer to clipboard

.. versionadded:: 3.0
%End

    virtual void pasteFromClipboard( QgsMapLayer * ) = 0;
%Docstring
Paste features from clipboard to the layer

.. versionadded:: 3.0
%End

    virtual int addToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the plugins toolbar
%End

    virtual QAction *addToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
Add a widget to the plugins toolbar.
To remove this widget again, call removeToolBarIcon()
with the returned QAction.

:param widget: widget to add. The toolbar will take ownership of this widget

:return: the QAction you can use to remove this widget from the toolbar
%End

    virtual void removeToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the plugin toolbar
%End

    virtual QAction *addRasterToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
Add a widget to the raster toolbar.
To remove this widget again, call removeRasterToolBarIcon()
with the returned QAction.

:param widget: widget to add. The toolbar will take ownership of this widget

:return: the QAction you can use to remove this widget from the toolbar
%End

    virtual int addRasterToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the Raster toolbar
%End

    virtual void removeRasterToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the Raster toolbar
%End

    virtual int addVectorToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the Vector toolbar
%End

    virtual QAction *addVectorToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
Add a widget to the vector toolbar.
To remove this widget again, call removeVectorToolBarIcon()
with the returned QAction.

:param widget: widget to add. The toolbar will take ownership of this widget

:return: the QAction you can use to remove this widget from the toolbar
%End

    virtual void removeVectorToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the Vector toolbar
%End

    virtual int addDatabaseToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the Database toolbar
%End

    virtual QAction *addDatabaseToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
Add a widget to the database toolbar.
To remove this widget again, call removeDatabaseToolBarIcon()
with the returned QAction.

:param widget: widget to add. The toolbar will take ownership of this widget

:return: the QAction you can use to remove this widget from the toolbar
%End

    virtual void removeDatabaseToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the Database toolbar
%End

    virtual int addWebToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the Web toolbar
%End

    virtual QAction *addWebToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
Add a widget to the web toolbar.
To remove this widget again, call removeWebToolBarIcon()
with the returned QAction.

:param widget: widget to add. The toolbar will take ownership of this widget

:return: the QAction you can use to remove this widget from the toolbar
%End

    virtual void removeWebToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the Web toolbar
%End

    virtual QToolBar *addToolBar( const QString &name ) = 0 /Factory/;
%Docstring
Add toolbar with specified name
%End

    virtual void addToolBar( QToolBar *toolbar /Transfer/, Qt::ToolBarArea area = Qt::TopToolBarArea ) = 0;
%Docstring
Add a toolbar

.. versionadded:: 2.3
%End

    virtual void openMessageLog() = 0;
%Docstring
Opens the message log dock widget.
%End

    virtual void addUserInputWidget( QWidget *widget ) = 0;
%Docstring
Adds a widget to the user input tool bar.
%End

    virtual void showLayoutManager() = 0;
%Docstring
Opens the layout manager dialog.

.. versionadded:: 3.0
%End

    virtual QgsLayoutDesignerInterface *openLayoutDesigner( QgsMasterLayoutInterface *layout ) = 0;
%Docstring
Opens a new layout designer dialog for the specified ``layout``, or
brings an already open designer window to the foreground if one
is already created for the layout.

.. versionadded:: 3.0
%End

    virtual void showOptionsDialog( QWidget *parent = 0, const QString &currentPage = QString() ) = 0;
%Docstring
Opens the options dialog. The ``currentPage`` argument can be used to force
the dialog to open at a specific page.

.. versionadded:: 3.0
%End

    virtual void buildStyleSheet( const QMap<QString, QVariant> &opts ) = 0;
%Docstring
Generate stylesheet

:param opts: generated default option values, or a changed copy of them
%End

    virtual void saveStyleSheetOptions( const QMap<QString, QVariant> &opts ) = 0;
%Docstring
Save changed default option keys/values to user settings
%End

    virtual void addPluginToMenu( const QString &name, QAction *action ) = 0;
%Docstring
Add action to the plugins menu
%End

    virtual void removePluginMenu( const QString &name, QAction *action ) = 0;
%Docstring
Remove action from the plugins menu
%End

    virtual void insertAddLayerAction( QAction *action ) = 0;
%Docstring
Add "add layer" action to layer menu
%End

    virtual void removeAddLayerAction( QAction *action ) = 0;
%Docstring
Remove "add layer" action from layer menu
%End

    virtual void addPluginToDatabaseMenu( const QString &name, QAction *action ) = 0;
%Docstring
Add action to the Database menu
%End

    virtual void removePluginDatabaseMenu( const QString &name, QAction *action ) = 0;
%Docstring
Remove action from the Database menu
%End

    virtual void addPluginToRasterMenu( const QString &name, QAction *action ) = 0;
%Docstring
Add action to the Raster menu
%End

    virtual void removePluginRasterMenu( const QString &name, QAction *action ) = 0;
%Docstring
Remove action from the Raster menu
%End

    virtual void addPluginToVectorMenu( const QString &name, QAction *action ) = 0;
%Docstring
Add action to the Vector menu
%End

    virtual void removePluginVectorMenu( const QString &name, QAction *action ) = 0;
%Docstring
Remove action from the Vector menu
%End

    virtual void addPluginToWebMenu( const QString &name, QAction *action ) = 0;
%Docstring
Add action to the Web menu
%End

    virtual void removePluginWebMenu( const QString &name, QAction *action ) = 0;
%Docstring
Remove action from the Web menu
%End

    virtual void addDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget ) = 0;
%Docstring
Adds a ``dock`` widget to the main window, in the specified dock ``area``.

.. seealso:: :py:func:`removeDockWidget`
%End

    virtual void removeDockWidget( QDockWidget *dockwidget ) = 0;
%Docstring
Removes the specified ``dock`` widget from main window (without deleting it).

.. seealso:: :py:func:`addDockWidget`
%End

    virtual void showLayerProperties( QgsMapLayer *l ) = 0;
%Docstring
Open layer properties dialog
%End

    virtual QDialog *showAttributeTable( QgsVectorLayer *l, const QString &filterExpression = QString() ) = 0;
%Docstring
Open attribute table dialog
%End

    virtual void addWindow( QAction *action ) = 0;
%Docstring
Add window to Window menu. The action title is the window title
and the action should raise, unminimize and activate the window.
%End

    virtual void removeWindow( QAction *action ) = 0;
%Docstring
Remove window from Window menu. Calling this is necessary only for
windows which are hidden rather than deleted when closed.
%End

    virtual bool registerMainWindowAction( QAction *action, const QString &defaultShortcut ) = 0;
%Docstring
Register action to the shortcuts manager so its shortcut can be changed in GUI
%End

    virtual bool unregisterMainWindowAction( QAction *action ) = 0;
%Docstring
Unregister a previously registered action. (e.g. when plugin is going to be unloaded)
%End

    virtual void registerMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) = 0;
%Docstring
Register a new tab in the vector layer properties dialog.

.. note::

   Ownership of the factory is not transferred, and the factory must
   be unregistered when plugin is unloaded.

.. seealso:: :py:class:`QgsMapLayerConfigWidgetFactory`

.. seealso:: :py:func:`unregisterMapLayerConfigWidgetFactory`

.. versionadded:: 2.16
%End

    virtual void unregisterMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) = 0;
%Docstring
Unregister a previously registered tab in the vector layer properties dialog.

.. seealso:: :py:class:`QgsMapLayerConfigWidgetFactory`

.. seealso:: :py:func:`registerMapLayerConfigWidgetFactory`

.. versionadded:: 2.16
%End

    virtual void registerOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;
%Docstring
Register a new tab in the options dialog.

.. note::

   Ownership of the factory is not transferred, and the factory must
   be unregistered when plugin is unloaded.

.. seealso:: :py:class:`QgsOptionsWidgetFactory`

.. seealso:: :py:func:`unregisterOptionsWidgetFactory`

.. versionadded:: 3.0
%End

    virtual void unregisterOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;
%Docstring
Unregister a previously registered tab in the options dialog.

.. seealso:: :py:class:`QgsOptionsWidgetFactory`

.. seealso:: :py:func:`registerOptionsWidgetFactory`

.. versionadded:: 3.0
%End

    virtual void registerCustomDropHandler( QgsCustomDropHandler *handler ) = 0;
%Docstring
Register a new custom drop handler.

.. note::

   Ownership of the factory is not transferred, and the factory must
   be unregistered when plugin is unloaded.

.. seealso:: :py:class:`QgsCustomDropHandler`

.. seealso:: :py:func:`unregisterCustomDropHandler`

.. versionadded:: 3.0
%End

    virtual void unregisterCustomDropHandler( QgsCustomDropHandler *handler ) = 0;
%Docstring
Unregister a previously registered custom drop handler.

.. seealso:: :py:class:`QgsCustomDropHandler`

.. seealso:: :py:func:`registerCustomDropHandler`

.. versionadded:: 3.0
%End

    virtual void registerCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler ) = 0;
%Docstring
Register a new custom drop ``handler`` for layout windows.

.. note::

   Ownership of the factory is not transferred, and the factory must
   be unregistered when plugin is unloaded.

.. seealso:: :py:class:`QgsLayoutCustomDropHandler`

.. seealso:: :py:func:`unregisterCustomLayoutDropHandler`

.. versionadded:: 3.0
%End

    virtual void unregisterCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler ) = 0;
%Docstring
Unregister a previously registered custom drop ``handler`` for layout windows.

.. seealso:: :py:class:`QgsLayoutCustomDropHandler`

.. seealso:: :py:func:`registerCustomLayoutDropHandler`

.. versionadded:: 3.0
%End



    virtual void openURL( const QString &url, bool useQgisDocDirectory = true ) = 0 /Deprecated/;
%Docstring
Open a url in the users browser. By default the QGIS doc directory is used
as the base for the URL. To open a URL that is not relative to the installed
QGIS documentation, set useQgisDocDirectory to ``False``.

:param url: URL to open
:param useQgisDocDirectory: If ``True``, the URL will be formed by concatenating
                            url to the QGIS documentation directory path (prefix/share/doc)

.. deprecated:: Use QDesktopServices instead
%End

    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false, bool showModal = true ) = 0;
%Docstring
Opens a new feature form.
Returns true if dialog was accepted (if shown modal, ``True`` otherwise).

:param l: vector layer
:param f: feature to show/modify
:param updateFeatureOnly: only update the feature update (don't change any attributes of the layer) [UNUSED]
:param showModal: if ``True``, will wait for the dialog to be executed (only shown otherwise)
%End

    virtual QgsAttributeDialog *getFeatureForm( QgsVectorLayer *l, QgsFeature &f ) = 0;
%Docstring
Returns a feature form for a given feature.

:param l: The layer for which the dialog will be created
:param f: The feature for which the dialog will be created

:return: A feature form
%End

    virtual void preloadForm( const QString &uifile ) = 0;
%Docstring
This method is only needed when using a UI form with a custom widget plugin and calling
openFeatureForm or getFeatureForm from Python (PyQt) and you haven't used the info tool first.
Python will crash bringing QGIS with it
if the custom form is not loaded from a C++ method call.

This method uses a QTimer to call QUiLoader in order to load the form via C++.
You only need to call this once. After that you can call openFeatureForm/getFeatureForm
like usual.

More information here: http://qt-project.org/forums/viewthread/27098
%End

    virtual void registerLocatorFilter( QgsLocatorFilter *filter /Transfer/ ) = 0;
%Docstring
Registers a locator ``filter`` for the app's locator bar. Ownership of the filter is transferred to the
locator.

.. warning::

   Plugins which register filters to the locator bar must take care to correctly call
   deregisterLocatorFilter() and deregister their filters upon plugin unload to avoid crashes.

.. seealso:: :py:func:`deregisterLocatorFilter`

.. versionadded:: 3.0
%End

    virtual void deregisterLocatorFilter( QgsLocatorFilter *filter ) = 0;
%Docstring
Deregisters a locator ``filter`` from the app's locator bar and deletes it. Calling this will block whilst
any currently running query is terminated.

Plugins which register filters to the locator bar must take care to correctly call
deregisterLocatorFilter() to deregister their filters upon plugin unload to avoid crashes.

.. seealso:: :py:func:`registerLocatorFilter`

.. versionadded:: 3.0
%End

    virtual void invalidateLocatorResults() = 0;
%Docstring
Invalidate results from the locator filter.

This might be useful if the configuration of the filter changed without going through main application settings.

.. versionadded:: 3.2
%End

    virtual bool askForDatumTransform( QgsCoordinateReferenceSystem sourceCrs, QgsCoordinateReferenceSystem destinationCrs ) = 0;
%Docstring
Checks available datum transforms and ask user if several are available and none
is chosen. Dialog is shown only if global option is set accordingly.

:return: ``True`` if a datum transform has been specifically chosen by user or only one is available.

.. versionadded:: 3.0
%End

    virtual QgsBrowserModel *browserModel() = 0;
%Docstring
Returns the application browser model. Using this shared model is more efficient than
creating a new browser model for every use.

.. versionadded:: 3.4
%End

  signals:

    void currentLayerChanged( QgsMapLayer *layer );
%Docstring
Emitted whenever current (selected) layer changes.
The pointer to layer can be ``None`` if no layer is selected.
%End

    void currentThemeChanged( const QString &theme );
%Docstring
Emitted when the current ``theme`` is changed so plugins
can change their tool button icons.

.. versionadded:: 3.0
%End

    void layoutDesignerOpened( QgsLayoutDesignerInterface *designer );
%Docstring
Emitted when a new layout ``designer`` has been opened.

.. seealso:: :py:func:`layoutDesignerWillBeClosed`

.. versionadded:: 3.0
%End

    void layoutDesignerWillBeClosed( QgsLayoutDesignerInterface *designer );
%Docstring
Emitted before a layout ``designer`` is going to be closed
and deleted.

.. seealso:: :py:func:`layoutDesignerClosed`

.. seealso:: :py:func:`layoutDesignerOpened`

.. versionadded:: 3.0
%End

    void layoutDesignerClosed();
%Docstring
Emitted after a layout designer window is closed.

.. seealso:: :py:func:`layoutDesignerWillBeClosed`

.. seealso:: :py:func:`layoutDesignerOpened`

.. versionadded:: 3.0
%End

    void initializationCompleted();
%Docstring
Emitted when the initialization is complete.
%End

    void projectRead();
%Docstring
Emitted when a project file is successfully read.

.. note::

   This is useful for plugins that store properties with project files.
   A plugin can connect to this signal. When it is emitted, the plugin
   knows to then check the project properties for any relevant state.
%End

    void newProjectCreated();
%Docstring
Emitted when starting an entirely new project.

.. note::

   This is similar to projectRead(); plugins might want to be notified
   that they're in a new project. Yes, projectRead() could have been
   overloaded to be used in the case of new projects instead. However,
   it's probably more semantically correct to have an entirely separate
   signal for when this happens.
%End

    void layerSavedAs( QgsMapLayer *l, const QString &path );
%Docstring
Emitted when a layer has been saved using save as.

.. versionadded:: 2.7
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgisinterface.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
