/***************************************************************************
  qgsvectortiledataprovider.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEDATAPROVIDER_H
#define QGSVECTORTILEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsdataprovider.h"

class QgsTileMatrixSet;
class QgsTileXYZ;
class QgsVectorTileRawData;
class QgsVectorTileMatrixSet;

#define SIP_NO_FILE

/**
 * Base class for vector tile layer data providers.
 *
 * \note Not available in Python bindings
 *
 * \ingroup core
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsVectorTileDataProvider : public QgsDataProvider
{
    Q_OBJECT

  public:

    /**
     * Enumeration with capabilities that vector tile data providers might implement.
     * \since QGIS 3.32
     */
    enum class ProviderCapability : int
    {
      ReadLayerMetadata = 1 << 1, //!< Provider can read layer metadata from data store. See QgsDataProvider::layerMetadata()
    };
    Q_ENUM( ProviderCapability )

    //! Provider capabilities
    Q_DECLARE_FLAGS( ProviderCapabilities, ProviderCapability )

    /**
     * Constructor for QgsVectorTileDataProvider, with the specified \a uri.
     */
    QgsVectorTileDataProvider( const QString &uri,
                               const QgsDataProvider::ProviderOptions &providerOptions,
                               QgsDataProvider::ReadFlags flags );

    /**
     * Copy constructor.
     */
    QgsVectorTileDataProvider( const QgsVectorTileDataProvider &other );

    /**
     * QgsVectorTileDataProvider cannot be assigned.
     */
    QgsVectorTileDataProvider &operator=( const QgsVectorTileDataProvider &other ) = delete;

    /**
     * Returns flags containing the supported capabilities of the data provider.
     * \since QGIS 3.32
     */
    virtual QgsVectorTileDataProvider::ProviderCapabilities providerCapabilities() const;

    QgsRectangle extent() const override;
    bool renderInPreview( const QgsDataProvider::PreviewContext &context ) override;

    /**
     * Returns the source path for the data.
     */
    virtual QString sourcePath() const = 0;

    /**
     * Returns a clone of the data provider.
     */
    virtual QgsVectorTileDataProvider *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the tile matrix set associated with the provider.
     */
    virtual const QgsVectorTileMatrixSet &tileMatrixSet() const = 0;

    /**
     * Returns TRUE if the provider supports async tile reading.
     *
     * The default implementation returns FALSE.
     */
    virtual bool supportsAsync() const;

    /**
     * Returns raw tile data for a single tile.
     */
    virtual QByteArray readTile( const QgsTileMatrixSet &tileMatrixSet, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const = 0;

    /**
     * Returns raw tile data for a range of tiles.
     */
    virtual QList<QgsVectorTileRawData> readTiles( const QgsTileMatrixSet &tileMatrixSet, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr ) const = 0;

    /**
     * Returns a network request for a tile.
     *
     * The default implementation returns an invalid request.
     */
    virtual QNetworkRequest tileRequest( const QgsTileMatrixSet &tileMatrixSet, const QgsTileXYZ &id, Qgis::RendererUsage usage ) const;

    /**
     * Returns the style definition for the provider, if available.
     *
     * \see styleUrl()
     * \see spriteDefinition()
     */
    virtual QVariantMap styleDefinition() const;

    /**
     * Returns the URL for the provider style, if available.
     *
     * If styleDefinition() is empty, then the layer style may be available
     * from this URL.
     */
    virtual QString styleUrl() const;

    /**
     * Returns the sprite definition for the provider, if available.
     *
     * \see spriteImage()
     * \see styleDefinition()
     */
    virtual QVariantMap spriteDefinition() const;

    /**
     * Returns the sprite image for the provider, if available.
     *
     * \see spriteDefinition()
     */
    virtual QImage spriteImage() const;
};


Q_DECLARE_OPERATORS_FOR_FLAGS( QgsVectorTileDataProvider::ProviderCapabilities )


#endif // QGSVECTORTILEDATAPROVIDER_H
