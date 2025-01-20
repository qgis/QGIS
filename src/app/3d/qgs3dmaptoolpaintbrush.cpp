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

void Qgs3DMapToolPaintBrush::activate()
{
  mCanvas->cameraController()->setInputHandlersEnabled( false );
  mRubberBand.reset( new QgsRubberBand3D( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity(), Qgis::GeometryType::Point ) );
  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( QCursor::pos(), mCanvas->size(), mCanvas->cameraController()->camera() );
  const float dist = ray.direction().y() == 0 ? 0 : ( mCanvas->cameraController()->camera()->farPlane() + mCanvas->cameraController()->camera()->nearPlane() ) / 2;
  const QVector3D hoverPoint = ray.origin() + ray.direction() * dist;
  const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hoverPoint, mCanvas->mapSettings()->origin() );
  mRubberBand->addPoint( QgsPoint( mapCoords.x(), mapCoords.y(), mapCoords.z() / canvas()->mapSettings()->terrainSettings()->verticalScale() ) );
  mRubberBand->setWidth( 32 );
  mRubberBand->setOutlineColor( mRubberBand->color() );
  mRubberBand->setColor( QColorConstants::Transparent );
  mIsActive = true;
}

void Qgs3DMapToolPaintBrush::deactivate()
{
  mRubberBand.reset();
  mIsActive = false;
  mCanvas->cameraController()->setInputHandlersEnabled( true );
}

QCursor Qgs3DMapToolPaintBrush::cursor() const
{
  return Qt::CrossCursor;
}

void Qgs3DMapToolPaintBrush::mousePressEvent( QMouseEvent *event )
{
  mMouseHasMoved = false;
  mMouseClickPos = event->pos();
}

void Qgs3DMapToolPaintBrush::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton && !mMouseHasMoved )
  {
    //TODO: add logic for selecting points inside the rubberband
    // handleClick( event->pos() );
  }
  else if ( event->button() == Qt::RightButton && !mMouseHasMoved )
  {
    //TODO: add logic for deselecting points inside the rubberband
    // handleClick( event->pos() );
  }
}

void Qgs3DMapToolPaintBrush::mouseMoveEvent( QMouseEvent *event )
{
  if ( !mMouseHasMoved && ( event->pos() - mMouseClickPos ).manhattanLength() >= QApplication::startDragDistance() )
  {
    mMouseHasMoved = true;
  }

  if ( mIsActive )
  {
    const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( event->pos(), mCanvas->size(), mCanvas->cameraController()->camera() );
    const float dist = ray.direction().y() == 0 ? 0 : ( mCanvas->cameraController()->camera()->farPlane() + mCanvas->cameraController()->camera()->nearPlane() ) / 2;
    const QVector3D hoverPoint = ray.origin() + ray.direction() * dist;
    const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hoverPoint, mCanvas->mapSettings()->origin() );
    mRubberBand->moveLastPoint( QgsPoint( mapCoords.x(), mapCoords.y(), mapCoords.z() / canvas()->mapSettings()->terrainSettings()->verticalScale() ) );
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
  mRubberBand->setWidth( mRubberBand->width() * zoomFactor );
}
