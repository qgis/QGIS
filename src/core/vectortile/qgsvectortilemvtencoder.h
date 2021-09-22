/***************************************************************************
  qgsvectortilemvtencoder.h
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

#ifndef QGSVECTORTILEMVTENCODER_H
#define QGSVECTORTILEMVTENCODER_H

#define SIP_NO_FILE

#include "qgstiles.h"
#include "qgsvectortilerenderer.h"
#include "vector_tile.pb.h"
#include "qgscoordinatetransform.h"

/**
 * \ingroup core
 * \brief Handles conversion of vector features to Mapbox vector tiles encoding.
 *
 * Geometries are stored as a series of MoveTo / LineTo / ClosePath commands
 * with coordinates in integer values (see resolution(), called "extent" in the spec).
 * Attributes are stored as key-value pairs of tags: keys are always strings, values
 * can be integers, floating point numbers, booleans or strings.
 *
 * See the specification for details:
 * https://github.com/mapbox/vector-tile-spec/blob/master/2.1/README.md
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileMVTEncoder
{
  public:
    //! Creates MVT encoder for the given tile coordinates for Web Mercator
    explicit QgsVectorTileMVTEncoder( QgsTileXYZ tileID );

    //! Creates MVT encoder for the given tile coordinates and tile matrix
    explicit QgsVectorTileMVTEncoder( QgsTileXYZ tileID, const QgsTileMatrix &tileMatrix );

    //! Returns resolution of coordinates of geometries within the tile. The default is 4096.
    int resolution() const { return mResolution; }
    //! Sets the resolution of coordinates of geometries within the tile
    void setResolution( int extent ) { mResolution = extent; }

    //! Returns size of the buffer zone around tile edges in integer tile coordinates. The default is 256 (~6%)
    int tileBuffer() const { return mBuffer; }
    //! Sets size of the buffer zone around tile edges in integer tile coordinates
    void setTileBuffer( int buffer ) { mBuffer = buffer; }

    //! Sets coordinate transform context for transforms between layers and tile matrix CRS
    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) { mTransformContext = transformContext; }

    /**
     * Fetches data from vector layer for the given tile, does reprojection and clipping
     *
     * Optional feedback object may be provided to support cancellation.
     */
    void addLayer( QgsVectorLayer *layer, QgsFeedback *feedback = nullptr, QString filterExpression = QString(), QString layerName = QString() );

    //! Encodes MVT using data stored previously with addLayer() calls
    QByteArray encode() const;

  private:
    void addFeature( vector_tile::Tile_Layer *tileLayer, const QgsFeature &f );

  private:
    QgsTileXYZ mTileID;
    int mResolution = 4096;
    int mBuffer = 256;
    QgsCoordinateTransformContext mTransformContext;

    QgsRectangle mTileExtent;
    QgsCoordinateReferenceSystem mCrs;

    QgsVectorTileFeatures mFeatures;

    vector_tile::Tile tile;

    QMap<QVariant, int> mKnownValues;

};

#endif // QGSVECTORTILEMVTENCODER_H
