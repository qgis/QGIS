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
#include "qgsrubberband3d.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgswindow3dengine.h"
#include "qgsframegraph.h"

#include <QApplication>
#include <QCursor>
#include <QMouseEvent>

Qgs3DMapToolPaintBrush::Qgs3DMapToolPaintBrush( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
}

Qgs3DMapToolPaintBrush::~Qgs3DMapToolPaintBrush() = default;

void Qgs3DMapToolPaintBrush::addSelection()
{
}

void Qgs3DMapToolPaintBrush::activate()
{
  mCanvas->cameraController()->setInputHandlersEnabled( false );
  mSelectionRubberBand.reset( new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Point, true ) );
  mSelectionRubberBand->setWidth( 32 );
  mSelectionRubberBand->setOutlineColor( mSelectionRubberBand->color() );
  mSelectionRubberBand->setColor( QColorConstants::Transparent );
  mSelectionRubberBand->addPoint( Qgs3DUtils::screenPointToMapCoordinates( QCursor::pos(), *mCanvas ) );
  mIsActive = true;
}

void Qgs3DMapToolPaintBrush::deactivate()
{
  reset();
  mSelectionRubberBand.reset();
  mIsActive = false;
  mCanvas->cameraController()->setInputHandlersEnabled( true );
}

QCursor Qgs3DMapToolPaintBrush::cursor() const
{
  return Qt::CrossCursor;
}

void Qgs3DMapToolPaintBrush::reset()
{
  mDragPositions.clear();
}

void Qgs3DMapToolPaintBrush::mousePressEvent( QMouseEvent *event )
{
  mIsClicked = true;
  const QgsPoint newPos = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), *mCanvas );
  mDragPositions.append( newPos );
  qDebug() << "New START position -> X: " << newPos.x() << " Y: " << newPos.y() << " Z: " << newPos.z();
}

void Qgs3DMapToolPaintBrush::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    const QgsPoint newPos = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), *mCanvas );
    mDragPositions.append( newPos );
    qDebug() << "New END position -> X: " << newPos.x() << " Y: " << newPos.y() << " Z: " << newPos.z();
    //TODO: add logic for selecting points inside the rubberband
    // handleClick( event->pos() );
  }
  mIsClicked = false;
}

void Qgs3DMapToolPaintBrush::mouseMoveEvent( QMouseEvent *event )
{
  if ( mIsActive )
  {
    const QgsPoint newPos = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), *mCanvas );
    mSelectionRubberBand->moveLastPoint( newPos );

    if ( mIsClicked )
    {
      mDragPositions.append( newPos );
      qDebug() << "New DRAG position -> X: " << newPos.x() << " Y: " << newPos.y() << " Z: " << newPos.z();
    }
  }
}

void Qgs3DMapToolPaintBrush::mouseWheelEvent( QWheelEvent *event )
{
  // Change the selection circle size. Moving the wheel forward (away) from the user makes
  // the circle smaller
  if ( event->angleDelta().y() == 0 )
  {
    event->accept();
    return;
  }

  const QgsSettings settings;
  const bool reverseZoom = settings.value( QStringLiteral( "qgis/reverse_wheel_zoom" ), false ).toBool();
  const bool shrink = reverseZoom ? event->angleDelta().y() < 0 : event->angleDelta().y() > 0;
  double zoomFactor = shrink ? 0.75 : 1.5;
  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );
  mSelectionRubberBand->setWidth( mSelectionRubberBand->width() * zoomFactor );
}
