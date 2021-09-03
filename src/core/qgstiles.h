/***************************************************************************
  qgstiles.h
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

#ifndef QGSTILES_H
#define QGSTILES_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsrectangle.h"

/**
 * \ingroup core
 * \brief Stores coordinates of a tile in a tile matrix set. Tile matrix is identified
 * by the zoomLevel(), and the position within tile matrix is given by column()
 * and row().
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTileXYZ
{
  public:
    //! Constructs a tile identifier from given column, row and zoom level indices
    QgsTileXYZ( int tc = -1, int tr = -1, int tz = -1 )
      : mColumn( tc ), mRow( tr ), mZoomLevel( tz )
    {
    }

    //! Returns tile's column index (X)
    int column() const { return mColumn; }
    //! Returns tile's row index (Y)
    int row() const { return mRow; }
    //! Returns tile's zoom level (Z)
    int zoomLevel() const { return mZoomLevel; }

    //! Returns tile coordinates in a formatted string
    QString toString() const { return QStringLiteral( "X=%1 Y=%2 Z=%3" ).arg( mColumn ).arg( mRow ).arg( mZoomLevel ); }

  private:
    int mColumn;
    int mRow;
    int mZoomLevel;
};


/**
 * \ingroup core
 * \brief Range of tiles in a tile matrix to be rendered. The selection is rectangular,
 * given by start/end row and column numbers.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTileRange
{
  public:
    //! Constructs a range of tiles from given span of columns and rows
    QgsTileRange( int c1 = -1, int c2 = -1, int r1 = -1, int r2 = -1 )
      : mStartColumn( c1 ), mEndColumn( c2 ), mStartRow( r1 ), mEndRow( r2 ) {}

    //! Returns whether the range is valid (when all row/column numbers are not negative)
    bool isValid() const { return mStartColumn >= 0 && mEndColumn >= 0 && mStartRow >= 0 && mEndRow >= 0; }

    //! Returns index of the first column in the range
    int startColumn() const { return mStartColumn; }
    //! Returns index of the last column in the range
    int endColumn() const { return mEndColumn; }
    //! Returns index of the first row in the range
    int startRow() const { return mStartRow; }
    //! Returns index of the last row in the range
    int endRow() const { return mEndRow; }

  private:
    int mStartColumn;
    int mEndColumn;
    int mStartRow;
    int mEndRow;
};


/**
 * \ingroup core
 * \brief Defines a matrix of tiles for a single zoom level: it is defined by its size (width * \brief height)
 * and map extent that it covers.
 *
 * Please note that we follow the XYZ convention of X/Y axes, i.e. top-left tile has [0,0] coordinate
 * (which is different from TMS convention where bottom-left tile has [0,0] coordinate).
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTileMatrix
{
  public:

    //! Returns a tile matrix for the usual web mercator
    static QgsTileMatrix fromWebMercator( int mZoomLevel );

    //! Returns zoom level of the tile matrix
    int zoomLevel() const { return mZoomLevel; }

    //! Returns number of columns of the tile matrix
    int matrixWidth() const { return mMatrixWidth; }

    //! Returns number of rows of the tile matrix
    int matrixHeight() const { return mMatrixHeight; }

    //! Returns extent of the tile matrix
    QgsRectangle extent() const { return mExtent; }

    //! Returns scale denominator of the tile matrix
    double scale() const { return mScaleDenom; }

    //! Returns extent of the given tile in this matrix
    QgsRectangle tileExtent( QgsTileXYZ id ) const;

    //! Returns center of the given tile in this matrix
    QgsPointXY tileCenter( QgsTileXYZ id ) const;

    //! Returns tile range that fully covers the given extent
    QgsTileRange tileRangeFromExtent( const QgsRectangle &mExtent );

    //! Returns row/column coordinates (floating point number) from the given point in map coordinates
    QPointF mapToTileCoordinates( const QgsPointXY &mapPoint ) const;

  private:
    //! Zoom level index associated with the tile matrix
    int mZoomLevel;
    //! Number of columns of the tile matrix
    int mMatrixWidth;
    //! Number of rows of the tile matrix
    int mMatrixHeight;
    //! Matrix extent in map units in the CRS of tile matrix set
    QgsRectangle mExtent;
    //! Scale denominator of the map scale associated with the tile matrix
    double mScaleDenom;
    //! Width of a single tile in map units (derived from extent and matrix size)
    double mTileXSpan;
    //! Height of a single tile in map units (derived from extent and matrix size)
    double mTileYSpan;
};

#endif // QGSTILES_H
