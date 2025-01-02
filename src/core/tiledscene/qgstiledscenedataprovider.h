/***************************************************************************
                         qgstiledscenedataprovider.h
                         --------------------
    begin                : June 2023
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

#ifndef QGSTILEDSCENEDATAPROVIDER_H
#define QGSTILEDSCENEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgsdataprovider.h"
#include "qgis.h"

class QgsTiledSceneBoundingVolume;
class QgsTiledSceneIndex;

/**
 * \ingroup core
 * \brief Base class for data providers for QgsTiledSceneLayer
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneDataProvider: public QgsDataProvider
{
    Q_OBJECT
  public:


    //! Constructor for QgsTiledSceneDataProvider
    QgsTiledSceneDataProvider( const QString &uri,
                               const QgsDataProvider::ProviderOptions &providerOptions,
                               Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );

    ~QgsTiledSceneDataProvider() override;

    QgsTiledSceneDataProvider( const QgsTiledSceneDataProvider &other );
    QgsTiledSceneDataProvider &operator=( const QgsTiledSceneDataProvider &other ) = delete;

    /**
     * Returns flags containing the supported capabilities for the data provider.
     */
    virtual Qgis::TiledSceneProviderCapabilities capabilities() const;

    /**
     * Returns a clone of the data provider.
     */
    virtual QgsTiledSceneDataProvider *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the original coordinate reference system for the tiled scene data.
     *
     * This may differ from the QgsDataProvider::crs(), which is the best CRS representation
     * for the data provider for 2D use.
     *
     * \warning Care must be taken to ensure that sceneCrs() is used instead of crs() whenever
     * transforming bounding volumes or geometries associated with the provider.
     */
    virtual const QgsCoordinateReferenceSystem sceneCrs() const = 0;

    /**
     * Returns the bounding volume for the data provider.
     *
     * This corresponds to the root node bounding volume.
     *
     * \warning Coordinates in the returned volume are in the sceneCrs() reference system, not the QgsDataProvider::crs() system.
     */
    virtual const QgsTiledSceneBoundingVolume &boundingVolume() const = 0;

    /**
     * Returns the provider's tile index.
     *
     * This is a shallow copy, implicitly shared container for an underlying QgsAbstractTiledSceneIndex
     * implementation.
     *
     * The index is thread safe and can be used safely across multiple threads or transferred between
     * threads.
     */
    virtual QgsTiledSceneIndex index() const = 0;

    /**
     * Returns the provider's z range, or an infinite range if this is not known.
     */
    virtual QgsDoubleRange zRange() const;

};

#endif // QGSTILEDSCENEDATAPROVIDER_H
