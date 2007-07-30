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
#include <QString>
#include <QMenu>

#include "qgisappinterface.h"
#include "qgisapp.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapcanvas.h"
#include "qgslegend.h"

QgisAppInterface::QgisAppInterface(QgisApp * _qgis)
  : qgis(_qgis)
{
    // connect signals
    connect ( qgis->legend(), SIGNAL(currentLayerChanged(QgsMapLayer *)),
              this, SIGNAL(currentLayerChanged(QgsMapLayer *)) );

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

bool QgisAppInterface::addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)
{
  qgis->addVectorLayer(vectorLayerPath, baseName, providerKey);
  //TODO fix this so it returns something meaningfull
  return true;
}

bool QgisAppInterface::addRasterLayer(QString rasterLayerPath)
{
  return qgis->addRasterLayer( QStringList(rasterLayerPath) );
}

bool QgisAppInterface::addRasterLayer(QgsRasterLayer * theRasterLayer, bool theForceRenderFlag)
{
  return qgis->addRasterLayer(theRasterLayer, theForceRenderFlag);
}

bool QgisAppInterface::addProject(QString theProjectName)
{
  return qgis->addProject(theProjectName);
}

void QgisAppInterface::newProject(bool thePromptToSaveFlag)
{
  qgis->fileNew(thePromptToSaveFlag);
}

QgsMapLayer *QgisAppInterface::activeLayer()
{
  return qgis->activeLayer();
}

void QgisAppInterface::addPluginMenu(QString name, QAction* action)
{
  qgis->addPluginMenu(name, action);
}

void QgisAppInterface::removePluginMenu(QString name, QAction* action)
{
  qgis->removePluginMenu(name, action);
}

int QgisAppInterface::addToolBarIcon(QAction * qAction)
{
  // add the menu to the master Plugins menu
  return qgis->addPluginToolBarIcon(qAction);
}
void QgisAppInterface::removeToolBarIcon(QAction *qAction)
{
  qgis->removePluginToolBarIcon(qAction);
}
QToolBar* QgisAppInterface::addToolBar(QString name)
{
  return qgis->addToolBar(name);
}
void QgisAppInterface::openURL(QString url, bool useQgisDocDirectory)
{
  qgis->openURL(url, useQgisDocDirectory);
}

std::map<QString, int> QgisAppInterface::menuMapByName()
{
  return qgis->menuMapByName();
}

std::map<int, QString> QgisAppInterface::menuMapById()
{
  return qgis->menuMapById();
}
  
QgsMapCanvas * QgisAppInterface::getMapCanvas()
{
  return qgis->getMapCanvas();
}

QWidget * QgisAppInterface::getMainWindow()
{
  return qgis;
}

QToolBox* QgisAppInterface::getToolBox()
{
  return qgis->toolBox;
}

void QgisAppInterface::refreshLegend(QgsMapLayer *l)
{
  qgis->legend()->refreshLayerSymbology(l->getLayerID());
}
