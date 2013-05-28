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
#include "qgscomposer.h"
#include "qgscomposerview.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgslegend.h"
#include "qgsshortcutsmanager.h"
#include "qgsattributedialog.h"
#include "qgsfield.h"
#include "qgsvectordataprovider.h"
#include "qgsfeatureaction.h"
#include "qgsattributeaction.h"
#include "qgsattributetabledialog.h"

QgisAppInterface::QgisAppInterface( QgisApp * _qgis )
    : qgis( _qgis ),
    legendIface( _qgis->legend() )
{
  // connect signals
  connect( qgis->legend(), SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SIGNAL( currentLayerChanged( QgsMapLayer * ) ) );
  connect( qgis, SIGNAL( currentThemeChanged( QString ) ),
           this, SIGNAL( currentThemeChanged( QString ) ) );
  connect( qgis, SIGNAL( composerAdded( QgsComposerView* ) ),
           this, SIGNAL( composerAdded( QgsComposerView* ) ) );
  connect( qgis, SIGNAL( composerWillBeRemoved( QgsComposerView* ) ),
           this, SIGNAL( composerWillBeRemoved( QgsComposerView* ) ) );
  connect( qgis, SIGNAL( initializationCompleted() ),
           this, SIGNAL( initializationCompleted() ) );
  connect( qgis, SIGNAL( newProject() ),
           this, SIGNAL( newProjectCreated() ) );
  connect( qgis, SIGNAL( projectRead() ),
           this, SIGNAL( projectRead() ) );
}

QgisAppInterface::~QgisAppInterface()
{
}

