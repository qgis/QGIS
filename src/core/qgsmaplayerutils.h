/***************************************************************************
                             qgsmaplayerutils.h
                             -------------------
    begin                : May 2021
    copyright            : (C) 2021 Nyall Dawson
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
#ifndef QGSMAPLAYERUTILS_H
#define QGSMAPLAYERUTILS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"

class QgsMapLayer;
class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsCoordinateTransformContext;

/**
 * \ingroup core
 * \brief Contains utility functions for working with map layers.
 * \since QGIS 3.20
*/
class CORE_EXPORT QgsMapLayerUtils
{

  public:

    /**
     * Returns the combined extent of a list of \a layers.
     *
     * The \a crs argument specifies the desired coordinate reference system for the combined extent.
     */
    static QgsRectangle combinedExtent( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext );

};

#endif // QGSMAPLAYERUTILS_H


