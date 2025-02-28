/***************************************************************************
  qgs3dmaptoolpaintbrush.cpp
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

#include "qgs3dmaptoolpaintbrush.h"
#include "moc_qgs3dmaptoolpaintbrush.cpp"
#include "qgs3deditutils.h"
#include "qgsrubberband3d.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgswindow3dengine.h"
#include "qgsframegraph.h"
#include "qgsgeometry.h"
#include "qgsguiutils.h"
#include "qgslinestring.h"


class QgsPointCloudAttribute;
Qgs3DMapToolPaintBrush::Qgs3DMapToolPaintBrush( Qgs3DMapCanvas *canvas )
  : Qgs3DMapToolPointCloudChangeAttribute( canvas )
{
}

Qgs3DMapToolPaintBrush::~Qgs3DMapToolPaintBrush() = default;

void Qgs3DMapToolPaintBrush::run()
{
  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );

  const QgsGeometry searchSkeleton = QgsGeometry( new QgsLineString( mDragPositions ) );
  const QgsGeometry searchPolygon = searchSkeleton.buffer( mSelectionRubberBand->width() / 2, 37 );
  Qgs3DEditUtils::changeAttributeValue( searchPolygon, mAttributeName, mNewValue, *mCanvas );
}


void Qgs3DMapToolPaintBrush::activate()
{
  mCanvas->cameraController()->setInputHandlersEnabled( false );
  mSelectionRubberBand.reset( new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Point ) );
  mSelectionRubberBand->setMarkerOutlineStyle( Qt::PenStyle::DotLine );
  mSelectionRubberBand->setWidth( 32 );
  mSelectionRubberBand->setOutlineColor( mSelectionRubberBand->color() );
  mSelectionRubberBand->setColor( QColorConstants::Transparent );
  mSelectionRubberBand->addPoint( Qgs3DUtils::screenPointToMapCoordinates( QCursor::pos(), *mCanvas ) );
  mIsActive = true;
  mHighlighterRubberBand.reset( new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Polygon ) );
  mHighlighterRubberBand->setMarkersEnabled( false );
  mHighlighterRubberBand->setEdgesEnabled( false );
}

void Qgs3DMapToolPaintBrush::deactivate()
{
  restart();
  mSelectionRubberBand.reset();
  mIsActive = false;
  mCanvas->cameraController()->setInputHandlersEnabled( true );
}

QCursor Qgs3DMapToolPaintBrush::cursor() const
{
  return Qt::CrossCursor;
}

void Qgs3DMapToolPaintBrush::restart()
{
  mDragPositions.clear();
  mHighlighterRubberBand->reset();
  mIsClicked = false;
}

void Qgs3DMapToolPaintBrush::generateHighlightArea()
{
  const QgsGeometry searchSkeleton = QgsGeometry( new QgsLineString( mDragPositions ) );
  const QgsGeometry searchGeometry = searchSkeleton.buffer( mSelectionRubberBand->width() / 2, 37 );
  QgsPolygon *searchPolygon = qgsgeometry_cast<QgsPolygon *>( searchGeometry.constGet() );
  Q_ASSERT( searchPolygon );
  auto transform = [this]( const QgsPoint &point ) -> QgsPoint {
    return Qgs3DUtils::screenPointToMapCoordinates( QPoint( point.x(), point.y() ), *mCanvas );
  };
  searchPolygon->addZValue( 0 );
  searchPolygon->transformVertices( transform );
  mHighlighterRubberBand->setGeometry( QgsGeometry( searchPolygon->clone() ) );
}

void Qgs3DMapToolPaintBrush::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton && !mIsMoving )
  {
    mIsClicked = true;
    mDragPositions.append( QgsPointXY( event->x(), event->y() ) );
  }
}

void Qgs3DMapToolPaintBrush::mouseReleaseEvent( QMouseEvent *event )
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

void Qgs3DMapToolPaintBrush::mouseMoveEvent( QMouseEvent *event )
{
  if ( mIsActive )
  {
    const QgsPoint newPos = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), *mCanvas );
    mSelectionRubberBand->moveLastPoint( newPos );

    if ( mIsClicked && !mIsMoving )
    {
      mDragPositions.append( QgsPointXY( event->x(), event->y() ) );
      generateHighlightArea();
    }
  }
}

void Qgs3DMapToolPaintBrush::mouseWheelEvent( QWheelEvent *event )
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
  const bool reverseZoom = settings.value( QStringLiteral( "qgis/reverse_wheel_zoom" ), false ).toBool();
  const bool shrink = reverseZoom ? event->angleDelta().y() < 0 : event->angleDelta().y() > 0;
  double zoomFactor = shrink ? 0.75 : 1.5;
  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );
  mSelectionRubberBand->setWidth( mSelectionRubberBand->width() * zoomFactor );
}

void Qgs3DMapToolPaintBrush::keyPressEvent( QKeyEvent *event )
{
  if ( mIsClicked && event->key() == Qt::Key_Escape )
  {
    restart();
  }

  if ( !mIsClicked && event->key() == Qt::Key_Space )
  {
    const bool newState = !mCanvas->cameraController()->inputHandlersEnabled();
    mCanvas->cameraController()->setInputHandlersEnabled( newState );
    mIsMoving = newState;
  }
}
