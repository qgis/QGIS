/***************************************************************************
                              qgslayoututils.cpp
                              ------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoututils.h"
#include "qgslayout.h"
#include "qgsrendercontext.h"
#include "qgslayoutitemmap.h"
#include <QPainter>
#include <cmath>

#ifndef M_DEG2RAD
#define M_DEG2RAD 0.0174532925
#endif

void QgsLayoutUtils::rotate( double angle, double &x, double &y )
{
  double rotToRad = angle * M_PI / 180.0;
  double xRot, yRot;
  xRot = x * std::cos( rotToRad ) - y * std::sin( rotToRad );
  yRot = x * std::sin( rotToRad ) + y * std::cos( rotToRad );
  x = xRot;
  y = yRot;
}

double QgsLayoutUtils::normalizedAngle( const double angle, const bool allowNegative )
{
  double clippedAngle = angle;
  if ( clippedAngle >= 360.0 || clippedAngle <= -360.0 )
  {
    clippedAngle = std::fmod( clippedAngle, 360.0 );
  }
  if ( !allowNegative && clippedAngle < 0.0 )
  {
    clippedAngle += 360.0;
  }
  return clippedAngle;
}

double QgsLayoutUtils::snappedAngle( double angle )
{
  //normalize angle to 0-360 degrees
  double clippedAngle = normalizedAngle( angle );

  //snap angle to 45 degree
  if ( clippedAngle >= 22.5 && clippedAngle < 67.5 )
  {
    return 45.0;
  }
  else if ( clippedAngle >= 67.5 && clippedAngle < 112.5 )
  {
    return 90.0;
  }
  else if ( clippedAngle >= 112.5 && clippedAngle < 157.5 )
  {
    return 135.0;
  }
  else if ( clippedAngle >= 157.5 && clippedAngle < 202.5 )
  {
    return 180.0;
  }
  else if ( clippedAngle >= 202.5 && clippedAngle < 247.5 )
  {
    return 225.0;
  }
  else if ( clippedAngle >= 247.5 && clippedAngle < 292.5 )
  {
    return 270.0;
  }
  else if ( clippedAngle >= 292.5 && clippedAngle < 337.5 )
  {
    return 315.0;
  }
  else
  {
    return 0.0;
  }
}

QgsRenderContext QgsLayoutUtils::createRenderContextForMap( QgsLayoutItemMap *map, QPainter *painter, double dpi )
{
  if ( !map )
  {
    QgsRenderContext context;
    context.setPainter( painter );
    if ( dpi < 0 && painter && painter->device() )
    {
      context.setScaleFactor( painter->device()->logicalDpiX() / 25.4 );
    }
    else if ( dpi > 0 )
    {
      context.setScaleFactor( dpi / 25.4 );
    }
    else
    {
      context.setScaleFactor( 3.465 ); //assume 88 dpi as standard value
    }
    return context;
  }
  else
  {
    // default to 88 dpi if no painter specified
    if ( dpi < 0 )
    {
      dpi = ( painter && painter->device() ) ? painter->device()->logicalDpiX() : 88;
    }
#if 0
    double dotsPerMM = dpi / 25.4;
// TODO
    // get map settings from reference map
    QgsRectangle extent = *( map->currentMapExtent() );
    QSizeF mapSizeMM = map->rect().size();
    QgsMapSettings ms = map->mapSettings( extent, mapSizeMM * dotsPerMM, dpi );
#endif
    QgsRenderContext context; // = QgsRenderContext::fromMapSettings( ms );
    if ( painter )
      context.setPainter( painter );

    context.setFlags( map->layout()->context().renderContextFlags() );
    return context;
  }
}

QgsRenderContext QgsLayoutUtils::createRenderContextForLayout( QgsLayout *layout, QPainter *painter, double dpi )
{
  QgsLayoutItemMap *referenceMap = layout ? layout->referenceMap() : nullptr;
  QgsRenderContext context = createRenderContextForMap( referenceMap, painter, dpi );
  if ( layout )
    context.setFlags( layout->context().renderContextFlags() );
  return context;
}

void QgsLayoutUtils::relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter )
{
  //linearly scale rectToResize relative to the scaling from boundsBefore to boundsAfter
  double left = relativePosition( rectToResize.left(), boundsBefore.left(), boundsBefore.right(), boundsAfter.left(), boundsAfter.right() );
  double right = relativePosition( rectToResize.right(), boundsBefore.left(), boundsBefore.right(), boundsAfter.left(), boundsAfter.right() );
  double top = relativePosition( rectToResize.top(), boundsBefore.top(), boundsBefore.bottom(), boundsAfter.top(), boundsAfter.bottom() );
  double bottom = relativePosition( rectToResize.bottom(), boundsBefore.top(), boundsBefore.bottom(), boundsAfter.top(), boundsAfter.bottom() );

  rectToResize.setRect( left, top, right - left, bottom - top );
}

double QgsLayoutUtils::relativePosition( const double position, const double beforeMin, const double beforeMax, const double afterMin, const double afterMax )
{
  //calculate parameters for linear scale between before and after ranges
  double m = ( afterMax - afterMin ) / ( beforeMax - beforeMin );
  double c = afterMin - ( beforeMin * m );

  //return linearly scaled position
  return m * position + c;
}
QFont QgsLayoutUtils::scaledFontPixelSize( const QFont &font )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont scaledFont = font;
  double pixelSize = pointsToMM( scaledFont.pointSizeF() ) * FONT_WORKAROUND_SCALE + 0.5;
  scaledFont.setPixelSize( pixelSize );
  return scaledFont;
}

double QgsLayoutUtils::fontAscentMM( const QFont &font )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.ascent() / FONT_WORKAROUND_SCALE );
}

double QgsLayoutUtils::fontDescentMM( const QFont &font )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.descent() / FONT_WORKAROUND_SCALE );

}

double QgsLayoutUtils::fontHeightMM( const QFont &font )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.height() / FONT_WORKAROUND_SCALE );

}

double QgsLayoutUtils::fontHeightCharacterMM( const QFont &font, QChar character )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.boundingRect( character ).height() / FONT_WORKAROUND_SCALE );
}

double QgsLayoutUtils::textWidthMM( const QFont &font, const QString &text )
{
  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );
  return ( fontMetrics.width( text ) / FONT_WORKAROUND_SCALE );
}

double QgsLayoutUtils::textHeightMM( const QFont &font, const QString &text, double multiLineHeight )
{
  QStringList multiLineSplit = text.split( '\n' );
  int lines = multiLineSplit.size();

  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont metricsFont = scaledFontPixelSize( font );
  QFontMetricsF fontMetrics( metricsFont );

  double fontHeight = fontMetrics.ascent() + fontMetrics.descent(); // ignore +1 for baseline
  double textHeight = fontMetrics.ascent() + static_cast< double >( ( lines - 1 ) * fontHeight * multiLineHeight );

  return textHeight / FONT_WORKAROUND_SCALE;
}

void QgsLayoutUtils::drawText( QPainter *painter, QPointF position, const QString &text, const QFont &font, const QColor &color )
{
  if ( !painter )
  {
    return;
  }

  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont textFont = scaledFontPixelSize( font );

  painter->save();
  painter->setFont( textFont );
  if ( color.isValid() )
  {
    painter->setPen( color );
  }
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  painter->scale( scaleFactor, scaleFactor );
  painter->drawText( position * FONT_WORKAROUND_SCALE, text );
  painter->restore();
}

void QgsLayoutUtils::drawText( QPainter *painter, const QRectF &rect, const QString &text, const QFont &font, const QColor &color, const Qt::AlignmentFlag halignment, const Qt::AlignmentFlag valignment, const int flags )
{
  if ( !painter )
  {
    return;
  }

  //upscale using FONT_WORKAROUND_SCALE
  //ref: http://osgeo-org.1560.x6.nabble.com/Multi-line-labels-and-font-bug-td4157152.html
  QFont textFont = scaledFontPixelSize( font );

  QRectF scaledRect( rect.x() * FONT_WORKAROUND_SCALE, rect.y() * FONT_WORKAROUND_SCALE,
                     rect.width() * FONT_WORKAROUND_SCALE, rect.height() * FONT_WORKAROUND_SCALE );

  painter->save();
  painter->setFont( textFont );
  if ( color.isValid() )
  {
    painter->setPen( color );
  }
  double scaleFactor = 1.0 / FONT_WORKAROUND_SCALE;
  painter->scale( scaleFactor, scaleFactor );
  painter->drawText( scaledRect, halignment | valignment | flags, text );
  painter->restore();
}

QRectF QgsLayoutUtils::largestRotatedRectWithinBounds( const QRectF &originalRect, const QRectF &boundsRect, const double rotation )
{
  double originalWidth = originalRect.width();
  double originalHeight = originalRect.height();
  double boundsWidth = boundsRect.width();
  double boundsHeight = boundsRect.height();
  double ratioBoundsRect = boundsWidth / boundsHeight;

  double clippedRotation = normalizedAngle( rotation );

  //shortcut for some rotation values
  if ( qgsDoubleNear( clippedRotation, 0.0 ) || qgsDoubleNear( clippedRotation, 90.0 ) || qgsDoubleNear( clippedRotation, 180.0 ) || qgsDoubleNear( clippedRotation, 270.0 ) )
  {
    double rectScale;
    if ( qgsDoubleNear( clippedRotation, 0.0 ) || qgsDoubleNear( clippedRotation, 180.0 ) )
    {
      rectScale = ( ( originalWidth / originalHeight ) > ratioBoundsRect ) ? boundsWidth / originalWidth : boundsHeight / originalHeight;
    }
    else
    {
      rectScale = ( ( originalHeight / originalWidth ) > ratioBoundsRect ) ? boundsWidth / originalHeight : boundsHeight / originalWidth;
    }
    double rectScaledWidth = rectScale * originalWidth;
    double rectScaledHeight = rectScale * originalHeight;

    if ( qgsDoubleNear( clippedRotation, 0.0 ) || qgsDoubleNear( clippedRotation, 180.0 ) )
    {
      return QRectF( ( boundsWidth - rectScaledWidth ) / 2.0, ( boundsHeight - rectScaledHeight ) / 2.0, rectScaledWidth, rectScaledHeight );
    }
    else
    {
      return QRectF( ( boundsWidth - rectScaledHeight ) / 2.0, ( boundsHeight - rectScaledWidth ) / 2.0, rectScaledWidth, rectScaledHeight );
    }
  }

  //convert angle to radians and flip
  double angleRad = -clippedRotation * M_DEG2RAD;
  double cosAngle = std::cos( angleRad );
  double sinAngle = std::sin( angleRad );

  //calculate size of bounds of rotated rectangle
  double widthBoundsRotatedRect = originalWidth * std::fabs( cosAngle ) + originalHeight * std::fabs( sinAngle );
  double heightBoundsRotatedRect = originalHeight * std::fabs( cosAngle ) + originalWidth * std::fabs( sinAngle );

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
  offsetX += std::fabs( minX );
  double offsetY = ratioBoundsRotatedRect > ratioBoundsRect ? ( boundsHeight - rectScale * heightBoundsRotatedRect ) / 2.0 : 0;
  offsetY += std::fabs( minY );

  return QRectF( offsetX, offsetY, rectScaledWidth, rectScaledHeight );

}

double QgsLayoutUtils::pointsToMM( const double pointSize )
{
  //conversion to mm based on 1 point = 1/72 inch
  return ( pointSize * 0.3527 );
}

double QgsLayoutUtils::mmToPoints( const double mmSize )
{
  //conversion to points based on 1 point = 1/72 inch
  return ( mmSize / 0.3527 );
}
