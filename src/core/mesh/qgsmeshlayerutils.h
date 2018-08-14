/***************************************************************************
                         qgsmeshlayerutils.h
                         --------------------------
    begin                : August 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYERUTILS_H
#define QGSMESHLAYERUTILS_H

#include "qgis_core.h"

class QgsMeshDataProvider;
class QgsMeshDatasetIndex;

#include <QVector>

///@cond PRIVATE

/**
 * \ingroup core
 * Misc utility functions used for mesh layer support
 *
 * \note not available in Python bindings
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshLayerUtils
{
  public:

    //! Calculates min/max values from the given vector of values
    static void calculateMinimumMaximum( double &min, double &max, const QVector<double> &arr );

    //! Calculates min/max values for the whole dataset group (considering all datasets within it)
    static void calculateMinMaxForDatasetGroup( double &min, double &max, QgsMeshDataProvider *provider, int groupIndex );

    //! Calculates min/max values for one dataset
    static void calculateMinMaxForDataset( double &min, double &max, QgsMeshDataProvider *provider, QgsMeshDatasetIndex index );
};

///@endcond

#endif // QGSMESHLAYERUTILS_H
