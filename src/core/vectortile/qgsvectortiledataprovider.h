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

class QgsTileMatrix;
class QgsTileXYZ;
class QgsVectorTileRawData;

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
     * Constructor for QgsVectorTileDataProvider, with the specified \a uri.
     */
    QgsVectorTileDataProvider( const QString &uri,
                               const QgsDataProvider::ProviderOptions &providerOptions,
                               QgsDataProvider::ReadFlags flags );

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
     * Returns TRUE if the provider supports async tile reading.
     *
     * The default implementation returns FALSE.
     */
    virtual bool supportsAsync() const;

    /**
     * Returns raw tile data for a single tile.
     */
    virtual QByteArray readTile( const QgsTileMatrix &tileMatrix, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const = 0;

    /**
     * Returns raw tile data for a range of tiles.
     */
    virtual QList<QgsVectorTileRawData> readTiles( const QgsTileMatrix &tileMatrix, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr ) const = 0;

    /**
     * Returns a network request for a tile.
     *
     * The default implementation returns an invalid request.
     */
    virtual QNetworkRequest tileRequest( const QgsTileMatrix &tileMatrix, const QgsTileXYZ &id, Qgis::RendererUsage usage ) const;
};

#endif // QGSVECTORTILEDATAPROVIDER_H
