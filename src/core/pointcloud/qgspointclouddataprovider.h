/***************************************************************************
                         qgspointclouddataprovider.h
                         ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDDATAPROVIDER_H
#define QGSPOINTCLOUDDATAPROVIDER_H

#include "qgis_core.h"
#include "qgsdataprovider.h"
#include "qgspointcloudattribute.h"
#include <memory>

class QgsPointCloudIndex;
class QgsPointCloudRenderer;

/**
 * \ingroup core
 * Base class for providing data for QgsPointCloudLayer
 *
 * Responsible for reading native point cloud data and returning the indexed data.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudDataProvider: public QgsDataProvider
{
    Q_OBJECT
  public:

    /**
     * Capabilities that providers may implement.
     */
    enum Capability
    {
      NoCapabilities = 0,       //!< Provider has no capabilities
      ReadLayerMetadata = 1 << 0, //!< Provider can read layer metadata from data store.
      WriteLayerMetadata = 1 << 1, //!< Provider can write layer metadata to the data store. See QgsDataProvider::writeLayerMetadata()
      CreateRenderer = 1 << 2, //!< Provider can create 2D renderers using backend-specific formatting information. See QgsPointCloudDataProvider::createRenderer().
    };

    Q_DECLARE_FLAGS( Capabilities, Capability )

    //! Ctor
    QgsPointCloudDataProvider( const QString &uri,
                               const QgsDataProvider::ProviderOptions &providerOptions,
                               QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsPointCloudDataProvider() override;

    /**
     * Returns flags containing the supported capabilities for the data provider.
     */
    virtual QgsPointCloudDataProvider::Capabilities capabilities() const;

    /**
     * Returns the attributes available from this data provider.
     */
    virtual QgsPointCloudAttributeCollection attributes() const = 0;

    /**
     * Returns the point cloud index associated with the provider.
     *
     * Can be nullptr (e.g. the index is being created)
     *
     * \note Not available in Python bindings
     */
    virtual QgsPointCloudIndex *index() const SIP_SKIP {return nullptr;}

    /**
     * Creates a new 2D point cloud renderer, using provider backend specific information.
     *
     * The \a configuration map can be used to pass provider-specific configuration maps to the provider to
     * allow customization of the returned renderer. Support and format of \a configuration varies by provider.
     *
     * When called with an empty \a configuration map the provider's default renderer will be returned.
     *
     * This method returns a new renderer and the caller takes ownership of the returned object.
     *
     * Only providers which report the CreateRenderer capability will return a 2D renderer. Other
     * providers will return NULLPTR.
     */
    virtual QgsPointCloudRenderer *createRenderer( const QVariantMap &configuration = QVariantMap() ) const SIP_FACTORY;

};

#endif // QGSMESHDATAPROVIDER_H
