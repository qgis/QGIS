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
#ifndef QGSRASTERVIEWPORT_H
#define QGSRASTERVIEWPORT_H

#include <qgspoint.h>
#include "qgscoordinatetransform.h"

/** \ingroup core
 *  This class provides details of the viewable area that a raster will
 *  be rendered into.
 *
 *  The qgsrasterviewport class sets up a viewport / area of interest to be used
 *  by rasterlayer draw functions at the point of drawing to the screen.
 */

struct QgsRasterViewPort
{
  // NOT IN MAP SPACE BUT DEVICE SPACE
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

  // intersection of current map extent and layer extent
  QgsRectangle mDrawnExtent;

  // Source coordinate system
  QgsCoordinateReferenceSystem mSrcCRS;

  // Target coordinate system
  QgsCoordinateReferenceSystem mDestCRS;
};

#endif //QGSRASTERVIEWPORT_H
