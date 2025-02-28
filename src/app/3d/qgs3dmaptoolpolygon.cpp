/***************************************************************************
    qgs3dmaptoolpolygon.cpp
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
#include "qgs3dmaptoolpolygon.h"
#include "moc_qgs3dmaptoolpolygon.cpp"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgsframegraph.h"
#include "qgsguiutils.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgsrubberband3d.h"
#include "qgswindow3dengine.h"
#include "qgs3deditutils.h"

#include <QApplication>
#include <QMouseEvent>

Qgs3DMapToolPolygon::Qgs3DMapToolPolygon( Qgs3DMapCanvas *canvas )
  : Qgs3DMapToolPointCloudChangeAttribute( canvas )
{
}

Qgs3DMapToolPolygon::~Qgs3DMapToolPolygon() = default;

void Qgs3DMapToolPolygon::mousePressEvent( QMouseEvent *event )
{
  if ( !mIsMoving )
  {
    mClickPoint = event->pos();
  }
}

void Qgs3DMapToolPolygon::mouseMoveEvent( QMouseEvent *event )
{
  if ( !mIsMoving )
  {
    const QgsPoint movedPoint = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), *mCanvas );
    mPolygonRubberBand->moveLastPoint( movedPoint );
  }
}

void Qgs3DMapToolPolygon::keyPressEvent( QKeyEvent *event )
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
      mPolygonRubberBand->removeLastPoint();
    }
  }
  else if ( event->key() == Qt::Key_Escape )
  {
    restart();
  }
  else if ( event->key() == Qt::Key_Space )
  {
    const bool newState = !mCanvas->cameraController()->inputHandlersEnabled();
    mCanvas->cameraController()->setInputHandlersEnabled( newState );
    mIsMoving = newState;
  }
}

void Qgs3DMapToolPolygon::mouseReleaseEvent( QMouseEvent *event )
{
  if ( ( event->pos() - mClickPoint ).manhattanLength() > QApplication::startDragDistance() || mIsMoving )
    return;

  const QgsPoint newPoint = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), *mCanvas );

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

void Qgs3DMapToolPolygon::activate()
{
  // cannot move this to the constructor as there are no mapSettings available yet when the tool is created
  if ( !mPolygonRubberBand )
  {
    mPolygonRubberBand = new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
    mPolygonRubberBand->setHideLastMarker( true );
  }
}

void Qgs3DMapToolPolygon::deactivate()
{
  restart();
}

void Qgs3DMapToolPolygon::run()
{
  if ( mScreenPoints.size() < 3 )
    return;

  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );

  const QgsGeometry searchPolygon = QgsGeometry( new QgsPolygon( new QgsLineString( mScreenPoints ) ) );
  Qgs3DEditUtils::changeAttributeValue( searchPolygon, mAttributeName, mNewValue, *mCanvas );
}

void Qgs3DMapToolPolygon::restart()
{
  mCanvas->cameraController()->setInputHandlersEnabled( true );
  mScreenPoints.clear();
  mPolygonRubberBand->reset();
}