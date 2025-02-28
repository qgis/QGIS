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

Qgs3DMapToolPolygon::Qgs3DMapToolPolygon( Qgs3DMapCanvas *canvas, const ToolType type = Polygon )
  : Qgs3DMapToolPointCloudChangeAttribute( canvas ), mToolType( type )
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
    if ( mToolType != Polygon && mScreenPoints.size() == 2 )
      return;
    const QgsPoint movedPoint = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), *mCanvas );
    if ( mToolType == Polygon )
    {
      mPolygonRubberBand->moveLastPoint( movedPoint );
    }
    else
    {
      mLineRubberBand->moveLastPoint( movedPoint );
      if ( !mPolygonRubberBand->isEmpty() )
      {
        mPolygonRubberBand->removeLastPoint();
        mPolygonRubberBand->moveLastPoint( Qgs3DUtils::screenPointToMapCoordinates( QPoint( event->x(), mToolType == AboveLinePolygon ? 0 : mCanvas->height() ), *mCanvas ) );
        mPolygonRubberBand->addPoint( movedPoint );
      }
    }
  }
}

void Qgs3DMapToolPolygon::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete )
  {
    if ( mScreenPoints.size() == 1 )
    {
      //removing first point, so restart everything
      restart();
    }
    else if ( mScreenPoints.size() > 1 )
    {
      mScreenPoints.removeLast();
      mPolygonRubberBand->removeLastPoint();
      if ( mToolType != Polygon )
      {
        mPolygonRubberBand->removeLastPoint();
        mLineRubberBand->removeLastPoint();
      }
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
    if ( mToolType != Polygon && mScreenPoints.size() > 2 )
      return;

    mScreenPoints.append( QgsPointXY( event->x(), event->y() ) );

    if ( mToolType == Polygon )
    {
      if ( mPolygonRubberBand->isEmpty() )
      {
        mPolygonRubberBand->addPoint( newPoint );
        mCanvas->cameraController()->setInputHandlersEnabled( false );
      }
      mPolygonRubberBand->addPoint( newPoint );
    }
    else
    {
      const QgsPoint screenEdgePoint = Qgs3DUtils::screenPointToMapCoordinates( QPoint( event->x(), mToolType == AboveLinePolygon ? 0 : mCanvas->height() ), *mCanvas );
      if ( mLineRubberBand->isEmpty() )
      {
        mLineRubberBand->addPoint( newPoint );

        mPolygonRubberBand->addPoint( newPoint );
        mPolygonRubberBand->addPoint( screenEdgePoint );
        mCanvas->cameraController()->setInputHandlersEnabled( false );
      }
      mLineRubberBand->addPoint( newPoint );
      if ( mScreenPoints.size() < 2 )
      {
        mPolygonRubberBand->addPoint( screenEdgePoint );
        mPolygonRubberBand->addPoint( newPoint );
      }
    }
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
    if ( mToolType == Polygon )
    {
      mPolygonRubberBand->setHideLastMarker( true );
    }
    else
    {
      mPolygonRubberBand->setEdgesEnabled( false );
      mPolygonRubberBand->setMarkersEnabled( false );
    }
  }
  if ( !mLineRubberBand && mToolType != Polygon )
  {
    mLineRubberBand = new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Line );
    mLineRubberBand->setHideLastMarker( true );
  }
}

void Qgs3DMapToolPolygon::deactivate()
{
  restart();
}

void Qgs3DMapToolPolygon::run()
{
  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );

  if ( mToolType != Polygon )
  {
    if ( mToolType == AboveLinePolygon )
    {
      mScreenPoints.append( { QgsPointXY( mScreenPoints[1].x(), 0 ), QgsPointXY( mScreenPoints[0].x(), 0 ) } );
    }
    else if ( mToolType == BelowLinePolygon )
    {
      mScreenPoints.append( { QgsPointXY( mScreenPoints[1].x(), mCanvas->height() ), QgsPointXY( mScreenPoints[0].x(), mCanvas->height() ) } );
    }
  }

  if ( mScreenPoints.size() < 3 )
    return;

  const QgsGeometry searchPolygon = QgsGeometry( new QgsPolygon( new QgsLineString( mScreenPoints ) ) );
  Qgs3DEditUtils::changeAttributeValue( searchPolygon, mAttributeName, mNewValue, *mCanvas );
}

void Qgs3DMapToolPolygon::restart()
{
  mCanvas->cameraController()->setInputHandlersEnabled( true );
  mScreenPoints.clear();
  mPolygonRubberBand->reset();
  if ( mToolType != Polygon )
  {
    mLineRubberBand->reset();
  }
}