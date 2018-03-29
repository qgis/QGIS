/***************************************************************************
    qgsmaptoolselectionhandler.cpp
    ---------------------
    begin                : March 2018
    copyright            : (C) 2018 by Viktor Sklencar
    email                : vsklencar at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsmaptoolselectionhandler.h"
//#include "qgsmaptoolselectutils.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentify.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaplayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrubberband.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsgeometryutils.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"
#include "qgscoordinateutils.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgsmaptoolselectionhandler.h"

#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>
#include <QMenu>

QgsMapToolSelectionHandler::QgsMapToolSelectionHandler( QgsMapCanvas *canvas )
  : QObject()
  , mLastMapUnitsPerPixel( -1.0 )
  , mCoordinatePrecision( 6 )
  , mCanvas(canvas)
  , mSelectionMode( QgsMapToolSelectionHandler::SelectSimple )
{
    mFillColor = QColor( 254, 178, 76, 63 );
    mStrokeColor = QColor( 254, 58, 29, 100 );
}

QgsMapToolSelectionHandler::~QgsMapToolSelectionHandler()
{

}

void QgsMapToolSelectionHandler::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectSimple:
      selectFeaturesReleaseEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      selectPolygonReleaseEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      selectFreehandReleaseEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      selectRadiusReleaseEvent( e );
      break;
  }
}


void QgsMapToolSelectionHandler::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e );
}

void QgsMapToolSelectionHandler::canvasPressEvent( QgsMapMouseEvent *e, QgsMapToolSelectionHandler::SelectionMode selectionMode )
{
  switch ( selectionMode )
  {
    case QgsMapToolSelectionHandler::SelectSimple:
      mSelectionRubberBand.reset();
      initRubberBand();
      mInitDragPos = e -> pos();
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      break;
  }
}

//void QgsMapToolSelectionHandler::canvasReleaseEvent( QgsMapMouseEvent *e )
//{
//  Q_UNUSED( e );
//}

//void QgsMapToolSelectionHandler::activate()
//{
//  QgsMapTool::activate();
//}

//void QgsMapToolSelectionHandler::deactivate()
//{
//  QgsMapTool::deactivate();
//}

void QgsMapToolSelectionHandler::selectFeaturesMoveEvent( QgsMapMouseEvent *e )
{
  if ( e->buttons() != Qt::LeftButton )
    return;

  QRect rect;
  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    rect = QRect( e->pos(), e->pos() );
  }
  else
  {
    rect = QRect( e->pos(), mInitDragPos );
  }
  // TODO @vsklencar refactor
  this->setRubberBand( mCanvas, rect, mSelectionRubberBand.get() );
}

void QgsMapToolSelectionHandler::selectFeaturesReleaseEvent( QgsMapMouseEvent *e )
{
  QPoint point = e->pos() - mInitDragPos;
  if ( !mSelectionActive || ( point.manhattanLength() < QApplication::startDragDistance() ) )
  {
    mSelectionGeometry = QgsGeometry::fromPointXY( toMapCoordinates( e ->pos() ) );
    mSelectionActive = false;
  }

  if ( mSelectionRubberBand && mSelectionActive )
  {
    mSelectionGeometry = mSelectionRubberBand->asGeometry();
    mSelectionRubberBand.reset();
  }

  mSelectionActive = false;
}

QgsPointXY QgsMapToolSelectionHandler::toMapCoordinates( QPoint point )
{
  return mCanvas->getCoordinateTransform()->toMapCoordinates( point );
}

void QgsMapToolSelectionHandler::selectPolygonMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionRubberBand )
    return;

  if ( mSelectionRubberBand->numberOfVertices() > 0 )
  {
      mSelectionRubberBand->movePoint( this->toMapCoordinates( e->pos() ) );
  }
}

void QgsMapToolSelectionHandler::selectPolygonReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionRubberBand )
  {
    initRubberBand();
  }
  if ( e->button() == Qt::LeftButton )
  {
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
  }
  else
  {
    if ( mSelectionRubberBand->numberOfVertices() > 2 )
    {
      mSelectionGeometry = mSelectionRubberBand->asGeometry();
    }
    mSelectionRubberBand.reset();
    // TODO @vsklencar
    //mJustFinishedSelection = true;
  }
}

void QgsMapToolSelectionHandler::selectFreehandMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionActive || !mSelectionRubberBand )
    return;

  mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
}

void QgsMapToolSelectionHandler::selectFreehandReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionActive )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    if ( !mSelectionRubberBand )
    {
      initRubberBand();
    }
    else
    {
      mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    }
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    mSelectionActive = true;
  }
  else
  {
    if ( e->button() == Qt::LeftButton )
    {
      if ( mSelectionRubberBand && mSelectionRubberBand->numberOfVertices() > 2 )
      {
        mSelectionGeometry = mSelectionRubberBand->asGeometry();
      }
    }

    mSelectionRubberBand.reset();
    mSelectionActive = false;
  }
}

void QgsMapToolSelectionHandler::selectRadiusMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointXY radiusEdge = e->snapPoint();

  if ( !mSelectionActive )
    return;

  if ( !mSelectionRubberBand )
  {
    initRubberBand();
  }

  updateRadiusFromEdge( radiusEdge );
}

void QgsMapToolSelectionHandler::selectRadiusReleaseEvent( QgsMapMouseEvent *e )
{

  if ( e->button() == Qt::RightButton )
  {
    mSelectionActive = false;
    return;
  }

  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    mRadiusCenter = e->snapPoint();
  }
  else
  {
    if ( !mSelectionRubberBand )
    {
      initRubberBand();
    }
    mSelectionGeometry = mSelectionRubberBand->asGeometry();
    mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    mSelectionActive = false;
  }
}


void QgsMapToolSelectionHandler::initRubberBand()
{
  mSelectionRubberBand = qgis::make_unique< QgsRubberBand>( mCanvas, QgsWkbTypes::PolygonGeometry );
  mSelectionRubberBand->setFillColor( mFillColor );
  mSelectionRubberBand->setStrokeColor( mStrokeColor );
}

void QgsMapToolSelectionHandler::updateRadiusFromEdge( QgsPointXY &radiusEdge )
{
  double radius = std::sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
  mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  const int RADIUS_SEGMENTS = 80;
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    QgsPointXY radiusPoint( mRadiusCenter.x() + radius * std::cos( theta ),
                            mRadiusCenter.y() + radius * std::sin( theta ) );
    mSelectionRubberBand->addPoint( radiusPoint, false );
  }
  mSelectionRubberBand->closePoints( true );
}

QgsGeometry QgsMapToolSelectionHandler::selectedGeometry()
{
    return mSelectionGeometry;
}

void QgsMapToolSelectionHandler::setSelectedGeometry(QgsGeometry geometry)
{
    mSelectionGeometry = geometry;
}

void QgsMapToolSelectionHandler::setRubberBand( QgsMapCanvas *canvas, QRect &selectRect, QgsRubberBand *rubberBand )
{
  const QgsMapToPixel *transform = canvas->getCoordinateTransform();
  QgsPointXY ll = transform->toMapCoordinates( selectRect.left(), selectRect.bottom() );
  QgsPointXY lr = transform->toMapCoordinates( selectRect.right(), selectRect.bottom() );
  QgsPointXY ul = transform->toMapCoordinates( selectRect.left(), selectRect.top() );
  QgsPointXY ur = transform->toMapCoordinates( selectRect.right(), selectRect.top() );

  if ( rubberBand )
  {
    rubberBand->reset( QgsWkbTypes::PolygonGeometry );
    rubberBand->addPoint( ll, false );
    rubberBand->addPoint( lr, false );
    rubberBand->addPoint( ur, false );
    rubberBand->addPoint( ul, true );
  }
}

