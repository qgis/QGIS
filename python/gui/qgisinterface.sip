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
%Docstring
 :rtype: QgsPluginManagerInterface
%End

    virtual QgsLayerTreeView *layerTreeView() = 0;
%Docstring
 :rtype: QgsLayerTreeView
%End

    virtual void addCustomActionForLayerType( QAction *action, QString menu,
        QgsMapLayer::LayerType type, bool allLayers ) = 0;
%Docstring
 Add action to context menu for layers in the layer tree.
 If allLayers is true, then the action will be available for all layers of given type,
 otherwise the action will be available only for specific layers added with addCustomActionForLayer()
 after this call.

 If menu argument is not empty, the action will be also added to a menu within the main window,
 creating menu with the given name if it does not exist yet.

.. seealso:: removeCustomActionForLayerType()
.. seealso:: addCustomActionForLayer()
%End

    virtual void addCustomActionForLayer( QAction *action, QgsMapLayer *layer ) = 0;
%Docstring
 Add action to context menu for a specific layer in the layer tree.
 It is necessary to first call addCustomActionForLayerType() with allLayers=false
 in order for this method to have any effect.
.. seealso:: addCustomActionForLayerType()
%End

    virtual bool removeCustomActionForLayerType( QAction *action ) = 0;
%Docstring
 Remove action for layers in the layer tree previously added with addCustomActionForLayerType()
.. seealso:: addCustomActionForLayerType()
 :rtype: bool
%End

    virtual QList< QgsMapCanvas * > mapCanvases() = 0;
%Docstring
 Returns a list of all map canvases open in the app.
.. versionadded:: 3.0
 :rtype: list of QgsMapCanvas
%End

    virtual QgsMapCanvas *createNewMapCanvas( const QString &name ) = 0;
%Docstring
 Create a new map canvas with the specified unique ``name``.
.. versionadded:: 3.0
.. seealso:: closeMapCanvas()
 :rtype: QgsMapCanvas
%End

    virtual void closeMapCanvas( const QString &name ) = 0;
%Docstring
 Closes the additional map canvas with matching ``name``.
.. versionadded:: 3.0
.. seealso:: createNewMapCanvas()
%End

    virtual QSize iconSize( bool dockedToolbar = false ) const = 0;
%Docstring
 Returns the toolbar icon size. If ``dockedToolbar`` is true, the icon size
 for toolbars contained within docks is returned.
 :rtype: QSize
%End

    virtual QList<QgsMapLayer *> editableLayers( bool modified = false ) const = 0;
%Docstring
 Return vector layers in edit mode
 \param modified whether to return only layers that have been modified
 :return: list of layers in legend order, or empty list *
 :rtype: list of QgsMapLayer
%End

    virtual QgsMapLayer *activeLayer() = 0;
%Docstring
Get pointer to the active layer (layer selected in the legend)
 :rtype: QgsMapLayer
%End

    virtual QgsMapCanvas *mapCanvas() = 0;
%Docstring
Return a pointer to the map canvas
 :rtype: QgsMapCanvas
%End

    virtual QgsLayerTreeMapCanvasBridge *layerTreeCanvasBridge() = 0;
%Docstring
 Returns a pointer to the layer tree canvas bridge

.. versionadded:: 2.12
 :rtype: QgsLayerTreeMapCanvasBridge
%End

    virtual QWidget *mainWindow() = 0;
%Docstring
Return a pointer to the main window (instance of QgisApp in case of QGIS)
 :rtype: QWidget
%End

    virtual QgsMessageBar *messageBar() = 0;
%Docstring
Return the message bar of the main app
 :rtype: QgsMessageBar
%End

    virtual QList<QgsComposerInterface *> openComposers() = 0;
%Docstring
 Returns all currently open composer windows.
.. versionadded:: 3.0
 :rtype: list of QgsComposerInterface
%End

    virtual QList<QgsLayoutDesignerInterface *> openLayoutDesigners() = 0;
%Docstring
 Returns all currently open layout designers.
.. versionadded:: 3.0
 :rtype: list of QgsLayoutDesignerInterface
%End


    virtual QMap<QString, QVariant> defaultStyleSheetOptions() = 0;
%Docstring
Return changeable options built from settings and/or defaults
 :rtype: QMap<str, QVariant>
%End

    virtual QFont defaultStyleSheetFont() = 0;
%Docstring
Get reference font for initial qApp (may not be same as QgisApp)
 :rtype: QFont
