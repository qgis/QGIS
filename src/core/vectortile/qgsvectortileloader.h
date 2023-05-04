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
 * \brief Keeps track of raw tile data that need to be decoded
 *
 * \since QGIS 3.14
 */
class QgsVectorTileRawData
{
  public:
    //! Constructs a raw tile object
    QgsVectorTileRawData( QgsTileXYZ tileID = QgsTileXYZ(), const QByteArray &raw = QByteArray() )
      : id( tileID ), data( raw ) {}

    //! Tile position in tile matrix set
    QgsTileXYZ id;
    //! Raw tile data
    QByteArray data;
};


/**
 * \ingroup core
 * \brief The loader class takes care of loading raw vector tile data from a tile source.
 *
 * \since QGIS 3.14
 */
class QgsVectorTileLoader : public QObject
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
      QgsFeedback *feedback = nullptr );

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
    QList<QgsTileDownloadManagerReply *> mReplies;

    QString mError;

};

#endif // QGSVECTORTILELOADER_H
