/***************************************************************************
                         qgscomposerutils.cpp
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerutils.h"
#include "qgscomposition.h"
#include <QPainter>

#define FONT_WORKAROUND_SCALE 10 //scale factor for upscaling fontsize and downscaling painter

#ifndef M_DEG2RAD
#define M_DEG2RAD 0.0174532925
#endif

void QgsComposerUtils::drawArrowHead( QPainter *p, const double x, const double y, const double angle, const double arrowHeadWidth )
{
  if ( !p )
  {
    return;
  }

  double angleRad = angle / 180.0 * M_PI;
  QPointF middlePoint( x, y );
  //rotate both arrow points
  QPointF p1 = QPointF( -arrowHeadWidth / 2.0, arrowHeadWidth );
  QPointF p2 = QPointF( arrowHeadWidth / 2.0, arrowHeadWidth );

  QPointF p1Rotated, p2Rotated;
  p1Rotated.setX( p1.x() * cos( angleRad ) + p1.y() * -sin( angleRad ) );
  p1Rotated.setY( p1.x() * sin( angleRad ) + p1.y() * cos( angleRad ) );
  p2Rotated.setX( p2.x() * cos( angleRad ) + p2.y() * -sin( angleRad ) );
  p2Rotated.setY( p2.x() * sin( angleRad ) + p2.y() * cos( angleRad ) );

  QPolygonF arrowHeadPoly;
  arrowHeadPoly << middlePoint;
  arrowHeadPoly << QPointF( middlePoint.x() + p1Rotated.x(), middlePoint.y() + p1Rotated.y() );
  arrowHeadPoly << QPointF( middlePoint.x() + p2Rotated.x(), middlePoint.y() + p2Rotated.y() );

  p->save();

  QPen arrowPen = p->pen();
  arrowPen.setJoinStyle( Qt::RoundJoin );
  QBrush arrowBrush = p->brush();
  arrowBrush.setStyle( Qt::SolidPattern );
  p->setPen( arrowPen );
  p->setBrush( arrowBrush );
  arrowBrush.setStyle( Qt::SolidPattern );
  p->drawPolygon( arrowHeadPoly );

  p->restore();
}

double QgsComposerUtils::angle( const QPointF &p1, const QPointF &p2 )
{
  double xDiff = p2.x() - p1.x();
  double yDiff = p2.y() - p1.y();
  double length = sqrt( xDiff * xDiff + yDiff * yDiff );
  if ( length <= 0 )
  {
    return 0;
  }

  double angle = acos(( -yDiff * length ) / ( length * length ) ) * 180 / M_PI;
  if ( xDiff < 0 )
  {
    return ( 360 - angle );
  }
  return angle;
}

void QgsComposerUtils::rotate( double angle, double &x, double &y )
{
  double rotToRad = angle * M_PI / 180.0;
  double xRot, yRot;
  xRot = x * cos( rotToRad ) - y * sin( rotToRad );
  yRot = x * sin( rotToRad ) + y * cos( rotToRad );
  x = xRot;
  y = yRot;
}

QRectF QgsComposerUtils::largestRotatedRectWithinBounds( const QRectF originalRect, const QRectF boundsRect, const double rotation )
{
  double originalWidth = originalRect.width();
  double originalHeight = originalRect.height();
  double boundsWidth = boundsRect.width();
  double boundsHeight = boundsRect.height();
  double ratioBoundsRect = boundsWidth / boundsHeight;

  //shortcut for some rotation values
  if ( rotation == 0 || rotation == 90 || rotation == 180 || rotation == 270 )
  {
    double originalRatio = originalWidth / originalHeight;
    double rectScale = originalRatio > ratioBoundsRect ? boundsWidth / originalWidth : boundsHeight / originalHeight;
    double rectScaledWidth = rectScale * originalWidth;
    double rectScaledHeight = rectScale * originalHeight;

    if ( rotation == 0 || rotation == 180 )
    {
      return QRectF(( boundsWidth - rectScaledWidth ) / 2.0, ( boundsHeight - rectScaledHeight ) / 2.0, rectScaledWidth, rectScaledHeight );
    }
    else
    {
      return QRectF(( boundsWidth - rectScaledHeight ) / 2.0, ( boundsHeight - rectScaledWidth ) / 2.0, rectScaledHeight, rectScaledWidth );
    }
  }

  //convert angle to radians and flip
  double angleRad = -rotation * M_DEG2RAD;
  double cosAngle = cos( angleRad );
  double sinAngle = sin( angleRad );

  //calculate size of bounds of rotated rectangle
  double widthBoundsRotatedRect = originalWidth * fabs( cosAngle ) + originalHeight * fabs( sinAngle );
  double heightBoundsRotatedRect = originalHeight * fabs( cosAngle ) + originalWidth * fabs( sinAngle );

  //compare ratio of rotated rect with bounds rect and calculate scaling of rotated
  //rect to fit within bounds
  double ratioBoundsRotatedRect = widthBoundsRotatedRect / heightBoundsRotatedRect;
  double rectScale = ratioBoundsRotatedRect > ratioBoundsRect ? boundsWidth / widthBoundsRotatedRect : boundsHeight / heightBoundsRotatedRect;
  double rectScaledWidth = rectScale * originalWidth;
  double rectScaledHeight = rectScale * originalHeight;

  //now calculate offset so that rotated rectangle is centered within bounds
  //first calculate min x and y coordinates
  double currentCornerX = 0;
  double minX = 0;
  currentCornerX += rectScaledWidth * cosAngle;
  minX = minX < currentCornerX ? minX : currentCornerX;
  currentCornerX += rectScaledHeight * sinAngle;
  minX = minX < currentCornerX ? minX : currentCornerX;
  currentCornerX -= rectScaledWidth * cosAngle;
  minX = minX < currentCornerX ? minX : currentCornerX;

  double currentCornerY = 0;
  double minY = 0;
  currentCornerY -= rectScaledWidth * sinAngle;
  minY = minY < currentCornerY ? minY : currentCornerY;
  currentCornerY += rectScaledHeight * cosAngle;
  minY = minY < currentCornerY ? minY : currentCornerY;
  currentCornerY += rectScaledWidth * sinAngle;
  minY = minY < currentCornerY ? minY : currentCornerY;

  //now calculate offset position of rotated rectangle
  double offsetX = ratioBoundsRotatedRect > ratioBoundsRect ? 0 : ( boundsWidth - rectScale * widthBoundsRotatedRect ) / 2.0;
  offsetX += fabs( minX );
  double offsetY = ratioBoundsRotatedRect > ratioBoundsRect ? ( boundsHeight - rectScale * heightBoundsRotatedRect ) / 2.0 : 0;
  offsetY += fabs( minY );

  return QRectF( offsetX, offsetY, rectScaledWidth, rectScaledHeight );
}
