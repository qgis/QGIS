/***************************************************************************
                             qgsmaplayerutils.cpp
                             -------------------
    begin                : May 2021
    copyright            : (C) 2021 Nyall Dawson
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

#include "qgsmaplayerutils.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsreferencedgeometry.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"

QgsRectangle QgsMapLayerUtils::combinedExtent( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext )
{
  // We can't use a constructor since QgsRectangle normalizes the rectangle upon construction
  QgsRectangle fullExtent;
  fullExtent.setMinimal();

  // iterate through the map layers and test each layers extent
  // against the current min and max values
  QgsDebugMsgLevel( QStringLiteral( "Layer count: %1" ).arg( layers.count() ), 5 );
  for ( const QgsMapLayer *layer : layers )
  {
    QgsDebugMsgLevel( "Updating extent using " + layer->name(), 5 );
    QgsDebugMsgLevel( "Input extent: " + layer->extent().toString(), 5 );

    if ( layer->extent().isNull() )
      continue;

    // Layer extents are stored in the coordinate system (CS) of the
    // layer. The extent must be projected to the canvas CS
    QgsCoordinateTransform ct( layer->crs(), crs, transformContext );
    ct.setBallparkTransformsAreAppropriate( true );
    try
    {
      const QgsRectangle extent = ct.transformBoundingBox( layer->extent() );

      QgsDebugMsgLevel( "Output extent: " + extent.toString(), 5 );
      fullExtent.combineExtentWith( extent );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not reproject layer extent" ) );
    }
  }

  if ( fullExtent.width() == 0.0 || fullExtent.height() == 0.0 )
  {
    // If all of the features are at the one point, buffer the
    // rectangle a bit. If they are all at zero, do something a bit
    // more crude.

    if ( fullExtent.xMinimum() == 0.0 && fullExtent.xMaximum() == 0.0 &&
         fullExtent.yMinimum() == 0.0 && fullExtent.yMaximum() == 0.0 )
    {
      fullExtent.set( -1.0, -1.0, 1.0, 1.0 );
    }
    else
    {
      const double padFactor = 1e-8;
      double widthPad = fullExtent.xMinimum() * padFactor;
      double heightPad = fullExtent.yMinimum() * padFactor;
      double xmin = fullExtent.xMinimum() - widthPad;
      double xmax = fullExtent.xMaximum() + widthPad;
      double ymin = fullExtent.yMinimum() - heightPad;
      double ymax = fullExtent.yMaximum() + heightPad;
      fullExtent.set( xmin, ymin, xmax, ymax );
    }
  }

  QgsDebugMsgLevel( "Full extent: " + fullExtent.toString(), 5 );
  return fullExtent;
}
