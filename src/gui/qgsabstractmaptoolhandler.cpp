/***************************************************************************
    qgsabstractmaptoolhandler.cpp
    ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractmaptoolhandler.h"

QgsAbstractMapToolHandler::QgsAbstractMapToolHandler( QgsMapTool *tool, QAction *action )
  : mMapTool( tool )
  , mAction( action )
{
}

QgsAbstractMapToolHandler::~QgsAbstractMapToolHandler() = default;


QgsMapTool *QgsAbstractMapToolHandler::mapTool()
{
  return mMapTool;
}

QAction *QgsAbstractMapToolHandler::action()
{
  return mAction;
}

void QgsAbstractMapToolHandler::setLayerForTool( QgsMapLayer * )
{
}
