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

#include "qgs3dmapscenepickhandler.h"

class Qgs3DMapToolMeasureLinePickHandler : public Qgs3DMapScenePickHandler
{
  public:
    Qgs3DMapToolMeasureLinePickHandler( Qgs3DMapToolMeasureLine *measureLineTool ): mMeasureLineTool( measureLineTool ) {}
    void handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection, Qt3DRender::QPickEvent *event ) override;
  private:
    Qgs3DMapToolMeasureLine *mMeasureLineTool = nullptr;
};

void Qgs3DMapToolMeasureLinePickHandler::handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection, Qt3DRender::QPickEvent *event )
{
  if ( event->button() == Qt3DRender::QPickEvent::LeftButton )
  {
    if ( mMeasureLineTool->done() )
    {
      mMeasureLineTool->mDialog->restart();
    }

    // Left button, keep addinng point
    QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates(
                              QgsVector3D( worldIntersection.x(),
                                           worldIntersection.y(),
                                           worldIntersection.z() ), mMeasureLineTool->mCanvas->map()->origin() );
    QgsPoint pt( mapCoords.x(), mapCoords.y(), mapCoords.z() );
    QgsDebugMsg( QStringLiteral( "Loading profiles path from global config at %1, %2, %3" ).arg( pt.x() ).arg( pt.y() ).arg( pt.z() ) );

    mMeasureLineTool->mDone = false;
    mMeasureLineTool->addPoint( pt );
    mMeasureLineTool->mDialog->show();
  }
  else if ( event->button() == Qt3DRender::QPickEvent::RightButton )
  {
    // Finish measurement
    QgsDebugMsg( "Finish measurement" );
    mMeasureLineTool->mDone = true;
  }
  else if ( event->button() == Qt3DRender::QPickEvent::MiddleButton )
  {
    QgsDebugMsg( "Undo - middle button" );
    mMeasureLineTool->undo();

  }
}

Qgs3DMapToolMeasureLine::Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  connect( mCanvas->scene(), &Qgs3DMapScene::terrainEntityChanged, this, &Qgs3DMapToolMeasureLine::onTerrainEntityChanged );
  mPickHandler.reset( new Qgs3DMapToolMeasureLinePickHandler( this ) );

  // Line style
  mLineSymbol = new QgsLine3DSymbol;
  mLineSymbol->setRenderAsSimpleLines( true );
  mLineSymbol->setWidth( 4 );
  mLineSymbol->setAltitudeClamping( Qgs3DTypes::AltClampAbsolute );
  QgsPhongMaterialSettings phongMaterial;
  phongMaterial.setAmbient( Qt::yellow );
  mLineSymbol->setMaterial( phongMaterial );

  mLineSymbolRenderer = new QgsVectorLayer3DRenderer( mLineSymbol );

  // Dialog
  mDialog = new Qgs3DMeasureDialog( this );
  mDialog->setWindowFlags( mDialog->windowFlags() | Qt::Tool );
  mDialog->restorePosition();
}

Qgs3DMapToolMeasureLine::~Qgs3DMapToolMeasureLine() = default;

void Qgs3DMapToolMeasureLine::activate()
{
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    connect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }

  mCanvas->scene()->registerPickHandler( mPickHandler.get() );

  // Initialize the measurement line
  mMeasurementLine = new QgsLineString();
  mMeasurementLine->addZValue();

  // Initialize measurement feature
  mMeasurementFeature = new QgsFeature( QgsFeatureId( 1 ) );
  mMeasurementFeature->setGeometry( QgsGeometry( mMeasurementLine ) );

  // Initialize the line layer
  QString mapCRS = mCanvas->map()->crs().authid();
  mMeasurementLayer = new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=" ) + mapCRS, QStringLiteral( "Measurement" ), QStringLiteral( "memory" ) );

  // Add feature to layer
  mMeasurementLayer->startEditing();
  mMeasurementLayer->addFeature( *mMeasurementFeature );
  mMeasurementLayer->commitChanges();

  // Set style
  setMeasurementLayerRenderer( mMeasurementLayer );

  // Add layer to canvas
  //mCanvas->map()->setLayers( mCanvas->map()->layers() << mMeasurementLayer );
  mCanvas->map()->setRenderers( QList<QgsAbstract3DRenderer *>() << mMeasurementLayer->renderer3D()->clone() );

  // Show dialog
  mDialog->show();
}