%End

    virtual QgsAdvancedDigitizingDockWidget *cadDockWidget() = 0;
%Docstring
 Advanced digitizing dock widget
.. versionadded:: 2.12
 :rtype: QgsAdvancedDigitizingDockWidget
%End


    virtual QMenu *projectMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *editMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *viewMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *layerMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *newLayerMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *addLayerMenu() = 0;
%Docstring
.. versionadded:: 2.5
 :rtype: QMenu
%End
    virtual QMenu *settingsMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *pluginMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *rasterMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *databaseMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *vectorMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *webMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *firstRightStandardMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *windowMenu() = 0;
%Docstring
 :rtype: QMenu
%End
    virtual QMenu *helpMenu() = 0;
%Docstring
 :rtype: QMenu
%End

    virtual QToolBar *fileToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *layerToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *mapNavToolToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *digitizeToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *advancedDigitizeToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *attributesToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *pluginToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *helpToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *rasterToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *vectorToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *databaseToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End
    virtual QToolBar *webToolBar() = 0;
%Docstring
 :rtype: QToolBar
%End

    virtual QAction *actionNewProject() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionOpenProject() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSaveProject() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSaveProjectAs() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSaveMapAsImage() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionProjectProperties() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionPrintComposer() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionShowComposerManager() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionExit() = 0;
%Docstring
 :rtype: QAction
%End

    virtual QAction *actionCutFeatures() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionCopyFeatures() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionPasteFeatures() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddFeature() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionDeleteSelected() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionMoveFeature() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSplitFeatures() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSplitParts() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddRing() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddPart() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSimplifyFeature() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionDeleteRing() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionDeletePart() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionNodeTool() = 0;
%Docstring
 :rtype: QAction
%End

    virtual QAction *actionPan() = 0;
%Docstring
Get access to the native pan action. Call trigger() on it to set the default pan map tool.
 :rtype: QAction
%End
    virtual QAction *actionPanToSelected() = 0;
%Docstring
Get access to the native pan to selected action. Call trigger() on it to pan the map canvas to the selection.
 :rtype: QAction
%End
    virtual QAction *actionZoomIn() = 0;
%Docstring
Get access to the native zoom in action. Call trigger() on it to set the default zoom in map tool.
 :rtype: QAction
%End
    virtual QAction *actionZoomOut() = 0;
%Docstring
Get access to the native zoom out action. Call trigger() on it to set the default zoom out map tool.
 :rtype: QAction
%End
    virtual QAction *actionSelect() = 0;
%Docstring
Get access to the native select action. Call trigger() on it to set the default select map tool.
 :rtype: QAction
%End
    virtual QAction *actionSelectRectangle() = 0;
%Docstring
Get access to the native select rectangle action. Call trigger() on it to set the default select rectangle map tool.
 :rtype: QAction
%End
    virtual QAction *actionSelectPolygon() = 0;
%Docstring
Get access to the native select polygon action. Call trigger() on it to set the default select polygon map tool.
 :rtype: QAction
%End
    virtual QAction *actionSelectFreehand() = 0;
%Docstring
Get access to the native select freehand action. Call trigger() on it to set the default select freehand map tool.
 :rtype: QAction
%End
    virtual QAction *actionSelectRadius() = 0;
%Docstring
Get access to the native select radius action. Call trigger() on it to set the default select radius map tool.
 :rtype: QAction
%End
    virtual QAction *actionIdentify() = 0;
%Docstring
Get access to the native identify action. Call trigger() on it to set the default identify map tool.
 :rtype: QAction
%End
    virtual QAction *actionFeatureAction() = 0;
%Docstring
Get access to the native run action feature action. Call trigger() on it to set the default run feature action map tool.
 :rtype: QAction
%End
    virtual QAction *actionMeasure() = 0;
%Docstring
Get access to the native measure action. Call trigger() on it to set the default measure map tool.
 :rtype: QAction
%End
    virtual QAction *actionMeasureArea() = 0;
%Docstring
Get access to the native measure area action. Call trigger() on it to set the default measure area map tool.
 :rtype: QAction
%End
    virtual QAction *actionZoomFullExtent() = 0;
%Docstring
Get access to the native zoom full extent action. Call trigger() on it to zoom to the full extent.
 :rtype: QAction
%End
    virtual QAction *actionZoomToLayer() = 0;
%Docstring
Get access to the native zoom to layer action. Call trigger() on it to zoom to the active layer.
 :rtype: QAction
