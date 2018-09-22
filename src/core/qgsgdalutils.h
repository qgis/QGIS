/***************************************************************************
                             qgsgdalutils.h
                             --------------
    begin                : September 2018
    copyright            : (C) 2018 Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGDALUTILS_H
#define QGSGDALUTILS_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include <gdal.h>

/**
 * \ingroup core
 * \class QgsGdalUtils
 * \brief Utilities for working with GDAL
 *
 * \note not available in Python bindings
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsGdalUtils
{
  public:

    /**
     * Reads whether a driver supports GDALCreate() for raster purposes.
     * \param driver GDAL driver
     * \returns true if a driver supports GDALCreate() for raster purposes.
     */
    static bool supportsRasterCreate( GDALDriverH driver );
};

#endif // QGSGDALUTILS_H
