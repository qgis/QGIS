/***************************************************************************
                         qgsrastervectorfieldvaluesource.cpp
                         -----------------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastervectorfieldvaluesource.h"

#include <algorithm>
#include <cmath>

#include "qgsrasterblock.h"

///@cond PRIVATE

static const double sNoValue = std::numeric_limits<double>::quiet_NaN();

//! Returns the value of \a block at \a row, \a column, or NaN if the cell is out of range or holds no data
static double cellValue( const QgsRasterBlock *block, int row, int column )
{
  if ( !block || row < 0 || column < 0 || row >= block->height() || column >= block->width() )
    return sNoValue;

  bool isNoData = false;
  const double value = block->valueAndNoData( row, column, isNoData );
  if ( isNoData || std::isnan( value ) )
    return sNoValue;

  return value;
}


//
// QgsRasterVectorFieldValueSourceBase
//

QgsRasterVectorFieldValueSourceBase::QgsRasterVectorFieldValueSourceBase( const QgsRectangle &blockExtent, const QgsRectangle &dataExtent, int columns, int rows )
  : mBlockExtent( blockExtent )
  , mDataExtent( dataExtent )
  , mColumns( columns )
  , mRows( rows )
{
  if ( mColumns > 0 && mRows > 0 && !mBlockExtent.isEmpty() )
  {
    mXResolution = mBlockExtent.width() / mColumns;
    mYResolution = mBlockExtent.height() / mRows;
  }
}

bool QgsRasterVectorFieldValueSourceBase::isValid() const
{
  return mColumns > 0 && mRows > 0 && mXResolution > 0 && mYResolution > 0;
}

QgsRectangle QgsRasterVectorFieldValueSourceBase::extent() const
{
  return mDataExtent;
}

bool QgsRasterVectorFieldValueSourceBase::nativeLayout( QgsPointXY &origin, double &spacingX, double &spacingY ) const
{
  if ( !isValid() )
    return false;

  // one position per cell of the block, placed on its center
  spacingX = mXResolution;
  spacingY = mYResolution;
  origin = QgsPointXY( mBlockExtent.xMinimum() + spacingX / 2, mBlockExtent.yMaximum() - spacingY / 2 );
  return true;
}

QVector<QgsPointXY> QgsRasterVectorFieldValueSourceBase::seedPoints( const QgsRectangle &extent ) const
{
  QVector<QgsPointXY> points;
  if ( !isValid() )
    return points;

  const QgsRectangle seedExtent = mBlockExtent.intersect( extent );
  if ( seedExtent.isEmpty() )
    return points;

  // the cells whose center falls within the requested extent
  const int firstColumn = std::max( 0, static_cast<int>( std::ceil( ( seedExtent.xMinimum() - mBlockExtent.xMinimum() ) / mXResolution - 0.5 ) ) );
  const int lastColumn = std::min( mColumns - 1, static_cast<int>( std::floor( ( seedExtent.xMaximum() - mBlockExtent.xMinimum() ) / mXResolution - 0.5 ) ) );
  const int firstRow = std::max( 0, static_cast<int>( std::ceil( ( mBlockExtent.yMaximum() - seedExtent.yMaximum() ) / mYResolution - 0.5 ) ) );
  const int lastRow = std::min( mRows - 1, static_cast<int>( std::floor( ( mBlockExtent.yMaximum() - seedExtent.yMinimum() ) / mYResolution - 0.5 ) ) );

  if ( firstColumn > lastColumn || firstRow > lastRow )
    return points;

  // a fine raster holds far more cells than can usefully seed a trace, so thin them out evenly
  const qint64 candidates = static_cast<qint64>( lastColumn - firstColumn + 1 ) * ( lastRow - firstRow + 1 );
  int step = 1;
  if ( candidates > MAXIMUM_SEED_POINTS )
    step = static_cast<int>( std::ceil( std::sqrt( static_cast<double>( candidates ) / MAXIMUM_SEED_POINTS ) ) );

  points.reserve( static_cast<int>( candidates / ( static_cast<qint64>( step ) * step ) ) + 1 );
  for ( int row = firstRow; row <= lastRow; row += step )
  {
    const double y = mBlockExtent.yMaximum() - ( row + 0.5 ) * mYResolution;
    for ( int column = firstColumn; column <= lastColumn; column += step )
    {
      if ( !cellHasValue( row, column ) )
        continue;

      points.append( QgsPointXY( mBlockExtent.xMinimum() + ( column + 0.5 ) * mXResolution, y ) );
    }
  }

  return points;
}


//
// QgsRasterVectorFieldValueSource
//

//! Returns the number of columns the two component blocks have in common
static int commonColumns( const std::shared_ptr<const QgsRasterBlock> &first, const std::shared_ptr<const QgsRasterBlock> &second )
{
  return first && second ? std::min( first->width(), second->width() ) : 0;
}

//! Returns the number of rows the two component blocks have in common
static int commonRows( const std::shared_ptr<const QgsRasterBlock> &first, const std::shared_ptr<const QgsRasterBlock> &second )
{
  return first && second ? std::min( first->height(), second->height() ) : 0;
}

QgsRasterVectorFieldValueSource::QgsRasterVectorFieldValueSource(
  std::shared_ptr<const QgsRasterBlock> xValues, std::shared_ptr<const QgsRasterBlock> yValues, const QgsRectangle &blockExtent, const QgsRectangle &dataExtent, double maximumMagnitude
)
  : QgsRasterVectorFieldValueSourceBase( blockExtent, dataExtent, commonColumns( xValues, yValues ), commonRows( xValues, yValues ) )
  , mXValues( std::move( xValues ) )
  , mYValues( std::move( yValues ) )
  , mMaximumMagnitude( maximumMagnitude )
{}

bool QgsRasterVectorFieldValueSource::cellHasValue( int row, int column ) const
{
  return !std::isnan( cellValue( mXValues.get(), row, column ) ) && !std::isnan( cellValue( mYValues.get(), row, column ) );
}

QgsRasterVectorFieldValueSource *QgsRasterVectorFieldValueSource::clone() const
{
  // the blocks are shared and never written to, so the copy is cheap
  return new QgsRasterVectorFieldValueSource( *this );
}

double QgsRasterVectorFieldValueSource::sampledValue( const QgsRasterBlock *block, const QgsPointXY &point ) const
{
  // position of the point in cell units, measured from the center of the top left cell
  const double x = ( point.x() - mBlockExtent.xMinimum() ) / mXResolution - 0.5;
  const double y = ( mBlockExtent.yMaximum() - point.y() ) / mYResolution - 0.5;

  const int column = static_cast<int>( std::floor( x ) );
  const int row = static_cast<int>( std::floor( y ) );

  const double v00 = cellValue( block, row, column );
  const double v01 = cellValue( block, row, column + 1 );
  const double v10 = cellValue( block, row + 1, column );
  const double v11 = cellValue( block, row + 1, column + 1 );

  if ( std::isnan( v00 ) || std::isnan( v01 ) || std::isnan( v10 ) || std::isnan( v11 ) )
  {
    // at the edges of the data, and around nodata cells, fall back to the nearest cell so that the
    // last row and column of the raster still carry values
    return cellValue( block, static_cast<int>( std::floor( y + 0.5 ) ), static_cast<int>( std::floor( x + 0.5 ) ) );
  }

  const double dx = x - column;
  const double dy = y - row;

  return v00 * ( 1 - dx ) * ( 1 - dy ) + v01 * dx * ( 1 - dy ) + v10 * ( 1 - dx ) * dy + v11 * dx * dy;
}

QgsVector QgsRasterVectorFieldValueSource::vectorValue( const QgsPointXY &point ) const
{
  if ( !isValid() || !mBlockExtent.contains( point ) )
    return QgsVector( sNoValue, sNoValue );

  const double x = sampledValue( mXValues.get(), point );
  const double y = sampledValue( mYValues.get(), point );
  if ( std::isnan( x ) || std::isnan( y ) )
    return QgsVector( sNoValue, sNoValue );

  return QgsVector( x, y );
}

double QgsRasterVectorFieldValueSource::maximumMagnitude() const
{
  return mMaximumMagnitude;
}

std::unique_ptr<QgsRasterInterface> QgsRasterVectorFieldValueSource::magnitudeSource( const QgsRenderContext &, QSize ) const
{
  if ( !isValid() )
    return nullptr;

  return std::make_unique<QgsRasterVectorFieldMagnitudeInterface>( mXValues, mYValues, mBlockExtent );
}


//
// QgsRasterVectorFieldEncodedDirectionValueSource
//

QgsRasterVectorFieldEncodedDirectionValueSource::QgsRasterVectorFieldEncodedDirectionValueSource(
  std::shared_ptr<const QgsRasterBlock> values, Qgis::RasterDirectionEncoding encoding, const QgsRectangle &blockExtent, const QgsRectangle &dataExtent
)
  : QgsRasterVectorFieldValueSourceBase( blockExtent, dataExtent, values ? values->width() : 0, values ? values->height() : 0 )
  , mValues( std::move( values ) )
  , mEncoding( encoding )
{}

QgsRasterVectorFieldEncodedDirectionValueSource *QgsRasterVectorFieldEncodedDirectionValueSource::clone() const
{
  // the block is shared and never written to, so the copy is cheap
  return new QgsRasterVectorFieldEncodedDirectionValueSource( *this );
}

int QgsRasterVectorFieldEncodedDirectionValueSource::directionOrdinal( Qgis::RasterDirectionEncoding encoding, double value )
{
  if ( std::isnan( value ) )
    return -1;

  // the encodings are all integer codes, a value between two of them names no direction
  const double rounded = std::round( value );
  if ( std::abs( value - rounded ) > 1e-6 || std::abs( rounded ) > std::numeric_limits<int>::max() )
    return -1;

  int code = static_cast<int>( rounded );

  switch ( encoding )
  {
    case Qgis::RasterDirectionEncoding::Esri:
      // 32   64  128
      // 16         1
      //  8    4    2
      switch ( code )
      {
        case 64:
          return 0; // N
        case 128:
          return 1; // NE
        case 1:
          return 2; // E
        case 2:
          return 3; // SE
        case 4:
          return 4; // S
        case 8:
          return 5; // SW
        case 16:
          return 6; // W
        case 32:
          return 7; // NW
        default:
          return -1;
      }

    case Qgis::RasterDirectionEncoding::Grass:
      // 3   2   1
      // 4       8
      // 5   6   7
      code = std::abs( code );
      switch ( code )
      {
        case 2:
          return 0; // N
        case 1:
          return 1; // NE
        case 8:
          return 2; // E
        case 7:
          return 3; // SE
        case 6:
          return 4; // S
        case 5:
          return 5; // SW
        case 4:
          return 6; // W
        case 3:
          return 7; // NW
        default:
          return -1;
      }

    case Qgis::RasterDirectionEncoding::Saga:
      // clockwise from north, which is the ordinal itself
      // 7   0   1
      // 6       2
      // 5   4   3
      return code >= 0 && code <= 7 ? code : -1;

    case Qgis::RasterDirectionEncoding::PcRaster:
      // 7   8   9
      // 4       6
      // 1   2   3
      switch ( code )
      {
        case 8:
          return 0; // N
        case 9:
          return 1; // NE
        case 6:
          return 2; // E
        case 3:
          return 3; // SE
        case 2:
          return 4; // S
        case 1:
          return 5; // SW
        case 4:
          return 6; // W
        case 7:
          return 7; // NW
        default:
          return -1;
      }
  }

  return -1;
}

QgsVector QgsRasterVectorFieldEncodedDirectionValueSource::ordinalVector( int ordinal )
{
  constexpr double D = M_SQRT1_2;
  // clockwise from north, north up
  switch ( ordinal )
  {
    case 0:
      return QgsVector( 0, 1 ); // N
    case 1:
      return QgsVector( D, D ); // NE
    case 2:
      return QgsVector( 1, 0 ); // E
    case 3:
      return QgsVector( D, -D ); // SE
    case 4:
      return QgsVector( 0, -1 ); // S
    case 5:
      return QgsVector( -D, -D ); // SW
    case 6:
      return QgsVector( -1, 0 ); // W
    case 7:
      return QgsVector( -D, D ); // NW
    default:
      return QgsVector( sNoValue, sNoValue );
  };
}

void QgsRasterVectorFieldEncodedDirectionValueSource::setSamplingWindow( double width, double height )
{
  mWindowWidth = std::max( 0.0, width );
  mWindowHeight = std::max( 0.0, height );
}

bool QgsRasterVectorFieldEncodedDirectionValueSource::cellHasValue( int row, int column ) const
{
  return directionOrdinal( mEncoding, cellValue( mValues.get(), row, column ) ) >= 0;
}

int QgsRasterVectorFieldEncodedDirectionValueSource::nearestOrdinal( const QgsPointXY &point ) const
{
  const int column = static_cast<int>( std::floor( ( point.x() - mBlockExtent.xMinimum() ) / mXResolution ) );
  const int row = static_cast<int>( std::floor( ( mBlockExtent.yMaximum() - point.y() ) / mYResolution ) );

  return directionOrdinal( mEncoding, cellValue( mValues.get(), row, column ) );
}

int QgsRasterVectorFieldEncodedDirectionValueSource::majorityOrdinal( const QgsPointXY &point ) const
{
  // the cells whose centers fall within the window centered on the point
  const double xMinimum = point.x() - mWindowWidth / 2;
  const double xMaximum = point.x() + mWindowWidth / 2;
  const double yMinimum = point.y() - mWindowHeight / 2;
  const double yMaximum = point.y() + mWindowHeight / 2;

  const int firstColumn = std::max( 0, static_cast<int>( std::ceil( ( xMinimum - mBlockExtent.xMinimum() ) / mXResolution - 0.5 ) ) );
  const int lastColumn = std::min( mColumns - 1, static_cast<int>( std::floor( ( xMaximum - mBlockExtent.xMinimum() ) / mXResolution - 0.5 ) ) );
  const int firstRow = std::max( 0, static_cast<int>( std::ceil( ( mBlockExtent.yMaximum() - yMaximum ) / mYResolution - 0.5 ) ) );
  const int lastRow = std::min( mRows - 1, static_cast<int>( std::floor( ( mBlockExtent.yMaximum() - yMinimum ) / mYResolution - 0.5 ) ) );

  // a window narrower than a cell can fall between two cell centers, fall back to the cell it is in
  if ( firstColumn > lastColumn || firstRow > lastRow )
    return nearestOrdinal( point );

  int counts[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int best = 0;
  QgsVector resultant( 0, 0 );

  for ( int row = firstRow; row <= lastRow; ++row )
  {
    for ( int column = firstColumn; column <= lastColumn; ++column )
    {
      const int ordinal = directionOrdinal( mEncoding, cellValue( mValues.get(), row, column ) );
      if ( ordinal < 0 )
        continue;

      best = std::max( best, ++counts[ordinal] );
      resultant += ordinalVector( ordinal );
    }
  }

  if ( best == 0 )
    return -1; // the window holds no direction at all

  // several directions can be equally common, in which case the one closest to the resultant of the
  // whole window is the most representative. The lowest ordinal breaks a resultant which cancels out
  int majority = -1;
  double bestProjection = std::numeric_limits<double>::lowest();
  for ( int ordinal = 0; ordinal < 8; ++ordinal )
  {
    if ( counts[ordinal] != best )
      continue;

    const double projection = resultant * ordinalVector( ordinal );
    if ( projection > bestProjection )
    {
      bestProjection = projection;
      majority = ordinal;
    }
  }

  return majority;
}

QgsVector QgsRasterVectorFieldEncodedDirectionValueSource::vectorValue( const QgsPointXY &point ) const
{
  if ( !isValid() || !mBlockExtent.contains( point ) )
    return QgsVector( sNoValue, sNoValue );

  const bool hasWindow = mWindowWidth > 0 && mWindowHeight > 0;
  return ordinalVector( hasWindow ? majorityOrdinal( point ) : nearestOrdinal( point ) );
}

double QgsRasterVectorFieldEncodedDirectionValueSource::maximumMagnitude() const
{
  return 1.0;
}


//
// QgsRasterVectorFieldMagnitudeInterface
//

QgsRasterVectorFieldMagnitudeInterface::QgsRasterVectorFieldMagnitudeInterface( std::shared_ptr<const QgsRasterBlock> xValues, std::shared_ptr<const QgsRasterBlock> yValues, const QgsRectangle &blockExtent )
  : mXValues( std::move( xValues ) )
  , mYValues( std::move( yValues ) )
  , mBlockExtent( blockExtent )
{}

QgsRasterInterface *QgsRasterVectorFieldMagnitudeInterface::clone() const
{
  return new QgsRasterVectorFieldMagnitudeInterface( mXValues, mYValues, mBlockExtent );
}

Qgis::DataType QgsRasterVectorFieldMagnitudeInterface::dataType( int ) const
{
  return Qgis::DataType::Float64;
}

int QgsRasterVectorFieldMagnitudeInterface::bandCount() const
{
  return 1;
}

QgsRectangle QgsRasterVectorFieldMagnitudeInterface::extent() const
{
  return mBlockExtent;
}

QgsRasterBlock *QgsRasterVectorFieldMagnitudeInterface::block( int, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  auto outputBlock = std::make_unique<QgsRasterBlock>( Qgis::DataType::Float64, width, height );
  // the color ramp shader must see the holes in the data, which are encoded as NaN
  outputBlock->setNoDataValue( std::numeric_limits<double>::quiet_NaN() );

  if ( !mXValues || !mYValues || width <= 0 || height <= 0 || extent.isEmpty() || mBlockExtent.isEmpty() )
    return outputBlock.release();

  const int columns = std::min( mXValues->width(), mYValues->width() );
  const int rows = std::min( mXValues->height(), mYValues->height() );
  if ( columns <= 0 || rows <= 0 )
    return outputBlock.release();

  // the requested extent and size are those of the streamline field, which generally match neither
  // the extent nor the resolution of the cached blocks, so resample by geographic position
  const double xResolution = mBlockExtent.width() / columns;
  const double yResolution = mBlockExtent.height() / rows;
  const double outputXResolution = extent.width() / width;
  const double outputYResolution = extent.height() / height;

  for ( int row = 0; row < height; ++row )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    const double y = extent.yMaximum() - ( row + 0.5 ) * outputYResolution;
    const int sourceRow = static_cast<int>( std::floor( ( mBlockExtent.yMaximum() - y ) / yResolution ) );

    for ( int column = 0; column < width; ++column )
    {
      const double x = extent.xMinimum() + ( column + 0.5 ) * outputXResolution;
      const int sourceColumn = static_cast<int>( std::floor( ( x - mBlockExtent.xMinimum() ) / xResolution ) );

      const double xValue = cellValue( mXValues.get(), sourceRow, sourceColumn );
      const double yValue = cellValue( mYValues.get(), sourceRow, sourceColumn );

      outputBlock->setValue( row, column, std::hypot( xValue, yValue ) );
    }
  }

  return outputBlock.release();
}

///@endcond
