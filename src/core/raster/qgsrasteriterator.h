/***************************************************************************
    qgsrasteriterator.h
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERITERATOR_H
#define QGSRASTERITERATOR_H

#include "qgis_core.h"
#include "qgsrectangle.h"
#include "qgis_sip.h"
#include <QMap>

class QgsMapToPixel;
class QgsRasterBlock;
class QgsRasterBlockFeedback;
class QgsRasterInterface;
class QgsRasterProjector;
struct QgsRasterViewPort;

/**
 * \ingroup core
 * \brief Iterator for sequentially processing raster cells.
 */
class CORE_EXPORT QgsRasterIterator
{
  public:

    /**
     * Constructor for QgsRasterIterator, iterating over the specified \a input raster source.
     *
     * Since QGIS 3.34 the tileOverlapPixels can be used to specify a margin in pixels for retrieving pixels overlapping into neighbor cells.
     */
    QgsRasterIterator( QgsRasterInterface *input, int tileOverlapPixels = 0 );

    /**
     * Sets the "snap to pixel" factor in pixels.
     *
     * When set to a value greater than 1, the raster blocks will be snapped to boundaries
     * matching exact multiples of this factor.
     *
     * Set to 1 to disable this behavior (default).
     *
     * \warning When the "snap to pixel" factor is set, the iterated portion of the raster may not cover the entire input raster extent.
     * A band of pixels on the right and bottom, with size at most of ``snap to pixel factor - 1``, may be skipped if they cannot be snapped
     * exactly to the factor.
     *
     * \see snapToPixelFactor()
     * \since QGIS 3.42
     */
    void setSnapToPixelFactor( int factor ) { mSnapToPixelFactor = factor > 0 ? factor : 1; }

    /**
     * Returns the current "snap to pixel" factor in pixels.
     *
     * \warning When the "snap to pixel" factor is set, the iterated portion of the raster may not cover the entire input raster extent.
     * A band of pixels on the right and bottom, with size at most of ``snap to pixel factor - 1``, may be skipped if they cannot be snapped
     * exactly to the factor.
     *
     * \see setSnapToPixelFactor()
     * \since QGIS 3.42
     */
    int snapToPixelFactor() const { return mSnapToPixelFactor; }

    /**
     * Given an overall raster extent and width and height in pixels, calculates the sub region
     * of the raster covering the specified \a subRegion.
     *
     * \param rasterExtent overall raster extent
     * \param rasterWidth overall raster width
     * \param rasterHeight overall raster height
     * \param subRegion desired sub region extent
     * \param subRegionWidth width in pixels of sub region
     * \param subRegionHeight height in pixels of sub region
     * \param subRegionLeft starting column of left side of sub region
     * \param subRegionTop starting row of top side of sub region
     * \param resamplingFactor optional resampling factor to snap boundaries to. When specified the calculated subregion will always be shrunk to snap to the pixel boundaries. (since QGIS 3.42)
     *
     * \returns sub region geographic extent, snapped to exact pixel boundaries
     *
     * \since QGIS 3.26
     */
    static QgsRectangle subRegion( const QgsRectangle &rasterExtent, int rasterWidth, int rasterHeight, const QgsRectangle &subRegion, int &subRegionWidth SIP_OUT, int &subRegionHeight SIP_OUT, int &subRegionLeft SIP_OUT, int &subRegionTop SIP_OUT, int resamplingFactor = 1 );

    /**
     * Start reading of raster band. Raster data can then be retrieved by calling readNextRasterPart until it returns FALSE.
     * \param bandNumber number of raster band to read
     * \param nCols number of columns
     * \param nRows number of rows
     * \param extent area to read
     * \param feedback optional raster feedback object for cancellation/preview. Added in QGIS 3.0.
     */
    void startRasterRead( int bandNumber, qgssize nCols, qgssize nRows, const QgsRectangle &extent, QgsRasterBlockFeedback *feedback = nullptr );

    /**
     * Fetches details of the next part of the raster data. This method does NOT actually fetch the raster
     * data itself, rather it calculates and iterates over the details of the raster alone.
     *
     * It's useful for iterating over several layers using a target "reference" layer. E.g. summing
     * the pixels in n rasters whilst aligning the result to a reference layer which is not being summed.
     *
     * Note that calling this method also advances the iterator, just like calling readNextRasterPart().
     *
     * \param bandNumber band to read
     * \param columns number of columns on output device
     * \param rows number of rows on output device
     * \param topLeftColumn top left column
     * \param topLeftRow top left row
     * \param blockExtent exact extent of returned raster block
     * \returns FALSE if the last part was already returned
     *
     * \since QGIS 3.6
    */
    bool next( int bandNumber, int &columns SIP_OUT, int &rows SIP_OUT, int &topLeftColumn SIP_OUT, int &topLeftRow SIP_OUT, QgsRectangle &blockExtent SIP_OUT );

