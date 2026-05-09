/***************************************************************************
    qgsailayerchunker.h
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAILAYERCHUNKER_H
#define QGSAILAYERCHUNKER_H

#include "qgis_app.h"
#include "qgsaiworkspaceindex.h"

#include <QList>
#include <QString>

class QgsVectorLayer;
class QgsRasterLayer;

/**
 * Builds RAG chunks from QGIS map layers, ready to be embedded and stored
 * by QgsAiWorkspaceIndex.
 *
 * Vector layers: features are packed into auto-sized chunks driven by
 * QgsAiWorkspaceIndex::CHUNK_TARGET_CHARS. The text fed to the embedding
 * model is **semantic** — attribute key/value pairs plus the bounding box
 * and geometry type of each feature. The full WKT of every feature in a
 * chunk is collected separately and saved (gzipped) in `wktBlob` so that
 * retrieval can return the precise geometries to the LLM without polluting
 * the embedding with unhelpful coordinate noise.
 *
 * Raster layers: a single chunk per layer carrying metadata + per-band
 * statistics. Pixel sampling is intentionally skipped (low semantic value
 * for similarity search).
 */
class APP_EXPORT QgsAiLayerChunker
{
  public:
    static QList<QgsAiWorkspaceIndex::Chunk> chunkVector( QgsVectorLayer *layer );
    static QList<QgsAiWorkspaceIndex::Chunk> chunkRaster( QgsRasterLayer *layer );
};

#endif // QGSAILAYERCHUNKER_H