%End
    virtual QAction *actionZoomToSelected() = 0;
%Docstring
Get access to the native zoom to selected action. Call trigger() on it to zoom to the current selection.
 :rtype: QAction
%End
    virtual QAction *actionZoomLast() = 0;
%Docstring
Get access to the native zoom last action. Call trigger() on it to zoom to last.
 :rtype: QAction
%End
    virtual QAction *actionZoomNext() = 0;
%Docstring
Get access to the native zoom next action. Call trigger() on it to zoom to next.
 :rtype: QAction
%End
    virtual QAction *actionZoomActualSize() = 0;
%Docstring
Get access to the native zoom resolution (100%) action. Call trigger() on it to zoom to actual size.
 :rtype: QAction
%End
    virtual QAction *actionMapTips() = 0;
%Docstring
Get access to the native map tips action. Call trigger() on it to toggle map tips.
 :rtype: QAction
%End
    virtual QAction *actionNewBookmark() = 0;
%Docstring
Get access to the native new bookmark action. Call trigger() on it to open the new bookmark dialog.
 :rtype: QAction
%End
    virtual QAction *actionShowBookmarks() = 0;
%Docstring
Get access to the native show bookmarks action. Call trigger() on it to open the bookmarks dialog.
 :rtype: QAction
%End
    virtual QAction *actionDraw() = 0;
%Docstring
Get access to the native draw action.
 :rtype: QAction
%End

    virtual QAction *actionNewVectorLayer() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddOgrLayer() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddRasterLayer() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddPgLayer() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddWmsLayer() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddAfsLayer() = 0;
%Docstring
Get access to the native Add ArcGIS FeatureServer action.
 :rtype: QAction
%End
    virtual QAction *actionAddAmsLayer() = 0;
%Docstring
Get access to the native Add ArcGIS MapServer action.
 :rtype: QAction
%End
    virtual QAction *actionCopyLayerStyle() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionPasteLayerStyle() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionOpenTable() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionOpenFieldCalculator() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionToggleEditing() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSaveActiveLayerEdits() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAllEdits() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSaveEdits() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionSaveAllEdits() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionRollbackEdits() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionRollbackAllEdits() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionCancelEdits() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionCancelAllEdits() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionLayerSaveAs() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionDuplicateLayer() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionLayerProperties() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddToOverview() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAddAllToOverview() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionRemoveAllFromOverview() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionHideAllLayers() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionShowAllLayers() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionHideSelectedLayers() = 0;
%Docstring
 :rtype: QAction
%End

    virtual QAction *actionHideDeselectedLayers() = 0;
%Docstring
 Returns the Hide Deselected Layers action.
.. versionadded:: 3.0
 :rtype: QAction
%End
    virtual QAction *actionShowSelectedLayers() = 0;
%Docstring
 :rtype: QAction
%End

    virtual QAction *actionManagePlugins() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionPluginListSeparator() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionShowPythonDialog() = 0;
%Docstring
 :rtype: QAction
%End

    virtual QAction *actionToggleFullScreen() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionOptions() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionCustomProjection() = 0;
%Docstring
 :rtype: QAction
%End

    virtual QAction *actionHelpContents() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionQgisHomePage() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionCheckQgisVersion() = 0;
%Docstring
 :rtype: QAction
%End
    virtual QAction *actionAbout() = 0;
%Docstring
 :rtype: QAction
%End

    virtual QgsVectorLayerTools *vectorLayerTools() = 0;
%Docstring
 Access the vector layer tools instance.
 With the help of this you can access methods like addFeature, startEditing
 or stopEditing while giving the user the appropriate dialogs.

 :return: An instance of the vector layer tools
 :rtype: QgsVectorLayerTools
%End

    virtual int messageTimeout() = 0;
%Docstring
Get timeout for timed messages: default of 5 seconds
 :rtype: int
%End

    virtual QgsStatusBar *statusBarIface() = 0;
%Docstring
 Returns a pointer to the app's status bar interface. This should be
 used for interacting and adding widgets and messages to the app's
 status bar (do not use the native Qt statusBar() method).
.. versionadded:: 3.0
 :rtype: QgsStatusBar
%End

  public slots: // TODO: do these functions really need to be slots?


    virtual void zoomFull() = 0;
%Docstring
Zoom to full extent of map layers
%End

    virtual void zoomToPrevious() = 0;
%Docstring
Zoom to previous view extent
%End

    virtual void zoomToNext() = 0;
