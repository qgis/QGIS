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
#include "qgsrectangle.h"

class QgsFeedback;
class QgsTileXYZ;
class QgsVectorLayer;


/**
 * \ingroup core
 * Takes care of writing vector tiles. The intended use is to set up the class
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
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileWriter
{
    Q_DECLARE_TR_FUNCTIONS( QgsVectorTileWriter )

  public:
    QgsVectorTileWriter();

    /**
     * \ingroup core
     * Configuration of a single input vector layer to be included in the output
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

      private:
        QgsVectorLayer *mLayer;
        //QString mFilterExpression;
        //QString mLayerName;
    };

    /**
     * Sets where and how the vector tiles will be written.
     * See the class description about the syntax of destination URIs.
     */
    void setDestinationUri( const QString &uri ) { mDestinationUri = uri; }

    /**
     * Sets extent of vector tile output. Currently always in EPSG:3857.
     * If unset, it will use the standard range of the Web Mercator system.
     */
    void setExtent( const QgsRectangle &extent ) { mExtent = extent; }

    //! Sets the minimum zoom level of tiles. Allowed values are in interval [0,24]
    void setMinZoom( int minZoom ) { mMinZoom = minZoom; }
    //! Sets the maximum zoom level of tiles. Allowed values are in interval [0,24]
    void setMaxZoom( int maxZoom ) { mMaxZoom = maxZoom; }

    //! Sets vector layers and their configuration for output of vector tiles
    void setLayers( const QList<QgsVectorTileWriter::Layer> &layers ) { mLayers = layers; }

    /**
     * Writes vector tiles according to the configuration.
     * Returns true on success (upon failure one can get error cause using errorMessage())
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

  private:
    bool writeTileFileXYZ( const QString &sourcePath, QgsTileXYZ tileID, const QByteArray &tileData );
    QString mbtilesJsonSchema();

  private:
    QgsRectangle mExtent;
    int mMinZoom = 0;
    int mMaxZoom = 4;
    QList<Layer> mLayers;
    QString mDestinationUri;

    QString mErrorMessage;
};

#endif // QGSVECTORTILEWRITER_H
