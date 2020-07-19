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

  if ( mIsAlreadyActivated )
  {
    restart();
    updateSettings();
  }
  else
  {
    QgsLineString *measurementLine = new QgsLineString();
    measurementLine->addZValue();

    QgsFeature measurementFeature( QgsFeatureId( 1 ) );
    measurementFeature.setGeometry( QgsGeometry( measurementLine ) );

    // Initialize the line layer
    QString mapCRS = mCanvas->map()->crs().authid();
    mMeasurementLayer = new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=" ) + mapCRS, QStringLiteral( "Measurement" ), QStringLiteral( "memory" ) );
    QgsProject::instance()->addMapLayer( mMeasurementLayer );

    // Add feature to layer
    mMeasurementLayer->startEditing();
    mMeasurementLayer->addFeature( measurementFeature );
    mMeasurementLayer->commitChanges();

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

  // Update scale if the terrain vertical scale changed
  connect( mCanvas->map(), &Qgs3DMapSettings::terrainVerticalScaleChanged, this, &Qgs3DMapToolMeasureLine::updateMeasurementLayer );
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
    QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( QgsVector3D( worldIntersection.x(),
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

void Qgs3DMapToolMeasureLine::updateMeasurementLayer()
{
  if ( !mMeasurementLayer )
    return;
  double verticalScale = canvas()->map()->terrainVerticalScale();
  QgsLineString *line;
  if ( verticalScale != 1.0 )
  {
    QVector<QgsPoint> descaledPoints;
    QVector<QgsPoint>::const_iterator it;
    QgsPoint point;
    for ( it = mPoints.constBegin(); it != mPoints.constEnd(); ++it )
    {
      point = *it;
      descaledPoints.append(
        QgsPoint( it->x(), it->y(), it->z() / verticalScale )
      );
    }
    line = new QgsLineString( descaledPoints );
  }
  else
  {
    line = new QgsLineString( mPoints );
  }
  QgsGeometry lineGeometry( line );

  QgsGeometryMap geometryMap;
  geometryMap.insert( 1, lineGeometry );
  mMeasurementLayer->dataProvider()->changeGeometryValues( geometryMap );
  mMeasurementLayer->reload();
  mCanvas->map()->setRenderers( QList<QgsAbstract3DRenderer *>() << mMeasurementLayer->renderer3D()->clone() );
}

void Qgs3DMapToolMeasureLine::updateSettings()
{
  if ( !mMeasurementLayer )
    return;
  // Line style
  QgsLine3DSymbol *lineSymbol = new QgsLine3DSymbol;
  lineSymbol->setRenderAsSimpleLines( true );
  lineSymbol->setWidth( 4 );
  lineSymbol->setAltitudeClamping( Qgs3DTypes::AltClampAbsolute );

  QgsPhongMaterialSettings phongMaterial;
  QgsSettings settings;
  int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
  int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
  int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();
  phongMaterial.setAmbient( QColor( myRed, myGreen, myBlue ) );
  lineSymbol->setMaterial( phongMaterial );

  // Set renderer
  QgsVectorLayer3DRenderer *lineSymbolRenderer = new QgsVectorLayer3DRenderer( lineSymbol );
  mMeasurementLayer->setRenderer3D( lineSymbolRenderer );
  lineSymbolRenderer->setLayer( mMeasurementLayer );
  mCanvas->map()->setRenderers( QList<QgsAbstract3DRenderer *>() << mMeasurementLayer->renderer3D()->clone() );
}

void Qgs3DMapToolMeasureLine::addPoint( const QgsPoint &point )
{
  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  QgsPoint addedPoint( point );

  mPoints.append( addedPoint );
  updateMeasurementLayer();
  mDialog->addPoint();
}

void Qgs3DMapToolMeasureLine::restart()
{
  mPoints.clear();
  mDone = true;
  updateMeasurementLayer();
  mDialog->resetTable();
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
    updateMeasurementLayer();
    mDialog->removeLastPoint();
  }
}

QVector<QgsPoint> Qgs3DMapToolMeasureLine::points() const
{
  return mPoints;
}
