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

#include "qgis.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsreadwritecontext.h"

class QgsRenderContext;

/**
 * \ingroup core
 * \brief Stores coordinates of a tile in a tile matrix set.
 *
 * Tile matrix is identified
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

    bool operator==( const QgsTileXYZ &other ) const { return mColumn == other.mColumn && mRow == other.mRow && mZoomLevel == other.mZoomLevel; }
    bool operator!=( const QgsTileXYZ &other ) const { return !( *this == other ); }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = QStringLiteral( "<QgsTileXYZ: %1, %2, %3>" ).arg( sipCpp->column() ).arg( sipCpp->row() ).arg( sipCpp->zoomLevel() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
    int mColumn = -1;
    int mRow = -1;
    int mZoomLevel = -1;
};

/**
 * Returns a hash for a tile \a id.
 *
 * \since QGIS 3.32
 */
CORE_EXPORT inline uint qHash( QgsTileXYZ id ) SIP_SKIP
{
  return id.column() + id.row() + id.zoomLevel();

  const uint h1 = qHash( static_cast< quint64 >( id.column( ) ) );
  const uint h2 = qHash( static_cast< quint64 >( id.row() ) );
  const uint h3 = qHash( static_cast< quint64 >( id.zoomLevel() ) );
  return h1 ^ ( h2 << 1 ) ^ ( h3 );
}


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
    int mStartColumn = -1;
    int mEndColumn = -1;
    int mStartRow = -1;
    int mEndRow = -1;
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
    static QgsTileMatrix fromWebMercator( int zoomLevel );

    /**
     * Returns a tile matrix for a specific CRS, top left point, zoom level 0 dimension in CRS units.
     *
     * The \a z0Dimension argument must specify the dimension (width or height, in map units) of the root tiles in zoom level 0.
     */
    static QgsTileMatrix fromCustomDef( int zoomLevel, const QgsCoordinateReferenceSystem &crs,
                                        const QgsPointXY &z0TopLeftPoint, double z0Dimension,
                                        int z0MatrixWidth = 1, int z0MatrixHeight = 1 );

    //! Returns a tile matrix based on another one
    static QgsTileMatrix fromTileMatrix( int zoomLevel, const QgsTileMatrix &tileMatrix );

    /**
     * Returns the crs of the tile matrix.
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /**
     * Sets the \a crs of the tile matrix.
     *
     * \see crs()
     * \since QGIS 3.22.6
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs ) { mCrs = crs;}

    /**
     * Returns the zoom level of the tile matrix.
     *
     * \see setZoomLevel()
     */
    int zoomLevel() const { return mZoomLevel; }

    /**
     * Sets the zoom \a level of the tile matrix.
     *
     * \see zoomLevel()
     * \since QGIS 3.22.6
     */
    void setZoomLevel( int level ) { mZoomLevel = level; }

    //! Returns number of columns of the tile matrix
    int matrixWidth() const { return mMatrixWidth; }

    //! Returns number of rows of the tile matrix
    int matrixHeight() const { return mMatrixHeight; }

    //! Returns extent of the tile matrix
    QgsRectangle extent() const { return mExtent; }

    /**
     * Returns scale denominator of the tile matrix.
     *
     * \see setScale()
     */
    double scale() const { return mScaleDenom; }

    /**
     * Sets the scale denominator of the tile matrix.
     *
     * \see scale()
     * \since QGIS 3.22.6
     */
    void setScale( double scale ) { mScaleDenom = scale; }

    //! Returns extent of the given tile in this matrix
    QgsRectangle tileExtent( QgsTileXYZ id ) const;

    //! Returns center of the given tile in this matrix
    QgsPointXY tileCenter( QgsTileXYZ id ) const;

    //! Returns tile range that fully covers the given extent
    QgsTileRange tileRangeFromExtent( const QgsRectangle &mExtent ) const;

    //! Returns row/column coordinates (floating point number) from the given point in map coordinates
    QPointF mapToTileCoordinates( const QgsPointXY &mapPoint ) const;

    //! Returns the root status of the tile matrix (zoom level == 0)
    bool isRootTileMatrix() const { return mZoomLevel == 0; }

  private:
    //! Crs associated with the tile matrix
    QgsCoordinateReferenceSystem mCrs;
    //! Zoom level index associated with the tile matrix
    int mZoomLevel = -1;
    //! Number of columns of the tile matrix
    int mMatrixWidth = 0;
    //! Number of rows of the tile matrix
    int mMatrixHeight = 0;
    //! Matrix extent in map units in the CRS of tile matrix set
    QgsRectangle mExtent;
    //! Scale denominator of the map scale associated with the tile matrix
    double mScaleDenom = 0;
    //! Width of a single tile in map units (derived from extent and matrix size)
    double mTileXSpan = 0;
    //! Height of a single tile in map units (derived from extent and matrix size)
    double mTileYSpan = 0;

    friend class QgsTileMatrixSet;
};


/**
 * \ingroup core
 * \brief Defines a set of tile matrices for multiple zoom levels.
 *
 * \since QGIS 3.22.6
 */
class CORE_EXPORT QgsTileMatrixSet
{

  public:

