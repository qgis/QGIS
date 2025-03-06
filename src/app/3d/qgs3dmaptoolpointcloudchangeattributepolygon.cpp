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

Qgs3DMapToolPointCloudChangeAttributePolygon::Qgs3DMapToolPointCloudChangeAttributePolygon( Qgs3DMapCanvas *canvas, const ToolType type = Polygon )
  : Qgs3DMapToolPointCloudChangeAttribute( canvas ), mToolType( type )
{
}

Qgs3DMapToolPointCloudChangeAttributePolygon::~Qgs3DMapToolPointCloudChangeAttributePolygon() = default;

void Qgs3DMapToolPointCloudChangeAttributePolygon::mousePressEvent( QMouseEvent *event )
{
  mClickPoint = event->pos();
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::mouseMoveEvent( QMouseEvent *event )
{
  if ( mToolType != Polygon && mScreenPoints.size() == 2 )
    return;
  const QgsPoint movedPoint = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() );
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
      mPolygonRubberBand->moveLastPoint( Qgs3DUtils::screenPointToMapCoordinates( QPoint( event->x(), mToolType == AboveLine ? 0 : mCanvas->height() ), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() ) );
      mPolygonRubberBand->addPoint( movedPoint );
    }
  }
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::keyPressEvent( QKeyEvent *event )
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
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::mouseReleaseEvent( QMouseEvent *event )
{
  if ( ( event->pos() - mClickPoint ).manhattanLength() > QApplication::startDragDistance() )
    return;

  const QgsPoint newPoint = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() );

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
      const QgsPoint screenEdgePoint = Qgs3DUtils::screenPointToMapCoordinates( QPoint( event->x(), mToolType == AboveLine ? 0 : mCanvas->height() ), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() );
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

void Qgs3DMapToolPointCloudChangeAttributePolygon::activate()
{
  // cannot move this to the constructor as there are no mapSettings available yet when the tool is created
  if ( !mPolygonRubberBand )
  {
    mPolygonRubberBand = std::make_unique<QgsRubberBand3D>( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
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
    mLineRubberBand = std::make_unique<QgsRubberBand3D>( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Line );
    mLineRubberBand->setHideLastMarker( true );
  }
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::deactivate()
{
  restart();
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::run()
{
  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );

  if ( mToolType == AboveLine || mToolType == BelowLine )
  {
    const double y = mToolType == AboveLine ? 0 : mCanvas->height();
    mScreenPoints.append( { QgsPointXY( mScreenPoints[1].x(), y ), QgsPointXY( mScreenPoints[0].x(), y ) } );
  }

  if ( mScreenPoints.size() < 3 )
    return;

  const QgsGeometry searchPolygon = QgsGeometry( new QgsPolygon( new QgsLineString( mScreenPoints ) ) );
  changeAttributeValue( searchPolygon, mAttributeName, mNewValue, *mCanvas, QgisApp::instance()->activeLayer() );
}

void Qgs3DMapToolPointCloudChangeAttributePolygon::restart()
{
  mCanvas->cameraController()->setInputHandlersEnabled( true );
  mScreenPoints.clear();
  mPolygonRubberBand->reset();
  if ( mToolType != Polygon )
  {
    mLineRubberBand->reset();
  }
}