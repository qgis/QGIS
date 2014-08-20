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

class QgsAttributeDialog;
class QgsComposerView;
class QgsFeature;
class QgsLayerTreeView;
class QgsLegendInterface;
class QgsMapCanvas;
class QgsMapLayer;
class QgsMessageBar;
class QgsPluginManagerInterface;
class QgsRasterLayer;
class QgsVectorLayer;
class QgsVectorLayerTools;

#include <QObject>
#include <QFont>
#include <QPair>
#include <map>

#include <qgis.h>


/** \ingroup gui
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

class GUI_EXPORT QgisInterface : public QObject
{
    Q_OBJECT

  public:

    /** Constructor */
    QgisInterface();

    /** Virtual destructor */
    virtual ~QgisInterface();

    /** Get pointer to legend interface
      \note added in 1.4
     */
    virtual QgsLegendInterface* legendInterface() = 0;

    virtual QgsPluginManagerInterface* pluginManagerInterface() = 0;

    virtual QgsLayerTreeView* layerTreeView() = 0;

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
    virtual QgsVectorLayer* addVectorLayer( QString vectorLayerPath, QString baseName, QString providerKey ) = 0;

    //! Add a raster layer given a raster layer file name
    virtual QgsRasterLayer* addRasterLayer( QString rasterLayerPath, QString baseName = QString() ) = 0;

    //! Add a WMS layer
    virtual QgsRasterLayer* addRasterLayer( const QString& url, const QString& layerName, const QString& providerKey ) = 0;

    //! Add a project
    virtual bool addProject( QString theProject ) = 0;
    //! Start a blank project
    virtual void newProject( bool thePromptToSaveFlag = false ) = 0;

    //! Get pointer to the active layer (layer selected in the legend)
    virtual QgsMapLayer *activeLayer() = 0;

    //! Set the active layer (layer gets selected in the legend)
    //! returns true if the layer exists, false otherwise
    //! added in 1.4
    virtual bool setActiveLayer( QgsMapLayer * ) = 0;

    //! Add an icon to the plugins toolbar
    virtual int addToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the plugins toolbar.
     * To remove this widget again, call {@link removeToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction* addToolBarWidget( QWidget* widget ) = 0;

    //! Remove an action (icon) from the plugin toolbar
    virtual void removeToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the raster toolbar.
     * To remove this widget again, call {@link removeRasterToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction* addRasterToolBarWidget( QWidget* widget ) = 0;

    //! Add an icon to the Raster toolbar
    //! @note added in 2.0
    virtual int addRasterToolBarIcon( QAction *qAction ) = 0;

    //! Remove an action (icon) from the Raster toolbar
    //! @note added in 2.0
    virtual void removeRasterToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Vector toolbar
    //! @note added in 2.0

    virtual int addVectorToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the vector toolbar.
     * To remove this widget again, call {@link removeVectorToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction* addVectorToolBarWidget( QWidget* widget ) = 0;

    //! Remove an action (icon) from the Vector toolbar
    //! @note added in 2.0
    virtual void removeVectorToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Database toolbar
    //! @note added in 2.0
    virtual int addDatabaseToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the database toolbar.
     * To remove this widget again, call {@link removeDatabaseToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction* addDatabaseToolBarWidget( QWidget* widget ) = 0;

    //! Remove an action (icon) from the Database toolbar
    //! @note added in 2.0
    virtual void removeDatabaseToolBarIcon( QAction *qAction ) = 0;

    //! Add an icon to the Web toolbar
    //! @note added in 2.0
    virtual int addWebToolBarIcon( QAction *qAction ) = 0;

    /**
     * Add a widget to the web toolbar.
     * To remove this widget again, call {@link removeWebToolBarIcon}
     * with the returned QAction.
     *
     * @param widget widget to add. The toolbar will take ownership of this widget
     * @return the QAction you can use to remove this widget from the toolbar
     */
    virtual QAction* addWebToolBarWidget( QWidget* widget ) = 0;

    //! Remove an action (icon) from the Web toolbar
    //! @note added in 2.0
    virtual void removeWebToolBarIcon( QAction *qAction ) = 0;

    //! Add toolbar with specified name
    virtual QToolBar *addToolBar( QString name ) = 0;

    //! Add a toolbar
    //! @note added in 2.3
    virtual void addToolBar( QToolBar* toolbar, Qt::ToolBarArea area = Qt::TopToolBarArea ) = 0;

    /** Return a pointer to the map canvas */
    virtual QgsMapCanvas * mapCanvas() = 0;

    /** Return a pointer to the main window (instance of QgisApp in case of QGIS) */
    virtual QWidget * mainWindow() = 0;

    /** Return the message bar of the main app */
    virtual QgsMessageBar * messageBar() = 0;

    /** Return mainwindows / composer views of running composer instances (currently only one) */
    virtual QList<QgsComposerView*> activeComposers() = 0;

    /** Create a new composer
     * @param title window title for new composer (one will be generated if empty)
     * @return pointer to composer's view
     * @note new composer window will be shown and activated (added in 1.9)
     */
    virtual QgsComposerView* createNewComposer( QString title = QString( "" ) ) = 0;

    /** Duplicate an existing parent composer from composer view
     * @param composerView pointer to existing composer view
     * @param title window title for duplicated composer (one will be generated if empty)
     * @return pointer to duplicate composer's view
     * @note dupicate composer window will be hidden until loaded, then shown and activated (added in 1.9)
     */
    virtual QgsComposerView* duplicateComposer( QgsComposerView* composerView, QString title = QString( "" ) ) = 0;

    /** Deletes parent composer of composer view, after closing composer window
     * @note (added in 1.9)
     */
    virtual void deleteComposer( QgsComposerView* composerView ) = 0;

    /** Return changeable options built from settings and/or defaults
     * @note (added in 1.9)
     */
    virtual QMap<QString, QVariant> defaultStyleSheetOptions() = 0;

    /** Generate stylesheet
     * @param opts generated default option values, or a changed copy of them
     * @note added in 1.9
     */
    virtual void buildStyleSheet( const QMap<QString, QVariant>& opts ) = 0;

    /** Save changed default option keys/values to user settings
      * @note added in 1.9
      */
    virtual void saveStyleSheetOptions( const QMap<QString, QVariant>& opts ) = 0;

    /** Get reference font for initial qApp (may not be same as QgisApp)
     * @note added in 1.9
     */
    virtual QFont defaultStyleSheetFont() = 0;

    /** Add action to the plugins menu */
    virtual void addPluginToMenu( QString name, QAction* action ) = 0;

    /** Remove action from the plugins menu */
    virtual void removePluginMenu( QString name, QAction* action ) = 0;

    /** Add "add layer" action to layer menu
     * @note added in 1.7
     */
    virtual void insertAddLayerAction( QAction *action ) = 0;

    /** Remove "add layer" action from layer menu
     * @note added in 1.7
     */
    virtual void removeAddLayerAction( QAction *action ) = 0;

    /** Add action to the Database menu
     * @note added in 1.7
     */
    virtual void addPluginToDatabaseMenu( QString name, QAction* action ) = 0;

    /** Remove action from the Database menu
     * @note added in 1.7
     */
    virtual void removePluginDatabaseMenu( QString name, QAction* action ) = 0;

    /** Add action to the Raster menu
     * @note added in 2.0
     */
    virtual void addPluginToRasterMenu( QString name, QAction* action ) = 0;

    /** Remove action from the Raster menu
     * @note added in 2.0
     */
    virtual void removePluginRasterMenu( QString name, QAction* action ) = 0;

    /** Add action to the Vector menu
     * @note added in 2.0
     */
    virtual void addPluginToVectorMenu( QString name, QAction* action ) = 0;

    /** Remove action from the Vector menu
     * @note added in 2.0
     */
    virtual void removePluginVectorMenu( QString name, QAction* action ) = 0;

    /** Add action to the Web menu
     * @note added in 2.0
     */
    virtual void addPluginToWebMenu( QString name, QAction* action ) = 0;

    /** Remove action from the Web menu
     * @note added in 2.0
     */
    virtual void removePluginWebMenu( QString name, QAction* action ) = 0;

    /** Add a dock widget to the main window */
    virtual void addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget ) = 0;

    /** Remove specified dock widget from main window (doesn't delete it).
     * @note Added in 1.1
     */
    virtual void removeDockWidget( QDockWidget * dockwidget ) = 0;

    /** open layer properties dialog
     \note added in 1.5
     */
    virtual void showLayerProperties( QgsMapLayer *l ) = 0;

    /** open attribute table dialog
     \note added in 1.7
     */
    virtual void showAttributeTable( QgsVectorLayer *l ) = 0;

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

    // @todo is this deprecated in favour of QgsContextHelp?
    /** Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to false.
     * @param url URL to open
     * @param useQgisDocDirectory If true, the URL will be formed by concatenating
     * url to the QGIS documentation directory path (prefix/share/doc)
     * @deprecated
     */
#ifndef Q_MOC_RUN
    Q_DECL_DEPRECATED
#endif
    virtual void openURL( QString url, bool useQgisDocDirectory = true ) = 0;


    /** Accessors for inserting items into menus and toolbars.
     * An item can be inserted before any existing action.
     */

    // Menus
#ifndef Q_MOC_RUN
    Q_DECL_DEPRECATED
#endif
    virtual QMenu *fileMenu() = 0;
    virtual QMenu *projectMenu() = 0;
    virtual QMenu *editMenu() = 0;
    virtual QMenu *viewMenu() = 0;
    virtual QMenu *layerMenu() = 0;
    /** \note added in 2.0
    */
    virtual QMenu *newLayerMenu() = 0;
    virtual QMenu *settingsMenu() = 0;
    virtual QMenu *pluginMenu() = 0;
    virtual QMenu *rasterMenu() = 0;
    /** \note added in 1.7
    */
    virtual QMenu *databaseMenu() = 0;
    /** \note added in 2.0
    */
    virtual QMenu *vectorMenu() = 0;
    /** \note added in 2.0
    */
    virtual QMenu *webMenu() = 0;
    virtual QMenu *firstRightStandardMenu() = 0;
    virtual QMenu *windowMenu() = 0;
    virtual QMenu *helpMenu() = 0;

    // ToolBars
    virtual QToolBar *fileToolBar() = 0;
    virtual QToolBar *layerToolBar() = 0;
    virtual QToolBar *mapNavToolToolBar() = 0;
    virtual QToolBar *digitizeToolBar() = 0;
    virtual QToolBar *advancedDigitizeToolBar() = 0; // added in v1.5
    virtual QToolBar *attributesToolBar() = 0;
    virtual QToolBar *pluginToolBar() = 0;
    virtual QToolBar *helpToolBar() = 0;
    /** \note added in 1.7
    */
    virtual QToolBar *rasterToolBar() = 0;
    /** \note added in 2.0
    */
    virtual QToolBar *vectorToolBar() = 0;
    /** \note added in 2.0
    */
    virtual QToolBar *databaseToolBar() = 0;
    /** \note added in 2.0
    */
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
    //! Get access to the native touch action.
    virtual QAction *actionTouch() = 0;
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
    //! Get access to the native zoom actual size action. Call trigger() on it to zoom to actual size.
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
    /** @note added in 1.9 */
    virtual QAction *actionCopyLayerStyle() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionPasteLayerStyle() = 0;
    virtual QAction *actionOpenTable() = 0;
    virtual QAction *actionOpenFieldCalculator() = 0;
    virtual QAction *actionToggleEditing() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionSaveActiveLayerEdits() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionAllEdits() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionSaveEdits() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionSaveAllEdits() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionRollbackEdits() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionRollbackAllEdits() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionCancelEdits() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionCancelAllEdits() = 0;
    virtual QAction *actionLayerSaveAs() = 0;
    /** @deprecated in 2.4 - returns null pointer */
#ifndef Q_MOC_RUN
    Q_DECL_DEPRECATED
#endif
    virtual QAction *actionLayerSelectionSaveAs() = 0;
    virtual QAction *actionRemoveLayer() = 0;
    /** @note added in 1.9 */
    virtual QAction *actionDuplicateLayer() = 0;
    virtual QAction *actionLayerProperties() = 0;
    virtual QAction *actionAddToOverview() = 0;
    virtual QAction *actionAddAllToOverview() = 0;
    virtual QAction *actionRemoveAllFromOverview() = 0;
    virtual QAction *actionHideAllLayers() = 0;
    virtual QAction *actionShowAllLayers() = 0;

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
     * Open feature form
     * @param l vector layer
     * @param f feature to show/modify
     * @param updateFeatureOnly only update the feature update (don't change any attributes of the layer) [UNUSED]
     * @param showModal if true, will wait for the dialog to be executed (only shown otherwise)
     * @note added in 1.6
     */
    virtual bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false, bool showModal = true ) = 0;

    /**
     * Returns a feature form for a given feature
     *
     * @param l The layer for which the dialog will be created
     * @param f The feature for which the dialog will be created
     *
     * @return A feature form
     */
    virtual QgsAttributeDialog* getFeatureForm( QgsVectorLayer *l, QgsFeature &f ) = 0;

    /**
     * Access the vector layer tools instance.
     * With the help of this you can access methods like addFeature, startEditing
     * or stopEditing while giving the user the appropriate dialogs.
     *
     * @return An instance of the vector layer tools
     */
    virtual QgsVectorLayerTools* vectorLayerTools() = 0;

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
    virtual void preloadForm( QString uifile ) = 0;

    /** Return vector layers in edit mode
     * @param modified whether to return only layers that have been modified
     * @returns list of layers in legend order, or empty list
     * @note added in 1.9 */
    virtual QList<QgsMapLayer *> editableLayers( bool modified = false ) const = 0;

    /** Get timeout for timed messages: default of 5 seconds
     * @note added in 1.9 */
    virtual int messageTimeout() = 0;

  signals:
    /** Emitted whenever current (selected) layer changes.
     *  The pointer to layer can be null if no layer is selected
     */
    void currentLayerChanged( QgsMapLayer * layer );

    /**
     * This signal is emitted when a new composer instance has been created
     * @note added in 1.4
     */
    void composerAdded( QgsComposerView* v );

    /**
     * This signal is emitted before a new composer instance is going to be removed
     * @note added in 1.4
     */
    void composerWillBeRemoved( QgsComposerView* v );
    /**
     * This signal is emitted when the initialization is complete
     * @note added in 1.6
     */
    void initializationCompleted();
    /** emitted when a project file is successfully read
        @note
        This is useful for plug-ins that store properties with project files.  A
        plug-in can connect to this signal.  When it is emitted, the plug-in
        knows to then check the project properties for any relevant state.

        Added in 1.6
     */
    void projectRead();
    /** emitted when starting an entirely new project
        @note
        This is similar to projectRead(); plug-ins might want to be notified
        that they're in a new project.  Yes, projectRead() could have been
        overloaded to be used in the case of new projects instead.  However,
        it's probably more semantically correct to have an entirely separate
        signal for when this happens.

        Added in 1.6
      */
    void newProjectCreated();
};

// FIXME: also in core/qgis.h
#ifndef QGISEXTERN
#ifdef WIN32
#  define QGISEXTERN extern "C" __declspec( dllexport )
#else
#  define QGISEXTERN extern "C"
#endif
#endif

#endif //#ifndef QGISINTERFACE_H
