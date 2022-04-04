/***************************************************************************
                          qgisappinterface.cpp
                          Interface class for accessing exposed functions
                          in QgisApp
                             -------------------
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFileInfo>
#include <QString>
#include <QMenu>
#include <QDialog>
#include <QAbstractButton>
#include <QSignalMapper>
#include <QTimer>
#include <QUiLoader>

#include "qgisappinterface.h"
#include "qgisappstylesheet.h"
#include "qgisapp.h"
#include "qgsapplayertreeviewmenuprovider.h"
#include "qgsdatumtransformdialog.h"
#include "qgsgui.h"
#include "qgsmaplayer.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgslayertreeview.h"
#include "qgslayoutdesignerdialog.h"
#include "qgsshortcutsmanager.h"
#include "qgsattributedialog.h"
#include "qgsfields.h"
#include "qgsvectordataprovider.h"
#include "qgsfeatureaction.h"
#include "qgsactionmanager.h"
#include "qgsattributetabledialog.h"
#include "qgslocatorwidget.h"
#include "qgslocator.h"
#include "qgsmessagebar.h"
#include "qgsappmaptools.h"
#include "qgsmaptoolmodifyannotation.h"

QgisAppInterface::QgisAppInterface( QgisApp *_qgis )
  : qgis( _qgis )
  , pluginManagerIface( _qgis->pluginManager() )
{
  // connect signals
  connect( qgis, &QgisApp::activeLayerChanged,
           this, &QgisInterface::currentLayerChanged );
  connect( qgis, &QgisApp::currentThemeChanged,
           this, &QgisAppInterface::currentThemeChanged );

  connect( qgis, &QgisApp::layoutDesignerOpened, this, &QgisAppInterface::layoutDesignerOpened );
  connect( qgis, &QgisApp::layoutDesignerWillBeClosed, this, &QgisAppInterface::layoutDesignerWillBeClosed );
  connect( qgis, &QgisApp::layoutDesignerClosed, this, &QgisAppInterface::layoutDesignerClosed );

  connect( qgis, &QgisApp::initializationCompleted,
           this, &QgisInterface::initializationCompleted );
  connect( qgis, &QgisApp::newProject,
           this, &QgisInterface::newProjectCreated );
  connect( qgis, &QgisApp::projectRead,
           this, &QgisInterface::projectRead );
  connect( qgis, &QgisApp::layerSavedAs,
           this, &QgisInterface::layerSavedAs );
}

QgsPluginManagerInterface *QgisAppInterface::pluginManagerInterface()
{
  return &pluginManagerIface;
}

QgsLayerTreeView *QgisAppInterface::layerTreeView()
{
  return qgis->layerTreeView();
}

void QgisAppInterface::addCustomActionForLayerType( QAction *action,
    QString menu, QgsMapLayerType type, bool allLayers )
{
  QgsAppLayerTreeViewMenuProvider *menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider *>( qgis->layerTreeView()->menuProvider() );
  if ( !menuProvider )
    return;

  menuProvider->addLegendLayerAction( action, menu, type, allLayers );
}

void QgisAppInterface::addCustomActionForLayer( QAction *action, QgsMapLayer *layer )
{
  QgsAppLayerTreeViewMenuProvider *menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider *>( qgis->layerTreeView()->menuProvider() );
  if ( !menuProvider )
    return;

  menuProvider->addLegendLayerActionForLayer( action, layer );
}

bool QgisAppInterface::removeCustomActionForLayerType( QAction *action )
{
  QgsAppLayerTreeViewMenuProvider *menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider *>( qgis->layerTreeView()->menuProvider() );
  if ( !menuProvider )
    return false;

  return menuProvider->removeLegendLayerAction( action );
}

void QgisAppInterface::zoomFull()
{
  qgis->zoomFull();
}

void QgisAppInterface::zoomToPrevious()
{
  qgis->zoomToPrevious();
}

void QgisAppInterface::zoomToNext()
{
  qgis->zoomToNext();
}

void QgisAppInterface::zoomToActiveLayer()
{
  qgis->zoomToLayerExtent();
}

QgsVectorLayer *QgisAppInterface::addVectorLayer( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey )
{
  QString nonNullBaseBame = baseName;
  if ( nonNullBaseBame.isEmpty() )
  {
    const QFileInfo fi( vectorLayerPath );
    nonNullBaseBame = fi.completeBaseName();
  }
  return qgis->addVectorLayer( vectorLayerPath, nonNullBaseBame, providerKey );
}

QgsRasterLayer *QgisAppInterface::addRasterLayer( const QString &rasterLayerPath, const QString &baseName )
{
  QString nonNullBaseName = baseName;
  if ( nonNullBaseName.isEmpty() )
  {
    const QFileInfo fi( rasterLayerPath );
    nonNullBaseName = fi.completeBaseName();
  }
  return qgis->addRasterLayer( rasterLayerPath, nonNullBaseName, QString() );
}

QgsRasterLayer *QgisAppInterface::addRasterLayer( const QString &url, const QString &baseName, const QString &providerKey )
{
  return qgis->addRasterLayer( url, baseName, providerKey );
}

QgsMeshLayer *QgisAppInterface::addMeshLayer( const QString &url, const QString &baseName, const QString &providerKey )
{
  return qgis->addMeshLayer( url, baseName, providerKey );
}

QgsVectorTileLayer *QgisAppInterface::addVectorTileLayer( const QString &url, const QString &baseName )
{
  return qgis->addVectorTileLayer( url, baseName );
}

QgsPointCloudLayer *QgisAppInterface::addPointCloudLayer( const QString &url, const QString &baseName, const QString &providerKey )
{
  return qgis->addPointCloudLayer( url, baseName, providerKey );
}

bool QgisAppInterface::addProject( const QString &projectName )
{
  return qgis->addProject( projectName );
}

bool QgisAppInterface::newProject( bool promptToSaveFlag )
{
  return qgis->fileNew( promptToSaveFlag );
}

void QgisAppInterface::reloadConnections()
{
  qgis->reloadConnections( );
}

QgsMapLayer *QgisAppInterface::activeLayer()
{
  return qgis->activeLayer();
}

bool QgisAppInterface::setActiveLayer( QgsMapLayer *layer )
{
  return qgis->setActiveLayer( layer );
}

void QgisAppInterface::copySelectionToClipboard( QgsMapLayer *layer )
{
  return qgis->copySelectionToClipboard( layer );
}

void QgisAppInterface::pasteFromClipboard( QgsMapLayer *layer )
{
  return qgis->pasteFromClipboard( layer );
}

void QgisAppInterface::addPluginToMenu( const QString &name, QAction *action )
{
  qgis->addPluginToMenu( name, action );
}

void QgisAppInterface::insertAddLayerAction( QAction *action )
{
  qgis->insertAddLayerAction( action );
}

void QgisAppInterface::removeAddLayerAction( QAction *action )
{
  qgis->removeAddLayerAction( action );
}

void QgisAppInterface::removePluginMenu( const QString &name, QAction *action )
{
  qgis->removePluginMenu( name, action );
}

void QgisAppInterface::addPluginToDatabaseMenu( const QString &name, QAction *action )
{
  qgis->addPluginToDatabaseMenu( name, action );
}

void QgisAppInterface::removePluginDatabaseMenu( const QString &name, QAction *action )
{
  qgis->removePluginDatabaseMenu( name, action );
}

void QgisAppInterface::addPluginToRasterMenu( const QString &name, QAction *action )
{
  qgis->addPluginToRasterMenu( name, action );
}

void QgisAppInterface::removePluginRasterMenu( const QString &name, QAction *action )
{
  qgis->removePluginRasterMenu( name, action );
}

void QgisAppInterface::addPluginToVectorMenu( const QString &name, QAction *action )
{
  qgis->addPluginToVectorMenu( name, action );
}

void QgisAppInterface::removePluginVectorMenu( const QString &name, QAction *action )
{
  qgis->removePluginVectorMenu( name, action );
}

void QgisAppInterface::addPluginToWebMenu( const QString &name, QAction *action )
{
  qgis->addPluginToWebMenu( name, action );
}

void QgisAppInterface::removePluginWebMenu( const QString &name, QAction *action )
{
  qgis->removePluginWebMenu( name, action );
}

void QgisAppInterface::addPluginToMeshMenu( const QString &name, QAction *action )
{
  qgis->addPluginToMeshMenu( name, action );
}

void QgisAppInterface::removePluginMeshMenu( const QString &name, QAction *action )
{
  qgis->removePluginMeshMenu( name, action );
}

int QgisAppInterface::addToolBarIcon( QAction *qAction )
{
  return qgis->addPluginToolBarIcon( qAction );
}

QAction *QgisAppInterface::addToolBarWidget( QWidget *widget )
{
  return qgis->addPluginToolBarWidget( widget );
}

void QgisAppInterface::removeToolBarIcon( QAction *qAction )
{
  qgis->removePluginToolBarIcon( qAction );
}

int QgisAppInterface::addRasterToolBarIcon( QAction *qAction )
{
  return qgis->addRasterToolBarIcon( qAction );
}

QAction *QgisAppInterface::addRasterToolBarWidget( QWidget *widget )
{
  return qgis->addRasterToolBarWidget( widget );
}

void QgisAppInterface::removeRasterToolBarIcon( QAction *qAction )
{
  qgis->removeRasterToolBarIcon( qAction );
}

int QgisAppInterface::addVectorToolBarIcon( QAction *qAction )
{
  return qgis->addVectorToolBarIcon( qAction );
}

QAction *QgisAppInterface::addVectorToolBarWidget( QWidget *widget )
{
  return qgis->addVectorToolBarWidget( widget );
}

void QgisAppInterface::removeVectorToolBarIcon( QAction *qAction )
{
  qgis->removeVectorToolBarIcon( qAction );
}

int QgisAppInterface::addDatabaseToolBarIcon( QAction *qAction )
{
  return qgis->addDatabaseToolBarIcon( qAction );
}

QAction *QgisAppInterface::addDatabaseToolBarWidget( QWidget *widget )
{
  return qgis->addDatabaseToolBarWidget( widget );
}

void QgisAppInterface::removeDatabaseToolBarIcon( QAction *qAction )
{
  qgis->removeDatabaseToolBarIcon( qAction );
}

int QgisAppInterface::addWebToolBarIcon( QAction *qAction )
{
  return qgis->addWebToolBarIcon( qAction );
}

QAction *QgisAppInterface::addWebToolBarWidget( QWidget *widget )
{
  return qgis->addWebToolBarWidget( widget );
}

void QgisAppInterface::removeWebToolBarIcon( QAction *qAction )
{
  qgis->removeWebToolBarIcon( qAction );
}

QToolBar *QgisAppInterface::addToolBar( const QString &name )
{
  return qgis->addToolBar( name );
}

void QgisAppInterface::addToolBar( QToolBar *toolbar, Qt::ToolBarArea area )
{
  return qgis->addToolBar( toolbar, area );
}

void QgisAppInterface::openURL( const QString &url, bool useQgisDocDirectory )
{
  qgis->openURL( url, useQgisDocDirectory );
}

QgsMapCanvas *QgisAppInterface::mapCanvas()
{
  return qgis->mapCanvas();
}

QList<QgsMapCanvas *> QgisAppInterface::mapCanvases()
{
  return qgis->mapCanvases();
}

QgsMapCanvas *QgisAppInterface::createNewMapCanvas( const QString &name )
{
  return qgis->createNewMapCanvas( name );
}

void QgisAppInterface::closeMapCanvas( const QString &name )
{
  qgis->closeMapCanvas( name );
}

QSize QgisAppInterface::iconSize( bool dockedToolbar ) const
{
  return qgis->iconSize( dockedToolbar );
}

QgsLayerTreeMapCanvasBridge *QgisAppInterface::layerTreeCanvasBridge()
{
  return qgis->layerTreeCanvasBridge();
}

QWidget *QgisAppInterface::mainWindow()
{
  return qgis;
}

QgsMessageBar *QgisAppInterface::messageBar()
{
  return qgis->messageBar();
}

void QgisAppInterface::openMessageLog()
{
  qgis->openMessageLog();
}


void QgisAppInterface::addUserInputWidget( QWidget *widget )
{
  qgis->addUserInputWidget( widget );
}

void QgisAppInterface::showLayoutManager()
{
  qgis->showLayoutManager();
}

QList<QgsLayoutDesignerInterface *> QgisAppInterface::openLayoutDesigners()
{
  QList<QgsLayoutDesignerInterface *> designerInterfaceList;
  if ( qgis )
  {
    const QSet<QgsLayoutDesignerDialog *> designerList = qgis->layoutDesigners();
    QSet<QgsLayoutDesignerDialog *>::const_iterator it = designerList.constBegin();
    for ( ; it != designerList.constEnd(); ++it )
    {
      if ( *it )
      {
        QgsLayoutDesignerInterface *v = ( *it )->iface();
        if ( v )
        {
          designerInterfaceList << v;
        }
      }
    }
  }
  return designerInterfaceList;
}

QgsLayoutDesignerInterface *QgisAppInterface::openLayoutDesigner( QgsMasterLayoutInterface *layout )
{
  QgsLayoutDesignerDialog *designer = qgis->openLayoutDesignerDialog( layout );
  if ( designer )
  {
    return designer->iface();
  }
  return nullptr;
}

void QgisAppInterface::showOptionsDialog( QWidget *parent, const QString &currentPage )
{
  return qgis->showOptionsDialog( parent, currentPage );
}

void QgisAppInterface::showProjectPropertiesDialog( const QString &currentPage )
{
  return qgis->showProjectProperties( currentPage );
}

QMap<QString, QVariant> QgisAppInterface::defaultStyleSheetOptions()
{
  return qgis->styleSheetBuilder()->defaultOptions();
}

void QgisAppInterface::buildStyleSheet( const QMap<QString, QVariant> &opts )
{
  qgis->styleSheetBuilder()->buildStyleSheet( opts );
}

void QgisAppInterface::saveStyleSheetOptions( const QMap<QString, QVariant> &opts )
{
  qgis->styleSheetBuilder()->saveToSettings( opts );
}

QFont QgisAppInterface::defaultStyleSheetFont()
{
  return qgis->styleSheetBuilder()->defaultFont();
}

void QgisAppInterface::addDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget )
{
  qgis->addDockWidget( area, dockwidget );
}

void QgisAppInterface::addTabifiedDockWidget( Qt::DockWidgetArea area, QDockWidget *dockwidget, const QStringList &tabifyWith, bool raiseTab )
{
  qgis->addTabifiedDockWidget( area, dockwidget, tabifyWith, raiseTab );
}

void QgisAppInterface::removeDockWidget( QDockWidget *dockwidget )
{
  qgis->removeDockWidget( dockwidget );
}


QgsAdvancedDigitizingDockWidget *QgisAppInterface::cadDockWidget()
{
  return qgis->cadDockWidget();
}

void QgisAppInterface::showLayerProperties( QgsMapLayer *l, const QString &page )
{
  if ( l && qgis )
  {
    qgis->showLayerProperties( l, page );
  }
}

QDialog *QgisAppInterface::showAttributeTable( QgsVectorLayer *l, const QString &filterExpression )
{
  if ( l && l->dataProvider() )
  {
    QgsAttributeTableDialog *dialog = new QgsAttributeTableDialog( l, QgsAttributeTableFilterModel::ShowFilteredList );
    dialog->setFilterExpression( filterExpression );
    dialog->show();
    return dialog;
  }
  return nullptr;
}

void QgisAppInterface::addWindow( QAction *action )
{
  qgis->addWindow( action );
}

void QgisAppInterface::removeWindow( QAction *action )
{
  qgis->removeWindow( action );
}

bool QgisAppInterface::registerMainWindowAction( QAction *action, const QString &defaultShortcut )
{
  return QgsGui::shortcutsManager()->registerAction( action, defaultShortcut );
}

bool QgisAppInterface::unregisterMainWindowAction( QAction *action )
{
  return QgsGui::shortcutsManager()->unregisterAction( action );
}

void QgisAppInterface::registerMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory )
{
  qgis->registerMapLayerPropertiesFactory( factory );
}

void QgisAppInterface::unregisterMapLayerConfigWidgetFactory( QgsMapLayerConfigWidgetFactory *factory )
{
  qgis->unregisterMapLayerPropertiesFactory( factory );
}

void QgisAppInterface::registerOptionsWidgetFactory( QgsOptionsWidgetFactory *factory )
{
  qgis->registerOptionsWidgetFactory( factory );
}

void QgisAppInterface::unregisterOptionsWidgetFactory( QgsOptionsWidgetFactory *factory )
{
  qgis->unregisterOptionsWidgetFactory( factory );
}

void QgisAppInterface::registerProjectPropertiesWidgetFactory( QgsOptionsWidgetFactory *factory )
{
  qgis->registerProjectPropertiesWidgetFactory( factory );
}

void QgisAppInterface::unregisterProjectPropertiesWidgetFactory( QgsOptionsWidgetFactory *factory )
{
  qgis->unregisterProjectPropertiesWidgetFactory( factory );
}

void QgisAppInterface::registerDevToolWidgetFactory( QgsDevToolWidgetFactory *factory )
{
  qgis->registerDevToolFactory( factory );
}

void QgisAppInterface::unregisterDevToolWidgetFactory( QgsDevToolWidgetFactory *factory )
{
  qgis->unregisterDevToolFactory( factory );
}

void QgisAppInterface::registerApplicationExitBlocker( QgsApplicationExitBlockerInterface *blocker )
{
  qgis->registerApplicationExitBlocker( blocker );
}

void QgisAppInterface::unregisterApplicationExitBlocker( QgsApplicationExitBlockerInterface *blocker )
{
  qgis->unregisterApplicationExitBlocker( blocker );
}

void QgisAppInterface::registerMapToolHandler( QgsAbstractMapToolHandler *handler )
{
  qgis->registerMapToolHandler( handler );
}

void QgisAppInterface::unregisterMapToolHandler( QgsAbstractMapToolHandler *handler )
{
  qgis->unregisterMapToolHandler( handler );
}

void QgisAppInterface::registerCustomDropHandler( QgsCustomDropHandler *handler )
{
  qgis->registerCustomDropHandler( handler );
}

void QgisAppInterface::registerCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler )
{
  qgis->registerCustomLayoutDropHandler( handler );
}

void QgisAppInterface::unregisterCustomLayoutDropHandler( QgsLayoutCustomDropHandler *handler )
{
  qgis->unregisterCustomLayoutDropHandler( handler );
}

void QgisAppInterface::unregisterCustomDropHandler( QgsCustomDropHandler *handler )
{
  qgis->unregisterCustomDropHandler( handler );
}

void QgisAppInterface::registerCustomProjectOpenHandler( QgsCustomProjectOpenHandler *handler )
{
  qgis->registerCustomProjectOpenHandler( handler );
}

void QgisAppInterface::unregisterCustomProjectOpenHandler( QgsCustomProjectOpenHandler *handler )
{
  qgis->unregisterCustomProjectOpenHandler( handler );
}

QMenu *QgisAppInterface::projectMenu() { return qgis->projectMenu(); }
QMenu *QgisAppInterface::editMenu() { return qgis->editMenu(); }
QMenu *QgisAppInterface::viewMenu() { return qgis->viewMenu(); }
QMenu *QgisAppInterface::layerMenu() { return qgis->layerMenu(); }
QMenu *QgisAppInterface::newLayerMenu() { return qgis->newLayerMenu(); }
QMenu *QgisAppInterface::addLayerMenu() { return qgis->addLayerMenu(); }
QMenu *QgisAppInterface::settingsMenu() { return qgis->settingsMenu(); }
QMenu *QgisAppInterface::pluginMenu() { return qgis->pluginMenu(); }
QMenu *QgisAppInterface::pluginHelpMenu() { return qgis->pluginHelpMenu(); }
QMenu *QgisAppInterface::rasterMenu() { return qgis->rasterMenu(); }
QMenu *QgisAppInterface::vectorMenu() { return qgis->vectorMenu(); }
QMenu *QgisAppInterface::databaseMenu() { return qgis->databaseMenu(); }
QMenu *QgisAppInterface::webMenu() { return qgis->webMenu(); }
QMenu *QgisAppInterface::firstRightStandardMenu() { return qgis->firstRightStandardMenu(); }
QMenu *QgisAppInterface::windowMenu() { return qgis->windowMenu(); }
QMenu *QgisAppInterface::helpMenu() { return qgis->helpMenu(); }

QToolBar *QgisAppInterface::fileToolBar() { return qgis->fileToolBar(); }
QToolBar *QgisAppInterface::layerToolBar() { return qgis->layerToolBar(); }
QToolBar *QgisAppInterface::dataSourceManagerToolBar() { return qgis->dataSourceManagerToolBar(); }
QToolBar *QgisAppInterface::mapNavToolToolBar() { return qgis->mapNavToolToolBar(); }
QToolBar *QgisAppInterface::digitizeToolBar() { return qgis->digitizeToolBar(); }
QToolBar *QgisAppInterface::advancedDigitizeToolBar() { return qgis->advancedDigitizeToolBar(); }
QToolBar *QgisAppInterface::shapeDigitizeToolBar() { return qgis->shapeDigitizeToolBar(); }
QToolBar *QgisAppInterface::attributesToolBar() { return qgis->attributesToolBar(); }
QToolBar *QgisAppInterface::selectionToolBar() { return qgis->selectionToolBar(); }
QToolBar *QgisAppInterface::pluginToolBar() { return qgis->pluginToolBar(); }
QToolBar *QgisAppInterface::helpToolBar() { return qgis->helpToolBar(); }
QToolBar *QgisAppInterface::rasterToolBar() { return qgis->rasterToolBar(); }
QToolBar *QgisAppInterface::vectorToolBar() { return qgis->vectorToolBar(); }
QToolBar *QgisAppInterface::databaseToolBar() { return qgis->databaseToolBar(); }
QToolBar *QgisAppInterface::webToolBar() { return qgis->webToolBar(); }
QActionGroup *QgisAppInterface::mapToolActionGroup() { return qgis->mMapToolGroup; }
QAction *QgisAppInterface::actionNewProject() { return qgis->actionNewProject(); }
QAction *QgisAppInterface::actionOpenProject() { return qgis->actionOpenProject(); }
QAction *QgisAppInterface::actionSaveProject() { return qgis->actionSaveProject(); }
QAction *QgisAppInterface::actionSaveProjectAs() { return qgis->actionSaveProjectAs(); }
QAction *QgisAppInterface::actionSaveMapAsImage() { return qgis->actionSaveMapAsImage(); }
QAction *QgisAppInterface::actionProjectProperties() { return qgis->actionProjectProperties(); }
QAction *QgisAppInterface::actionCreatePrintLayout() { return qgis->actionNewPrintLayout(); }
QAction *QgisAppInterface::actionShowLayoutManager() { return qgis->actionShowLayoutManager(); }
QAction *QgisAppInterface::actionExit() { return qgis->actionExit(); }

QAction *QgisAppInterface::actionCutFeatures() { return qgis->actionCutFeatures(); }
QAction *QgisAppInterface::actionCopyFeatures() { return qgis->actionCopyFeatures(); }
QAction *QgisAppInterface::actionPasteFeatures() { return qgis->actionPasteFeatures(); }
QAction *QgisAppInterface::actionAddFeature() { return qgis->actionAddFeature(); }
QAction *QgisAppInterface::actionDeleteSelected() { return qgis->actionDeleteSelected(); }
QAction *QgisAppInterface::actionMoveFeature() { return qgis->actionMoveFeature(); }
QAction *QgisAppInterface::actionSplitFeatures() { return qgis->actionSplitFeatures(); }
QAction *QgisAppInterface::actionSplitParts() { return qgis->actionSplitParts(); }
QAction *QgisAppInterface::actionAddRing() { return qgis->actionAddRing(); }
QAction *QgisAppInterface::actionAddPart() { return qgis->actionAddPart(); }
QAction *QgisAppInterface::actionSimplifyFeature() { return qgis->actionSimplifyFeature(); }
QAction *QgisAppInterface::actionDeleteRing() { return qgis->actionDeleteRing(); }
QAction *QgisAppInterface::actionDeletePart() { return qgis->actionDeletePart(); }
QAction *QgisAppInterface::actionVertexTool() { return qgis->actionVertexTool(); }
QAction *QgisAppInterface::actionVertexToolActiveLayer() { return qgis->actionVertexToolActiveLayer(); }

QAction *QgisAppInterface::actionPan() { return qgis->actionPan(); }
QAction *QgisAppInterface::actionPanToSelected() { return qgis->actionPanToSelected(); }
QAction *QgisAppInterface::actionZoomIn() { return qgis->actionZoomIn(); }
QAction *QgisAppInterface::actionZoomOut() { return qgis->actionZoomOut(); }
QAction *QgisAppInterface::actionSelect() { return qgis->actionSelect(); }
QAction *QgisAppInterface::actionSelectRectangle() { return qgis->actionSelectRectangle(); }
QAction *QgisAppInterface::actionSelectPolygon() { return qgis->actionSelectPolygon(); }
QAction *QgisAppInterface::actionSelectFreehand() { return qgis->actionSelectFreehand(); }
QAction *QgisAppInterface::actionSelectRadius() { return qgis->actionSelectRadius(); }
QAction *QgisAppInterface::actionIdentify() { return qgis->actionIdentify(); }
QAction *QgisAppInterface::actionFeatureAction() { return qgis->actionFeatureAction(); }
QAction *QgisAppInterface::actionMeasure() { return qgis->actionMeasure(); }
QAction *QgisAppInterface::actionMeasureArea() { return qgis->actionMeasureArea(); }
QAction *QgisAppInterface::actionZoomFullExtent() { return qgis->actionZoomFullExtent(); }
QAction *QgisAppInterface::actionZoomToLayer() { return qgis->actionZoomToLayer(); }
QAction *QgisAppInterface::actionZoomToLayers() { return qgis->actionZoomToLayers(); }
QAction *QgisAppInterface::actionZoomToSelected() { return qgis->actionZoomToSelected(); }
QAction *QgisAppInterface::actionZoomLast() { return qgis->actionZoomLast(); }
QAction *QgisAppInterface::actionZoomNext() { return qgis->actionZoomNext(); }
QAction *QgisAppInterface::actionZoomActualSize() { return qgis->actionZoomActualSize(); }
QAction *QgisAppInterface::actionMapTips() { return qgis->actionMapTips(); }
QAction *QgisAppInterface::actionNewBookmark() { return qgis->actionNewBookmark(); }
QAction *QgisAppInterface::actionShowBookmarks() { return qgis->actionShowBookmarks(); }
QAction *QgisAppInterface::actionDraw() { return qgis->actionDraw(); }
//! Layer menu actions
QAction *QgisAppInterface::actionNewVectorLayer() { return qgis->actionNewVectorLayer(); }
QAction *QgisAppInterface::actionAddOgrLayer() { return qgis->actionAddOgrLayer(); }
QAction *QgisAppInterface::actionAddRasterLayer() { return qgis->actionAddRasterLayer(); }
QAction *QgisAppInterface::actionAddPgLayer() { return qgis->actionAddPgLayer(); }
QAction *QgisAppInterface::actionAddWmsLayer() { return qgis->actionAddWmsLayer(); }
QAction *QgisAppInterface::actionAddXyzLayer() { return qgis->actionAddXyzLayer(); }
QAction *QgisAppInterface::actionAddVectorTileLayer() { return qgis->actionAddVectorTileLayer(); }
QAction *QgisAppInterface::actionAddPointCloudLayer() { return qgis->actionAddPointCloudLayer(); }
QAction *QgisAppInterface::actionAddAfsLayer() { return qgis->actionAddAfsLayer(); }
QAction *QgisAppInterface::actionAddAmsLayer() { return qgis->actionAddAfsLayer(); }
QAction *QgisAppInterface::actionCopyLayerStyle() { return qgis->actionCopyLayerStyle(); }
QAction *QgisAppInterface::actionPasteLayerStyle() { return qgis->actionPasteLayerStyle(); }
QAction *QgisAppInterface::actionOpenTable() { return qgis->actionOpenTable(); }
QAction *QgisAppInterface::actionOpenFieldCalculator() { return qgis->actionOpenFieldCalculator(); }
QAction *QgisAppInterface::actionOpenStatisticalSummary() { return qgis->actionStatisticalSummary(); }
QAction *QgisAppInterface::actionToggleEditing() { return qgis->actionToggleEditing(); }
QAction *QgisAppInterface::actionSaveActiveLayerEdits() { return qgis->actionSaveActiveLayerEdits(); }
QAction *QgisAppInterface::actionAllEdits() { return qgis->actionAllEdits(); }
QAction *QgisAppInterface::actionSaveEdits() { return qgis->actionSaveEdits(); }
QAction *QgisAppInterface::actionSaveAllEdits() { return qgis->actionSaveAllEdits(); }
QAction *QgisAppInterface::actionRollbackEdits() { return qgis->actionRollbackEdits(); }
QAction *QgisAppInterface::actionRollbackAllEdits() { return qgis->actionRollbackAllEdits(); }
QAction *QgisAppInterface::actionCancelEdits() { return qgis->actionCancelEdits(); }
QAction *QgisAppInterface::actionCancelAllEdits() { return qgis->actionCancelAllEdits(); }
QAction *QgisAppInterface::actionLayerSaveAs() { return qgis->actionLayerSaveAs(); }
QAction *QgisAppInterface::actionDuplicateLayer() { return qgis->actionDuplicateLayer(); }
QAction *QgisAppInterface::actionLayerProperties() { return qgis->actionLayerProperties(); }
QAction *QgisAppInterface::actionAddToOverview() { return qgis->actionAddToOverview(); }
QAction *QgisAppInterface::actionAddAllToOverview() { return qgis->actionAddAllToOverview(); }
QAction *QgisAppInterface::actionRemoveAllFromOverview() { return qgis->actionRemoveAllFromOverview(); }
QAction *QgisAppInterface::actionHideAllLayers() { return qgis->actionHideAllLayers(); }
QAction *QgisAppInterface::actionShowAllLayers() { return qgis->actionShowAllLayers(); }
QAction *QgisAppInterface::actionHideSelectedLayers() { return qgis->actionHideSelectedLayers(); }
QAction *QgisAppInterface::actionToggleSelectedLayers() { return qgis->actionToggleSelectedLayers(); }
QAction *QgisAppInterface::actionToggleSelectedLayersIndependently() { return qgis->actionToggleSelectedLayersIndependently(); }
QAction *QgisAppInterface::actionHideDeselectedLayers() { return qgis->actionHideDeselectedLayers(); }
QAction *QgisAppInterface::actionShowSelectedLayers() { return qgis->actionShowSelectedLayers(); }

QAction *QgisAppInterface::actionManagePlugins() { return qgis->actionManagePlugins(); }
QAction *QgisAppInterface::actionPluginListSeparator() { return qgis->actionPluginListSeparator(); }
QAction *QgisAppInterface::actionShowPythonDialog() { return qgis->actionShowPythonDialog(); }

QAction *QgisAppInterface::actionToggleFullScreen() { return qgis->actionToggleFullScreen(); }
QAction *QgisAppInterface::actionOptions() { return qgis->actionOptions(); }
QAction *QgisAppInterface::actionCustomProjection() { return qgis->actionCustomProjection(); }

QAction *QgisAppInterface::actionHelpContents() { return qgis->actionHelpContents(); }
QAction *QgisAppInterface::actionQgisHomePage() { return qgis->actionQgisHomePage(); }
QAction *QgisAppInterface::actionCheckQgisVersion() { return qgis->actionCheckQgisVersion(); }
QAction *QgisAppInterface::actionAbout() { return qgis->actionAbout(); }

bool QgisAppInterface::openFeatureForm( QgsVectorLayer *vlayer, QgsFeature &f, bool updateFeatureOnly, bool showModal )
{
  Q_UNUSED( updateFeatureOnly )
  if ( !vlayer )
    return false;

  QgsFeatureAction action( tr( "Attributes changed" ), f, vlayer, QString(), -1, QgisApp::instance() );
  if ( vlayer->isEditable() )
  {
    return action.editFeature( showModal );
  }
  else
  {
    action.viewFeatureForm();
    return true;
  }
}

void QgisAppInterface::preloadForm( const QString &uifile )
{
  QTimer::singleShot( 0, this, [ = ]
  {
    cacheloadForm( uifile );
  } );
}

void QgisAppInterface::cacheloadForm( const QString &uifile )
{
  QFile file( uifile );

  if ( file.open( QFile::ReadOnly ) )
  {
    QUiLoader loader;

    const QFileInfo fi( uifile );
    loader.setWorkingDirectory( fi.dir() );
    QWidget *myWidget = loader.load( &file );
    file.close();
    delete myWidget;
  }
}

QgsAttributeDialog *QgisAppInterface::getFeatureForm( QgsVectorLayer *l, QgsFeature &feature )
{
  QgsDistanceArea myDa;

  myDa.setSourceCrs( l->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  QgsAttributeEditorContext context( QgisApp::instance()->createAttributeEditorContext() );
  context.setDistanceArea( myDa );
  QgsAttributeDialog *dialog = new QgsAttributeDialog( l, &feature, false, qgis, true, context );
  if ( !feature.isValid() )
  {
    dialog->setMode( QgsAttributeEditorContext::AddFeatureMode );
  }
  return dialog;
}

QgsVectorLayerTools *QgisAppInterface::vectorLayerTools()
{
  return qgis->vectorLayerTools();
}

QList<QgsMapLayer *> QgisAppInterface::editableLayers( bool modified ) const
{
  return qgis->editableLayers( modified );
}

int QgisAppInterface::messageTimeout()
{
  return QgsMessageBar::defaultMessageTimeout();
}

QgsStatusBar *QgisAppInterface::statusBarIface()
{
  return qgis->statusBarIface();
}

void QgisAppInterface::locatorSearch( const QString &searchText )
{
  qgis->mLocatorWidget->invalidateResults();
  qgis->mLocatorWidget->search( searchText );
}

void QgisAppInterface::registerLocatorFilter( QgsLocatorFilter *filter )
{
  qgis->mLocatorWidget->locator()->registerFilter( filter );
}

void QgisAppInterface::deregisterLocatorFilter( QgsLocatorFilter *filter )
{
  qgis->mLocatorWidget->locator()->deregisterFilter( filter );
}

void QgisAppInterface::invalidateLocatorResults()
{
  qgis->mLocatorWidget->invalidateResults();
}

bool QgisAppInterface::askForDatumTransform( QgsCoordinateReferenceSystem sourceCrs, QgsCoordinateReferenceSystem destinationCrs )
{
  return qgis->askUserForDatumTransform( sourceCrs, destinationCrs );
}

void QgisAppInterface::takeAppScreenShots( const QString &saveDirectory, const int categories )
{
  qgis->takeAppScreenShots( saveDirectory, categories );
}

QgsBrowserGuiModel *QgisAppInterface::browserModel()
{
  return qgis->mBrowserModel;
}

QgsLayerTreeRegistryBridge::InsertionPoint QgisAppInterface::layerTreeInsertionPoint()
{
  return qgis->layerTreeInsertionPoint();
}

void QgisAppInterface::setGpsPanelConnection( QgsGpsConnection *connection )
{
  qgis->setGpsPanelConnection( connection );
}

QList<QgsMapDecoration *> QgisAppInterface::activeDecorations()
{
  return qgis->activeDecorations();
}