%Docstring
Zoom to next view extent
%End

    virtual void zoomToActiveLayer() = 0;
%Docstring
Zoom to extent of the active layer
%End

    virtual QgsVectorLayer *addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey ) = 0;
%Docstring
Add a vector layer
 :rtype: QgsVectorLayer
%End

    virtual QgsRasterLayer *addRasterLayer( const QString &rasterLayerPath, const QString &baseName = QString() ) = 0;
%Docstring
Add a raster layer given a raster layer file name
 :rtype: QgsRasterLayer
%End

    virtual QgsRasterLayer *addRasterLayer( const QString &url, const QString &layerName, const QString &providerKey ) = 0;
%Docstring
Add a WMS layer
 :rtype: QgsRasterLayer
%End

    virtual bool addProject( const QString &project ) = 0;
%Docstring
Add a project
 :rtype: bool
%End
    virtual void newProject( bool promptToSaveFlag = false ) = 0;
%Docstring
Start a blank project
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
 returns true if the layer exists, false otherwise
 :rtype: bool
%End

    virtual int addToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the plugins toolbar
 :rtype: int
%End

    virtual QAction *addToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
 Add a widget to the plugins toolbar.
 To remove this widget again, call removeToolBarIcon()
 with the returned QAction.

 \param widget widget to add. The toolbar will take ownership of this widget
 :return: the QAction you can use to remove this widget from the toolbar
 :rtype: QAction
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

 \param widget widget to add. The toolbar will take ownership of this widget
 :return: the QAction you can use to remove this widget from the toolbar
 :rtype: QAction
%End

    virtual int addRasterToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the Raster toolbar
 :rtype: int
%End

    virtual void removeRasterToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the Raster toolbar
%End

    virtual int addVectorToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the Vector toolbar
 :rtype: int
%End

    virtual QAction *addVectorToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
 Add a widget to the vector toolbar.
 To remove this widget again, call removeVectorToolBarIcon()
 with the returned QAction.

 \param widget widget to add. The toolbar will take ownership of this widget
 :return: the QAction you can use to remove this widget from the toolbar
 :rtype: QAction
%End

    virtual void removeVectorToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the Vector toolbar
%End

    virtual int addDatabaseToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the Database toolbar
 :rtype: int
%End

    virtual QAction *addDatabaseToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
 Add a widget to the database toolbar.
 To remove this widget again, call removeDatabaseToolBarIcon()
 with the returned QAction.

 \param widget widget to add. The toolbar will take ownership of this widget
 :return: the QAction you can use to remove this widget from the toolbar
 :rtype: QAction
%End

    virtual void removeDatabaseToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the Database toolbar
%End

    virtual int addWebToolBarIcon( QAction *qAction ) = 0;
%Docstring
Add an icon to the Web toolbar
 :rtype: int
%End

    virtual QAction *addWebToolBarWidget( QWidget *widget /Transfer/ ) = 0;
%Docstring
 Add a widget to the web toolbar.
 To remove this widget again, call removeWebToolBarIcon()
 with the returned QAction.

 \param widget widget to add. The toolbar will take ownership of this widget
 :return: the QAction you can use to remove this widget from the toolbar
 :rtype: QAction
%End

    virtual void removeWebToolBarIcon( QAction *qAction ) = 0;
%Docstring
Remove an action (icon) from the Web toolbar
%End

    virtual QToolBar *addToolBar( const QString &name ) = 0 /Factory/;
%Docstring
Add toolbar with specified name
 :rtype: QToolBar
%End

    virtual void addToolBar( QToolBar *toolbar /Transfer/, Qt::ToolBarArea area = Qt::TopToolBarArea ) = 0;
%Docstring
 Add a toolbar
.. versionadded:: 2.3
%End

    virtual void openMessageLog() = 0;
%Docstring
Open the message log dock widget *
%End

    virtual void addUserInputWidget( QWidget *widget ) = 0;
%Docstring
Adds a widget to the user input tool bar.
%End

    virtual QgsComposerInterface *openComposer( QgsComposition *composition ) = 0;
%Docstring
 Opens a new composer window for the specified ``composition``, or
 brings an already open composer window to the foreground if one
 is already created for the composition.
.. versionadded:: 3.0
.. seealso:: closeComposer()
 :rtype: QgsComposerInterface
%End

    virtual void closeComposer( QgsComposition *composition ) = 0;
%Docstring
 Closes an open composer window showing the specified ``composition``.
 The composition remains unaffected.
