/***************************************************************************
  qgsmapunitscale.cpp
  -------------------
   begin                : April 2014
   copyright            : (C) Sandro Mani
   email                : smani at sourcepole dot ch

 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapunitscale.h"
#include "qgsrendercontext.h"

double QgsMapUnitScale::computeMapUnitsPerPixel( const QgsRenderContext &c ) const
{
  double mup = c.mapToPixel().mapUnitsPerPixel();
  double renderScale = c.rendererScale();
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