    /**
     * Fetches next part of raster data, caller takes ownership of the block and
     * caller should delete the block.
     * \param bandNumber band to read
     * \param nCols number of columns on output device
     * \param nRows number of rows on output device
     * \param block address of block pointer
     * \param topLeftCol top left column
     * \param topLeftRow top left row
     * \returns FALSE if the last part was already returned
    */
    bool readNextRasterPart( int bandNumber,
                             int &nCols, int &nRows,
                             QgsRasterBlock **block SIP_TRANSFERBACK,
                             int &topLeftCol, int &topLeftRow );

    /**
     * Fetches next part of raster data.
     * \param bandNumber band to read
     * \param nCols number of columns on output device
     * \param nRows number of rows on output device
     * \param block address of block pointer
     * \param topLeftCol top left column
     * \param topLeftRow top left row
     * \param blockExtent optional storage for exact extent of returned raster block
     * \param tileColumns optional storage for number of columns in the iterated tile (excluding any tile overlap pixels)
     * \param tileRows optional storage for number of rows in the iterated tile (excluding any tile overlap pixels)
     * \param tileTopLeftColumn optional storage for the top left column in the iterated tile (excluding any tile overlap pixels)
     * \param tileTopLeftRow optional storage for the top left row in the iterated tile (excluding any tile overlap pixels)
     * \returns FALSE if the last part was already returned
     * \note Not available in Python bindings
     * \since QGIS 3.2
    */
    bool readNextRasterPart( int bandNumber,
                             int &nCols, int &nRows,
                             std::unique_ptr< QgsRasterBlock > &block,
                             int &topLeftCol, int &topLeftRow,
                             QgsRectangle *blockExtent = nullptr,
                             int *tileColumns = nullptr, int *tileRows = nullptr, int *tileTopLeftColumn = nullptr, int *tileTopLeftRow = nullptr ) SIP_SKIP;

    /**
     * Cancels the raster iteration and resets the iterator.
     */
    void stopRasterRead( int bandNumber );

    /**
     * Returns the input raster interface which is being iterated over.
     */
    const QgsRasterInterface *input() const { return mInput; }

    /**
     * Sets the maximum tile width returned during iteration.
     * \see maximumTileWidth()
     * \see setMaximumTileHeight()
     */
    void setMaximumTileWidth( int w ) { mMaximumTileWidth = w; }

    /**
     * Returns the maximum tile width returned during iteration.
     * \see setMaximumTileWidth()
     * \see maximumTileHeight()
     */
    int maximumTileWidth() const { return mMaximumTileWidth; }

    /**
     * Sets the minimum tile height returned during iteration.
     * \see maximumTileHeight()
     * \see setMaximumTileWidth()
     */
    void setMaximumTileHeight( int h ) { mMaximumTileHeight = h; }

    /**
     * Returns the minimum tile width returned during iteration.
     * \see setMaximumTileHeight()
     * \see maximumTileWidth()
     */
    int maximumTileHeight() const { return mMaximumTileHeight; }

    /**
     * Returns the total number of blocks which cover the width of the input raster.
     *
     * \see blockCount()
     * \see blockCountHeight()
     * \since QGIS 3.42
     */
    int blockCountWidth() const { return mNumberBlocksWidth; }

    /**
     * Returns the total number of blocks which cover the height of the input raster.
     *
     * \see blockCount()
     * \see blockCountWidth()
     * \since QGIS 3.42
     */
    int blockCountHeight() const { return mNumberBlocksHeight; }

    /**
     * Returns the total number of blocks required to iterate over the input raster.
     *
     * \see blockCountWidth()
     * \see blockCountHeight()
     * \since QGIS 3.42
     */
    qgssize blockCount() const { return static_cast< qgssize >( mNumberBlocksHeight ) * mNumberBlocksWidth; }

    /**
     * Returns the raster iteration progress as a fraction from 0 to 1.0, for the specified \a bandNumber.
     *
     * \since QGIS 3.42
     */
    double progress( int bandNumber ) const;

    //! Default maximum tile width
    static const int DEFAULT_MAXIMUM_TILE_WIDTH = 2000;

    //! Default maximum tile height
    static const int DEFAULT_MAXIMUM_TILE_HEIGHT = 2000;

  private:
    //Stores information about reading of a raster band. Columns and rows are in unsampled coordinates
    struct RasterPartInfo
    {
      qgssize currentCol;
      qgssize currentRow;
      qgssize nCols;
      qgssize nRows;
    };

    QgsRasterInterface *mInput = nullptr;
    QMap<int, RasterPartInfo> mRasterPartInfos;
    QgsRectangle mExtent;
    QgsRasterBlockFeedback *mFeedback = nullptr;

    int mTileOverlapPixels = 0;
    int mMaximumTileWidth;
    int mMaximumTileHeight;
    int mSnapToPixelFactor = 1;

    int mNumberBlocksWidth = 0;
    int mNumberBlocksHeight = 0;

    //! Remove part into and release memory
    void removePartInfo( int bandNumber );
    bool readNextRasterPartInternal( int bandNumber, int &nCols, int &nRows, std::unique_ptr<QgsRasterBlock> *block, int &topLeftCol, int &topLeftRow, QgsRectangle *blockExtent, int &tileColumns, int &tileRows, int &tileTopLeftColumn, int &tileTopLeftRow );

};

#endif // QGSRASTERITERATOR_H
