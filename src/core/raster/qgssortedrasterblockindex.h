/***************************************************************************
    qgssortedrasterblockindex.h
     --------------------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSORTEDRASTERBLOCKINDEX_H
#define QGSSORTEDRASTERBLOCKINDEX_H

#include <variant>

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsRasterBlock;

/**
 * \ingroup core
 * \brief Creates a flat index over a QgsRasterBlock, sorted by cell values.
 *
 * This index allows for efficient retrieval of raster block cells by their sorted
 * order. The index is sorted once during initialization, and cell values or
 * coordinates can be requested in either ascending or descending order on demand.
 *
 * \warning Because nodata cells are ignored during index creation, the total number
 * of indexed cells is limited to sortedCount(). Do not attempt to retrieve ranks
 * outside of this range.
 *
 * \warning Raster block data is not stored in this class, and the source QgsRasterBlock must exist for the lifetime of this object.
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsSortedRasterBlockIndex
{
  public:
    /**
   * Constructor for QgsSortedRasterBlockIndex, indexing the specified raster \a block.
   *
   * \warning Raster block data is not stored in this class, and the source QgsRasterBlock must exist for the lifetime of this object.
   */
    QgsSortedRasterBlockIndex( const QgsRasterBlock *block );

    /**
   * Returns the number of valid (non-NoData) sorted cells.
   */
    qgssize sortedCount() const;

    /**
   * Returns a pointer to source QgsRasterBlock.
   */
    const QgsRasterBlock *block() const { return mBlock; }

    /**
   * Returns the source raster block index corresponding to the specified sorted \a rank.
   *
   * \param rank Sorted position (0 to sortedCount() - 1). E.g. 0 for the lowest value, when using an ascending order.
   * \param order Sort order to query
   */
    qgssize sortedIndex( qgssize rank, Qt::SortOrder order = Qt::AscendingOrder ) const;

    /**
   * Retrieves the source raster block column and row corresponding to the specified sorted \a rank.
   *
   * \param rank Sorted position (0 to sortedCount() - 1). E.g. 0 for the lowest value, when using an ascending order.
   * \param column Output column
   * \param row Output row
   * \param order Sort order to query
   */
    void sortedColumnRow( qgssize rank, int &column SIP_OUT, int &row SIP_OUT, Qt::SortOrder order = Qt::AscendingOrder ) const;

    /**
   * Returns the raster block value corresponding to the specified sorted \a rank.
   *
   * \param rank Sorted position (0 to sortedCount() - 1). E.g. 0 for the lowest value, when using an ascending order.
   * \param order Sort order to query
   */
    double sortedValue( qgssize rank, Qt::SortOrder order = Qt::AscendingOrder ) const;

  private:
    const QgsRasterBlock *mBlock = nullptr;

    int mCols = 0;
    int mRows = 0;

    std::variant<std::vector<uint32_t>, std::vector<qgssize>> mSortedIndices;

    inline void indexToColumnAndRow( qgssize index, int &column, int &row ) const
    {
      column = static_cast<int>( index % mCols );
      row = static_cast<int>( index / mCols );
    }
};

#endif // QGSSORTEDRASTERBLOCKINDEX_H
