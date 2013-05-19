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

#include "qgsrectangle.h"
#include <QMap>

class QgsMapToPixel;
class QgsRasterBlock;
class QgsRasterInterface;
class QgsRasterProjector;
struct QgsRasterViewPort;

/** \ingroup core
 * Iterator for sequentially processing raster cells.
 */
class CORE_EXPORT QgsRasterIterator
{
  public:

    QgsRasterIterator( QgsRasterInterface* input );
    ~QgsRasterIterator();

    /**Start reading of raster band. Raster data can then be retrieved by calling readNextRasterPart until it returns false.
      @param bandNumber number of raster band to read
      @param nCols number of columns
      @param nRows number of rows
      @param extent area to read
     */
    void startRasterRead( int bandNumber, int nCols, int nRows, const QgsRectangle& extent );

    /**Fetches next part of raster data, caller takes ownership of the block and
       caller should delete the block.
       @param bandNumber band to read
       @param nCols number of columns on output device
       @param nRows number of rows on output device
       @param block address of block pointer
       @param topLeftCol top left column
       @param topLeftRow top left row
       @return false if the last part was already returned*/
    bool readNextRasterPart( int bandNumber,
                             int& nCols, int& nRows,
                             QgsRasterBlock **block,
                             int& topLeftCol, int& topLeftRow );

    void stopRasterRead( int bandNumber );

    const QgsRasterInterface* input() const { return mInput; }

    void setMaximumTileWidth( int w ) { mMaximumTileWidth = w; }
    int maximumTileWidth() const { return mMaximumTileWidth; }

    void setMaximumTileHeight( int h ) { mMaximumTileHeight = h; }
    int maximumTileHeight() const { return mMaximumTileHeight; }

  private:
    //Stores information about reading of a raster band. Columns and rows are in unsampled coordinates
    struct RasterPartInfo
    {
      int currentCol;
      int currentRow;
      int nCols;
      int nRows;
      QgsRasterProjector* prj; //raster projector (or 0 if no reprojection is done)
    };

    QgsRasterInterface* mInput;
    QMap<int, RasterPartInfo> mRasterPartInfos;
    QgsRectangle mExtent;

    int mMaximumTileWidth;
    int mMaximumTileHeight;

    /**Remove part into and release memory*/
    void removePartInfo( int bandNumber );
};

#endif // QGSRASTERITERATOR_H
