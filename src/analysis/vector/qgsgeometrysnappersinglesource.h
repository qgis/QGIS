/***************************************************************************
  qgsgeometrysnappersinglesource.h
  ---------------------
  Date                 : May 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYSNAPPERSINGLESOURCE_H
#define QGSGEOMETRYSNAPPERSINGLESOURCE_H

#include "qgis_analysis.h"

class QgsFeatureSink;
class QgsFeatureSource;
class QgsFeedback;

/**
 * \ingroup analysis
 *
 * Makes sure that any two vertices of the vector layer are at least at distance given by the threshold value.
 * The algorithm moves nearby vertices to one location and adds vertices to segments that are passing around other
 * vertices within the threshold. It does not remove any vertices. Also, it does not modify geometries unless
 * needed (it does not snap coordinates to a grid).
 *
 * This algorithm comes handy when doing vector overlay operations such as intersection, union or difference
 * to prevent possible topological errors caused by numerical errors if coordinates are very close to each other.
 *
 * After running the algorithm some previously valid geometries may become invalid and therefore it may be useful
 * to run Fix geometries algorithm afterwards.
 *
 * \note Originally ported from GRASS implementation of Vect_snap_lines_list()
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometrySnapperSingleSource
{
  public:

    /**
     * Run the algorithm on given source and output results to the sink, using threshold value in the source's map units.
     * Returns number of modified geometries.
     */
    static int run( const QgsFeatureSource &source, QgsFeatureSink &sink, double thresh, QgsFeedback *feedback );
};

#endif // QGSGEOMETRYSNAPPERSINGLESOURCE_H
