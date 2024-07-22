/***************************************************************************
  qgsvectortileloader.h
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

#ifndef QGSVECTORTILELOADER_H
#define QGSVECTORTILELOADER_H

#define SIP_NO_FILE

#include "qgstiles.h"

class QgsFeedback;
class QgsTileDownloadManagerReply;
class QgsVectorTileDataProvider;

class QByteArray;
class QNetworkReply;
class QEventLoop;


/**
 * \ingroup core
 * \brief Keeps track of raw tile data from one or more sources that need to be decoded
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileRawData
{
  public:
    //! Constructs a raw tile object for single source
    QgsVectorTileRawData( QgsTileXYZ tileID = QgsTileXYZ(), const QByteArray &data = QByteArray() )
      : id( tileID ), tileGeometryId( tileID ), data( { { QString(), data } } ) {}

    //! Constructs a raw tile object for one or more sources
    QgsVectorTileRawData( QgsTileXYZ tileID, const QMap<QString, QByteArray> &data )
      : id( tileID ), tileGeometryId( tileID ), data( data ) {}

    //! Tile position in tile matrix set
    QgsTileXYZ id;

    /**
     * Tile id associated with the raw tile data.
     *
     * This may differ from the tile id in the situation where lower zoom level tiles have been used to replace
     * missing higher zoom level tiles. In this case, the tileGeometryId should be used when decoding tiles
     * to features in order to obtain correct geometry scaling and placement, while the actual tile id
     * should be used when determining the region of the tile for clipping purposes.
     */
    QgsTileXYZ tileGeometryId;

    //! Raw tile data by source ID
    QMap<QString, QByteArray> data;
};


/**
 * \ingroup core
 * \brief The loader class takes care of loading raw vector tile data from a tile source.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileLoader : public QObject
{
    Q_OBJECT
  public:

    //! Returns raw tile data for the specified range of tiles. Blocks the caller until all tiles are fetched.
    static QList<QgsVectorTileRawData> blockingFetchTileRawData(
      const QgsVectorTileDataProvider *provider,
      const QgsTileMatrixSet &tileMatrixSet,
      const QPointF &viewCenter,
      const QgsTileRange &range,
      int zoomLevel,
      QgsFeedback *feedback = nullptr,
      Qgis::RendererUsage usage = Qgis::RendererUsage::Unknown );

    //
    // non-static stuff
    //

    //! Constructs tile loader for doing asynchronous requests and starts network requests
    QgsVectorTileLoader( const QgsVectorTileDataProvider *provider, const QgsTileMatrixSet &tileMatrixSet, const QgsTileRange &range, int zoomLevel, const QPointF &viewCenter,
                         QgsFeedback *feedback, Qgis::RendererUsage usage );
    ~QgsVectorTileLoader();

    //! Blocks the caller until all asynchronous requests are finished (with a success or a failure)
    void downloadBlocking();

    //! Returns a eventual error that occurred during loading, void if no error.
    QString error() const;

  private:
    void loadFromNetworkAsync( const QgsTileXYZ &id, const QgsTileMatrixSet &tileMatrixSet, const QgsVectorTileDataProvider *provider, Qgis::RendererUsage usage );

  private slots:
    void tileReplyFinished();
    void canceled();

  signals:
    //! Emitted when a tile request has finished. If a tile request has failed, the returned raw tile byte array is empty.
    void tileRequestFinished( const QgsVectorTileRawData &rawTile );

  private:
    //! Event loop used for blocking download
    std::unique_ptr<QEventLoop> mEventLoop;
    //! Feedback object that allows cancellation of pending requests
    QgsFeedback *mFeedback;

    //! Running tile requests
    QHash<QgsTileXYZ, QList<QgsTileDownloadManagerReply *>> mReplies;

    //! Raw data is stored until all sources are fetched
    QHash<QgsTileXYZ, QMap<QString, QByteArray>> mPendingRawData;

    QString mError;
};

#endif // QGSVECTORTILELOADER_H
