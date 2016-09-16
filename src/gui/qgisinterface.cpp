/***************************************************************************
 *                           qgisinterface.cpp
 *                            Abstract base class for interfaces to functions in QgisApp
 *                           -------------------
 *      begin                : 2004-02-11
 *      copyright            : (C) 2004 by Gary E.Sherman
 *      email                : sherman at mrcc.com
 * ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#include "qgisinterface.h"
#include "qgsmapcanvas.h"

QgisInterface *QgisInterface::smInstance = nullptr;

QgisInterface::QgisInterface()
{
  smInstance = this;
  connect( this, SIGNAL( currentMapCanvasChanged( QgsMapCanvas* ) ), this, SLOT( notifiedCurrentMapCanvasChanged( QgsMapCanvas* ) ) );
}

QgisInterface::~QgisInterface()
{
  disconnect( this, SIGNAL( currentMapCanvasChanged( QgsMapCanvas* ) ), this, SLOT( notifiedCurrentMapCanvasChanged( QgsMapCanvas* ) ) );
}

QgisInterface *QgisInterface::instance()
{
  return smInstance;
}

void QgisInterface::notifiedCurrentMapCanvasChanged( QgsMapCanvas* mapCanvas )
{
  emit mapCanvas->scaleChanged( mapCanvas->scale() );
  emit mapCanvas->extentsChanged();
  emit mapCanvas->rotationChanged( mapCanvas->rotation() );
  emit mapCanvas->magnificationChanged( mapCanvas->magnificationFactor() );
  emit mapCanvas->layersChanged();
  emit mapCanvas->hasCrsTransformEnabledChanged( mapCanvas->hasCrsTransformEnabled() );
  emit mapCanvas->destinationCrsChanged();
  emit mapCanvas->mapUnitsChanged();
  emit mapCanvas->currentLayerChanged( mapCanvas->currentLayer() );
}