QgsLegendInterface* QgisAppInterface::legendInterface()
{
  return &legendIface;
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

QgsVectorLayer* QgisAppInterface::addVectorLayer( QString vectorLayerPath, QString baseName, QString providerKey )
{
  if ( baseName.isEmpty() )
  {
    QFileInfo fi( vectorLayerPath );
    baseName = fi.completeBaseName();
  }
  return qgis->addVectorLayer( vectorLayerPath, baseName, providerKey );
}

QgsRasterLayer* QgisAppInterface::addRasterLayer( QString rasterLayerPath, QString baseName )
{
  if ( baseName.isEmpty() )
  {
    QFileInfo fi( rasterLayerPath );
    baseName = fi.completeBaseName();
  }
  return qgis->addRasterLayer( rasterLayerPath, baseName );
}

QgsRasterLayer* QgisAppInterface::addRasterLayer( const QString& url, const QString& baseName, const QString& providerKey )
{
  return qgis->addRasterLayer( url, baseName, providerKey );
}

bool QgisAppInterface::addProject( QString theProjectName )
{
  return qgis->addProject( theProjectName );
}

void QgisAppInterface::newProject( bool thePromptToSaveFlag )
{
  qgis->fileNew( thePromptToSaveFlag );
}

QgsMapLayer *QgisAppInterface::activeLayer()
{
  return qgis->activeLayer();
}

bool QgisAppInterface::setActiveLayer( QgsMapLayer *layer )
{
  return qgis->setActiveLayer( layer );
}

void QgisAppInterface::addPluginToMenu( QString name, QAction* action )
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

void QgisAppInterface::removePluginMenu( QString name, QAction* action )
{
  qgis->removePluginMenu( name, action );
}

void QgisAppInterface::addPluginToDatabaseMenu( QString name, QAction* action )
{
  qgis->addPluginToDatabaseMenu( name, action );
}

void QgisAppInterface::removePluginDatabaseMenu( QString name, QAction* action )
{
  qgis->removePluginDatabaseMenu( name, action );
}

void QgisAppInterface::addPluginToRasterMenu( QString name, QAction* action )
{
  qgis->addPluginToRasterMenu( name, action );
}

void QgisAppInterface::removePluginRasterMenu( QString name, QAction* action )
{
  qgis->removePluginRasterMenu( name, action );
}

void QgisAppInterface::addPluginToVectorMenu( QString name, QAction* action )
{
  qgis->addPluginToVectorMenu( name, action );
}

void QgisAppInterface::removePluginVectorMenu( QString name, QAction* action )
{
  qgis->removePluginVectorMenu( name, action );
}

void QgisAppInterface::addPluginToWebMenu( QString name, QAction* action )
{
  qgis->addPluginToWebMenu( name, action );
}

void QgisAppInterface::removePluginWebMenu( QString name, QAction* action )
{
  qgis->removePluginWebMenu( name, action );
}

int QgisAppInterface::addToolBarIcon( QAction * qAction )
{
  return qgis->addPluginToolBarIcon( qAction );
}

QAction *QgisAppInterface::addToolBarWidget( QWidget* widget )
{
  return qgis->addPluginToolBarWidget( widget );
}

void QgisAppInterface::removeToolBarIcon( QAction *qAction )
{
  qgis->removePluginToolBarIcon( qAction );
}

int QgisAppInterface::addRasterToolBarIcon( QAction * qAction )
{
  return qgis->addRasterToolBarIcon( qAction );
}

QAction *QgisAppInterface::addRasterToolBarWidget( QWidget* widget )
{
  return qgis->addRasterToolBarWidget( widget );
}

void QgisAppInterface::removeRasterToolBarIcon( QAction *qAction )
{
  qgis->removeRasterToolBarIcon( qAction );
}

int QgisAppInterface::addVectorToolBarIcon( QAction * qAction )
{
  return qgis->addVectorToolBarIcon( qAction );
}

QAction *QgisAppInterface::addVectorToolBarWidget( QWidget* widget )
{
  return qgis->addVectorToolBarWidget( widget );
}

void QgisAppInterface::removeVectorToolBarIcon( QAction *qAction )
{
  qgis->removeVectorToolBarIcon( qAction );
}

int QgisAppInterface::addDatabaseToolBarIcon( QAction * qAction )
{
  return qgis->addDatabaseToolBarIcon( qAction );
}

QAction *QgisAppInterface::addDatabaseToolBarWidget( QWidget* widget )
{
  return qgis->addDatabaseToolBarWidget( widget );
}

void QgisAppInterface::removeDatabaseToolBarIcon( QAction *qAction )
{
  qgis->removeDatabaseToolBarIcon( qAction );
}

int QgisAppInterface::addWebToolBarIcon( QAction * qAction )
{
  return qgis->addWebToolBarIcon( qAction );
}

QAction *QgisAppInterface::addWebToolBarWidget( QWidget* widget )
{
  return qgis->addWebToolBarWidget( widget );
}

void QgisAppInterface::removeWebToolBarIcon( QAction *qAction )
{
  qgis->removeWebToolBarIcon( qAction );
}

QToolBar* QgisAppInterface::addToolBar( QString name )
{
  return qgis->addToolBar( name );
}

void QgisAppInterface::openURL( QString url, bool useQgisDocDirectory )
{
  qgis->openURL( url, useQgisDocDirectory );
}

QgsMapCanvas * QgisAppInterface::mapCanvas()
{
  return qgis->mapCanvas();
}

QWidget * QgisAppInterface::mainWindow()
{
  return qgis;
}

QgsMessageBar * QgisAppInterface::messageBar()
{
  return qgis->messageBar();
}

QList<QgsComposerView*> QgisAppInterface::activeComposers()
{
  QList<QgsComposerView*> composerViewList;
  if ( qgis )
  {
    const QSet<QgsComposer*> composerList = qgis->printComposers();
    QSet<QgsComposer*>::const_iterator it = composerList.constBegin();
    for ( ; it != composerList.constEnd(); ++it )
    {
      if ( *it )
      {
        QgsComposerView* v = ( *it )->view();
        if ( v )
        {
          composerViewList.push_back( v );
        }
      }
    }
  }
  return composerViewList;
}

QgsComposerView* QgisAppInterface::createNewComposer( QString title )
{
  QgsComposer* composerObj = 0;
  composerObj = qgis->createNewComposer( title );
  if ( composerObj )
  {
    return composerObj->view();
  }
  return 0;
}

QgsComposerView* QgisAppInterface::duplicateComposer( QgsComposerView* composerView, QString title )
{
  QgsComposer* composerObj = 0;
  composerObj = qobject_cast<QgsComposer *>( composerView->composerWindow() );
  if ( composerObj )
  {
    QgsComposer* dupComposer = qgis->duplicateComposer( composerObj, title );
    if ( dupComposer )
    {
      return dupComposer->view();
    }
  }
  return 0;
}

void QgisAppInterface::deleteComposer( QgsComposerView* composerView )
{
  composerView->composerWindow()->close();

  QgsComposer* composerObj = 0;
  composerObj = qobject_cast<QgsComposer *>( composerView->composerWindow() );
  if ( composerObj )
  {
    qgis->deleteComposer( composerObj );
  }
}

QMap<QString, QVariant> QgisAppInterface::defaultStyleSheetOptions()
{
  return qgis->styleSheetBuilder()->defaultOptions();
}

void QgisAppInterface::buildStyleSheet( const QMap<QString, QVariant>& opts )
{
  qgis->styleSheetBuilder()->buildStyleSheet( opts );
}

void QgisAppInterface::saveStyleSheetOptions( const QMap<QString, QVariant>& opts )
{
  qgis->styleSheetBuilder()->saveToSettings( opts );
}

QFont QgisAppInterface::defaultStyleSheetFont()
{
  return qgis->styleSheetBuilder()->defaultFont();
}

void QgisAppInterface::addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget )
{
  qgis->addDockWidget( area, dockwidget );
}

