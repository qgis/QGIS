/***************************************************************************
    qgslocaleutils.h
     -------------
    Date                 : September 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOCALEUTILS_H
#define QGSLOCALEUTILS_H

#include "qgis_core.h"
#include "qgsrectangle.h"

/**
 * \ingroup core
 * \class QgsLocaleUtils
 * \brief Contains locale related utility functions.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsLocaleUtils
{
  public:

    /**
     * Returns the approximate bounds of a \a country (in EPSG:4326).
     *
     * Returns an invalid rectangle if the country could not be matched.
     *
     * \see territoryBounds()
     */
    static QgsRectangle countryBounds( const QString &country );

    /**
     * Returns the approximate bounds of a locale, by territory (in EPSG:4326).
     *
     * Returns an invalid rectangle if the territory could not be matched.
     *
     * \see countryBounds()
     * \see systemLocaleGeographicBounds()
     */
    static QgsRectangle territoryBounds( const QLocale &locale );

    /**
     * Returns the approximate bounds of the current system locale (in EPSG:4326).
     *
     * This method retrieves the territory from the system locale, and attempts to
     * retrieve the approximate bounds of the matching country.
     *
     * Returns an invalid rectangle if the system locale territory could not be matched.
     */
    static QgsRectangle systemLocaleGeographicBounds();
};

#endif // QGSLOCALEUTILS_H
