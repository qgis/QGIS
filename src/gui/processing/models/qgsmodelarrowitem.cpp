/***************************************************************************
                             qgsmodelarrowitem.cpp
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelarrowitem.h"
#include "qgsapplication.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelcomponentgraphicitem.h"
#include <QPainter>
#include <QApplication>
#include <QPalette>

///@cond NOT_STABLE


QgsModelArrowItem::QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Qt::Edge startEdge, int startIndex, bool startIsOutgoing,
                                      QgsModelComponentGraphicItem *endItem, Qt::Edge endEdge, int endIndex, bool endIsIncoming )
  : QObject( nullptr )
  , mStartItem( startItem )
  , mStartEdge( startEdge )
  , mStartIndex( startIndex )
  , mStartIsOutgoing( startIsOutgoing )
  , mEndItem( endItem )
  , mEndEdge( endEdge )
  , mEndIndex( endIndex )
  , mEndIsIncoming( endIsIncoming )
{
  setCacheMode( QGraphicsItem::DeviceCoordinateCache );
  setFlag( QGraphicsItem::ItemIsSelectable, false );
  mColor = QApplication::palette().color( QPalette::WindowText );
  mColor.setAlpha( 150 );
  setPen( QPen( mColor, 8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
  setZValue( QgsModelGraphicsScene::ArrowLink );
  updatePath();

  connect( mStartItem, &QgsModelComponentGraphicItem::updateArrowPaths, this, &QgsModelArrowItem::updatePath );
  connect( mStartItem, &QgsModelComponentGraphicItem::repaintArrows, this, [ = ] { update(); } );
  connect( mEndItem, &QgsModelComponentGraphicItem::updateArrowPaths, this, &QgsModelArrowItem::updatePath );
  connect( mEndItem, &QgsModelComponentGraphicItem::repaintArrows, this, [ = ] { update(); } );
}

QgsModelArrowItem::QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Qt::Edge startEdge, int startIndex, QgsModelComponentGraphicItem *endItem )
  : QgsModelArrowItem( startItem, startEdge, startIndex, true, endItem, Qt::LeftEdge, -1, true )
{
}

QgsModelArrowItem::QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, QgsModelComponentGraphicItem *endItem, Qt::Edge endEdge, int endIndex )
  : QgsModelArrowItem( startItem, Qt::LeftEdge, -1, true, endItem, endEdge, endIndex, true )
{
}

QgsModelArrowItem::QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, QgsModelComponentGraphicItem *endItem )
  : QgsModelArrowItem( startItem, Qt::LeftEdge, -1, true, endItem, Qt::LeftEdge, -1, true )
{
}

void QgsModelArrowItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *, QWidget * )
{
  QColor color = mColor;

  if ( mStartItem->state() == QgsModelComponentGraphicItem::Selected || mEndItem->state() == QgsModelComponentGraphicItem::Selected )
    color.setAlpha( 220 );
  else if ( mStartItem->state() == QgsModelComponentGraphicItem::Hover || mEndItem->state() == QgsModelComponentGraphicItem::Hover )
    color.setAlpha( 150 );
  else
    color.setAlpha( 80 );

  QPen p = pen();
  p.setColor( color );
  p.setWidth( 1 );
  painter->setPen( p );
  painter->setBrush( color );
  painter->setRenderHint( QPainter::Antialiasing );

  for ( const QPointF &point : qgis::as_const( mNodePoints ) )
  {
    painter->drawEllipse( point, 3.0, 3.0 );
  }

  painter->setBrush( Qt::NoBrush );
  painter->drawPath( path() );
}

void QgsModelArrowItem::setPenStyle( Qt::PenStyle style )
{
  QPen p = pen();
  p.setStyle( style );
  setPen( p );
  update();
}

void QgsModelArrowItem::updatePath()
{
  mNodePoints.clear();
  QList< QPointF > controlPoints;

  // is there a fixed start or end point?
  QPointF startPt;
  bool hasStartPt = false;
  if ( mStartIndex != -1 )
  {
    startPt = mStartItem->linkPoint( mStartEdge, mStartIndex, !mStartIsOutgoing );
    hasStartPt = true;
  }
  QPointF endPt;
  bool hasEndPt = false;
  if ( mEndIndex != -1 )
  {
    endPt = mEndItem->linkPoint( mEndEdge, mEndIndex, mEndIsIncoming );
    hasEndPt = true;
  }

  if ( !hasStartPt )
  {
    Qt::Edge startEdge;
    QPointF pt;
    if ( !hasEndPt )
      pt = mStartItem->calculateAutomaticLinkPoint( mEndItem, startEdge );
    else
      pt = mStartItem->calculateAutomaticLinkPoint( endPt + mEndItem->pos(), startEdge );

    controlPoints.append( pt );
    mNodePoints.append( pt );
    controlPoints.append( bezierPointForCurve( pt, startEdge, !mStartIsOutgoing ) );
  }
  else
  {
    mNodePoints.append( mStartItem->pos() + startPt );
    controlPoints.append( mStartItem->pos() + startPt );
    controlPoints.append( bezierPointForCurve( mStartItem->pos() + startPt, mStartEdge == Qt::BottomEdge ? Qt::RightEdge : Qt::LeftEdge, !mStartIsOutgoing ) );
  }

  if ( !hasEndPt )
  {
    Qt::Edge endEdge;
    QPointF pt;
    if ( !hasStartPt )
      pt = mEndItem->calculateAutomaticLinkPoint( mStartItem, endEdge );
    else
      pt = mEndItem->calculateAutomaticLinkPoint( startPt + mStartItem->pos(), endEdge );

    controlPoints.append( bezierPointForCurve( pt, endEdge, mEndIsIncoming ) );
    controlPoints.append( pt );
    mNodePoints.append( pt );
  }
  else
  {
    mNodePoints.append( mEndItem->pos() + endPt );
    controlPoints.append( bezierPointForCurve( mEndItem->pos() + endPt, mEndEdge == Qt::BottomEdge ? Qt::RightEdge : Qt::LeftEdge, mEndIsIncoming ) );
    controlPoints.append( mEndItem->pos() + endPt );
  }

  QPainterPath path;
  path.moveTo( controlPoints.at( 0 ) );
  path.cubicTo( controlPoints.at( 1 ), controlPoints.at( 2 ), controlPoints.at( 3 ) );
  setPath( path );
}

QPointF QgsModelArrowItem::bezierPointForCurve( const QPointF &point, Qt::Edge edge, bool incoming ) const
{
  switch ( edge )
  {
    case Qt::LeftEdge:
      return point + QPointF( incoming ? -50 : 50, 0 );

    case Qt::RightEdge:
      return point + QPointF( incoming ? -50 : 50, 0 );

    case Qt::TopEdge:
      return point + QPointF( 0, incoming ? -30 : 30 );

    case Qt::BottomEdge:
      return point + QPointF( 0, incoming ? -30 : 30 );
  }
  return QPointF();
}


///@endcond

