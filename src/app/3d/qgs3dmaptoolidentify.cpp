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


#include "qgs3dmapscenepickhandler.h"

class Qgs3DMapToolIdentifyPickHandler : public Qgs3DMapScenePickHandler
{
  public:
    Qgs3DMapToolIdentifyPickHandler( Qgs3DMapToolIdentify *identifyTool ): mIdentifyTool( identifyTool ) {}
    void handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection ) override;
  private:
    Qgs3DMapToolIdentify *mIdentifyTool = nullptr;
};


void Qgs3DMapToolIdentifyPickHandler::handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection )
{
  QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( worldIntersection, mIdentifyTool->mCanvas->map()->origin() );
  QgsPoint pt( mapCoords.x(), mapCoords.y(), mapCoords.z() );

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->showResultsForFeature( vlayer, id, pt );
}


//////


Qgs3DMapToolIdentify::Qgs3DMapToolIdentify( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  connect( mCanvas->scene(), &Qgs3DMapScene::terrainEntityChanged, this, &Qgs3DMapToolIdentify::onTerrainEntityChanged );

  mPickHandler.reset( new Qgs3DMapToolIdentifyPickHandler( this ) );
}

Qgs3DMapToolIdentify::~Qgs3DMapToolIdentify() = default;


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

  mCanvas->scene()->registerPickHandler( mPickHandler.get() );
}

void Qgs3DMapToolIdentify::deactivate()
{
  Qt3DRender::QObjectPicker *picker = mCanvas->scene()->terrainEntity()->terrainPicker();
  disconnect( picker, &Qt3DRender::QObjectPicker::pressed, this, &Qgs3DMapToolIdentify::onTerrainPicked );

  mCanvas->scene()->unregisterPickHandler( mPickHandler.get() );
}

void Qgs3DMapToolIdentify::onTerrainPicked( Qt3DRender::QPickEvent *event )
{
  if ( event->button() != Qt3DRender::QPickEvent::LeftButton )
    return;

  QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( event->worldIntersection(), mCanvas->map()->origin() );

  QgsGeometry geom = QgsGeometry::fromPointXY( QgsPointXY( mapCoords.x(), mapCoords.y() ) );

  // estimate search radius
  Qgs3DMapScene *scene = mCanvas->scene();
  double searchRadiusMM = QgsMapTool::searchRadiusMM();
  double pixelsPerMM = mCanvas->logicalDpiX() / 25.4;
  double searchRadiusPx = searchRadiusMM * pixelsPerMM;
  double searchRadiusMapUnits = scene->worldSpaceError( searchRadiusPx, event->distance() );

  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();

  identifyTool2D->identifyAndShowResults( geom, searchRadiusMapUnits );
}

void Qgs3DMapToolIdentify::onTerrainEntityChanged()
{
  // no need to disconnect from the previous entity: it has been destroyed
  // start listening to the new terrain entity
  Qt3DRender::QObjectPicker *picker = mCanvas->scene()->terrainEntity()->terrainPicker();
  connect( picker, &Qt3DRender::QObjectPicker::pressed, this, &Qgs3DMapToolIdentify::onTerrainPicked );
}
