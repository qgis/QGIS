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

#include "qgis_app.h"
#include "qgisinterface.h"
#include "qgsapppluginmanagerinterface.h"

class QTimer;

class QgisApp;

/**
 * \class QgisAppInterface
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
     * \param qgis Pointer to the QgisApp object
     */
    QgisAppInterface( QgisApp *qgisapp );

    QgisAppInterface( QgisAppInterface const & ) = delete;
    QgisAppInterface &operator=( QgisAppInterface const & ) = delete;

    QgsPluginManagerInterface *pluginManagerInterface() override;

    QgsLayerTreeView *layerTreeView() override;

    void addCustomActionForLayerType( QAction *action, QString menu,
                                      QgsMapLayerType type, bool allLayers ) override;
    void addCustomActionForLayer( QAction *action, QgsMapLayer *layer ) override;
    bool removeCustomActionForLayerType( QAction *action ) override;

    /* Exposed functions */

    void zoomFull() override;
    void zoomToPrevious() override;
    void zoomToNext() override;
    void zoomToActiveLayer() override;
    QgsVectorLayer *addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey ) override;
    QgsRasterLayer *addRasterLayer( const QString &rasterLayerPath, const QString &baseName ) override;
    QgsRasterLayer *addRasterLayer( const QString &url, const QString &baseName, const QString &providerKey ) override;
    QgsMeshLayer *addMeshLayer( const QString &url, const QString &baseName, const QString &providerKey ) override;
    QgsVectorTileLayer *addVectorTileLayer( const QString &url, const QString &baseName ) override;
    QgsPointCloudLayer *addPointCloudLayer( const QString &url, const QString &baseName, const QString &providerKey ) override;
    bool addProject( const QString &projectName ) override;
    bool newProject( bool promptToSaveFlag = false ) override;
    void reloadConnections( ) override;
    QgsMapLayer *activeLayer() override;
    bool setActiveLayer( QgsMapLayer *layer ) override;
    void copySelectionToClipboard( QgsMapLayer *layer ) override;
    void pasteFromClipboard( QgsMapLayer *layer ) override;
    int addToolBarIcon( QAction *qAction ) override;
    QAction *addToolBarWidget( QWidget *widget ) override;
    void removeToolBarIcon( QAction *qAction ) override;
    int addRasterToolBarIcon( QAction *qAction ) override;
    QAction *addRasterToolBarWidget( QWidget *widget ) override;
    void removeRasterToolBarIcon( QAction *qAction ) override;
    int addVectorToolBarIcon( QAction *qAction ) override;
    QAction *addVectorToolBarWidget( QWidget *widget ) override;
    void removeVectorToolBarIcon( QAction *qAction ) override;
    int addDatabaseToolBarIcon( QAction *qAction ) override;
    QAction *addDatabaseToolBarWidget( QWidget *widget ) override;
    void removeDatabaseToolBarIcon( QAction *qAction ) override;
    int addWebToolBarIcon( QAction *qAction ) override;
    QAction *addWebToolBarWidget( QWidget *widget ) override;
    void removeWebToolBarIcon( QAction *qAction ) override;
    QToolBar *addToolBar( const QString &name ) override;
    void addToolBar( QToolBar *toolbar, Qt::ToolBarArea area = Qt::TopToolBarArea ) override;

#ifndef Q_MOC_RUN
    Q_DECL_DEPRECATED
