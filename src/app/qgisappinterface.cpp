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

#include <iostream>
#include <QFileInfo>
#include <QString>
#include <QMenu>

#include "qgisappinterface.h"
#include "qgisapp.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapcanvas.h"
#include "qgslegend.h"

QgisAppInterface::QgisAppInterface( QgisApp * _qgis )
    : qgis( _qgis )
{
  // connect signals
  connect( qgis->legend(), SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SIGNAL( currentLayerChanged( QgsMapLayer * ) ) );

}

QgisAppInterface::~QgisAppInterface()
{
}

void QgisAppInterface::zoomFull()
{
  qgis->zoomFull();
}

void QgisAppInterface::zoomPrevious()
{
  qgis->zoomPrevious();
}

void QgisAppInterface::zoomActiveLayer()
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
QToolBar * QgisAppInterface::fileToolBar()
{
  return qgis->fileToolBar();
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

void QgisAppInterface::addDockWidget( Qt::DockWidgetArea area, QDockWidget * dockwidget )
{
  qgis->addDockWidget( area, dockwidget );
}

void QgisAppInterface::refreshLegend( QgsMapLayer *l )
{
  if ( l && qgis && qgis->legend() )
  {
    qgis->legend()->refreshLayerSymbology( l->getLayerID() );
  }
}
