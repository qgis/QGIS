/***************************************************************************
  qgs3dmaptoolidentify.cpp
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmaptoolidentify.h"

#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgsterrainentity_p.h"

#include "qgisapp.h"
#include "qgsmaptoolidentifyaction.h"

#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>

Qgs3DMapToolIdentify::Qgs3DMapToolIdentify( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  connect( mCanvas->scene(), &Qgs3DMapScene::terrainEntityChanged, this, &Qgs3DMapToolIdentify::onTerrainEntityChanged );
}

void Qgs3DMapToolIdentify::mousePressEvent( QMouseEvent *event )
{
  Q_UNUSED( event );

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->clearResults();
}

void Qgs3DMapToolIdentify::activate()
{
  Qt3DRender::QObjectPicker *picker = mCanvas->scene()->terrainEntity()->terrainPicker();
  connect( picker, &Qt3DRender::QObjectPicker::pressed, this, &Qgs3DMapToolIdentify::onTerrainPicked );
}

void Qgs3DMapToolIdentify::deactivate()
{
  Qt3DRender::QObjectPicker *picker = mCanvas->scene()->terrainEntity()->terrainPicker();
  disconnect( picker, &Qt3DRender::QObjectPicker::pressed, this, &Qgs3DMapToolIdentify::onTerrainPicked );
}

void Qgs3DMapToolIdentify::onTerrainPicked( Qt3DRender::QPickEvent *event )
{
  QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( event->worldIntersection(), mCanvas->map()->origin() );

  QgsGeometry geom = QgsGeometry::fromPointXY( QgsPointXY( mapCoords.x(), mapCoords.y() ) );

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();

  identifyTool2D->identifyAndShowResults( geom );
}

void Qgs3DMapToolIdentify::onTerrainEntityChanged()
{
  // no need to disconnect from the previous entity: it has been destroyed
  // start listening to the new terrain entity
  Qt3DRender::QObjectPicker *picker = mCanvas->scene()->terrainEntity()->terrainPicker();
  connect( picker, &Qt3DRender::QObjectPicker::pressed, this, &Qgs3DMapToolIdentify::onTerrainPicked );
}
