/***************************************************************************
  qgsvectortilewriter.h
  --------------------------------------
  Date                 : April 2020
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

#ifndef QGSVECTORTILEWRITER_H
#define QGSVECTORTILEWRITER_H

#include <QCoreApplication>
#include "qgstiles.h"
#include "qgsrectangle.h"
#include "qgscoordinatetransformcontext.h"
#include "qgscoordinatereferencesystem.h"

class QgsFeedback;
class QgsTileMatrix;
class QgsTileXYZ;
class QgsVectorLayer;


/**
 * \ingroup core
 * \brief Takes care of writing vector tiles. The intended use is to set up the class
 * by setting the destination URI, extent, zoom level range and input vector
 * layers. Then with a call to writeTiles() the data gets written. In case
 * of a failure, errorMessage() returns the cause of the problem during writing.
 *
 * The syntax of destination URIs is the same like the data source string
 * used by vector tile layers: parameters are encoded as a HTTP query string,
 * where "type" key determines the type of the destination container,
 * the "url" key is normally the path. Currently supported types:
 *
 * - "xyz" - tile data written as local files, using a template where {x},{y},{z}
 *   are replaced by the actual tile column, row and zoom level numbers, e.g.:
 *   file:///home/qgis/tiles/{z}/{x}/{y}.pbf
 * - "mbtiles" - tile data written to a new MBTiles file, the "url" key should
 *   be ordinary file system path, e.g.: /home/qgis/output.mbtiles
 *
 * Currently the writer only support MVT encoding of data.
 *
 * Metadata support: it is possible to pass a QVariantMap with metadata. This
 * is backend dependent. Currently only "mbtiles" source type supports writing
 * of metadata. The key-value pairs will be written to the "metadata" table
 * in the MBTiles file. Some useful metadata keys listed here, but see MBTiles spec
 * for more details:
 *
 * - "name" - human-readable name of the tileset
 * - "bounds" - extent in WGS 84: "minlon,minlat,maxlon,maxlat"
 * - "center" - default view of the map: "longitude,latitude,zoomlevel"
 * - "minzoom" - lowest zoom level
 * - "maxzoom" - highest zoom level
 * - "attribution" - string that explains the sources of data
 * - "description" - descriptions of the content
 * - "type" - whether this is an overlay or a basemap
 * - "version" - version of the tileset
 *
 * Vector tile writer always writes "format" and "json" metadata. If not specified
 * in metadata by the client, tile writer also writes "name", "bounds", "minzoom"
 * and "maxzoom".
 *
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileWriter
{
    Q_DECLARE_TR_FUNCTIONS( QgsVectorTileWriter )

  public:
    QgsVectorTileWriter();

    /**
     * \ingroup core
     * \brief Configuration of a single input vector layer to be included in the output
     * \since QGIS 3.14
     */
    class Layer
    {
      public:
        //! Constructs an entry for a vector layer
        explicit Layer( QgsVectorLayer *layer )
          : mLayer( layer )
        {
        }

        //! Returns vector layer of this entry
        QgsVectorLayer *layer() const { return mLayer; }

        //! Returns filter expression. If not empty, only features matching the expression will be used
        QString filterExpression() const { return mFilterExpression; }
        //! Sets filter expression. If not empty, only features matching the expression will be used
        void setFilterExpression( const QString &expr ) { mFilterExpression = expr; }

        //! Returns layer name in the output. If not set, layer()->name() will be used.
        QString layerName() const { return mLayerName; }
        //! Sets layer name in the output. If not set, layer()->name() will be used.
        void setLayerName( const QString &name ) { mLayerName = name; }

        //! Returns minimum zoom level at which this layer will be used. Negative value means no min. zoom level
        int minZoom() const { return mMinZoom; }
        //! Sets minimum zoom level at which this layer will be used. Negative value means no min. zoom level
        void setMinZoom( int minzoom ) { mMinZoom = minzoom; }

        //! Returns maximum zoom level at which this layer will be used. Negative value means no max. zoom level
        int maxZoom() const { return mMaxZoom; }
        //! Sets maximum zoom level at which this layer will be used. Negative value means no max. zoom level
        void setMaxZoom( int maxzoom ) { mMaxZoom = maxzoom; }

      private:
        QgsVectorLayer *mLayer;
        QString mFilterExpression;
        QString mLayerName;
        int mMinZoom = -1;
        int mMaxZoom = -1;
    };

    /**
     * Sets where and how the vector tiles will be written.
     * See the class description about the syntax of destination URIs.
     */
    void setDestinationUri( const QString &uri ) { mDestinationUri = uri; }

    /**
     * Sets extent of vector tile output.
     * If unset, it will use the full extent of all input layers combined
     */
    void setExtent( const QgsRectangle &extent ) { mExtent = extent; }

    //! Sets the minimum zoom level of tiles. Allowed values are in interval [0,24]
    void setMinZoom( int minZoom ) { mMinZoom = minZoom; }
    //! Sets the maximum zoom level of tiles. Allowed values are in interval [0,24]
    void setMaxZoom( int maxZoom ) { mMaxZoom = maxZoom; }

    //! Sets vector layers and their configuration for output of vector tiles
    void setLayers( const QList<QgsVectorTileWriter::Layer> &layers ) { mLayers = layers; }

    //! Sets that will be written to the output dataset. See class description for more on metadata support
    void setMetadata( const QVariantMap &metadata ) { mMetadata = metadata; }

    //! Sets coordinate transform context for transforms between layers and tile matrix CRS
    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) { mTransformContext = transformContext; }

    /**
     * Sets zoom level 0 tile matrix
     */
    bool setRootTileMatrix( const QgsTileMatrix &tileMatrix );

    /**
     * Writes vector tiles according to the configuration.
     * Returns TRUE on success (upon failure one can get error cause using errorMessage())
     *
     * If a pointer to a feedback object is provided, it can be used to track progress or
     * provide cancellation functionality.
     */
    bool writeTiles( QgsFeedback *feedback = nullptr );

    /**
     * Returns error message related to the previous call to writeTiles(). Will return
     * an empty string if writing was successful.
     */
    QString errorMessage() const { return mErrorMessage; }

    //! Returns calculated extent that combines extent of all input layers
    QgsRectangle fullExtent() const;

    /**
     * Encodes single MVT tile
     *
     * \param tileID Tile identifier
     * \param feedback  optional, provide cancellation functionality
     * \param resolution the resolution of coordinates of geometries within the tile. The default is 4096
     * \param buffer size of the buffer zone around tile edges in integer tile coordinates. The default is 256 (~6%)
     *
     * Returns a QByteArray with the encoded data
     *
     * \since QGIS 3.22
    */
    QByteArray writeSingleTile( QgsTileXYZ tileID, QgsFeedback *feedback = nullptr, int buffer = 256, int resolution = 4096 ) const;

  private:
    bool writeTileFileXYZ( const QString &sourcePath, QgsTileXYZ tileID, const QgsTileMatrix &tileMatrix, const QByteArray &tileData );
    QString mbtilesJsonSchema();

  private:
    QgsTileMatrix mRootTileMatrix;
    QgsRectangle mExtent;
    int mMinZoom = 0;
    int mMaxZoom = 4;
    QList<Layer> mLayers;
    QString mDestinationUri;
    QVariantMap mMetadata;
    QgsCoordinateTransformContext mTransformContext;

    QString mErrorMessage;
};

#endif // QGSVECTORTILEWRITER_H