#endif
    void openURL( const QString &url, bool useQgisDocDirectory = true ) override;

    QgsMapCanvas *mapCanvas() override;
    QList< QgsMapCanvas * > mapCanvases() override;
    QgsMapCanvas *createNewMapCanvas( const QString &name ) override;
    void closeMapCanvas( const QString &name ) override;
    QSize iconSize( bool dockedToolbar = false ) const override;
    QgsLayerTreeMapCanvasBridge *layerTreeCanvasBridge() override;
    QWidget *mainWindow() override;
    QgsMessageBar *messageBar() override;
    void openMessageLog() override;
    void addUserInputWidget( QWidget *widget ) override;
    void showLayoutManager() override;
    QList<QgsLayoutDesignerInterface *> openLayoutDesigners() override;
    QgsLayoutDesignerInterface *openLayoutDesigner( QgsMasterLayoutInterface *layout ) override;
    void showOptionsDialog( QWidget *parent = nullptr, const QString &currentPage = QString() ) override;
    void showProjectPropertiesDialog( const QString &currentPage = QString() ) override;
    QMap<QString, QVariant> defaultStyleSheetOptions() override;
    void buildStyleSheet( const QMap<QString, QVariant> &opts ) override;
    void saveStyleSheetOptions( const QMap<QString, QVariant> &opts ) override;
    QFont defaultStyleSheetFont() override;
    void addPluginToMenu( const QString &name, QAction *action ) override;
    void removePluginMenu( const QString &name, QAction *action ) override;
    void addPluginToDatabaseMenu( const QString &name, QAction *action ) override;
    void removePluginDatabaseMenu( const QString &name, QAction *action ) override;
    void addPluginToRasterMenu( const QString &name, QAction *action ) override;
    void removePluginRasterMenu( const QString &name, QAction *action ) override;
    void addPluginToVectorMenu( const QString &name, QAction *action ) override;
    void removePluginVectorMenu( const QString &name, QAction *action ) override;
    void addPluginToWebMenu( const QString &name, QAction *action ) override;
    void removePluginWebMenu( const QString &name, QAction *action ) override;
    void addPluginToMeshMenu( const QString &name, QAction *action ) override;
    void removePluginMeshMenu( const QString &name, QAction *action ) override;
    void insertAddLayerAction( QAction *action ) override;
    void removeAddLayerAction( QAction *action ) override;
    void addDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget ) override;
    void addTabifiedDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget, const QStringList &tabifyWith = QStringList(), bool raiseTab = false ) override;
    void removeDockWidget( QDockWidget *dockwidget ) override;
    QgsAdvancedDigitizingDockWidget *cadDockWidget() override;
    void showLayerProperties( QgsMapLayer *l, const QString &page = QString() ) override;
    QDialog *showAttributeTable( QgsVectorLayer *l, const QString &filterExpression = QString() ) override;
    void addWindow( QAction *action ) override;
    void removeWindow( QAction *action ) override;
    bool registerMainWindowAction( QAction *action, const QString &defaultShortcut ) override;
    bool unregisterMainWindowAction( QAction *action ) override;
    void registerMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) override;
    void unregisterMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory ) override;
    void registerOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) override;
    void unregisterOptionsWidgetFactory( QgsOptionsWidgetFactory *factory ) override;
    void registerProjectPropertiesWidgetFactory( QgsOptionsWidgetFactory *factory ) override;
    void unregisterProjectPropertiesWidgetFactory( QgsOptionsWidgetFactory *factory ) override;
    void registerDevToolWidgetFactory( QgsDevToolWidgetFactory *factory ) override;
    void unregisterDevToolWidgetFactory( QgsDevToolWidgetFactory *factory ) override;
    void registerApplicationExitBlocker( QgsApplicationExitBlockerInterface *blocker ) override;
    void unregisterApplicationExitBlocker( QgsApplicationExitBlockerInterface *blocker ) override;
    void registerMapToolHandler( QgsAbstractMapToolHandler *handler ) override;
    void unregisterMapToolHandler( QgsAbstractMapToolHandler *handler ) override;
    void registerCustomDropHandler( QgsCustomDropHandler *handler ) override;
    void unregisterCustomDropHandler( QgsCustomDropHandler *handler ) override;
    void registerCustomProjectOpenHandler( QgsCustomProjectOpenHandler *handler ) override;
    void unregisterCustomProjectOpenHandler( QgsCustomProjectOpenHandler *handler ) override;
    void registerCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler ) override;
    void unregisterCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler ) override;
    QMenu *projectMenu() override;
    QMenu *editMenu() override;
    QMenu *viewMenu() override;
    QMenu *layerMenu() override;
    QMenu *newLayerMenu() override;
    QMenu *addLayerMenu() override;
    QMenu *settingsMenu() override;
    QMenu *pluginMenu() override;
    QMenu *pluginHelpMenu() override;
    QMenu *rasterMenu() override;
    QMenu *vectorMenu() override;
    QMenu *databaseMenu() override;
    QMenu *webMenu() override;
    QMenu *firstRightStandardMenu() override;
    QMenu *windowMenu() override;
    QMenu *helpMenu() override;
    QToolBar *fileToolBar() override;
    QToolBar *layerToolBar() override;
    QToolBar *dataSourceManagerToolBar() override;
    QToolBar *mapNavToolToolBar() override;
    QToolBar *digitizeToolBar() override;
    QToolBar *advancedDigitizeToolBar() override;
    QToolBar *shapeDigitizeToolBar() override;
    QToolBar *attributesToolBar() override;
    QToolBar *selectionToolBar() override;
    QToolBar *pluginToolBar() override;
    QToolBar *helpToolBar() override;
    QToolBar *rasterToolBar() override;
    QToolBar *vectorToolBar() override;
    QToolBar *databaseToolBar() override;
    QToolBar *webToolBar() override;
    QActionGroup *mapToolActionGroup() override;
    QAction *actionNewProject() override;
    QAction *actionOpenProject() override;
    QAction *actionSaveProject() override;
    QAction *actionSaveProjectAs() override;
    QAction *actionSaveMapAsImage() override;
    QAction *actionProjectProperties() override;
    QAction *actionCreatePrintLayout() override;
    QAction *actionShowLayoutManager() override;
    QAction *actionExit() override;
    QAction *actionCutFeatures() override;
    QAction *actionCopyFeatures() override;
    QAction *actionPasteFeatures() override;
    QAction *actionAddFeature() override;
    QAction *actionDeleteSelected() override;
    QAction *actionMoveFeature() override;
    QAction *actionSplitFeatures() override;
    QAction *actionSplitParts() override;
    QAction *actionAddRing() override;
    QAction *actionAddPart() override;
    QAction *actionSimplifyFeature() override;
    QAction *actionDeleteRing() override;
    QAction *actionDeletePart() override;
    QAction *actionVertexTool() override;
    QAction *actionVertexToolActiveLayer() override;
    QAction *actionPan() override;
    QAction *actionPanToSelected() override;
    QAction *actionZoomIn() override;
    QAction *actionZoomOut() override;
    QAction *actionSelect() override;
    QAction *actionSelectRectangle() override;
    QAction *actionSelectPolygon() override;
    QAction *actionSelectFreehand() override;
    QAction *actionSelectRadius() override;
    QAction *actionIdentify() override;
    QAction *actionFeatureAction() override;
    QAction *actionMeasure() override;
    QAction *actionMeasureArea() override;
    QAction *actionZoomFullExtent() override;
    QAction *actionZoomToLayer() override;
    QAction *actionZoomToLayers() override;
    QAction *actionZoomToSelected() override;
    QAction *actionZoomLast() override;
    QAction *actionZoomNext() override;
    QAction *actionZoomActualSize() override;
    QAction *actionMapTips() override;
    QAction *actionNewBookmark() override;
    QAction *actionShowBookmarks() override;
    QAction *actionDraw() override;
    QAction *actionNewVectorLayer() override;
    QAction *actionAddOgrLayer() override;
    QAction *actionAddRasterLayer() override;
    QAction *actionAddPgLayer() override;
    QAction *actionAddWmsLayer() override;
    QAction *actionAddXyzLayer() override;
    QAction *actionAddVectorTileLayer() override;
    QAction *actionAddPointCloudLayer() override;
    QAction *actionAddAfsLayer() override;
    QAction *actionAddAmsLayer() override;
    QAction *actionCopyLayerStyle() override;
    QAction *actionPasteLayerStyle() override;
    QAction *actionOpenTable() override;
    QAction *actionOpenFieldCalculator() override;
    QAction *actionOpenStatisticalSummary() override;
    QAction *actionToggleEditing() override;
    QAction *actionSaveActiveLayerEdits() override;
    QAction *actionAllEdits() override;
    QAction *actionSaveEdits() override;
    QAction *actionSaveAllEdits() override;
    QAction *actionRollbackEdits() override;
    QAction *actionRollbackAllEdits() override;
    QAction *actionCancelEdits() override;
    QAction *actionCancelAllEdits() override;
    QAction *actionLayerSaveAs() override;
    QAction *actionDuplicateLayer() override;
    QAction *actionLayerProperties() override;
    QAction *actionAddToOverview() override;
    QAction *actionAddAllToOverview() override;
    QAction *actionRemoveAllFromOverview() override;
    QAction *actionHideAllLayers() override;
    QAction *actionShowAllLayers() override;
    QAction *actionHideSelectedLayers() override;
    QAction *actionToggleSelectedLayers() override;
    QAction *actionToggleSelectedLayersIndependently() override;
    QAction *actionHideDeselectedLayers() override;
    QAction *actionShowSelectedLayers() override;
    QAction *actionManagePlugins() override;
    QAction *actionPluginListSeparator() override;
    QAction *actionShowPythonDialog() override;
    QAction *actionToggleFullScreen() override;
    QAction *actionOptions() override;
    QAction *actionCustomProjection() override;
    QAction *actionHelpContents() override;
    QAction *actionQgisHomePage() override;
    QAction *actionCheckQgisVersion() override;
    QAction *actionAbout() override;

    bool openFeatureForm( QgsVectorLayer *l, QgsFeature &f, bool updateFeatureOnly = false, bool showModal = true ) override;
    QgsAttributeDialog *getFeatureForm( QgsVectorLayer *layer, QgsFeature &feature ) override;
    QgsVectorLayerTools *vectorLayerTools() override;
    void preloadForm( const QString &uifile ) override;
    QList<QgsMapLayer *> editableLayers( bool modified = false ) const override;
    int messageTimeout() override;
    QgsStatusBar *statusBarIface() override;
    void locatorSearch( const QString &searchText ) override;
    void registerLocatorFilter( QgsLocatorFilter *filter ) override;
    void deregisterLocatorFilter( QgsLocatorFilter *filter ) override;
    void invalidateLocatorResults() override;
    bool askForDatumTransform( QgsCoordinateReferenceSystem sourceCrs, QgsCoordinateReferenceSystem destinationCrs ) override;
    void takeAppScreenShots( const QString &saveDirectory, const int categories = 0 ) override;
    QgsBrowserGuiModel *browserModel() override;
    QgsLayerTreeRegistryBridge::InsertionPoint layerTreeInsertionPoint() override;
    void setGpsPanelConnection( QgsGpsConnection *connection ) override;
    QList<QgsMapDecoration *> activeDecorations() override;

  private slots:

    void cacheloadForm( const QString &uifile = QString() );

  private:

    //! Pointer to the QgisApp object
    QgisApp *qgis = nullptr;

    //! Pointer to the PluginManagerInterface object
    QgsAppPluginManagerInterface pluginManagerIface;
};
Q_NOWARN_DEPRECATED_POP

#endif //#define QGISAPPINTERFACE_H
