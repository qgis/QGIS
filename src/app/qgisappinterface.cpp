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
/* $Id$ */

#include <QFileInfo>
#include <QString>
#include <QMenu>

#include "qgisappinterface.h"
#include "qgisapp.h"
#include "qgscomposer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapcanvas.h"
#include "qgslegend.h"
#include "qgsshortcutsmanager.h"

QgisAppInterface::QgisAppInterface( QgisApp * _qgis )
    : qgis( _qgis ),
    legendIface( _qgis->legend() )
{
  // connect signals
  connect( qgis->legend(), SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SIGNAL( currentLayerChanged( QgsMapLayer * ) ) );
  connect( qgis, SIGNAL( currentThemeChanged( QString ) ),
           this, SIGNAL( currentThemeChanged( QString ) ) );
  connect( qgis, SIGNAL( composerAdded( QgsComposerView* ) ), this, SIGNAL( composerAdded( QgsComposerView* ) ) );
  connect( qgis, SIGNAL( composerWillBeRemoved( QgsComposerView* ) ), this, SIGNAL( composerWillBeRemoved( QgsComposerView* ) ) );
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

QgsRasterLayer* QgisAppInterface::addRasterLayer( const QString& url, const QString& baseName, const QString& providerKey,
    const QStringList& layers, const QStringList& styles, const QString& format, const QString& crs )
{
  return qgis->addRasterLayer( url, baseName, providerKey, layers, styles, format, crs );
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

void QgisAppInterface::removePluginMenu( QString name, QAction* action )
{
  qgis->removePluginMenu( name, action );
}

int QgisAppInterface::addToolBarIcon( QAction * qAction )
{
  // add the menu to the master Plugins menu
  return qgis->addPluginToolBarIcon( qAction );
}
void QgisAppInterface::removeToolBarIcon( QAction *qAction )
{
  qgis->removePluginToolBarIcon( qAction );
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

void QgisAppInterface::addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget )
{
  qgis->addDockWidget( area, dockwidget );
}

void QgisAppInterface::removeDockWidget( QDockWidget * dockwidget )
{
  qgis->removeDockWidget( dockwidget );
}

void QgisAppInterface::refreshLegend( QgsMapLayer *l )
{
  if ( l && qgis && qgis->legend() )
  {
    qgis->legend()->refreshLayerSymbology( l->getLayerID() );
  }
}

void QgisAppInterface::addWindow( QAction *action ) { qgis->addWindow( action ); }
void QgisAppInterface::removeWindow( QAction *action ) { qgis->removeWindow( action ); }


bool QgisAppInterface::registerMainWindowAction( QAction* action, QString defaultShortcut )
{
  return QgsShortcutsManager::instance()->registerAction( action, defaultShortcut );
}

bool QgisAppInterface::unregisterMainWindowAction( QAction* action )
{
  return QgsShortcutsManager::instance()->unregisterAction( action );
}

//! Menus
QMenu *QgisAppInterface::fileMenu() { return qgis->fileMenu(); }
QMenu *QgisAppInterface::editMenu() { return qgis->editMenu(); }
QMenu *QgisAppInterface::viewMenu() { return qgis->viewMenu(); }
QMenu *QgisAppInterface::layerMenu() { return qgis->layerMenu(); }
QMenu *QgisAppInterface::settingsMenu() { return qgis->settingsMenu(); }
QMenu *QgisAppInterface::pluginMenu() { return qgis->pluginMenu(); }
QMenu *QgisAppInterface::firstRightStandardMenu() { return qgis->firstRightStandardMenu(); }
QMenu *QgisAppInterface::windowMenu() { return qgis->windowMenu(); }
QMenu *QgisAppInterface::helpMenu() { return qgis->helpMenu(); }

//! ToolBars
QToolBar *QgisAppInterface::fileToolBar() { return qgis->fileToolBar(); }
QToolBar *QgisAppInterface::layerToolBar() { return qgis->layerToolBar(); }
QToolBar *QgisAppInterface::mapNavToolToolBar() { return qgis->mapNavToolToolBar(); }
QToolBar *QgisAppInterface::digitizeToolBar() { return qgis->digitizeToolBar(); }
QToolBar *QgisAppInterface::attributesToolBar() { return qgis->attributesToolBar(); }
QToolBar *QgisAppInterface::pluginToolBar() { return qgis->pluginToolBar(); }
QToolBar *QgisAppInterface::helpToolBar() { return qgis->helpToolBar(); }

//! File menu actions
QAction *QgisAppInterface::actionNewProject() { return qgis->actionNewProject(); }
QAction *QgisAppInterface::actionOpenProject() { return qgis->actionOpenProject(); }
QAction *QgisAppInterface::actionFileSeparator1() { return qgis->actionFileSeparator1(); }
QAction *QgisAppInterface::actionSaveProject() { return qgis->actionSaveProject(); }
QAction *QgisAppInterface::actionSaveProjectAs() { return qgis->actionSaveProjectAs(); }
QAction *QgisAppInterface::actionSaveMapAsImage() { return qgis->actionSaveMapAsImage(); }
QAction *QgisAppInterface::actionFileSeparator2() { return qgis->actionFileSeparator2(); }
QAction *QgisAppInterface::actionProjectProperties() { return qgis->actionProjectProperties(); }
QAction *QgisAppInterface::actionFileSeparator3() { return qgis->actionFileSeparator3(); }
QAction *QgisAppInterface::actionPrintComposer() { return qgis->actionNewPrintComposer(); }
QAction *QgisAppInterface::actionFileSeparator4() { return qgis->actionFileSeparator4(); }
QAction *QgisAppInterface::actionExit() { return qgis->actionExit(); }

//! Edit menu actions
QAction *QgisAppInterface::actionCutFeatures() { return qgis->actionCutFeatures(); }
QAction *QgisAppInterface::actionCopyFeatures() { return qgis->actionCopyFeatures(); }
QAction *QgisAppInterface::actionPasteFeatures() { return qgis->actionPasteFeatures(); }
QAction *QgisAppInterface::actionEditSeparator1() { return qgis->actionEditSeparator1(); }
QAction *QgisAppInterface::actionCapturePoint() { return qgis->actionCapturePoint(); }
QAction *QgisAppInterface::actionCaptureLine() { return qgis->actionCaptureLine(); }
QAction *QgisAppInterface::actionCapturePolygon() { return qgis->actionCapturePolygon(); }
QAction *QgisAppInterface::actionDeleteSelected() { return qgis->actionDeleteSelected(); }
QAction *QgisAppInterface::actionMoveFeature() { return qgis->actionMoveFeature(); }
QAction *QgisAppInterface::actionSplitFeatures() { return qgis->actionSplitFeatures(); }
//these three actions are removed from the ui as of qgis v1.4
//for plugin api completeness we now return a null pointer
//but these should be removed from the plugin interface for v2
QAction *QgisAppInterface::actionAddVertex() { return 0; }
QAction *QgisAppInterface::actionDeleteVertex() { return 0; }
QAction *QgisAppInterface::actionMoveVertex() { return 0; }
QAction *QgisAppInterface::actionAddRing() { return qgis->actionAddRing(); }
QAction *QgisAppInterface::actionAddIsland() { return qgis->actionAddIsland(); }
QAction *QgisAppInterface::actionEditSeparator2() { return qgis->actionEditSeparator2(); }

//! View menu actions
QAction *QgisAppInterface::actionPan() { return qgis->actionPan(); }
QAction *QgisAppInterface::actionZoomIn() { return qgis->actionZoomIn(); }
QAction *QgisAppInterface::actionZoomOut() { return qgis->actionZoomOut(); }
QAction *QgisAppInterface::actionSelect() { return qgis->actionSelect(); }
QAction *QgisAppInterface::actionIdentify() { return qgis->actionIdentify(); }
QAction *QgisAppInterface::actionMeasure() { return qgis->actionMeasure(); }
QAction *QgisAppInterface::actionMeasureArea() { return qgis->actionMeasureArea(); }
QAction *QgisAppInterface::actionViewSeparator1() { return qgis->actionViewSeparator1(); }
QAction *QgisAppInterface::actionZoomFullExtent() { return qgis->actionZoomFullExtent(); }
QAction *QgisAppInterface::actionZoomToLayer() { return qgis->actionZoomToLayer(); }
QAction *QgisAppInterface::actionZoomToSelected() { return qgis->actionZoomToSelected(); }
QAction *QgisAppInterface::actionZoomLast() { return qgis->actionZoomLast(); }
QAction *QgisAppInterface::actionZoomNext() { return qgis->actionZoomNext(); }
QAction *QgisAppInterface::actionZoomActualSize() { return qgis->actionZoomActualSize(); }
QAction *QgisAppInterface::actionViewSeparator2() { return qgis->actionViewSeparator2(); }
QAction *QgisAppInterface::actionMapTips() { return qgis->actionMapTips(); }
QAction *QgisAppInterface::actionNewBookmark() { return qgis->actionNewBookmark(); }
QAction *QgisAppInterface::actionShowBookmarks() { return qgis->actionShowBookmarks(); }
QAction *QgisAppInterface::actionDraw() { return qgis->actionDraw(); }
QAction *QgisAppInterface::actionViewSeparator3() { return qgis->actionViewSeparator3(); }

//! Layer menu actions
QAction *QgisAppInterface::actionNewVectorLayer() { return qgis->actionNewVectorLayer(); }
QAction *QgisAppInterface::actionAddOgrLayer() { return qgis->actionAddOgrLayer(); }
QAction *QgisAppInterface::actionAddRasterLayer() { return qgis->actionAddRasterLayer(); }
QAction *QgisAppInterface::actionAddPgLayer() { return qgis->actionAddPgLayer(); }
QAction *QgisAppInterface::actionAddWmsLayer() { return qgis->actionAddWmsLayer(); }
QAction *QgisAppInterface::actionLayerSeparator1() { return qgis->actionLayerSeparator1(); }
QAction *QgisAppInterface::actionOpenTable() { return qgis->actionOpenTable(); }
QAction *QgisAppInterface::actionToggleEditing() { return qgis->actionToggleEditing(); }
QAction *QgisAppInterface::actionLayerSaveAs() { return qgis->actionLayerSaveAs(); }
QAction *QgisAppInterface::actionLayerSelectionSaveAs() { return qgis->actionLayerSelectionSaveAs(); }
QAction *QgisAppInterface::actionRemoveLayer() { return qgis->actionRemoveLayer(); }
QAction *QgisAppInterface::actionLayerProperties() { return qgis->actionLayerProperties(); }
QAction *QgisAppInterface::actionLayerSeparator2() { return qgis->actionLayerSeparator2(); }
QAction *QgisAppInterface::actionAddToOverview() { return qgis->actionAddToOverview(); }
QAction *QgisAppInterface::actionAddAllToOverview() { return qgis->actionAddAllToOverview(); }
QAction *QgisAppInterface::actionRemoveAllFromOverview() { return qgis->actionRemoveAllFromOverview(); }
QAction *QgisAppInterface::actionLayerSeparator3() { return qgis->actionLayerSeparator3(); }
QAction *QgisAppInterface::actionHideAllLayers() { return qgis->actionHideAllLayers(); }
QAction *QgisAppInterface::actionShowAllLayers() { return qgis->actionShowAllLayers(); }

//! Plugin menu actions
QAction *QgisAppInterface::actionManagePlugins() { return qgis->actionManagePlugins(); }
QAction *QgisAppInterface::actionPluginSeparator1() { return qgis->actionPluginSeparator1(); }
QAction *QgisAppInterface::actionPluginListSeparator() { return qgis->actionPluginListSeparator(); }
QAction *QgisAppInterface::actionPluginSeparator2() { return qgis->actionPluginSeparator2(); }
QAction *QgisAppInterface::actionPluginPythonSeparator() { return qgis->actionPluginPythonSeparator(); }
QAction *QgisAppInterface::actionShowPythonDialog() { return qgis->actionShowPythonDialog(); }

//! Settings menu actions
QAction *QgisAppInterface::actionToggleFullScreen() { return qgis->actionToggleFullScreen(); }
QAction *QgisAppInterface::actionSettingsSeparator1() { return qgis->actionSettingsSeparator1(); }
QAction *QgisAppInterface::actionOptions() { return qgis->actionOptions(); }
QAction *QgisAppInterface::actionCustomProjection() { return qgis->actionCustomProjection(); }

//! Help menu actions
QAction *QgisAppInterface::actionHelpContents() { return qgis->actionHelpContents(); }
QAction *QgisAppInterface::actionHelpSeparator1() { return qgis->actionHelpSeparator1(); }
QAction *QgisAppInterface::actionQgisHomePage() { return qgis->actionQgisHomePage(); }
QAction *QgisAppInterface::actionCheckQgisVersion() { return qgis->actionCheckQgisVersion(); }
QAction *QgisAppInterface::actionHelpSeparator2() { return qgis->actionHelpSeparator2(); }
QAction *QgisAppInterface::actionAbout() { return qgis->actionAbout(); }
