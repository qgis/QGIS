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

int QgisIface::getInt()
{
  return qgis->getInt();
}

void QgisIface::addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)
{
  qgis->addVectorLayer(vectorLayerPath, baseName, providerKey);
}

void QgisIface::addRasterLayer(QString rasterLayerPath)
{
  qgis->addRasterLayer(rasterLayerPath);
}

QgsMapLayer *QgisIface::activeLayer()
{
  return qgis->activeLayer();
}

QString QgisIface::activeLayerSource()
{
  return qgis->activeLayerSource();
}

int QgisIface::addMenu(QString menuText, QPopupMenu * menu)
{
  QMenuBar *mainMenu = qgis->menuBar();
  // get the index of the help menu 
#ifdef DEBUG
  std::cout << "Menu item count is : " << mainMenu->count() << std::endl;
#endif
  return mainMenu->insertItem(menuText, menu, -1, mainMenu->count() - 1);
}

void QgisIface::openURL(QString url, bool useQgisDocDirectory)
{
  qgis->openURL(url, useQgisDocDirectory);
}