.. versionadded:: 3.0
.. seealso:: openComposer()
%End

    virtual QgsLayoutDesignerInterface *openLayoutDesigner( QgsLayout *layout ) = 0;
%Docstring
 Opens a new layout designer dialog for the specified ``layout``, or
 brings an already open designer window to the foreground if one
 is already created for the layout.
.. versionadded:: 3.0
.. seealso:: closeComposer()
 :rtype: QgsLayoutDesignerInterface
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
 \param opts generated default option values, or a changed copy of them
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
Add a dock widget to the main window
%End

    virtual void removeDockWidget( QDockWidget *dockwidget ) = 0;
%Docstring
Remove specified dock widget from main window (doesn't delete it).
%End

    virtual void showLayerProperties( QgsMapLayer *l ) = 0;
%Docstring
Open layer properties dialog
%End

    virtual QDialog *showAttributeTable( QgsVectorLayer *l, const QString &filterExpression = QString() ) = 0;
%Docstring
Open attribute table dialog
 :rtype: QDialog
%End

    virtual void addWindow( QAction *action ) = 0;
%Docstring
 Add window to Window menu. The action title is the window title
 and the action should raise, unminimize and activate the window. *
%End

    virtual void removeWindow( QAction *action ) = 0;
%Docstring
 Remove window from Window menu. Calling this is necessary only for
 windows which are hidden rather than deleted when closed. *
%End

    virtual bool registerMainWindowAction( QAction *action, const QString &defaultShortcut ) = 0;
%Docstring
Register action to the shortcuts manager so its shortcut can be changed in GUI
 :rtype: bool
%End

    virtual bool unregisterMainWindowAction( QAction *action ) = 0;
%Docstring
Unregister a previously registered action. (e.g. when plugin is going to be unloaded)
 :rtype: bool
%End

    virtual void registerMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) = 0;
%Docstring
 Register a new tab in the vector layer properties dialog.
.. versionadded:: 2.16
.. note::

   Ownership of the factory is not transferred, and the factory must
       be unregistered when plugin is unloaded.
.. seealso:: unregisterMapLayerPropertiesFactory() *
%End

    virtual void unregisterMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) = 0;
%Docstring
 Unregister a previously registered tab in the vector layer properties dialog.
.. versionadded:: 2.16
.. seealso:: registerMapLayerPropertiesFactory()
%End

    virtual void registerOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;
%Docstring
 Register a new tab in the options dialog.
.. versionadded:: 3.0
.. note::

   Ownership of the factory is not transferred, and the factory must
       be unregistered when plugin is unloaded.
.. seealso:: unregisterOptionsWidgetFactory() *
%End

    virtual void unregisterOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) = 0;
%Docstring
 Unregister a previously registered tab in the options dialog.
.. versionadded:: 3.0
.. seealso:: registerOptionsWidgetFactory()
%End

    virtual void registerCustomDropHandler( QgsCustomDropHandler *handler ) = 0;
%Docstring
 Register a new custom drop handler.
.. versionadded:: 3.0
.. note::

   Ownership of the factory is not transferred, and the factory must
       be unregistered when plugin is unloaded.
.. seealso:: unregisterCustomDropHandler() *
%End

    virtual void unregisterCustomDropHandler( QgsCustomDropHandler *handler ) = 0;
%Docstring
 Unregister a previously registered custom drop handler.
.. versionadded:: 3.0
.. seealso:: registerCustomDropHandler() *
%End


    virtual void openURL( const QString &url, bool useQgisDocDirectory = true ) = 0 /Deprecated/;
%Docstring
 Open a url in the users browser. By default the QGIS doc directory is used
 as the base for the URL. To open a URL that is not relative to the installed
 QGIS documentation, set useQgisDocDirectory to false.
 \param url URL to open
 \param useQgisDocDirectory If true, the URL will be formed by concatenating
 url to the QGIS documentation directory path (prefix/share/doc)
 \deprecated
%End

    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false, bool showModal = true ) = 0;
