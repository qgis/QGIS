/***************************************************************************
  qgsmaplayerfactory.h
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERFACTORY_H
#define QGSMAPLAYERFACTORY_H

#include "qgis_core.h"
#include "qgis.h"

#include <QString>

/**
 * \ingroup core
 * \brief Contains utility functions for creating map layers.
 *
 * \since QGIS 3.18.1
 */
class CORE_EXPORT QgsMapLayerFactory
{
  public:

    /**
     * Returns the map layer type corresponding a \a string value.
     *
     * \param string string to convert to map layer type
     * \param ok will be set to TRUE if \a string was successfully converted to a map layer type
     *
     * \returns converted map layer type
     *
     * \see typeToString()
     */
    static QgsMapLayerType typeFromString( const QString &string, bool &ok SIP_OUT );

    /**
     * Converts a map layer \a type to a string value.
     *
     * \see typeFromString()
     */
    static QString typeToString( QgsMapLayerType type );

};

#endif // QGSMAPLAYERFACTORY_H
