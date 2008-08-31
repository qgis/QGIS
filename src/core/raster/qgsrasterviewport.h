/***************************************************************************
                        qgsrasterviewport.h  -  description
                              -------------------
 begin                : Fri Jun 28 2002
 copyright            : (C) 2005 by T.Sutton
 email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgsrasterlayer.h 4380 2005-12-26 23:37:50Z timlinux $ */
#ifndef QGSRASTERVIEWPORT_H
#define QGSRASTERVIEWPORT_H

#include <qgspoint.h>

/** \ingroup core
 *  This class provides details of the viewable area that a raster will
 *  be rendered into.
 *
 *  The qgsrasterviewport class sets up a viewport / area of interest to be used
 *  by rasterlayer draw functions at the point of drawing to the screen.
 */

struct QgsRasterViewPort
{
  /** \brief  The offset from the left hand edge of the raster for the rectangle that will be drawn to screen.
   * TODO Check this explanation is correc!*/
  int   rectXOffset;
  float rectXOffsetFloat;
  /** \brief  The offset from the bottom edge of the raster for the rectangle that will be drawn to screen.
   * TODO Check this explanation is correc!*/
  int   rectYOffset;
  float rectYOffsetFloat;
  /** \brief Lower left X dimension of clipped raster image in raster pixel space.
   *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only
   *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to
   *  whatever the screen coordinates are (e.g. a 600x800 display window) */
  double clippedXMin;
  /** \brief Top Right X dimension of clipped raster image in raster pixel space.
   *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only
   *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to
   *  whatever the screen coordinates are (e.g. a 600x800 display window) */
  double clippedXMax;
  /** \brief Lower left Y dimension of clipped raster image in raster pixel space.
   *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only
   *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to
   *  whatever the screen coordinates are (e.g. a 600x800 display window) */
  double clippedYMin;
  /** \brief Top Right X dimension of clipped raster image in raster pixel space.
   *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only
   *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to
   *  whatever the screen coordinates are (e.g. a 600x800 display window) */
  double clippedYMax;
  /** \brief  Distance in pixels from clippedXMin to clippedXMax. */
  int clippedWidth;
  /** \brief Distance in pixels from clippedYMin to clippedYMax  */
  int clippedHeight;
  /** \brief Coordinate (in geographic coordinate system) of top left corner of the part of the raster that
   * is to be rendered.*/
  QgsPoint topLeftPoint;
  /** \brief Coordinate (in geographic coordinate system) of bottom right corner of the part of the raster that
   * is to be rendered.*/
  QgsPoint bottomRightPoint;
  /** \brief Distance in map units from left edge to right edge for the part of the raster that
   * is to be rendered.*/
  int drawableAreaXDim;
  /** \brief Distance in map units from bottom edge to top edge for the part of the raster that
   * is to be rendered.*/
  int drawableAreaYDim;
};

#endif //QGSRASTERVIEWPORT_H
