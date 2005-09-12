/***************************************************************************
                          qgsiface.cpp
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
#include <qstring.h>
#include <qmenubar.h>
#include <qglobal.h>
#include "qgisinterface.h"
#include "qgisapp.h"
#include "qgsmaplayer.h"

QgisIface::QgisIface(QgisApp * _qgis, const char *name):qgis(_qgis)
{

}

QgisIface::~QgisIface()
{
}

void QgisIface::zoomFull()
{
  qgis->zoomFull();
}

void QgisIface::zoomPrevious()
{
  qgis->zoomPrevious();
}

void QgisIface::zoomActiveLayer()
{
  qgis->zoomToLayerExtent();
}

bool QgisIface::addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)
{
  qgis->addVectorLayer(vectorLayerPath, baseName, providerKey);
  //TODO fix this so it returns something meaningfull
  return true;
}

bool QgisIface::addRasterLayer(QString rasterLayerPath)
{
// TODO: This doesn't work in Qt4.  morb_au thinks this is casting to 
// "bool addRasterLayer(QgsRasterLayer * theRasterLayer, bool theForceRedrawFlag=false)"
// in qgisapp.h; and causing a
// "invalid conversion from ‘QNoImplicitBoolCast’ to ‘QgsRasterLayer*’" error
// not sure why or how to fix.
#if QT_VERSION < 0x040000
  return qgis->addRasterLayer(rasterLayerPath);
#endif
}

bool QgisIface::addRasterLayer(QgsRasterLayer * theRasterLayer, bool theForceRenderFlag)
{
  return qgis->addRasterLayer(theRasterLayer, theForceRenderFlag);
}

bool QgisIface::addProject(QString theProjectName)
{
  return qgis->addProject(theProjectName);
}

void QgisIface::newProject(bool thePromptToSaveFlag)
{
  qgis->fileNew(thePromptToSaveFlag);
}

QgsMapLayer *QgisIface::activeLayer()
{
  return qgis->activeLayer();
}

QString QgisIface::activeLayerSource()
{
  return qgis->activeLayerSource();
}
/*
int QgisIface::oldAddMenu(QString menuText, QPopupMenu * menu)
{
  QMenuBar *mainMenu = qgis->menuBar();
  // get the index of the help menu 
#ifdef QGISDEBUG
  std::cout << "Menu item count is : " << mainMenu->count() << std::endl;
#endif
  return mainMenu->insertItem(menuText, menu, -1, mainMenu->count() - 1);
}
*/

int QgisIface::addMenu(QString menuText, QPopupMenu * menu)
{
  // add the menu to the master Plugins menu
  return qgis->addPluginMenu(menuText, menu);
}
QPopupMenu* QgisIface::getPluginMenu(QString menuName)
{
  return qgis->getPluginMenu(menuName);
}

void QgisIface::removePluginMenuItem(QString name, int menuId)
{
  qgis->removePluginMenuItem(name, menuId);
}

int QgisIface::addToolBarIcon(QAction * qAction)
{
  // add the menu to the master Plugins menu
  return qgis->addPluginToolBarIcon(qAction);
}
void QgisIface::removeToolBarIcon(QAction *qAction)
{
  qgis->removePluginToolBarIcon(qAction);
}
void QgisIface::openURL(QString url, bool useQgisDocDirectory)
{
  qgis->openURL(url, useQgisDocDirectory);
}

std::map<QString, int> QgisIface::menuMapByName()
{
  return qgis->menuMapByName();
}

std::map<int, QString> QgisIface::menuMapById()
{
  return qgis->menuMapById();
}
  
QgsMapCanvas * QgisIface::getMapCanvas()
{
  return qgis->getMapCanvas();
}

QgsMapLayerRegistry * QgisIface::getLayerRegistry() 
{
  return qgis->getLayerRegistry();
}


QgisApp * 
QgisIface::app()
{
    return qgis;
} // QgisIface::app()
