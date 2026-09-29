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

#include "qgssortedrasterblockindex.h"

#include <algorithm>

#include "qgsrasterblock.h"

namespace
{
  template<typename ValueT, typename IndexT> struct SortPair
  {
      ValueT val;
      IndexT idx;
      bool operator<( const SortPair &other ) const { return val < other.val; }
  };

  template<typename ValueT, typename IndexT> std::vector<IndexT> createSortedIndices( const QgsRasterBlock *block, qgssize totalCells )
  {
    const ValueT *rawData = reinterpret_cast<const ValueT *>( block->constBits() );
    const bool hasNoData = block->hasNoData();
    const bool hasNoDataValue = block->hasNoDataValue();
    const ValueT noDataVal = static_cast<ValueT>( block->noDataValue() );

    std::vector<SortPair<ValueT, IndexT>> pairs;
    if ( !hasNoData )
    {
      // here we know the total number of valid cells in advance, since there's no no-data cells
      pairs.resize( totalCells );
      std::for_each( pairs.begin(), pairs.end(), [rawData, &pairs]( SortPair<ValueT, IndexT> &pair ) {
        const IndexT idx = static_cast<IndexT>( &pair - pairs.data() );
        pair = SortPair<ValueT, IndexT> { rawData[idx], idx };
      } );
    }
    else
    {
      // slower path, because we have to test for no-data cells...
      pairs.reserve( totalCells );
      for ( qgssize i = 0; i < totalCells; ++i )
      {
        if ( hasNoDataValue )
        {
          const ValueT v = rawData[i];
          if ( std::isnan( static_cast<double>( v ) ) || v == noDataVal )
            continue;
          pairs.push_back( SortPair<ValueT, IndexT> { v, static_cast<IndexT>( i ) } );
        }
        else if ( !block->isNoData( i ) )
        {
          pairs.push_back( SortPair<ValueT, IndexT> { rawData[i], static_cast<IndexT>( i ) } );
        }
      }
    }

    std::sort( pairs.begin(), pairs.end() );

    // copy sorted index array to a more compact structure
    std::vector<IndexT> sortedIndices( pairs.size() );
    std::transform( pairs.begin(), pairs.end(), sortedIndices.begin(), []( const SortPair<ValueT, IndexT> &p ) { return p.idx; } );

    return sortedIndices;
  }

  template<typename IndexT> std::vector<IndexT> createSortedIndices( const QgsRasterBlock *block, qgssize totalCells )
  {
    switch ( block->dataType() )
    {
      case Qgis::DataType::Byte:
        return createSortedIndices<quint8, IndexT>( block, totalCells );
      case Qgis::DataType::Int8:
        return createSortedIndices<qint8, IndexT>( block, totalCells );
      case Qgis::DataType::UInt16:
        return createSortedIndices<quint16, IndexT>( block, totalCells );
      case Qgis::DataType::Int16:
        return createSortedIndices<qint16, IndexT>( block, totalCells );
      case Qgis::DataType::UInt32:
        return createSortedIndices<quint32, IndexT>( block, totalCells );
      case Qgis::DataType::Int32:
        return createSortedIndices<qint32, IndexT>( block, totalCells );
      case Qgis::DataType::Float32:
        return createSortedIndices<float, IndexT>( block, totalCells );
      case Qgis::DataType::Float64:
        return createSortedIndices<double, IndexT>( block, totalCells );
      case Qgis::DataType::UnknownDataType:
      case Qgis::DataType::CInt16:
      case Qgis::DataType::CInt32:
      case Qgis::DataType::CFloat32:
      case Qgis::DataType::CFloat64:
      case Qgis::DataType::ARGB32:
      case Qgis::DataType::ARGB32_Premultiplied:
        return std::vector<IndexT>();
    }
    BUILTIN_UNREACHABLE
  }

} //namespace

QgsSortedRasterBlockIndex::QgsSortedRasterBlockIndex( const QgsRasterBlock *block )
{
  if ( !block || block->isEmpty() )
    return;

  mBlock = block;
  mCols = block->width();
  mRows = block->height();
  const qgssize totalCells = static_cast<qgssize>( mCols ) * mRows;
  if ( totalCells <= static_cast<qgssize>( std::numeric_limits<uint32_t>::max() ) )
  {
    mSortedIndices = createSortedIndices<uint32_t>( mBlock, totalCells );
  }
  else
  {
    mSortedIndices = createSortedIndices<qgssize>( mBlock, totalCells );
  }
}

qgssize QgsSortedRasterBlockIndex::sortedCount() const
{
  return std::visit( []( const auto &vec ) -> qgssize { return vec.size(); }, mSortedIndices );
}

qgssize QgsSortedRasterBlockIndex::sortedIndex( qgssize rank, Qt::SortOrder order ) const
{
  return std::visit(
    [rank, order]( const auto &vec ) -> qgssize {
      const std::size_t mappedRank = ( order == Qt::AscendingOrder ) ? rank : ( vec.size() - 1 - rank );
      return static_cast<qgssize>( vec[mappedRank] );
    },
    mSortedIndices
  );
}

double QgsSortedRasterBlockIndex::sortedValue( qgssize rank, Qt::SortOrder order ) const
{
  return mBlock->value( sortedIndex( rank, order ) );
}

void QgsSortedRasterBlockIndex::sortedColumnRow( qgssize rank, int &column, int &row, Qt::SortOrder order ) const
{
  indexToColumnAndRow( sortedIndex( rank, order ), column, row );
}
