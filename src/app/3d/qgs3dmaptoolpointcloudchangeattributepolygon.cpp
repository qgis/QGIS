/***************************************************************************
    qgs3dmaptoolpointcloudchangeattributepolygon.cpp
    ---------------------
    begin                : February 2025
    copyright            : (C) 2025 by Matej Bagar
    email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgs3dmaptoolpointcloudchangeattributepolygon.h"
#include "moc_qgs3dmaptoolpointcloudchangeattributepolygon.cpp"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgsframegraph.h"
#include "qgsguiutils.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsrubberband3d.h"
#include "qgswindow3dengine.h"
#include "qgisapp.h"

#include <QApplication>
#include <QMouseEvent>


Qgs3DMapToolPointCloudChangeAttributePolygon::Qgs3DMapToolPointCloudChangeAttributePolygon( Qgs3DMapCanvas *canvas )
  : Qgs3DMapToolPointCloudChangeAttribute( canvas )
{
}

Qgs3DMapToolPointCloudChangeAttributePolygon::~Qgs3DMapToolPointCloudChangeAttributePolygon() = default;

void Qgs3DMapToolPointCloudChangeAttributePolygon::mousePressEvent( QMouseEvent *event )
{
  if ( !mIsMoving )
  {
    mClickPoint = event->pos();
  }
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::mouseMoveEvent( QMouseEvent *event )
{
  if ( !mIsMoving )
  {
    const QgsPoint movedPoint = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() );
    mPolygonRubberBand->moveLastPoint( movedPoint );
  }
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete )
  {
    if ( mScreenPoints.isEmpty() )
    {
      return;
    }
    else if ( mScreenPoints.size() == 1 )
    {
      //removing first point, so restart everything
      restart();
    }
    else
    {
      mScreenPoints.removeLast();
      mPolygonRubberBand->removePenultimatePoint();
    }
  }
  else if ( event->key() == Qt::Key_Escape )
  {
    restart();
  }
  else if ( event->key() == Qt::Key_Space )
  {
    const bool newState = !mCanvas->cameraController()->hasInputHandlersEnabled();
    mCanvas->cameraController()->setInputHandlersEnabled( newState );
    mIsMoving = newState;
  }
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::mouseReleaseEvent( QMouseEvent *event )
{
  if ( ( event->pos() - mClickPoint ).manhattanLength() > QApplication::startDragDistance() || mIsMoving )
    return;

  const QgsPoint newPoint = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() );

  if ( event->button() == Qt::LeftButton )
  {
    if ( mPolygonRubberBand->isEmpty() )
    {
      mPolygonRubberBand->addPoint( newPoint );
      mCanvas->cameraController()->setInputHandlersEnabled( false );
    }
    mPolygonRubberBand->addPoint( newPoint );
    mScreenPoints.append( QgsPointXY( event->x(), event->y() ) );
  }
  else if ( event->button() == Qt::RightButton )
  {
    run();
    restart();
  }
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::activate()
{
  // cannot move this to the constructor as there are no mapSettings available yet when the tool is created
  if ( !mPolygonRubberBand )
  {
    mPolygonRubberBand = new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
    mPolygonRubberBand->setHideLastMarker( true );
  }
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::deactivate()
{
  restart();
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::run()
{
  if ( mScreenPoints.size() < 3 )
    return;

  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );

  const QgsGeometry searchPolygon = QgsGeometry( new QgsPolygon( new QgsLineString( mScreenPoints ) ) );
  changeAttributeValue( searchPolygon, mAttributeName, mNewValue, *mCanvas, QgisApp::instance()->activeLayer() );
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::restart()
{
  mCanvas->cameraController()->setInputHandlersEnabled( true );
  mScreenPoints.clear();
  mPolygonRubberBand->reset();
}
