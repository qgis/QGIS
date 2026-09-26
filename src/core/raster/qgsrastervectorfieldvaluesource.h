/***************************************************************************
                         qgsrastervectorfieldvaluesource.h
                         ---------------------------------
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

#ifndef QGSRASTERVECTORFIELDVALUESOURCE_H
#define QGSRASTERVECTORFIELDVALUESOURCE_H

#include <memory>

#include "qgis.h"
#include "qgsrasterinterface.h"
#include "qgsrectangle.h"
#include "qgsvectorfieldvaluesource.h"

#define SIP_NO_FILE

class QgsRasterBlock;

///@cond PRIVATE

/**
 * \ingroup core
 *
 * \brief Base class of the vector field value sources which read a raster block.
 *
 * Holds the geometry shared by the sources, which all sample a block of a fixed size covering a
 * fixed extent, and implements the parts which only depend on that geometry.
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class QgsRasterVectorFieldValueSourceBase : public QgsVectorFieldValueSource
{
  public:
    /**
     * Constructs a source reading a block of \a columns by \a rows cells covering \a blockExtent.
     *
     * \a dataExtent is the extent over which the raster actually holds data.
     */
    QgsRasterVectorFieldValueSourceBase( const QgsRectangle &blockExtent, const QgsRectangle &dataExtent, int columns, int rows );

    QgsRectangle extent() const override;
    QVector<QgsPointXY> seedPoints( const QgsRectangle &extent ) const override;

    //! Returns the grid of the block being sampled, one position per cell, placed on its center
    bool nativeLayout( QgsPointXY &origin, double &spacingX, double &spacingY ) const override;

    //! Upper bound on the number of points returned by seedPoints()
    static constexpr int MAXIMUM_SEED_POINTS = 100'000;

  protected:
    //! Returns TRUE if the block has a usable geometry, which every sampling method needs
    bool isValid() const;

    //! Returns TRUE if the cell at \a row and \a column holds a value, which seedPoints() skips when it does not
    virtual bool cellHasValue( int row, int column ) const = 0;

    QgsRectangle mBlockExtent;
    QgsRectangle mDataExtent;

    int mColumns = 0;
    int mRows = 0;
    double mXResolution = 0;
    double mYResolution = 0;
};

/**
 * \ingroup core
 *
 * \brief Vector field value source backed by a pair of raster bands holding the x and y components.
 *
 * The two bands are read once, for the extent and at the resolution the renderer decided on, and
 * the source then samples them bilinearly at arbitrary positions.
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class QgsRasterVectorFieldValueSource : public QgsRasterVectorFieldValueSourceBase
{
  public:
    /**
     * Constructs a source sampling \a xValues and \a yValues, two blocks of identical size covering
     * \a blockExtent.
     *
     * \a dataExtent is the extent over which the raster actually holds data, and \a maximumMagnitude
     * the maximum magnitude of the whole raster, which must be stable for the entire rendering.
     */
    QgsRasterVectorFieldValueSource(
      std::shared_ptr<const QgsRasterBlock> xValues, std::shared_ptr<const QgsRasterBlock> yValues, const QgsRectangle &blockExtent, const QgsRectangle &dataExtent, double maximumMagnitude
    );

    QgsRasterVectorFieldValueSource *clone() const override;
    QgsVector vectorValue( const QgsPointXY &point ) const override;
    double maximumMagnitude() const override;
    std::unique_ptr<QgsRasterInterface> magnitudeSource( const QgsRenderContext &context, QSize size ) const override;

  private:
    /**
     * Returns the value of \a block at \a point, interpolated between the four surrounding cell
     * centers, or NaN if no data is available there.
     */
    double sampledValue( const QgsRasterBlock *block, const QgsPointXY &point ) const;

    bool cellHasValue( int row, int column ) const override;

    std::shared_ptr<const QgsRasterBlock> mXValues;
    std::shared_ptr<const QgsRasterBlock> mYValues;
    double mMaximumMagnitude = 0;
};

