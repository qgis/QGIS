/***************************************************************************
  qgsvectortilemvtdecoder.h
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

#ifndef QGSVECTORTILEMVTDECODER_H
#define QGSVECTORTILEMVTDECODER_H

#define SIP_NO_FILE

class QgsFeature;

#include <QStringList>
#include <QMap>

#include "vector_tile.pb.h"

#include "qgsvectortilerenderer.h"
#include "qgsvectortilematrixset.h"


/**
 * \ingroup core
 * \brief This class is responsible for decoding raw tile data written with Mapbox Vector Tiles encoding.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileMVTDecoder
{
  public:

    /**
     * Constructor for QgsVectorTileMVTDecoder, using the specified tile \a structure.
     */
    QgsVectorTileMVTDecoder( const QgsVectorTileMatrixSet &structure );
    ~QgsVectorTileMVTDecoder();

    //! Tries to decode raw tile data, returns true on success
    bool decode( QgsTileXYZ tileID, const QByteArray &rawTileData );

    //! Returns a list of sub-layer names in a tile. It can only be called after a successful decode()
    QStringList layers() const;

    //! Returns a list of all field names in a tile. It can only be called after a successful decode()
    QStringList layerFieldNames( const QString &layerName ) const;

    /**
     * Returns decoded features grouped by sub-layers. It can only be called after a successful decode()
     *
     * If \a layerSubset is specified then only features from the specified layers will be returned.
     */
    QgsVectorTileFeatures layerFeatures( const QMap<QString, QgsFields> &perLayerFields, const QgsCoordinateTransform &ct,
                                         const QSet< QString > *layerSubset = nullptr ) const;

  private:
    vector_tile::Tile tile;
    QgsTileXYZ mTileID;
    QgsVectorTileMatrixSet mStructure;
    QMap<QString, int> mLayerNameToIndex;
};

#endif // QGSVECTORTILEMVTDECODER_H
