/***************************************************************************
  qgsvectortilematrixset.h
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEMATRIXSET_H
#define QGSVECTORTILEMATRIXSET_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"
#include "qgstiles.h"

class QgsTileMatrix;

/**
 * Encapsulates properties of a vector tile matrix set, including tile origins and scaling information.
 * \ingroup core
 * \since QGIS 3.22.6
 */
class CORE_EXPORT QgsVectorTileMatrixSet : public QgsTileMatrixSet
{
  public:

    /**
     * Returns a vector tile structure corresponding to the standard web mercator/GoogleCRS84Quad setup.
     */
    static QgsVectorTileMatrixSet fromWebMercator();

    /**
     * Initializes the tile structure settings from an ESRI REST VectorTileService \a json map.
     *
     * \note This same structure is utilized in ESRI vtpk archives in the root.json file.
     */
    bool fromEsriJson( const QVariantMap &json );

};

#endif // QGSVECTORTILEMATRIXSET_H
