/***************************************************************************
                        qgsrasterbandstats.h  -  description
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

/** \file qgsrasterbandstats
 *  \brief This class provides statistics for a given raster band.
 *  
 *  The qgsrasterbandstats holds various stats relating to a given raster band.
 */
#ifndef QGSRASTERBANDSTATS
#define QGSRASTERBANDSTATS

#include <QString>
#include <Q3ValueVector>

#include "qgscolortable.h"
/** \brief The RasterBandStats struct is a container for statistics about a single
 * raster band.
 */
class QgsRasterBandStats
{
  public:
    /** \brief The name of the band that these stats belong to. */
    QString bandName;
    /** \brief The gdal band number (starts at 1)*/
    int bandNoInt; 
    /** \brief A flag to indicate whether this RasterBandStats struct 
     * is completely populated */
    bool statsGatheredFlag; 
    /** \brief The minimum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMinimum function. */
    double minValDouble;
    /** \brief The maximum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMaximmum function. */
    double maxValDouble;
    /** \brief The range is the distance between min & max. */
    double rangeDouble;
    /** \brief The mean cell value for the band. NO_DATA values are excluded. */
    double meanDouble;
    /** \brief The sum of the squares. Used to calculate standard deviation. */
    double sumSqrDevDouble; 
    /** \brief The standard deviation of the cell values. */
    double stdDevDouble;
    /** \brief The sum of all cells in the band. NO_DATA values are excluded. */
    double sumDouble;
    /** \brief The number of cells in the band. Equivalent to height x width. 
     * TODO: check if NO_DATA are excluded!*/
    int elementCountInt;    
    /** \brief Store the histogram for a given layer */
    typedef Q3ValueVector<int> HistogramVector;
    HistogramVector * histogramVector;
    /** \brief whteher histogram values are estimated or completely calculated */
    bool histogramEstimatedFlag;
    /** whehter histogram compuation should include out of range values */
    bool histogramOutOfRangeFlag;
    /** Color table */
    QgsColorTable colorTable;
};
#endif
