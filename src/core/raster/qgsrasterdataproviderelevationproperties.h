/***************************************************************************
                         qgsrasterdataproviderelevationproperties.h
                         ---------------
    begin                : May 2023
    copyright            : (C) 2023 by Nyall Dawson
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


#ifndef QGSRASTERDATAPROVIDERELEVATIONPROPERTIES_H
#define QGSRASTERDATAPROVIDERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsdataproviderelevationproperties.h"

/**
 * \class QgsRasterDataProviderElevationProperties
 * \ingroup core
 * \brief Handles elevation related properties for a raster data provider.
 *
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsRasterDataProviderElevationProperties : public QgsDataProviderElevationProperties
{

  public:

    /**
     * Constructor for QgsRasterDataProviderElevationProperties.
     */
    QgsRasterDataProviderElevationProperties();

};

#endif // QGSRASTERDATAPROVIDERELEVATIONPROPERTIES_H
