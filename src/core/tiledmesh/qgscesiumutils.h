/***************************************************************************
                         qgscesiumutils.h
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCESIUMUTILS_H
#define QGSCESIUMUTILS_H

#include "qgis_core.h"
#include "qgsbox3d.h"

/**
 * \brief Contains utilities for working with Cesium data.
 *
 * \ingroup core
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsCesiumUtils
{
  public:

    /**
     * Parses a \a region object from a Cesium JSON document to a 3D box.
     */
    static QgsBox3d parseRegion( const QVariantList &region );


};

#endif // QGSCESIUMUTILS_H
