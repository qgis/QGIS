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

#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <QKeyEvent>

#include "qgsterrainentity_p.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dutils.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include "qgsline3dsymbol.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgsmaplayer.h"
#include "qgs3dmeasuredialog.h"
#include "qgsrubberband3d.h"

#include "qgs3dmapscenepickhandler.h"

class Qgs3DMapToolMeasureLinePickHandler : public Qgs3DMapScenePickHandler
{
  public:
    Qgs3DMapToolMeasureLinePickHandler( Qgs3DMapToolMeasureLine *measureLineTool ): mMeasureLineTool( measureLineTool ) {}
    void handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection, Qt3DRender::QPickEvent *event ) override;
  private:
    Qgs3DMapToolMeasureLine *mMeasureLineTool = nullptr;
};

void Qgs3DMapToolMeasureLinePickHandler::handlePickOnVectorLayer( QgsVectorLayer *, QgsFeatureId, const QVector3D &worldIntersection, Qt3DRender::QPickEvent *event )
{
  mMeasureLineTool->handleClick( event, worldIntersection );
}

Qgs3DMapToolMeasureLine::Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  mPickHandler.reset( new Qgs3DMapToolMeasureLinePickHandler( this ) );

  // Dialog
  mDialog = new Qgs3DMeasureDialog( this );
  mDialog->setWindowFlags( mDialog->windowFlags() | Qt::Tool );
  mDialog->restorePosition();

  // Update scale if the terrain vertical scale changed
  connect( canvas, &Qgs3DMapCanvas::mapSettingsChanged, this, &Qgs3DMapToolMeasureLine::onMapSettingsChanged );

}

Qgs3DMapToolMeasureLine::~Qgs3DMapToolMeasureLine() = default;

void Qgs3DMapToolMeasureLine::activate()
{
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    connect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }

  mCanvas->scene()->registerPickHandler( mPickHandler.get() );

  mRubberBand.reset( new QgsRubberBand3D( *mCanvas->map(), mCanvas->cameraController(), mCanvas->scene() ) );

  if ( mIsAlreadyActivated )
  {
    restart();
    updateSettings();
  }
  else
  {
    // Set style
    updateSettings();
    mIsAlreadyActivated = true;
  }
  // Show dialog
  mDialog->updateSettings();
  mDialog->show();
}

void Qgs3DMapToolMeasureLine::deactivate()
{
  restart();
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    disconnect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }

  mCanvas->scene()->unregisterPickHandler( mPickHandler.get() );

  mRubberBand.reset();

  // Hide dialog
  mDialog->hide();
}

QCursor Qgs3DMapToolMeasureLine::cursor() const
{
  return Qt::CrossCursor;
}

void Qgs3DMapToolMeasureLine::onMapSettingsChanged()
{
  if ( !mIsAlreadyActivated )
    return;
  connect( mCanvas->scene(), &Qgs3DMapScene::terrainEntityChanged, this, &Qgs3DMapToolMeasureLine::onTerrainEntityChanged );
}

void Qgs3DMapToolMeasureLine::onTerrainPicked( Qt3DRender::QPickEvent *event )
{
  handleClick( event, event->worldIntersection() );
}

void Qgs3DMapToolMeasureLine::onTerrainEntityChanged()
{
  if ( !mIsAlreadyActivated )
    return;
  // no need to disconnect from the previous entity: it has been destroyed
  // start listening to the new terrain entity
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    connect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }
}

void Qgs3DMapToolMeasureLine::handleClick( Qt3DRender::QPickEvent *event, const QgsVector3D &worldIntersection )
{
  if ( event->button() == Qt3DRender::QPickEvent::LeftButton )
  {
    if ( mDone )
    {
      mDialog->restart();
    }

    mDone = false;
    const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( worldIntersection.x(),
                                  worldIntersection.y(),
                                  worldIntersection.z() ), mCanvas->map()->origin() );
    addPoint( QgsPoint( mapCoords.x(), mapCoords.y(), mapCoords.z() ) );
    mDialog->show();
  }
  else if ( event->button() == Qt3DRender::QPickEvent::RightButton )
  {
    // Finish measurement
    mDone = true;
    restart();
  }
  else if ( event->button() == Qt3DRender::QPickEvent::MiddleButton )
  {
    undo();
  }
}



void Qgs3DMapToolMeasureLine::updateSettings()
{
  if ( mRubberBand )
  {
    const QgsSettings settings;
    const int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
    const int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
    const int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();

    mRubberBand->setWidth( 3 );
    mRubberBand->setColor( QColor( myRed, myGreen, myBlue ) );
  }
}

void Qgs3DMapToolMeasureLine::addPoint( const QgsPoint &point )
{
  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  const QgsPoint addedPoint( point );

  mPoints.append( addedPoint );
  mDialog->addPoint();

  mRubberBand->addPoint( QgsPoint( point.x(), point.y(), point.z() / canvas()->map()->terrainVerticalScale() ) );
}

void Qgs3DMapToolMeasureLine::restart()
{
  mPoints.clear();
  mDone = true;
  mDialog->resetTable();

  mRubberBand->reset();
}

void Qgs3DMapToolMeasureLine::undo()
{
  if ( mPoints.empty() )
  {
    return;
  }
  if ( mPoints.size() == 1 )
  {
    //removing first point, so restart everything
    restart();
    mDialog->restart();
  }
  else
  {
    mPoints.removeLast();
    mDialog->removeLastPoint();

    mRubberBand->removeLastPoint();
  }
}

QVector<QgsPoint> Qgs3DMapToolMeasureLine::points() const
{
  return mPoints;
}