    QgsTileMatrixSet();

    virtual ~QgsTileMatrixSet() = default;

    /**
     * Returns TRUE if the matrix set is empty.
     */
    bool isEmpty() const;

    /**
     * Adds tile matrices corresponding to the standard web mercator/GoogleCRS84Quad setup.
     */
    void addGoogleCrs84QuadTiles( int minimumZoom = 0, int maximumZoom = 14 );

    /**
     * Returns the tile matrix corresponding to the specified \a zoom.
     */
    QgsTileMatrix tileMatrix( int zoom ) const;

    /**
     * Returns the root tile matrix (usually corresponding to zoom level 0).
     *
     * \since QGIS 3.28
     */
    QgsTileMatrix rootMatrix() const;

    /**
     * Sets the root tile \a matrix (usually corresponding to zoom level 0).
     *
     * \since QGIS 3.28
     */
    void setRootMatrix( const QgsTileMatrix &matrix );

    /**
     * Adds a \a matrix to the set.
     *
     * Any existing matrix with the same QgsTileMatrix::zoomLevel() will be replaced.
     */
    void addMatrix( const QgsTileMatrix &matrix );

    /**
     * Returns the minimum zoom level for tiles present in the set.
     *
     * \see maximumZoom()
     */
    int minimumZoom() const;

    /**
     * Returns the maximum zoom level for tiles present in the set.
     *
     * \see minimumZoom()
     */
    int maximumZoom() const;

    /**
     * Deletes any existing matrices which fall outside the zoom range specified
     * by \a minimumZoom to \a maximumZoom, inclusive.
     */
    void dropMatricesOutsideZoomRange( int minimumZoom, int maximumZoom );

    /**
     * Returns the availability of the given tile in this matrix.
     *
     * This method can be used to determine whether a particular tile actually
     * exists within the matrix, or is not available (e.g. due to holes within the matrix).
     *
     * This method returns Qgis::TileAvailability::Available by default, unless specific
     * tile availability is known for the given \a id.
     *
     * \since QGIS 3.32
     */
    Qgis::TileAvailability tileAvailability( QgsTileXYZ id ) const;

    /**
     * Returns the coordinate reference system associated with the tiles.
     *
     * In the case of a tile set containing mixed CRS at different zoom levels
     * this method will return the crs of the minimum zoom tile matrix.
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Calculates a fractional zoom level given a map \a scale denominator.
     *
     * The zoom level will be linearly interpolated between zoom levels present in the set.
     */
    double scaleToZoom( double scale ) const;

    /**
     * Finds the best fitting (integer) zoom level given a map \a scale denominator. An
     * optional \a clamp parameter can be set to FALSE to disable clamping zoom level
     * to the tile matrix set's minimum and maximum zoom level.
     *
     * Values are constrained to the zoom levels between minimumZoom() and maximumZoom().
     */
    int scaleToZoomLevel( double scale, bool clamp = true ) const;

    /**
     * Calculates the correct scale to use for the tiles when rendered using the specified render \a context.
     *
     * \since QGIS 3.26
     */
    double scaleForRenderContext( const QgsRenderContext &context ) const;

    /**
     * Calculates the correct scale to use for the tiles when rendered using the specified map properties.
     *
     * \since QGIS 3.26
     */
    double calculateTileScaleForMap( double actualMapScale,
                                     const QgsCoordinateReferenceSystem &mapCrs,
                                     const QgsRectangle &mapExtent,
                                     const QSize mapSize,
                                     const double mapDpi
                                   ) const;

    /**
     * Reads the set from an XML \a element.
     *
     * \see writeXml()
     */
    virtual bool readXml( const QDomElement &element, QgsReadWriteContext &context );

    /**
     * Writes the set to an XML element.
     */
    virtual QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Returns the scale to tile zoom method.
     *
     * \see setScaleToTileZoomMethod()
     */
    Qgis::ScaleToTileZoomLevelMethod scaleToTileZoomMethod() const { return mScaleToTileZoomMethod; }

    /**
     * Sets the scale to tile zoom method.
     *
     * \see scaleToTileZoomMethod()
     */
    void setScaleToTileZoomMethod( Qgis::ScaleToTileZoomLevelMethod method ) { mScaleToTileZoomMethod = method; }

    /**
     * Returns a list of tiles in the given tile range.
     *
     * \since QGIS 3.32
     */
    QVector<QgsTileXYZ> tilesInRange( QgsTileRange range, int zoomLevel ) const;

  protected:
    std::function< Qgis::TileAvailability( QgsTileXYZ id ) > mTileAvailabilityFunction;
    std::function< Qgis::TileAvailability( QgsTileXYZ id, QgsTileXYZ &replacement ) > mTileReplacementFunction;

    // Usually corresponds to zoom level 0, even if that zoom level is NOT present in the actual tile matrices for this set
    QgsTileMatrix mRootMatrix;
    QMap< int, QgsTileMatrix > mTileMatrices;
    Qgis::ScaleToTileZoomLevelMethod mScaleToTileZoomMethod = Qgis::ScaleToTileZoomLevelMethod::MapBox;
};

#endif // QGSTILES_H
