/***************************************************************************
  qgs3dmaptoolpointcloudchangeattributepaintbrush.cpp
  --------------------------------------
  Date                 : January 2025
  Copyright            : (C) 2025 by Matej Bagar
  Email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dmaptoolpointcloudchangeattributepaintbrush.h"

#include <memory>

#include "qgisapp.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgsframegraph.h"
#include "qgsgeometry.h"
#include "qgsguiutils.h"
#include "qgslinestring.h"
#include "qgsrubberband3d.h"
#include "qgswindow3dengine.h"

#include "moc_qgs3dmaptoolpointcloudchangeattributepaintbrush.cpp"

class QgsPointCloudAttribute;
Qgs3DMapToolPointCloudChangeAttributePaintbrush::Qgs3DMapToolPointCloudChangeAttributePaintbrush( Qgs3DMapCanvas *canvas )
  : Qgs3DMapToolPointCloudChangeAttribute( canvas )
{
}

Qgs3DMapToolPointCloudChangeAttributePaintbrush::~Qgs3DMapToolPointCloudChangeAttributePaintbrush() = default;

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::run()
{
  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );

  const QgsGeometry searchSkeleton = QgsGeometry( new QgsLineString( mDragPositions ) );
  const QgsGeometry searchPolygon = searchSkeleton.buffer( mSelectionRubberBand->width() / 2, 6 );
  changeAttributeValue( searchPolygon, mAttributeName, mNewValue, *mCanvas, QgisApp::instance()->activeLayer() );
}


void Qgs3DMapToolPointCloudChangeAttributePaintbrush::activate()
{
  mCanvas->cameraController()->setInputHandlersEnabled( false );
  mSelectionRubberBand = std::make_unique<QgsRubberBand3D>( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Point );
  mSelectionRubberBand->setMarkerOutlineStyle( Qt::PenStyle::DotLine );
  mSelectionRubberBand->setWidth( 32 );
  mSelectionRubberBand->setOutlineColor( mSelectionRubberBand->color() );
  mSelectionRubberBand->setColor( QColorConstants::Transparent );
  mSelectionRubberBand->addPoint( Qgs3DUtils::screenPointToMapCoordinates( QCursor::pos(), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() ) );
  mIsActive = true;
  mHighlighterRubberBand = std::make_unique<QgsRubberBand3D>( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon );
  mHighlighterRubberBand->setMarkersEnabled( false );
  mHighlighterRubberBand->setEdgesEnabled( false );
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::deactivate()
{
  restart();
  // this makes sure there are no leftover artifacts when switching from paintbrush tool to another
  mSelectionRubberBand->setMarkersEnabled( false );
  mSelectionRubberBand.reset();
  mIsActive = false;
  mCanvas->cameraController()->setInputHandlersEnabled( true );
}

QCursor Qgs3DMapToolPointCloudChangeAttributePaintbrush::cursor() const
{
  return Qt::CrossCursor;
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::restart()
{
  mDragPositions.clear();
  mHighlighterRubberBand->reset();
  mIsClicked = false;
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::generateHighlightArea()
{
  const QgsGeometry searchSkeleton = QgsGeometry( new QgsLineString( mDragPositions ) );
  const QgsGeometry searchGeometry = searchSkeleton.buffer( mSelectionRubberBand->width() / 2, 6 );
  std::unique_ptr< QgsPolygon > searchPolygon( qgsgeometry_cast<const QgsPolygon *>( searchGeometry.constGet() )->clone() );
  auto transform = [this]( const QgsPoint &point ) -> QgsPoint {
    return Qgs3DUtils::screenPointToMapCoordinates( QPoint( static_cast<int>( point.x() ), static_cast<int>( point.y() ) ), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() );
  };
  searchPolygon->addZValue( 0 );
  searchPolygon->transformVertices( transform );
  mHighlighterRubberBand->setGeometry( QgsGeometry( std::move( searchPolygon ) ) );
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton && !mIsMoving )
  {
    mIsClicked = true;
    mDragPositions.append( QgsPointXY( event->x(), event->y() ) );
  }
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::mouseReleaseEvent( QMouseEvent *event )
{
  if ( mIsClicked && event->button() == Qt::LeftButton && !mIsMoving )
  {
    mDragPositions.append( QgsPointXY( event->x(), event->y() ) );
    mHighlighterRubberBand->reset();
    run();
    mDragPositions.clear();
  }
  mIsClicked = false;
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::mouseMoveEvent( QMouseEvent *event )
{
  if ( mIsActive )
  {
    const QgsPoint newPos = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() );
    mSelectionRubberBand->moveLastPoint( newPos );

    if ( mIsClicked && !mIsMoving )
    {
      mDragPositions.append( QgsPointXY( event->x(), event->y() ) );
      generateHighlightArea();
    }
  }
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::mouseWheelEvent( QWheelEvent *event )
{
  // Moving horizontally or being in movement mode discards the event
  if ( event->angleDelta().y() == 0 || mIsMoving )
  {
    event->accept();
    return;
  }

  // Change the selection circle size. Moving the wheel forward (away) from the user makes
  // the circle smaller
  const QgsSettings settings;
  const bool reverseZoom = settings.value( u"qgis/reverse_wheel_zoom"_s, false ).toBool();
  const bool shrink = reverseZoom ? event->angleDelta().y() > 0 : event->angleDelta().y() < 0;

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  const float zoomFactor = 1 + 0.25f * std::fabs( static_cast<float>( event->angleDelta().y() ) ) / 120.f;
  const float newWidth = shrink ? mSelectionRubberBand->width() / zoomFactor : mSelectionRubberBand->width() * zoomFactor;
  mSelectionRubberBand->setWidth( std::clamp<float>( newWidth, 5.f, static_cast<float>( mCanvas->width() ) / 2.f ) );
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::keyPressEvent( QKeyEvent *event )
{
  if ( mIsClicked && event->key() == Qt::Key_Escape )
  {
    restart();
  }

  if ( event->key() == Qt::Key_Space && !event->isAutoRepeat() )
  {
    mCanvas->cameraController()->setInputHandlersEnabled( true );
    mIsMoving = true;
  }
}

void Qgs3DMapToolPointCloudChangeAttributePaintbrush::keyReleaseEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Space && !event->isAutoRepeat() )
  {
    mCanvas->cameraController()->setInputHandlersEnabled( false );
    mIsMoving = false;
  }
}
