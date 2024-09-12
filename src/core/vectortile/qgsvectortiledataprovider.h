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
#include "qgstiles.h"

#include <QCache>
#include <QReadWriteLock>

class QgsTileMatrixSet;
class QgsTileXYZ;
class QgsVectorTileRawData;
class QgsVectorTileMatrixSet;

#define SIP_NO_FILE

/**
 * Shared data class for vector tile layer data providers.
 *
 * \note Not available in Python bindings
 *
 * \ingroup core
 * \since QGIS 3.32
 */
class QgsVectorTileDataProviderSharedData
{
  public:

    QgsVectorTileDataProviderSharedData();

    /**
     * Retrieves previously cached raw tile data for a tile with matching \a id.
     *
     * Returns TRUE if tile data was already cached and could be retrieved.
     */
    bool getCachedTileData( QgsVectorTileRawData &data, QgsTileXYZ id );

    /**
     * Stores raw tile data in the shared cache.
     */
    void storeCachedTileData( const QgsVectorTileRawData &data );

    QCache< QgsTileXYZ, QgsVectorTileRawData > mTileCache;

    // cannot use a read/write lock here -- see https://bugreports.qt.io/browse/QTBUG-19794
    QMutex mMutex; //!< Access to all data members is guarded by the mutex

};

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

    //! Role to set column attribute in the request so it can be retrieved later
    static int DATA_COLUMN;
    //! Role to set row attribute in the request so it can be retrieved later
    static int DATA_ROW;
    //! Role to set zoom attribute in the request so it can be retrieved later
    static int DATA_ZOOM;
    //! Role to set source ID attribute in the request so it can be retrieved later
    static int DATA_SOURCE_ID;

    /**
     * Constructor for QgsVectorTileDataProvider, with the specified \a uri.
     */
    QgsVectorTileDataProvider( const QString &uri,
                               const QgsDataProvider::ProviderOptions &providerOptions,
                               Qgis::DataProviderReadFlags flags );

    QgsVectorTileDataProvider( const QgsVectorTileDataProvider &other );
    QgsVectorTileDataProvider &operator=( const QgsVectorTileDataProvider &other ) = delete;

    /**
     * Returns flags reflecting the behavior of the data provider.
     * \since QGIS 3.32
     */
    virtual Qgis::VectorTileProviderFlags providerFlags() const;

    /**
     * Returns flags containing the supported capabilities of the data provider.
     * \since QGIS 3.32
     */
    virtual Qgis::VectorTileProviderCapabilities providerCapabilities() const;

    QgsRectangle extent() const override;
    bool renderInPreview( const QgsDataProvider::PreviewContext &context ) override;

    /**
     * Returns the source path for the data.
     */
    virtual QString sourcePath() const = 0;


    /**
     * Returns the list of source paths for the data.
     * \since QGIS 3.40
     */
    virtual QgsStringMap sourcePaths() const
    {
      return { { QString(), sourcePath() } };
    }


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
    virtual QgsVectorTileRawData readTile( const QgsTileMatrixSet &tileMatrixSet, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const = 0;

    /**
     * Returns raw tile data for a range of tiles.
     */
    virtual QList<QgsVectorTileRawData> readTiles( const QgsTileMatrixSet &tileMatrixSet, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr, Qgis::RendererUsage usage = Qgis::RendererUsage::Unknown ) const = 0;

    /**
     * Returns a network request for a tile.
     *
     * The default implementation returns an invalid request.
     */
    virtual QList<QNetworkRequest> tileRequests( const QgsTileMatrixSet &tileMatrixSet, const QgsTileXYZ &id, Qgis::RendererUsage usage ) const;

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

  protected:

    std::shared_ptr<QgsVectorTileDataProviderSharedData> mShared;  //!< Mutable data shared between provider instances

};



#endif // QGSVECTORTILEDATAPROVIDER_H