void Qgs3DMapToolMeasureLine::deactivate()
{
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    disconnect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }

  // Delete previouse line
  mMeasurementLayer->startEditing();
  mMeasurementLayer->deleteFeature( mMeasurementFeature->id() );
  mMeasurementLayer->commitChanges();
  mMeasurementLine->clear();

  mCanvas->map()->setRenderers( QList<QgsAbstract3DRenderer *>() << mMeasurementLayer->renderer3D()->clone() );

  mCanvas->scene()->unregisterPickHandler( mPickHandler.get() );

  // Hide dialog
  mDialog->hide();
}

void Qgs3DMapToolMeasureLine::onTerrainPicked( Qt3DRender::QPickEvent *event )
{
  if ( event->button() == Qt3DRender::QPickEvent::LeftButton )
  {
    if ( mDone )
    {
      mDialog->restart();
    }

    const QVector3D worldIntersection = event->worldIntersection();
    QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( worldIntersection.x(),
                            worldIntersection.y(),
                            worldIntersection.z() ), mCanvas->map()->origin() );
    QgsDebugMsg( QStringLiteral( "Coord (onTerrainPicked): %1, %2, %3" ).arg( mapCoords.x() ).arg( mapCoords.y() ).arg( mapCoords.z() ) );

    mDone = false;
    addPoint( QgsPoint( mapCoords.x(), mapCoords.y(), mapCoords.z() ) );
    mDialog->show();
  }
  else if ( event->button() == Qt3DRender::QPickEvent::RightButton )
  {
    // Finish measurement
    QgsDebugMsg( QStringLiteral( "Finish measurement" ) );
    mDone = true;
  }
  else if ( event->button() == Qt3DRender::QPickEvent::MiddleButton )
  {
    QgsDebugMsg( QStringLiteral( "Undo - middle button" ) );
    undo();
  }



}


void Qgs3DMapToolMeasureLine::onTerrainEntityChanged()
{
  // no need to disconnect from the previous entity: it has been destroyed
  // start listening to the new terrain entity
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    connect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }
}

void Qgs3DMapToolMeasureLine::setMeasurementLayerRenderer( QgsVectorLayer *layer )
{
  layer->setRenderer3D( mLineSymbolRenderer );
  mLineSymbolRenderer->setLayer( layer );
}


void Qgs3DMapToolMeasureLine::addPoint( const QgsPoint &point )
{
  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  QgsPoint addedPoint( point );

  // Append point
  mPoints.append( addedPoint );

  mMeasurementLine->addVertex( addedPoint );

  mMeasurementLayer->startEditing();
  QgsGeometry *newMeasurementLine = new QgsGeometry( mMeasurementLine );
  QgsDebugMsg( QStringLiteral( "Current line: %s" ).arg( newMeasurementLine->asWkt() ) );
  mMeasurementLayer->changeGeometry( mMeasurementFeature->id(), *newMeasurementLine );
  mMeasurementLayer->commitChanges();

  mDialog->addPoint();

  mCanvas->map()->setRenderers( QList<QgsAbstract3DRenderer *>() << mMeasurementLayer->renderer3D()->clone() );
}

void Qgs3DMapToolMeasureLine::restart()
{
  mPoints.clear();

  mDone = true;
  mMeasurementLine->clear();

  mMeasurementLayer->startEditing();
  QgsGeometry *newMeasurementLine = new QgsGeometry( mMeasurementLine );
  QgsDebugMsg( QStringLiteral( "Current line: %s" ).arg( newMeasurementLine->asWkt() ) );
  mMeasurementLayer->changeGeometry( mMeasurementFeature->id(), *newMeasurementLine );
  mMeasurementLayer->commitChanges();

  mCanvas->map()->setRenderers( QList<QgsAbstract3DRenderer *>() << mMeasurementLayer->renderer3D()->clone() );
}

void Qgs3DMapToolMeasureLine::undo()
{
  QgsDebugMsg( QStringLiteral( "Removing last point." ) );
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
    mMeasurementLine->deleteVertex( QgsVertexId( 0, 0, mMeasurementLine->numPoints() - 1 ) );

    mMeasurementLayer->startEditing();
    QgsGeometry *newMeasurementLine = new QgsGeometry( mMeasurementLine );
    mMeasurementLayer->changeGeometry( mMeasurementFeature->id(), *newMeasurementLine );
    mMeasurementLayer->commitChanges();

    mCanvas->map()->setRenderers( QList<QgsAbstract3DRenderer *>() << mMeasurementLayer->renderer3D()->clone() );

    mDialog->removeLastPoint();
  }
}

QVector<QgsPoint> Qgs3DMapToolMeasureLine::points() const
{
  return mPoints;
}
