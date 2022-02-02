/***************************************************************************
    qgsresidualplotitem.cpp
     --------------------------------------
    Date                 : 10-May-2010
    Copyright            : (c) 2010 by Marco Hugentobler
    Email                : marco at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsresidualplotitem.h"
#include "qgsgeorefdatapoint.h"
#include "qgslayoututils.h"
#include <QPainter>
#include <cfloat>
#include <cmath>

QgsResidualPlotItem::QgsResidualPlotItem( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mConvertScaleToMapUnits( false )
{
  setBackgroundEnabled( false );
}

QgsLayoutItem::Flags QgsResidualPlotItem::itemFlags() const
{
  return QgsLayoutItem::FlagOverridesPaint;
}

void QgsResidualPlotItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle )
  Q_UNUSED( pWidget )
  if ( mGCPList.size() < 1 || !painter )
  {
    return;
  }

  const double widthMM = rect().width();
  const double heightMM = rect().height();

  const QPen enabledPen( QColor( 255, 0, 0, 255 ), 0.3, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin );
  const QPen disabledPen( QColor( 255, 0, 0, 85 ), 0.2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin );
  const QBrush enabledBrush( QColor( 255, 255, 255, 255 ) );
  const QBrush disabledBrush( QColor( 255, 255, 255, 127 ) );

  //draw all points and collect minimal mm/pixel ratio
  double minMMPixelRatio = std::numeric_limits<double>::max();
  double mmPixelRatio = 1;

  painter->setRenderHint( QPainter::Antialiasing, true );

  QgsGCPList::const_iterator gcpIt = mGCPList.constBegin();
  for ( ; gcpIt != mGCPList.constEnd(); ++gcpIt )
  {
    const QgsPointXY gcpCoords = ( *gcpIt )->sourceCoords();
    const double gcpItemMMX = ( gcpCoords.x() - mExtent.xMinimum() ) / mExtent.width() * widthMM;
    const double gcpItemMMY = ( 1 - ( gcpCoords.y() - mExtent.yMinimum() ) / mExtent.height() ) * heightMM;

    if ( ( *gcpIt )->isEnabled() )
    {
      painter->setPen( enabledPen );
      painter->setBrush( enabledBrush );
    }
    else
    {
      painter->setPen( disabledPen );
      painter->setBrush( disabledBrush );
    }
    painter->drawRect( QRectF( gcpItemMMX - 0.5, gcpItemMMY - 0.5, 1, 1 ) );
    QgsLayoutUtils::drawText( painter, QPointF( gcpItemMMX + 2, gcpItemMMY + 2 ), QString::number( ( *gcpIt )->id() ), QFont() );

    mmPixelRatio = maxMMToPixelRatioForGCP( *gcpIt, gcpItemMMX, gcpItemMMY );
    if ( mmPixelRatio < minMMPixelRatio )
    {
      minMMPixelRatio = mmPixelRatio;
    }
  }

  //draw residual arrows
  gcpIt = mGCPList.constBegin();
  for ( ; gcpIt != mGCPList.constEnd(); ++gcpIt )
  {
    const QgsPointXY gcpCoords = ( *gcpIt )->sourceCoords();
    const double gcpItemMMX = ( gcpCoords.x() - mExtent.xMinimum() ) / mExtent.width() * widthMM;
    const double gcpItemMMY = ( 1 - ( gcpCoords.y() - mExtent.yMinimum() ) / mExtent.height() ) * heightMM;
    if ( ( *gcpIt )->isEnabled() )
    {
      painter->setPen( enabledPen );
    }
    else
    {
      painter->setPen( disabledPen );
    }

    const QPointF p1( gcpItemMMX, gcpItemMMY );
    const QPointF p2( gcpItemMMX + ( *gcpIt )->residual().x() * minMMPixelRatio, gcpItemMMY + ( *gcpIt )->residual().y() * minMMPixelRatio );
    painter->drawLine( p1, p2 );
    painter->setBrush( QBrush( painter->pen().color() ) );
    drawArrowHead( painter, p2.x(), p2.y(), angle( p1, p2 ), 1 );
  }

  //draw scale bar
  double initialScaleBarWidth = rect().width() / 5;
  double scaleBarWidthUnits = rect().width() / 5 / minMMPixelRatio;

  //a simple method to round to next nice number
  int nDecPlaces;
  if ( scaleBarWidthUnits < 1 )
  {
    nDecPlaces = -std::floor( std::log10( scaleBarWidthUnits ) );
    scaleBarWidthUnits *= std::pow( 10.0, nDecPlaces );
    scaleBarWidthUnits = ( int )( scaleBarWidthUnits + 0.5 );
    scaleBarWidthUnits /= std::pow( 10.0, nDecPlaces );
  }
  else
  {
    nDecPlaces = static_cast<int>( std::log10( scaleBarWidthUnits ) );
    scaleBarWidthUnits /= std::pow( 10.0, nDecPlaces );
    scaleBarWidthUnits = ( int )( scaleBarWidthUnits + 0.5 );
    scaleBarWidthUnits *= std::pow( 10.0, nDecPlaces );
  }
  initialScaleBarWidth = scaleBarWidthUnits * minMMPixelRatio;



  painter->setPen( QColor( 0, 0, 0 ) );
  painter->drawLine( QPointF( 5, rect().height() - 5 ), QPointF( 5 + initialScaleBarWidth, rect().height() - 5 ) );
  painter->drawLine( QPointF( 5, rect().height() - 5 ), QPointF( 5, rect().height() - 7 ) );
  painter->drawLine( QPointF( 5 + initialScaleBarWidth, rect().height() - 5 ), QPointF( 5 + initialScaleBarWidth, rect().height() - 7 ) );
  QFont scaleBarFont;
  scaleBarFont.setPointSize( 9 );
  if ( mConvertScaleToMapUnits )
  {
    QgsLayoutUtils::drawText( painter, QPointF( 5, rect().height() - 4 + QgsLayoutUtils::fontAscentMM( scaleBarFont ) ), QStringLiteral( "%1 map units" ).arg( scaleBarWidthUnits ), QFont() );
  }
  else
  {
    QgsLayoutUtils::drawText( painter, QPointF( 5, rect().height() - 4 + QgsLayoutUtils::fontAscentMM( scaleBarFont ) ), QStringLiteral( "%1 pixels" ).arg( scaleBarWidthUnits ), QFont() );
  }

  if ( frameEnabled() )
  {
    const QgsScopedQPainterState painterState( painter );
    painter->setPen( pen() );
    painter->setBrush( Qt::NoBrush );
    painter->setRenderHint( QPainter::Antialiasing, true );
    painter->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }
}

void QgsResidualPlotItem::draw( QgsLayoutItemRenderContext & )
{

}

double QgsResidualPlotItem::maxMMToPixelRatioForGCP( const QgsGeorefDataPoint *p, double pixelXMM, double pixelYMM )
{
  if ( !p )
  {
    return 0;
  }

  //calculate intersections with upper / lower frame edge depending on the residual y sign
  double upDownDist = std::numeric_limits<double>::max(); //distance to frame intersection with lower or upper frame
  double leftRightDist = std::numeric_limits<double>::max(); //distance to frame intersection with left or right frame

  const QPointF residual = p->residual();
  const QLineF residualLine( pixelXMM, pixelYMM, pixelXMM + residual.x(), pixelYMM + residual.y() );
  QPointF intersectionPoint;

  if ( residual.y() > 0 )
  {
    const QLineF lowerFrameLine( 0, rect().height(), rect().width(), rect().height() );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    if ( residualLine.intersect( lowerFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
#else
    if ( residualLine.intersects( lowerFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
#endif
    {
      upDownDist = dist( QPointF( pixelXMM, pixelYMM ), intersectionPoint );
    }
  }
  else if ( residual.y() < 0 )
  {
    const QLineF upperFrameLine( 0, 0, mExtent.xMaximum(), 0 );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    if ( residualLine.intersect( upperFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
#else
    if ( residualLine.intersects( upperFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
#endif
    {
      upDownDist = dist( QPointF( pixelXMM, pixelYMM ), intersectionPoint );
    }
  }

  //calculate intersection with left / right frame edge depending on the residual x sign
  if ( residual.x() > 0 )
  {
    const QLineF rightFrameLine( rect().width(), 0, rect().width(), rect().height() );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    if ( residualLine.intersect( rightFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
#else
    if ( residualLine.intersects( rightFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
#endif
    {
      leftRightDist = dist( QPointF( pixelXMM, pixelYMM ), intersectionPoint );
    }
  }
  else if ( residual.x() < 0 )
  {
    const QLineF leftFrameLine( 0, 0, 0, rect().height() );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    if ( residualLine.intersect( leftFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
#else
    if ( residualLine.intersects( leftFrameLine, &intersectionPoint ) != QLineF::NoIntersection )
#endif
    {
      leftRightDist = dist( QPointF( pixelXMM, pixelYMM ), intersectionPoint );
    }
  }

  const double resTot = std::sqrt( residual.x() * residual.x() + residual.y() * residual.y() );
  if ( leftRightDist <= upDownDist )
  {
    return leftRightDist / resTot;
  }
  else
  {
    return upDownDist / resTot;
  }
}

double QgsResidualPlotItem::dist( QPointF p1, QPointF p2 ) const
{
  const double dx = p2.x() - p1.x();
  const double dy = p2.y() - p1.y();
  return std::sqrt( dx * dx + dy * dy );
}

void QgsResidualPlotItem::drawArrowHead( QPainter *p, const double x, const double y, const double angle, const double arrowHeadWidth )
{
  if ( !p )
  {
    return;
  }

  const double angleRad = angle / 180.0 * M_PI;
  const QPointF middlePoint( x, y );
  //rotate both arrow points
  const QPointF p1 = QPointF( -arrowHeadWidth / 2.0, arrowHeadWidth );
  const QPointF p2 = QPointF( arrowHeadWidth / 2.0, arrowHeadWidth );

  QPointF p1Rotated, p2Rotated;
  p1Rotated.setX( p1.x() * std::cos( angleRad ) + p1.y() * -std::sin( angleRad ) );
  p1Rotated.setY( p1.x() * std::sin( angleRad ) + p1.y() * std::cos( angleRad ) );
  p2Rotated.setX( p2.x() * std::cos( angleRad ) + p2.y() * -std::sin( angleRad ) );
  p2Rotated.setY( p2.x() * std::sin( angleRad ) + p2.y() * std::cos( angleRad ) );

  QPolygonF arrowHeadPoly;
  arrowHeadPoly << middlePoint;
  arrowHeadPoly << QPointF( middlePoint.x() + p1Rotated.x(), middlePoint.y() + p1Rotated.y() );
  arrowHeadPoly << QPointF( middlePoint.x() + p2Rotated.x(), middlePoint.y() + p2Rotated.y() );

  const QgsScopedQPainterState painterState( p );

  QPen arrowPen = p->pen();
  arrowPen.setJoinStyle( Qt::RoundJoin );
  QBrush arrowBrush = p->brush();
  arrowBrush.setStyle( Qt::SolidPattern );
  p->setPen( arrowPen );
  p->setBrush( arrowBrush );
  arrowBrush.setStyle( Qt::SolidPattern );
  p->drawPolygon( arrowHeadPoly );
}

double QgsResidualPlotItem::angle( QPointF p1, QPointF p2 )
{
  const double xDiff = p2.x() - p1.x();
  const double yDiff = p2.y() - p1.y();
  const double length = std::sqrt( xDiff * xDiff + yDiff * yDiff );
  if ( length <= 0 )
  {
    return 0;
  }

  const double angle = std::acos( ( -yDiff * length ) / ( length * length ) ) * 180 / M_PI;
  if ( xDiff < 0 )
  {
    return ( 360 - angle );
  }
  return angle;
}
