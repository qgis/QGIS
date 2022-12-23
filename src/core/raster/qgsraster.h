/***************************************************************************
              qgsraster.h - Raster namespace
     --------------------------------------
    Date                 : Apr 2013
    Copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTER_H
#define QGSRASTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QString>

#include "qgis.h"

/**
 * \ingroup core
 * \brief Raster namespace.
 */
class CORE_EXPORT QgsRaster
{
  public:

    /**
     * Check if the specified value is representable in the given data type.
     * Supported are numerical types Byte, UInt16, Int16, UInt32, Int32, Float32, Float64.
     * \param value
     * \param dataType
     *  \note not available in Python bindings
     * \since QGIS 2.16
     */
    static bool isRepresentableValue( double value, Qgis::DataType dataType ) SIP_SKIP;

    /**
     * Gets value representable by given data type.
     * Supported are numerical types Byte, UInt16, Int16, UInt32, Int32, Float32, Float64.
     * This is done through C casting, so you have to be sure that the provided value is
     * representable in the output data type. This can be checked with isRepresentableValue().
     * \param value
     * \param dataType
     * \since QGIS 2.1
     */
    static double representableValue( double value, Qgis::DataType dataType );
};

#endif