void QgisAppInterface::removeDockWidget( QDockWidget * dockwidget )
{
  qgis->removeDockWidget( dockwidget );
}

void QgisAppInterface::showLayerProperties( QgsMapLayer *l )
{
  if ( l && qgis )
  {
    qgis->showLayerProperties( l );
  }
}

void QgisAppInterface::showAttributeTable( QgsVectorLayer *l )
{
  if ( l )
  {
    QgsAttributeTableDialog *dialog = new QgsAttributeTableDialog( l );
    dialog->show();
  }
}

void QgisAppInterface::addWindow( QAction *action )
{
  qgis->addWindow( action );
}

void QgisAppInterface::removeWindow( QAction *action )
{
  qgis->removeWindow( action );
}

bool QgisAppInterface::registerMainWindowAction( QAction* action, QString defaultShortcut )
{
  return QgsShortcutsManager::instance()->registerAction( action, defaultShortcut );
}

bool QgisAppInterface::unregisterMainWindowAction( QAction* action )
{
  return QgsShortcutsManager::instance()->unregisterAction( action );
}

//! Menus
Q_DECL_DEPRECATED QMenu *QgisAppInterface::fileMenu() { return qgis->projectMenu(); }
QMenu *QgisAppInterface::projectMenu() { return qgis->projectMenu(); }
QMenu *QgisAppInterface::editMenu() { return qgis->editMenu(); }
QMenu *QgisAppInterface::viewMenu() { return qgis->viewMenu(); }
QMenu *QgisAppInterface::layerMenu() { return qgis->layerMenu(); }
QMenu *QgisAppInterface::newLayerMenu() { return qgis->newLayerMenu(); }
QMenu *QgisAppInterface::settingsMenu() { return qgis->settingsMenu(); }
QMenu *QgisAppInterface::pluginMenu() { return qgis->pluginMenu(); }
QMenu *QgisAppInterface::rasterMenu() { return qgis->rasterMenu(); }
QMenu *QgisAppInterface::vectorMenu() { return qgis->vectorMenu(); }
QMenu *QgisAppInterface::databaseMenu() { return qgis->databaseMenu(); }
QMenu *QgisAppInterface::webMenu() { return qgis->webMenu(); }
QMenu *QgisAppInterface::firstRightStandardMenu() { return qgis->firstRightStandardMenu(); }
QMenu *QgisAppInterface::windowMenu() { return qgis->windowMenu(); }
QMenu *QgisAppInterface::helpMenu() { return qgis->helpMenu(); }

