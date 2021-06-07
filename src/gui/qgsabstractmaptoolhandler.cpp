/***************************************************************************
    qgsabstractmaptoolhandler.cpp
    ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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


