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
class QgsAbstractDatabaseProviderConnection;

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

    /**
     * Creates and returns the (possibly NULLPTR) database connection for a \a layer.
     * Ownership is transferred to the caller.
     * \since QGIS 3.22
     */
    static QgsAbstractDatabaseProviderConnection *databaseConnection( const QgsMapLayer *layer ) SIP_FACTORY;

    /**
     * Returns TRUE if the source of the specified \a layer matches the given \a path.
     *
     * This method can be used to test whether a layer is associated with a file path.
     *
     * \since QGIS 3.22
     */
    static bool layerSourceMatchesPath( const QgsMapLayer *layer, const QString &path );

    /**
     * Updates a \a layer's data source, replacing its data source with a path referring to \a newPath.
     *
     * Returns TRUE if the layer was updated, or FALSE if the layer was not updated (e.g. it
     * uses a data provider which does not specify paths in a layer URI.
     *
     * \since QGIS 3.22
     */
    static bool updateLayerSourcePath( QgsMapLayer *layer, const QString &newPath );

    /**
     * Sorts a list of map \a layers by their layer type, respecting the \a order of types specified.
     *
     * Layer types which appear earlier in the \a order list will result in matching layers appearing earlier in the
     * result list.
     *
     * \since QGIS 3.26
     */
    static QList< QgsMapLayer * > sortLayersByType( const QList< QgsMapLayer * > &layers, const QList< QgsMapLayerType > &order );


};

#endif // QGSMAPLAYERUTILS_H