/**
 * \ingroup core
 *
 * \brief Vector field value source backed by a single raster band holding coded compass directions.
 *
 * The band holds one of the integer encodings of Qgis::RasterDirectionEncoding for the 8 cardinal directions.
 * The values therefore cannot be interpolated or averaged, so for an area the majority of values
 * is used, see setSamplingWindow().
 *
 * All vectors have a magnitude of 1.
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsRasterVectorFieldEncodedDirectionValueSource : public QgsRasterVectorFieldValueSourceBase
{
  public:
    /**
     * Constructs a source decoding \a values, a block of \a columns by \a rows cells covering
     * \a blockExtent, with \a encoding.
     *
     * \a dataExtent is the extent over which the raster actually holds data.
     */
    QgsRasterVectorFieldEncodedDirectionValueSource( std::shared_ptr<const QgsRasterBlock> values, Qgis::RasterDirectionEncoding encoding, const QgsRectangle &blockExtent, const QgsRectangle &dataExtent );

    QgsRasterVectorFieldEncodedDirectionValueSource *clone() const override;
    QgsVector vectorValue( const QgsPointXY &point ) const override;

    //! Always returns 1, as the encodings carry a direction only
    double maximumMagnitude() const override;

    /**
     * Sets the size, in map units, of the area each sample stands for.
     *
     * A sample then returns the direction held by most of the cells of that area, which is the only
     * aggregation which makes sense for coded values. A size of zero, which is the default, samples
     * the nearest cell alone, which is what the streamline and trace integrators need as they walk
     * the field point by point.
     */
    void setSamplingWindow( double width, double height ) override;

    /**
     * Returns the compass ordinal of \a value under \a encoding, or -1 when it encodes no direction.
     *
     * The ordinals run clockwise from north, 0 being north, 1 north east and 7 north west.
     */
    static int directionOrdinal( Qgis::RasterDirectionEncoding encoding, double value );

    //! Returns the unit vector of the compass \a ordinal, with a positive y pointing north
    static QgsVector ordinalVector( int ordinal );

  private:
    //! Returns the ordinal of the cell holding \a point, or -1 when it holds no direction
    int nearestOrdinal( const QgsPointXY &point ) const;

    //! Returns the ordinal held by most of the cells within the sampling window around \a point, or -1
    int majorityOrdinal( const QgsPointXY &point ) const;

    bool cellHasValue( int row, int column ) const override;

    std::shared_ptr<const QgsRasterBlock> mValues;
    Qgis::RasterDirectionEncoding mEncoding = Qgis::RasterDirectionEncoding::Esri;
    double mWindowWidth = 0;
    double mWindowHeight = 0;
};

/**
 * \ingroup core
 *
 * \brief Single band raster interface exposing the magnitude of a pair of vector component blocks.
 *
 * Used to build the color ramp background image of streamlines, raster equivalent to mesh layer's
 * QgsMeshLayerInterpolator.
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class QgsRasterVectorFieldMagnitudeInterface : public QgsRasterInterface
{
  public:
    //! Constructs an interface over \a xValues and \a yValues, two blocks of identical size covering \a blockExtent
    QgsRasterVectorFieldMagnitudeInterface( std::shared_ptr<const QgsRasterBlock> xValues, std::shared_ptr<const QgsRasterBlock> yValues, const QgsRectangle &blockExtent );

    QgsRasterInterface *clone() const override;
    Qgis::DataType dataType( int bandNo ) const override;
    int bandCount() const override;
    QgsRectangle extent() const override;
    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;

  private:
    std::shared_ptr<const QgsRasterBlock> mXValues;
    std::shared_ptr<const QgsRasterBlock> mYValues;
    QgsRectangle mBlockExtent;
};

///@endcond

#endif // QGSRASTERVECTORFIELDVALUESOURCE_H
