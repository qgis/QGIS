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