%Docstring
 Open feature form
 \param l vector layer
 \param f feature to show/modify
 \param updateFeatureOnly only update the feature update (don't change any attributes of the layer) [UNUSED]
 \param showModal if true, will wait for the dialog to be executed (only shown otherwise)
 :rtype: bool
%End

    virtual QgsAttributeDialog *getFeatureForm( QgsVectorLayer *l, QgsFeature &f ) = 0;
%Docstring
 Returns a feature form for a given feature

 \param l The layer for which the dialog will be created
 \param f The feature for which the dialog will be created

 :return: A feature form
 :rtype: QgsAttributeDialog
%End

    virtual void preloadForm( const QString &uifile ) = 0;
%Docstring
 This method is only needed when using a UI form with a custom widget plugin and calling
 openFeatureForm or getFeatureForm from Python (PyQt4) and you haven't used the info tool first.
 Python will crash bringing QGIS with it
 if the custom form is not loaded from a C++ method call.

 This method uses a QTimer to call QUiLoader in order to load the form via C++
 you only need to call this once after that you can call openFeatureForm/getFeatureForm
 like normal

 More information here: http://qt-project.org/forums/viewthread/27098
%End

    virtual void registerLocatorFilter( QgsLocatorFilter *filter /Transfer/ ) = 0;
%Docstring
 Registers a locator ``filter`` for the app's locator bar. Ownership of the filter is transferred to the
 locator.
 \warning Plugins which register filters to the locator bar must take care to correctly call
 deregisterLocatorFilter() and deregister their filters upon plugin unload to avoid crashes.
.. seealso:: deregisterLocatorFilter()
.. versionadded:: 3.0
%End

    virtual void deregisterLocatorFilter( QgsLocatorFilter *filter ) = 0;
%Docstring
 Deregisters a locator ``filter`` from the app's locator bar and deletes it. Calling this will block whilst
 any currently running query is terminated.

 Plugins which register filters to the locator bar must take care to correctly call
 deregisterLocatorFilter() to deregister their filters upon plugin unload to avoid crashes.

.. seealso:: registerLocatorFilter()
.. versionadded:: 3.0
%End

  signals:

    void currentLayerChanged( QgsMapLayer *layer );
%Docstring
 Emitted whenever current (selected) layer changes.
  The pointer to layer can be null if no layer is selected
%End

    void currentThemeChanged( const QString &theme );
%Docstring
 Signal emitted when the current ``theme`` is changed so plugins
 can change their tool button icons.
.. versionadded:: 3.0
%End

    void composerOpened( QgsComposerInterface *composer );
%Docstring
 This signal is emitted when a new composer window has been opened.
.. versionadded:: 3.0
.. seealso:: composerWillBeClosed()
%End

    void composerWillBeClosed( QgsComposerInterface *composer );
%Docstring
 This signal is emitted before a composer window is going to be closed
 and deleted.
.. versionadded:: 3.0
.. seealso:: composerClosed()
.. seealso:: composerOpened()
%End

    void composerClosed( QgsComposerInterface *composer );
%Docstring
 This signal is emitted after a composer window is closed.
.. versionadded:: 3.0
.. seealso:: composerWillBeClosed()
.. seealso:: composerOpened()
%End

    void layoutDesignerOpened( QgsLayoutDesignerInterface *designer );
%Docstring
 This signal is emitted when a new layout ``designer`` has been opened.
.. versionadded:: 3.0
.. seealso:: layoutDesignerWillBeClosed()
%End

    void layoutDesignerWillBeClosed( QgsLayoutDesignerInterface *designer );
%Docstring
 This signal is emitted before a layout ``designer`` is going to be closed
 and deleted.
.. versionadded:: 3.0
.. seealso:: layoutDesignerClosed()
.. seealso:: layoutDesignerOpened()
%End

    void layoutDesignerClosed();
%Docstring
 This signal is emitted after a layout designer window is closed.
.. versionadded:: 3.0
.. seealso:: layoutDesignerWillBeClosed()
.. seealso:: layoutDesignerOpened()
%End

    void initializationCompleted();
%Docstring
 This signal is emitted when the initialization is complete
%End

    void projectRead();
%Docstring
 Emitted when a project file is successfully read
 \note
 This is useful for plug-ins that store properties with project files.  A
 plug-in can connect to this signal.  When it is emitted, the plug-in
 knows to then check the project properties for any relevant state.
%End

    void newProjectCreated();
%Docstring
 Emitted when starting an entirely new project
 \note
 This is similar to projectRead(); plug-ins might want to be notified
 that they're in a new project.  Yes, projectRead() could have been
 overloaded to be used in the case of new projects instead.  However,
 it's probably more semantically correct to have an entirely separate
 signal for when this happens.
%End

    void layerSavedAs( QgsMapLayer *l, const QString &path );
%Docstring
 This signal is emitted when a layer has been saved using save as
 \note
 added in version 2.7
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgisinterface.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
