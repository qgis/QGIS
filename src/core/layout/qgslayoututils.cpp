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
#include <math.h>

double QgsLayoutUtils::normalizedAngle( const double angle, const bool allowNegative )
{
  double clippedAngle = angle;
  if ( clippedAngle >= 360.0 || clippedAngle <= -360.0 )
  {
    clippedAngle = fmod( clippedAngle, 360.0 );
  }
  if ( !allowNegative && clippedAngle < 0.0 )
  {
    clippedAngle += 360.0;
  }
  return clippedAngle;
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
    return context;
  }
}

QgsRenderContext QgsLayoutUtils::createRenderContextForLayout( QgsLayout *layout, QPainter *painter )
{
  QgsLayoutItemMap *referenceMap = layout ? layout->referenceMap() : nullptr;
  return createRenderContextForMap( referenceMap, painter );
}
