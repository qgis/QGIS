/***************************************************************************
    qgssensorthingsutils.h
    --------------------
    begin                : November 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSENSORTHINGSUTILS_H
#define QGSSENSORTHINGSUTILS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"

/**
 * \ingroup core
 * \brief Utility functions for working with OGC SensorThings API services.
 *
 * - 
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsSensorThingsUtils
{

  public:

    /**
     * Converts a string value to a Qgis::SensorThingsEntity type.
     */
    static Qgis::SensorThingsEntity stringToEntity( const QString &type );

};

#endif // QGSSENSORTHINGSUTILS_H
