/***************************************************************************
                         qgstiledmeshdataprovider.h
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

#ifndef QGSTILEDMESHDATAPROVIDER_H
#define QGSTILEDMESHDATAPROVIDER_H

#include "qgis_core.h"
#include "qgsdataprovider.h"
#include "qgis.h"

class QgsAbstractTiledMeshNodeBoundingVolume;
class QgsTiledMeshIndex;

/**
 * \ingroup core
 * \brief Base class for data providers for QgsTiledMeshLayer
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshDataProvider: public QgsDataProvider
{
    Q_OBJECT
  public:


    //! Constructor for QgsTiledMeshDataProvider
    QgsTiledMeshDataProvider( const QString &uri,
                              const QgsDataProvider::ProviderOptions &providerOptions,
                              QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsTiledMeshDataProvider() override;

    /**
     * Copy constructor.
     */
    QgsTiledMeshDataProvider( const QgsTiledMeshDataProvider &other );

    /**
     * QgsTiledMeshDataProvider cannot be assigned.
     */
    QgsTiledMeshDataProvider &operator=( const QgsTiledMeshDataProvider &other ) = delete;

    /**
     * Returns flags containing the supported capabilities for the data provider.
     */
    virtual Qgis::TiledMeshProviderCapabilities capabilities() const;

    /**
     * Returns a clone of the data provider.
     */
    virtual QgsTiledMeshDataProvider *clone() const = 0 SIP_FACTORY;

    /**
     * Returns metadata in a format suitable for feeding directly
     * into a subset of the GUI properties "Metadata" tab.
     */
    virtual QString htmlMetadata() const;

    /**
     * Returns the original coordinate reference system for the tiled mesh data.
     *
     * This may differ from the QgsDataProvider::crs(), which is the best CRS representation
     * for the data provider for 2D use.
     *
     * \warning Care must be taken to ensure that meshCrs() is used instead of crs() whenever
     * transforming bounding volumes or geometries associated with the provider.
     */
    virtual const QgsCoordinateReferenceSystem meshCrs() const = 0;

    /**
     * Returns the bounding volume for the data provider.
     *
     * This corresponds to the root node bounding volume.
     *
     * \warning Coordinates in the returned volume are in the meshCrs() reference system, not the QgsDataProvider::crs() system.
     */
    virtual const QgsAbstractTiledMeshNodeBoundingVolume *boundingVolume() const = 0;

    /**
     * Returns the provider's tile index.
     *
     * This is a shallow copy, implicitly shared container for an underlying QgsAbstractTiledMeshIndex
     * implementation.
     *
     * The index is thread safe and can be used safely across multiple threads or transferred between
     * threads.
     */
    virtual QgsTiledMeshIndex index() const = 0;

};

#endif // QGSTILEDMESHDATAPROVIDER_H
