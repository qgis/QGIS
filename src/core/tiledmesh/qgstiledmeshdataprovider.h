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

    /**
     * Capabilities that providers may implement.
     */
    enum class Capability : int
    {
      NoCapabilities = 0,       //!< Provider has no capabilities
      ReadLayerMetadata = 1 << 0, //!< Provider can read layer metadata from data store.
    };

    Q_ENUM( Capability )

    Q_DECLARE_FLAGS( Capabilities, Capability )

    //! Constructor for QgsTiledMeshDataProvider
    QgsTiledMeshDataProvider( const QString &uri,
                              const QgsDataProvider::ProviderOptions &providerOptions,
                              QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsTiledMeshDataProvider() override;

    /**
     * Returns flags containing the supported capabilities for the data provider.
     */
    virtual QgsTiledMeshDataProvider::Capabilities capabilities() const;

};

#endif // QGSTILEDMESHDATAPROVIDER_H