//! ToolBars
QToolBar *QgisAppInterface::fileToolBar() { return qgis->fileToolBar(); }
QToolBar *QgisAppInterface::layerToolBar() { return qgis->layerToolBar(); }
QToolBar *QgisAppInterface::mapNavToolToolBar() { return qgis->mapNavToolToolBar(); }
QToolBar *QgisAppInterface::digitizeToolBar() { return qgis->digitizeToolBar(); }
QToolBar *QgisAppInterface::advancedDigitizeToolBar() { return qgis->advancedDigitizeToolBar(); }
QToolBar *QgisAppInterface::attributesToolBar() { return qgis->attributesToolBar(); }
QToolBar *QgisAppInterface::pluginToolBar() { return qgis->pluginToolBar(); }
QToolBar *QgisAppInterface::helpToolBar() { return qgis->helpToolBar(); }
QToolBar *QgisAppInterface::rasterToolBar() { return qgis->rasterToolBar(); }
QToolBar *QgisAppInterface::vectorToolBar() { return qgis->vectorToolBar(); }
QToolBar *QgisAppInterface::databaseToolBar() { return qgis->databaseToolBar(); }
QToolBar *QgisAppInterface::webToolBar() { return qgis->webToolBar(); }

//! Project menu actions
QAction *QgisAppInterface::actionNewProject() { return qgis->actionNewProject(); }
QAction *QgisAppInterface::actionOpenProject() { return qgis->actionOpenProject(); }
QAction *QgisAppInterface::actionSaveProject() { return qgis->actionSaveProject(); }
QAction *QgisAppInterface::actionSaveProjectAs() { return qgis->actionSaveProjectAs(); }
QAction *QgisAppInterface::actionSaveMapAsImage() { return qgis->actionSaveMapAsImage(); }
QAction *QgisAppInterface::actionProjectProperties() { return qgis->actionProjectProperties(); }
QAction *QgisAppInterface::actionPrintComposer() { return qgis->actionNewPrintComposer(); }
QAction *QgisAppInterface::actionShowComposerManager() { return qgis->actionShowComposerManager(); }
QAction *QgisAppInterface::actionExit() { return qgis->actionExit(); }

//! Edit menu actions
QAction *QgisAppInterface::actionCutFeatures() { return qgis->actionCutFeatures(); }
QAction *QgisAppInterface::actionCopyFeatures() { return qgis->actionCopyFeatures(); }
QAction *QgisAppInterface::actionPasteFeatures() { return qgis->actionPasteFeatures(); }
QAction *QgisAppInterface::actionAddFeature() { return qgis->actionAddFeature(); }
QAction *QgisAppInterface::actionDeleteSelected() { return qgis->actionDeleteSelected(); }
QAction *QgisAppInterface::actionMoveFeature() { return qgis->actionMoveFeature(); }
QAction *QgisAppInterface::actionSplitFeatures() { return qgis->actionSplitFeatures(); }
QAction *QgisAppInterface::actionAddRing() { return qgis->actionAddRing(); }
QAction *QgisAppInterface::actionAddPart() { return qgis->actionAddPart(); }
QAction *QgisAppInterface::actionSimplifyFeature() { return qgis->actionSimplifyFeature(); }
QAction *QgisAppInterface::actionDeleteRing() { return qgis->actionDeleteRing(); }
QAction *QgisAppInterface::actionDeletePart() { return qgis->actionDeletePart(); }
QAction *QgisAppInterface::actionNodeTool() { return qgis->actionNodeTool(); }

