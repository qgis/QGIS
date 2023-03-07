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

#include <QKeyEvent>

#include "qgs3dmaptoolmeasureline.h"
#include "qgs3dutils.h"
#include "qgs3dmapscene.h"
#include "qgs3dmapcanvas.h"
#include "qgspoint.h"
#include "qgsmaplayer.h"
#include "qgs3dmeasuredialog.h"
#include "qgsrubberband3d.h"


Qgs3DMapToolMeasureLine::Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  // Dialog
  mDialog = new Qgs3DMeasureDialog( this );
  mDialog->setWindowFlags( mDialog->windowFlags() | Qt::Tool );
  mDialog->restorePosition();
}

Qgs3DMapToolMeasureLine::~Qgs3DMapToolMeasureLine() = default;

void Qgs3DMapToolMeasureLine::activate()
{
  mRubberBand.reset( new QgsRubberBand3D( *mCanvas->map(), mCanvas->engine(), mCanvas->scene() ) );

  if ( mIsAlreadyActivated )
  {
    restart();
    updateSettings();
  }
  else
  {
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

  mRubberBand.reset();

  // Hide dialog
  mDialog->hide();
}

QCursor Qgs3DMapToolMeasureLine::cursor() const
{
  return Qt::CrossCursor;
}

void Qgs3DMapToolMeasureLine::handleClick( const QPoint &screenPos )
{
  if ( mDone )
  {
    restart();
  }
  mDone = false;

  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( screenPos, mCanvas->windowSize(), mCanvas->cameraController()->camera() );
  const auto allHits = Qgs3DUtils::castRay( ray, mCanvas->scene() );

  if ( allHits.isEmpty() )
    return;

  QgsVector3D worldIntersection;
  float minDist = -1;
  for ( const auto &results : allHits )
  {
    const RayHit &result = results.first();
    const double resDist = result.distance;
    if ( minDist < 0 || resDist < minDist )
    {
      minDist = resDist;
      worldIntersection = QgsVector3D( result.pos.x(),
                                       result.pos.y(),
                                       result.pos.z()
                                     );
    }
  }
  const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( worldIntersection, mCanvas->map()->origin() );
  addPoint( QgsPoint( mapCoords.x(), mapCoords.y(), mapCoords.z() ) );
  mDialog->show();
}

void Qgs3DMapToolMeasureLine::updateSettings()
{
  if ( mRubberBand )
  {
    const QgsSettings settings;
    const int myRed = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
    const int myGreen = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
    const int myBlue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();

    mRubberBand->setWidth( 3 );
    mRubberBand->setColor( QColor( myRed, myGreen, myBlue ) );
  }
}

void Qgs3DMapToolMeasureLine::addPoint( const QgsPoint &point )
{
  // don't add points with the same coordinates
  if ( !mPoints.isEmpty() && mPoints.last() == point )
  {
    return;
  }

  const QgsPoint addedPoint( point );

  mPoints.append( addedPoint );
  mDialog->addPoint();

  const QgsPoint newPoint( point.x(), point.y(), point.z() / canvas()->map()->terrainVerticalScale() );
  if ( mPoints.size() == 1 )
  {
    mRubberBand->addPoint( newPoint );
  }
  else
  {
    mRubberBand->moveLastPoint( newPoint );
  }
  mRubberBand->addPoint( newPoint );
}

void Qgs3DMapToolMeasureLine::restart()
{
  mPoints.clear();
  mDone = true;
  mDialog->resetTable();

  mRubberBand->reset();
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
  }
  else
  {
    mPoints.removeLast();
    mDialog->removeLastPoint();

    mRubberBand->removeLastPoint();
  }
}

QVector<QgsPoint> Qgs3DMapToolMeasureLine::points() const
{
  return mPoints;
}

void Qgs3DMapToolMeasureLine::mousePressEvent( QMouseEvent *event )
{
  mMouseHasMoved = false;
  mMouseClickPos = event->pos();
}

void Qgs3DMapToolMeasureLine::mouseMoveEvent( QMouseEvent *event )
{
  if ( !mMouseHasMoved &&
       ( event->pos() - mMouseClickPos ).manhattanLength() > 3 )
  {
    mMouseHasMoved = true;
  }

  if ( mPoints.isEmpty() || mDone )
    return;

  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( event->pos(), mCanvas->windowSize(), mCanvas->cameraController()->camera() );
  const float dist = static_cast<float>( mPoints.last().z() - ray.origin().y() ) / ray.direction().y();
  const QVector3D hoverPoint = ray.origin() + ray.direction() * dist;
  const QgsVector3D mapCoords = Qgs3DUtils::worldToMapCoordinates( hoverPoint, mCanvas->map()->origin() );
  mRubberBand->moveLastPoint( QgsPoint( mapCoords.x(), mapCoords.y(), mapCoords.z() ) );
}

void Qgs3DMapToolMeasureLine::mouseReleaseEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton && !mMouseHasMoved )
  {
    handleClick( event->pos() );
  }
  else if ( event->button() == Qt::RightButton && !mMouseHasMoved )
  {
    if ( mDone || mPoints.size() <= 1 )
    {
      restart();
      return;
    }

    // Finish measurement
    mRubberBand->removeLastPoint();
    mDone = true;
  }
}

void Qgs3DMapToolMeasureLine::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Backspace ||
       event->key() == Qt::Key_Delete )
  {
    undo();
  }
  else if ( event->key() == Qt::Key_Escape )
  {
    restart();
  }
}
