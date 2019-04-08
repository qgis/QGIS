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

#include "qgspointxy.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsrectangle.h"

/**
 * \ingroup core
 *  This class provides details of the viewable area that a raster will
 *  be rendered into.
 *
 *  The qgsrasterviewport class sets up a viewport / area of interest to be used
 *  by rasterlayer draw functions at the point of drawing to the screen.
 */

struct CORE_EXPORT QgsRasterViewPort
{
#ifdef SIP_RUN
  % TypeHeaderCode
#include <qgsrasterviewport.h>
  % End
#endif

  /**
   * \brief Coordinate (in output device coordinate system) of top left corner
   *   of the part of the raster that is to be rendered.*/
  QgsPointXY mTopLeftPoint;

  /**
   * \brief Coordinate (in output device coordinate system) of bottom right corner
   *   of the part of the raster that is to be rendered.*/
  QgsPointXY mBottomRightPoint;

  //! \brief Width, number of columns to be rendered
  int mWidth;

  /**
   * \brief Distance in map units from bottom edge to top edge for the part of
   *  the raster that is to be rendered.*/
  //! \brief Height, number of rows to be rendered
  int mHeight;

  //! \brief Intersection of current map extent and layer extent
  QgsRectangle mDrawnExtent;

  //! \brief Source coordinate system
  QgsCoordinateReferenceSystem mSrcCRS;

  //! \brief Target coordinate system
  QgsCoordinateReferenceSystem mDestCRS;

  int mSrcDatumTransform;
  int mDestDatumTransform;

  /**
   * Coordinate transform context
   */
  QgsCoordinateTransformContext mTransformContext;
};

#endif //QGSRASTERVIEWPORT_H
