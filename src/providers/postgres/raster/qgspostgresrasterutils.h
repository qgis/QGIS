/***************************************************************************
  qgspostgresrasterutils.h - QgsPostgresRasterUtils

 ---------------------
 begin                : 8.1.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESRASTERUTILS_H
#define QGSPOSTGRESRASTERUTILS_H

#include "qgis.h"
#include <QVariantMap>


//! Raster utility functions
struct QgsPostgresRasterUtils
{
    /**
   * Parses a \a wkb raster hex and returns information as a variant map
   * for a particular \a bandNo or for all bands if bandNo is 0
   * See: https://git.osgeo.org/gitea/postgis/postgis/src/branch/master/raster/doc/RFC2-WellKnownBinaryFormat
   */
    static QVariantMap parseWkb( const QByteArray &wkb, int bandNo = 0 );
};


#endif // QGSPOSTGRESRASTERUTILS_H
