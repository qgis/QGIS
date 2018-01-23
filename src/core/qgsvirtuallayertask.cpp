/***************************************************************************
                          qgsvirtuallayertask.cpp  -  description
                             -------------------
    begin                : Jan 19, 2018
    copyright            : (C) 2017 by Paul Blottiere
    email                : blottiere.paul@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvirtuallayertask.h"

QgsVirtualLayerTask::QgsVirtualLayerTask( const QgsVirtualLayerDefinition &definition )
  : QgsTask()
  , mDefinition( definition )
{
  mDefinition.setPostpone( true );
  mLayer = qgis::make_unique<QgsVectorLayer>( mDefinition.toString(), "layer", "virtual" );
}

bool QgsVirtualLayerTask::run()
{
  mLayer->reload(); // blocking call because the loading is postponed
  return mLayer->isValid();
}

QgsVirtualLayerDefinition QgsVirtualLayerTask::definition() const
{
  return mDefinition;
}

QgsVectorLayer *QgsVirtualLayerTask::layer()
{
  return mLayer.get();
}

void QgsVirtualLayerTask::cancel()
{
  mLayer->dataProvider()->cancel();
  QgsTask::cancel();
}
