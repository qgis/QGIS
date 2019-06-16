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
    void handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection ) override;
  private:
    Qgs3DMapToolMeasureLine *mMeasureLineTool = nullptr;
};

void Qgs3DMapToolMeasureLinePickHandler::handlePickOnVectorLayer( QgsVectorLayer *vlayer, QgsFeatureId id, const QVector3D &worldIntersection )
{
  qInfo() << "handlePickOnVectorLayer" ;
  QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates(
                            QgsVector3D( worldIntersection.x(),
                                         worldIntersection.y(),
                                         worldIntersection.z() ), mMeasureLineTool->mCanvas->map()->origin() );
  QgsPoint pt( mapCoords.x(), mapCoords.y(), mapCoords.z() );
  qInfo() << "Coord (handlePickOnVectorLayer): " << pt.x() << " " << pt.y() << " " << pt.z();
  mMeasureLineTool->addPoint( pt );
}

Qgs3DMapToolMeasureLine::Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  qInfo() << "Constructed";
  connect( mCanvas->scene(), &Qgs3DMapScene::terrainEntityChanged, this, &Qgs3DMapToolMeasureLine::onTerrainEntityChanged );
  mPickHandler.reset( new Qgs3DMapToolMeasureLinePickHandler( this ) );

  // Line style
  QgsLine3DSymbol *mLineSymbol = new QgsLine3DSymbol;
  mLineSymbol->setRenderAsSimpleLines( true );
  mLineSymbol->setWidth( 4 );
  QgsPhongMaterialSettings phongMaterial;
  phongMaterial.setAmbient( Qt::yellow );
  phongMaterial.setDiffuse( Qt::green );
  mLineSymbol->setMaterial( phongMaterial );

  // Dialog
  mDialog = new Qgs3DMeasureDialog( this );
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
  qInfo() << "Current layer: " << mCanvas->map()->layers();
  mCanvas->map()->setLayers( mCanvas->map()->layers() << mMeasurementLayer );
  qInfo() << "Current layer after adding: " << mCanvas->map()->layers();

  // Show dialog
  mDialog->show();
}

void Qgs3DMapToolMeasureLine::deactivate()
{
  qInfo() << "Measure line deactivated";
  if ( QgsTerrainEntity *terrainEntity = mCanvas->scene()->terrainEntity() )
  {
    disconnect( terrainEntity->terrainPicker(), &Qt3DRender::QObjectPicker::clicked, this, &Qgs3DMapToolMeasureLine::onTerrainPicked );
  }

  // Delete previouse line
  mMeasurementLayer->startEditing();
  mMeasurementLayer->deleteFeature( mMeasurementFeature->id() );
  mMeasurementLayer->commitChanges();
  mMeasurementLine->clear();

  mCanvas->scene()->unregisterPickHandler( mPickHandler.get() );

  // Hide dialog
  mDialog->hide();
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
  addPoint( QgsPoint( mapCoords.x(), mapCoords.y(), mapCoords.z() ) );
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

void Qgs3DMapToolMeasureLine::setMeasurementLayerRenderer( QgsVectorLayer *layer )
{
  layer->setRenderer3D( new QgsVectorLayer3DRenderer( mLineSymbol ) );
}


void Qgs3DMapToolMeasureLine::addPoint( const QgsPoint &point )
{
  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  QgsPoint addedPoint( point );
  qInfo() << "Is 3D point? " << addedPoint.is3D();

  // Append point
  mPoints.append( addedPoint );

  mMeasurementLine->addVertex( addedPoint );

  mMeasurementLayer->startEditing();
  QgsGeometry *newMeasurementLine = new QgsGeometry( mMeasurementLine );
  qInfo() << "Current line: " << newMeasurementLine->asWkt();
  mMeasurementLayer->changeGeometry( mMeasurementFeature->id(), *newMeasurementLine );
  mMeasurementLayer->commitChanges();
}

void Qgs3DMapToolMeasureLine::restart()
{
  mPoints.clear();

  mDone = true;
  mMeasurementLine->clear();

  mMeasurementLayer->startEditing();
  QgsGeometry *newMeasurementLine = new QgsGeometry( mMeasurementLine );
  qInfo() << "Current line: " << newMeasurementLine->asWkt();
  mMeasurementLayer->changeGeometry( mMeasurementFeature->id(), *newMeasurementLine );
  mMeasurementLayer->commitChanges();
}

void Qgs3DMapToolMeasureLine::removeLastPoint()
{

  qInfo() << "Removing last point.";
  if ( mPoints.empty() )
  {
    return;
  }
  if ( mPoints.size() == 1 )
  {
    //removing first point, so restart everything
    restart();
  }
  else
  {
    mPoints.removeLast();
    mMeasurementLine->deleteVertex( QgsVertexId( 0, 0, mMeasurementLine->numPoints() - 1 ) );

    mMeasurementLayer->startEditing();
    QgsGeometry *newMeasurementLine = new QgsGeometry( mMeasurementLine );
    qInfo() << "Current line: " << newMeasurementLine->asWkt();
    mMeasurementLayer->changeGeometry( mMeasurementFeature->id(), *newMeasurementLine );
    mMeasurementLayer->commitChanges();
  }
}

// TODO: this is not picking the event
void Qgs3DMapToolMeasureLine::keyPressEvent( QKeyEvent *e )
{
  if ( ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
  {
    qInfo() << "Backspace or Delete key pressed";
    if ( !mDone )
    {
      removeLastPoint();
    }

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}