//! View menu actions
QAction *QgisAppInterface::actionPan() { return qgis->actionPan(); }
QAction *QgisAppInterface::actionTouch() { return qgis->actionTouch(); }
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
QAction *QgisAppInterface::actionCopyLayerStyle() { return qgis->actionCopyLayerStyle(); }
QAction *QgisAppInterface::actionPasteLayerStyle() { return qgis->actionPasteLayerStyle(); }
QAction *QgisAppInterface::actionOpenTable() { return qgis->actionOpenTable(); }
QAction *QgisAppInterface::actionOpenFieldCalculator() { return qgis->actionOpenFieldCalculator(); }
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
QAction *QgisAppInterface::actionLayerSelectionSaveAs() { return qgis->actionLayerSelectionSaveAs(); }
QAction *QgisAppInterface::actionRemoveLayer() { return qgis->actionRemoveLayer(); }
QAction *QgisAppInterface::actionDuplicateLayer() { return qgis->actionDuplicateLayer(); }
QAction *QgisAppInterface::actionLayerProperties() { return qgis->actionLayerProperties(); }
QAction *QgisAppInterface::actionAddToOverview() { return qgis->actionAddToOverview(); }
QAction *QgisAppInterface::actionAddAllToOverview() { return qgis->actionAddAllToOverview(); }
QAction *QgisAppInterface::actionRemoveAllFromOverview() { return qgis->actionRemoveAllFromOverview(); }
QAction *QgisAppInterface::actionHideAllLayers() { return qgis->actionHideAllLayers(); }
QAction *QgisAppInterface::actionShowAllLayers() { return qgis->actionShowAllLayers(); }

//! Plugin menu actions
QAction *QgisAppInterface::actionManagePlugins() { return qgis->actionManagePlugins(); }
QAction *QgisAppInterface::actionPluginListSeparator() { return qgis->actionPluginListSeparator(); }
QAction *QgisAppInterface::actionShowPythonDialog() { return qgis->actionShowPythonDialog(); }

//! Settings menu actions
QAction *QgisAppInterface::actionToggleFullScreen() { return qgis->actionToggleFullScreen(); }
QAction *QgisAppInterface::actionOptions() { return qgis->actionOptions(); }
QAction *QgisAppInterface::actionCustomProjection() { return qgis->actionCustomProjection(); }

//! Help menu actions
QAction *QgisAppInterface::actionHelpContents() { return qgis->actionHelpContents(); }
QAction *QgisAppInterface::actionQgisHomePage() { return qgis->actionQgisHomePage(); }
QAction *QgisAppInterface::actionCheckQgisVersion() { return qgis->actionCheckQgisVersion(); }
QAction *QgisAppInterface::actionAbout() { return qgis->actionAbout(); }

bool QgisAppInterface::openFeatureForm( QgsVectorLayer *vlayer, QgsFeature &f, bool updateFeatureOnly )
{
  Q_UNUSED( updateFeatureOnly );
  if ( !vlayer )
    return false;

  QgsFeatureAction action( tr( "Attributes changed" ), f, vlayer, -1, -1, QgisApp::instance() );
  if ( vlayer->isEditable() )
  {
    return action.editFeature();
  }
  else
  {
    return action.viewFeatureForm();
  }
}

void QgisAppInterface::preloadForm( QString uifile )
{
  QSignalMapper* signalMapper = new QSignalMapper( this );
  mTimer = new QTimer( this );

  connect( mTimer , SIGNAL( timeout() ), signalMapper, SLOT( map() ) );
  connect( signalMapper, SIGNAL( mapped( QString ) ), mTimer, SLOT( stop() ) );
  connect( signalMapper, SIGNAL( mapped( QString ) ), this, SLOT( cacheloadForm( QString ) ) );

  signalMapper->setMapping( mTimer, uifile );

  mTimer->start( 0 );
}

void QgisAppInterface::cacheloadForm( QString uifile )
{
  QFile file( uifile );

  if ( file.open( QFile::ReadOnly ) )
  {
    QUiLoader loader;

    QFileInfo fi( uifile );
    loader.setWorkingDirectory( fi.dir() );
    QWidget *myWidget = loader.load( &file );
    file.close();
    delete myWidget;
  }
}

QDialog* QgisAppInterface::getFeatureForm( QgsVectorLayer *l, QgsFeature &f )
{
  QgsDistanceArea myDa;

  myDa.setSourceCrs( l->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );

  QgsAttributeDialog *dialog = new QgsAttributeDialog( l, &f, false, myDa );
  return dialog->dialog();
}

QList<QgsMapLayer *> QgisAppInterface::editableLayers( bool modified ) const
{
  return qgis->editableLayers( modified );
}

int QgisAppInterface::messageTimeout()
{
  return qgis->messageTimeout();
}
