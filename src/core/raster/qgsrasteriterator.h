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
 * Iterator for sequentially processing raster cells.
 */
class CORE_EXPORT QgsRasterIterator
{
  public:

    /**
     * Constructor for QgsRasterIterator, iterating over the specified \a input raster source.
     */
    QgsRasterIterator( QgsRasterInterface *input );

    /**
     * Start reading of raster band. Raster data can then be retrieved by calling readNextRasterPart until it returns FALSE.
     * \param bandNumber number of raster band to read
     * \param nCols number of columns
     * \param nRows number of rows
     * \param extent area to read
     * \param feedback optional raster feedback object for cancellation/preview. Added in QGIS 3.0.
     */
    void startRasterRead( int bandNumber, int nCols, int nRows, const QgsRectangle &extent, QgsRasterBlockFeedback *feedback = nullptr );

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
                             QgsRasterBlock **block,
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
     * \returns FALSE if the last part was already returned
     * \note Not available in Python bindings
     * \since QGIS 3.2
    */
    bool readNextRasterPart( int bandNumber,
                             int &nCols, int &nRows,
                             std::unique_ptr< QgsRasterBlock > &block,
                             int &topLeftCol, int &topLeftRow,
                             QgsRectangle *blockExtent = nullptr ) SIP_SKIP;

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

    //! Default maximum tile width
    static const int DEFAULT_MAXIMUM_TILE_WIDTH = 2000;

    //! Default maximum tile height
    static const int DEFAULT_MAXIMUM_TILE_HEIGHT = 2000;

  private:
    //Stores information about reading of a raster band. Columns and rows are in unsampled coordinates
    struct RasterPartInfo
    {
      int currentCol;
      int currentRow;
      int nCols;
      int nRows;
    };

    QgsRasterInterface *mInput = nullptr;
    QMap<int, RasterPartInfo> mRasterPartInfos;
    QgsRectangle mExtent;
    QgsRasterBlockFeedback *mFeedback = nullptr;

    int mMaximumTileWidth;
    int mMaximumTileHeight;

    //! Remove part into and release memory
    void removePartInfo( int bandNumber );
    bool readNextRasterPartInternal( int bandNumber, int &nCols, int &nRows, std::unique_ptr<QgsRasterBlock> *block, int &topLeftCol, int &topLeftRow, QgsRectangle *blockExtent );
};

#endif // QGSRASTERITERATOR_H
