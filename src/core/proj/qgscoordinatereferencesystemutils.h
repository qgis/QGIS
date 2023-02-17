/***************************************************************************
                             qgscoordinatereferencesystemutils.h
                             -------------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOORDINATEREFERENCESYSTEMUTILS_H
#define QGSCOORDINATEREFERENCESYSTEMUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

class QgsCoordinateReferenceSystem;

/**
 * \class QgsCoordinateReferenceSystemUtils
 * \ingroup core
 * \brief Utility functions for working with QgsCoordinateReferenceSystem objects.
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsCoordinateReferenceSystemUtils
{
  public:

    /**
     * Returns the default coordinate order to use for the specified \a crs.
     *
     * \warning This is quite a "coarse" method, in that many possible CRS axis don't map well to a simply X/Y or Y/X order.
     * Accordingly this method will default to returning Qgis::CoordinateOrder::XY unless we are reasonably certain of a Y/X order.
     */
    static Qgis::CoordinateOrder defaultCoordinateOrderForCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns a translated abbreviation representing an \a axis direction.
     */
    static QString axisDirectionToAbbreviatedString( Qgis::CrsAxisDirection axis );
};

#endif // QGSCOORDINATEREFERENCESYSTEMUTILS_H
