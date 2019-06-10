/***************************************************************************
  qgs3dmaptoolmeasureline.cpp
  --------------------------------------
  Date                 : Jun 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>

#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>

#include "qgsterrainentity_p.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dutils.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapcanvas.h"

#include "qgs3dmapscenepickhandler.h"

class Qgs3DMapToolMeasureLinePickHandler : public Qgs3DMapScenePickHandler
{
  public:
    Qgs3DMapToolMeasureLinePickHandler( Qgs3DMapToolMeasureLine *measureLineTool ): mMesaureLineTool( measureLineTool ) {}
    void handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection ) override;
  private:
    Qgs3DMapToolMeasureLine *mMesaureLineTool = nullptr;
};

void Qgs3DMapToolMeasureLinePickHandler::handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection )
{
  qInfo() << "handlePickOnVectorLayer" ;
  QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates(
                            QgsVector3D( worldIntersection.x(),
                                         worldIntersection.y(),
                                         worldIntersection.z() ), mMesaureLineTool->mCanvas->map()->origin() );
  QgsPoint pt( mapCoords.x(), mapCoords.y(), mapCoords.z() );
  qInfo() << "Coord (handlePickOnVectorLayer): " << pt.x() << " " << pt.y() << " " << pt.z();
  //  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  //  identifyTool2D->showResultsForFeature( vlayer, id, pt );

}

Qgs3DMapToolMeasureLine::Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  qInfo() << "Constructed";
  connect( mCanvas->scene(), &Qgs3DMapScene::terrainEntityChanged, this, &Qgs3DMapToolMeasureLine::onTerrainEntityChanged );
  mPickHandler.reset( new Qgs3DMapToolMeasureLinePickHandler( this ) );
}

Qgs3DMapToolMeasureLine::~Qgs3DMapToolMeasureLine() = default;

void Qgs3DMapToolMeasureLine::activate()
{
  qInfo() << "Measure line activated";
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    connect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }

  mCanvas->scene()->registerPickHandler( mPickHandler.get() );
}

void Qgs3DMapToolMeasureLine::deactivate()
{
  qInfo() << "Measure line deactivated";
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    disconnect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }

  mCanvas->scene()->unregisterPickHandler( mPickHandler.get() );
}

void Qgs3DMapToolMeasureLine::onTerrainPicked( Qt3DRender::QPickEvent *event )
{
  qInfo() << "onTerrainPicked";
  if ( event->button() != Qt3DRender::QPickEvent::LeftButton )
    return;

  const QVector3D worldIntersection = event->worldIntersection();
  QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( worldIntersection.x(),
                          worldIntersection.y(),
                          worldIntersection.z() ), mCanvas->map()->origin() );
  qInfo() << "Coord (onTerrainPicked): " << mapCoords.x() << " " << mapCoords.y() << " " << mapCoords.z();
  QgsPointXY mapPoint( mapCoords.x(), mapCoords.y() );
}


void Qgs3DMapToolMeasureLine::onTerrainEntityChanged()
{
  qInfo() << "onTerrainEntityChanged";
  // no need to disconnect from the previous entity: it has been destroyed
  // start listening to the new terrain entity
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    connect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }
}
