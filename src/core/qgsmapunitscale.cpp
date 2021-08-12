/***************************************************************************
  qgsmapunitscale.cpp
  -------------------
   begin                : April 2014
   copyright            : (C) Sandro Mani
   email                : smani at sourcepole dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapunitscale.h"
#include "qgsrendercontext.h"

double QgsMapUnitScale::computeMapUnitsPerPixel( const QgsRenderContext &c ) const
{
  double mup = c.mapToPixel().mapUnitsPerPixel();
  const double renderScale = c.rendererScale();
  if ( !qgsDoubleNear( minScale, 0 ) )
  {
    mup = std::min( mup / ( renderScale / minScale ), mup );
  }
  if ( !qgsDoubleNear( maxScale, 0 ) )
  {
    mup = std::max( mup / ( renderScale / maxScale ), mup );
  }
  return mup;
}
