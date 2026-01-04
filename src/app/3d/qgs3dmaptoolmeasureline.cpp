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

#include "qgs3dmaptoolmeasureline.h"

#include <memory>

#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dmeasuredialog.h"
#include "qgs3dutils.h"
#include "qgsabstractterrainsettings.h"
#include "qgsframegraph.h"
#include "qgsmaplayer.h"
#include "qgspoint.h"
#include "qgsraycastcontext.h"
#include "qgsrubberband3d.h"
#include "qgswindow3dengine.h"

#include <QKeyEvent>

#include "moc_qgs3dmaptoolmeasureline.cpp"

Qgs3DMapToolMeasureLine::Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas )
  : Qgs3DMapTool( canvas )
{
  // Dialog
  mDialog = std::make_unique<Qgs3DMeasureDialog>( this );
  mDialog->setWindowFlags( mDialog->windowFlags() | Qt::Tool );
  mDialog->restorePosition();
}

Qgs3DMapToolMeasureLine::~Qgs3DMapToolMeasureLine() = default;

void Qgs3DMapToolMeasureLine::activate()
{
  mRubberBand = std::make_unique<QgsRubberBand3D>( *mCanvas->mapSettings(), mCanvas->engine(), mCanvas->engine()->frameGraph()->rubberBandsRootEntity() );

  restart();
  updateSettings();

  // Show dialog
  mDialog->updateSettings();
  mDialog->show();
}

void Qgs3DMapToolMeasureLine::deactivate()
{
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

  QgsRayCastContext context;
  context.setSingleResult( false );
  context.setMaximumDistance( mCanvas->cameraController()->camera()->farPlane() );
  context.setAngleThreshold( 0.5f );
  const QgsRayCastResult results = mCanvas->castRay( screenPos, context );

  if ( results.isEmpty() )
    return;

  QgsVector3D mapCoords;
  double minDist = -1;
  const QList<QgsRayCastHit> allHits = results.allHits();
  for ( const QgsRayCastHit &hit : allHits )
  {
    const double resDist = hit.distance();
    if ( minDist < 0 || resDist < minDist )
    {
      minDist = resDist;
      mapCoords = hit.mapCoordinates();
    }
  }
  addPoint( QgsPoint( mapCoords.x(), mapCoords.y(), mapCoords.z() ) );
  mDialog->show();
}

void Qgs3DMapToolMeasureLine::updateSettings()
{
  if ( mRubberBand )
  {
    const QgsSettings settings;
    const int myRed = settings.value( u"qgis/default_measure_color_red"_s, 222 ).toInt();
    const int myGreen = settings.value( u"qgis/default_measure_color_green"_s, 155 ).toInt();
    const int myBlue = settings.value( u"qgis/default_measure_color_blue"_s, 67 ).toInt();

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

  const QgsPoint newPoint( point.x(), point.y(), point.z() / canvas()->mapSettings()->terrainSettings()->verticalScale() );
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
  mDone = false;
  mDialog->resetTable();

  mRubberBand->reset();
  mRubberBand->setHideLastMarker( true );
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

    mRubberBand->removePenultimatePoint();
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
  if ( !mMouseHasMoved && ( event->pos() - mMouseClickPos ).manhattanLength() >= QApplication::startDragDistance() )
  {
    mMouseHasMoved = true;
  }

  if ( mPoints.isEmpty() || mDone )
    return;

  const QgsPoint pointMap = Qgs3DUtils::screenPointToMapCoordinates( event->pos(), mCanvas->size(), mCanvas->cameraController(), mCanvas->mapSettings() );
  mRubberBand->moveLastPoint( pointMap );
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
    mRubberBand->setHideLastMarker( false );
    mRubberBand->removeLastPoint();
    mDone = true;
  }
}

void Qgs3DMapToolMeasureLine::keyPressEvent( QKeyEvent *event )
{
  if ( !mDone && ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete ) )
  {
    undo();
  }
  else if ( event->key() == Qt::Key_Escape )
  {
    restart();
  }
}
