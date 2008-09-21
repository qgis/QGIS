/***************************************************************************
                            qgsrasterpyramid.h

                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by Gary E. Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERPYRAMID
#define QGSRASTERPYRAMID
/** \ingroup core
  * This struct is used to store pyramid info for the raster layer.
  */
class CORE_EXPORT QgsRasterPyramid
{
  public:
    /** \brief The pyramid level as implemented in gdal (level 2 is half orignal raster size etc) */
    int level;
    /** \brief XDimension for this pyramid layer */
    int xDim;
    /** \brief YDimension for this pyramid layer */
    int yDim;
    /** \brief Whether the pyramid layer has been built yet */
    bool exists;
    /** \brief Whether the pyramid should be built */
    bool build;

    QgsRasterPyramid()
    {
      level = 0;
      xDim = 0;
      yDim = 0;
      exists = false;
      build = false;
    }

};
#endif
