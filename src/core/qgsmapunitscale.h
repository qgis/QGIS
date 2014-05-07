/***************************************************************************
  qgsmapunitscale.h
  Struct for storing maximum and minimum scales for measurements in map units
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

#ifndef QGSMAPUNITSCALE_H
#define QGSMAPUNITSCALE_H

#include <QtCore>
#include "qgsrendercontext.h"

class CORE_EXPORT QgsMapUnitScale
{
  public:
    QgsMapUnitScale() : minScale( 0.0 ), maxScale( 0.0 ) {}
    QgsMapUnitScale( float _minScale, float _maxScale ) : minScale( _minScale ), maxScale( _maxScale ) {}

    /** The minimum scale, or 0.0 if unset */
    double minScale;
    /** The maximum scale, or 0.0 if unset */
    double maxScale;

    double computeMapUnitsPerPixel( const QgsRenderContext& c ) const
    {
      double mup = c.mapToPixel().mapUnitsPerPixel();
      double renderScale = c.rendererScale(); // Note: this value is 1 / scale
      if ( minScale != 0 )
      {
        mup = qMin( mup / ( minScale * renderScale ), mup );
      }
      if ( maxScale != 0 )
      {
        mup = qMax( mup / ( maxScale * renderScale ), mup );
      }
      return mup;
    }

    bool operator==( const QgsMapUnitScale& other ) const
    {
      return minScale == other.minScale && maxScale == other.maxScale;
    }

    bool operator!=( const QgsMapUnitScale& other ) const
    {
      return minScale != other.minScale || maxScale != other.maxScale;
    }
};


#endif // QGSMAPUNITSCALE_H



