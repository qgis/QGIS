/***************************************************************************
    qgsrasterblock.h - Class representing a block of raster data
     --------------------------------------
    Date                 : Oct 9, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERBLOCK_H
#define QGSRASTERBLOCK_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <limits>
#include <QImage>
#include "qgis.h"
#include "qgserror.h"
#include "qgslogger.h"
#include "qgsrasterrange.h"

class QgsRectangle;

/**
 * \ingroup core
 * Raster data container.
 */
class CORE_EXPORT QgsRasterBlock
{
  public:
    QgsRasterBlock();

    /**
     * \brief Constructor which allocates data block in memory
     *  \param dataType raster data type
     *  \param width width of data matrix
     *  \param height height of data matrix
     */
    QgsRasterBlock( Qgis::DataType dataType, int width, int height );

    virtual ~QgsRasterBlock();

    /**
     * \brief Reset block
     *  \param dataType raster data type
     *  \param width width of data matrix
     *  \param height height of data matrix
     *  \returns true on success
     */
    bool reset( Qgis::DataType dataType, int width, int height );

    // TODO: consider if use isValid() at all, isEmpty() should be sufficient
    // and works also if block is valid but empty - difference between valid and empty?

    /**
     * \brief Returns true if the block is valid (correctly filled with data).
     *  An empty block may still be valid (if zero size block was requested).
     *  If the block is not valid, error may be retrieved by error() method.
     */
    bool isValid() const { return mValid; }

    //! \brief Mark block as valid or invalid
    void setValid( bool valid ) { mValid = valid; }

    /**
     * Returns true if block is empty, i.e. its size is 0 (zero rows or cols).
     *  This method does not return true if size is not zero and all values are
     *  'no data' (null).
     */
    bool isEmpty() const;

    // Return data type size in bytes
    static int typeSize( int dataType )
    {
      // Modified and extended copy from GDAL
      switch ( dataType )
      {
        case Qgis::Byte:
          return 1;

        case Qgis::UInt16:
        case Qgis::Int16:
          return 2;

        case Qgis::UInt32:
        case Qgis::Int32:
        case Qgis::Float32:
        case Qgis::CInt16:
          return 4;

        case Qgis::Float64:
        case Qgis::CInt32:
        case Qgis::CFloat32:
          return 8;

        case Qgis::CFloat64:
          return 16;

        case Qgis::ARGB32:
        case Qgis::ARGB32_Premultiplied:
          return 4;

        default:
          return 0;
      }
    }

    // Data type in bytes
    int dataTypeSize() const
    {
      return typeSize( mDataType );
    }

    //! Returns true if data type is numeric
    static bool typeIsNumeric( Qgis::DataType type );

    //! Returns true if data type is color
    static bool typeIsColor( Qgis::DataType type );

    //! Returns data type
    Qgis::DataType dataType() const { return mDataType; }

    //! For given data type returns wider type and sets no data value
    static Qgis::DataType typeWithNoDataValue( Qgis::DataType dataType, double *noDataValue );

    /**
     * True if the block has no data value.
     * \returns true if the block has no data value
     * \see noDataValue(), setNoDataValue(), resetNoDataValue()
     */
    bool hasNoDataValue() const { return mHasNoDataValue; }

    /**
     * Returns true if the block may contain no data. It does not guarantee
     * that it really contains any no data. It can be used to speed up processing.
     * Not the difference between this method and hasNoDataValue().
     * \returns true if the block may contain no data */
    bool hasNoData() const;

    /**
     * Sets cell value that will be considered as "no data".
     * \since QGIS 3.0
     * \see noDataValue(), hasNoDataValue(), resetNoDataValue()
     */
    void setNoDataValue( double noDataValue );

    /**
     * Reset no data value: if there was a no data value previously set,
     * it will be discarded.
     * \since QGIS 3.0
     * \see noDataValue(), hasNoDataValue(), setNoDataValue()
     */
    void resetNoDataValue();

    /**
     * Return no data value. If the block does not have a no data value the
     *  returned value is undefined.
     * \returns No data value
     * \see hasNoDataValue(), setNoDataValue(), resetNoDataValue()
     */
    double noDataValue() const { return mNoDataValue; }

    /**
     * Get byte array representing a value.
     * \param dataType data type
     * \param value value
     * \returns byte array representing the value */
    static QByteArray valueBytes( Qgis::DataType dataType, double value );

    /**
     * \brief Read a single value if type of block is numeric. If type is color,
     *  returned value is undefined.
     *  \param row row index
     *  \param column column index
     *  \returns value */
    double value( int row, int column ) const;

    /**
     * \brief Read a single value if type of block is numeric. If type is color,
     *  returned value is undefined.
     *  \param index data matrix index (long type in Python)
     *  \returns value */
    double value( qgssize index ) const;

    /**
     * \brief Read a single color
     *  \param row row index
     *  \param column column index
     *  \returns color */
    QRgb color( int row, int column ) const;

    /**
     * \brief Read a single value
     *  \param index data matrix index (long type in Python)
     *  \returns color */
    QRgb color( qgssize index ) const;

    /**
     * \brief Check if value at position is no data
     *  \param row row index
     *  \param column column index
     *  \returns true if value is no data */
    bool isNoData( int row, int column );

    /**
     * \brief Check if value at position is no data
     *  \param index data matrix index (long type in Python)
     *  \returns true if value is no data */
    bool isNoData( qgssize index );

    /**
     * \brief Set value on position
     *  \param row row index
     *  \param column column index
     *  \param value the value to be set
     *  \returns true on success */
    bool setValue( int row, int column, double value );

    /**
     * \brief Set value on index (indexed line by line)
     *  \param index data matrix index (long type in Python)
     *  \param value the value to be set
     *  \returns true on success */
    bool setValue( qgssize index, double value );

    /**
     * \brief Set color on position
     *  \param row row index
     *  \param column column index
     *  \param color the color to be set, QRgb value
     *  \returns true on success */
    bool setColor( int row, int column, QRgb color );

    /**
     * \brief Set color on index (indexed line by line)
     *  \param index data matrix index (long type in Python)
     *  \param color the color to be set, QRgb value
     *  \returns true on success */
    bool setColor( qgssize index, QRgb color );

    /**
     * \brief Set no data on pixel
     *  \param row row index
     *  \param column column index
     *  \returns true on success */
    bool setIsNoData( int row, int column );

    /**
     * \brief Set no data on pixel
     *  \param index data matrix index (long type in Python)
     *  \returns true on success */
    bool setIsNoData( qgssize index );

    /**
     * \brief Set the whole block to no data
     *  \returns true on success */
    bool setIsNoData();

    /**
     * \brief Set the whole block to no data except specified rectangle
     *  \returns true on success */
    bool setIsNoDataExcept( QRect exceptRect );

    /**
     * \brief Remove no data flag on pixel. If the raster block does not have an explicit
     * no data value set then an internal map of no data pixels is maintained for the block.
     * In this case it is possible to reset a pixel to flag it as having valid data using this
     * method. This method has no effect for raster blocks with an explicit no data value set.
     *  \param row row index
     *  \param column column index
     *  \since QGIS 2.10 */
    void setIsData( int row, int column );

    /**
     * \brief Remove no data flag on pixel. If the raster block does not have an explicit
     * no data value set then an internal map of no data pixels is maintained for the block.
     * In this case it is possible to reset a pixel to flag it as having valid data using this
     * method. This method has no effect for raster blocks with an explicit no data value set.
     *  \param index data matrix index (long type in Python)
     *  \since QGIS 2.10 */
    void setIsData( qgssize index );

    /**
     * Get access to raw data.
     * The returned QByteArray instance is not a copy of the data: it only refers to the array
     * owned by the QgsRasterBlock, therefore it is only valid while the QgsRasterBlock object
     * still exists. Writing to the returned QByteArray will not affect the original data:
     * a deep copy of the data will be made and only the local copy will be modified.
     * \note in Python the method returns ordinary bytes object as the
     * \since QGIS 3.0
     */
    QByteArray data() const;

    /**
     * Rewrite raw pixel data.
     * If the data array is shorter than the internal array within the raster block object,
     * pixels at the end will stay untouched. If the data array is longer than the internal
     * array, only the initial data from the input array will be used.
     * Optionally it is possible to set non-zero offset (in bytes) if the input data should
     * overwrite data somewhere in the middle of the internal buffer.
     * \since QGIS 3.0
     */
    void setData( const QByteArray &data, int offset = 0 );

    /**
     * \brief Get pointer to data
     *  \param row row index
     *  \param column column index
     *  \returns pointer to data
     *  \note not available in Python bindings
     */
    char *bits( int row, int column ) SIP_SKIP;

    /**
     * \brief Get pointer to data
     *  \param index data matrix index (long type in Python)
     *  \returns pointer to data
     *  \note not available in Python bindings
     */
    char *bits( qgssize index ) SIP_SKIP;

    /**
     * \brief Get pointer to data
     *  \returns pointer to data
     *  \note not available in Python bindings
     */
    char *bits() SIP_SKIP;

    /**
     * \brief Print double value with all necessary significant digits.
     *         It is ensured that conversion back to double gives the same number.
     *  \param value the value to be printed
     *  \returns string representing the value*/
    static QString printValue( double value );

    /**
     * \brief Print float value with all necessary significant digits.
     *         It is ensured that conversion back to float gives the same number.
     *  \param value the value to be printed
     *  \returns string representing the value
     * \since QGIS 2.16
     * \note not available in Python bindings
     */
    static QString printValue( float value ) SIP_SKIP;

    /**
     * \brief Convert data to different type.
     *  \param destDataType dest data type
     *  \returns true on success */
    bool convert( Qgis::DataType destDataType );

    /**
     * \brief Get image if type is color.
    *   \returns image */
    QImage image() const;

    /**
     * \brief set image.
     *  \param image image
     *  \returns true on success */
    bool setImage( const QImage *image );

    //! \note not available in Python bindings
    inline static double readValue( void *data, Qgis::DataType type, qgssize index ) SIP_SKIP;

    //! \note not available in Python bindings
    inline static void writeValue( void *data, Qgis::DataType type, qgssize index, double value ) SIP_SKIP;

    void applyNoDataValues( const QgsRasterRangeList &rangeList );

    /**
     * Apply band scale and offset to raster block values
     * \since QGIS 2.3
    */
    void applyScaleOffset( double scale, double offset );

    //! \brief Get error
    QgsError error() const { return mError; }

    //! \brief Set error
    void setError( const QgsError &error ) { mError = error;}

    QString toString() const;

    /**
     * \brief For extent and width, height find rectangle covered by subextent.
     * The output rect has x oriented from left to right and y from top to bottom
     * (upper-left to lower-right orientation).
     * \param extent extent, usually the larger
     * \param width numbers of columns in theExtent
     * \param height numbers of rows in theExtent
     * \param subExtent extent, usually smaller than theExtent
     * \returns the rectangle covered by sub extent
     */
    static QRect subRect( const QgsRectangle &extent, int width, int height, const QgsRectangle &subExtent );

    /**
     * Returns the width (number of columns) of the raster block.
     * \see height
     * \since QGIS 2.10
     */
    int width() const { return mWidth; }

    /**
     * Returns the height (number of rows) of the raster block.
     * \see width
     * \since QGIS 2.10
     */
    int height() const { return mHeight; }

  private:
    static QImage::Format imageFormat( Qgis::DataType dataType );
    static Qgis::DataType dataType( QImage::Format format );

    /**
     * Test if value is nodata comparing to noDataValue
     * \param value tested value
     * \param noDataValue no data value
     * \returns true if value is nodata */
    static bool isNoDataValue( double value, double noDataValue );

    /**
     * Test if value is nodata for specific band
     * \param value tested value
     * \returns true if value is nodata */
    bool isNoDataValue( double value ) const;

    /**
     * Allocate no data bitmap
     *  \returns true on success */
    bool createNoDataBitmap();

    /**
     * \brief Convert block of data from one type to another. Original block memory
     *         is not release.
     *  \param srcData source data
     *  \param srcDataType source data type
     *  \param destDataType dest data type
     *  \param size block size (width * height)
     *  \returns block of data in destDataType */
    static void *convert( void *srcData, Qgis::DataType srcDataType, Qgis::DataType destDataType, qgssize size );

    // Valid
    bool mValid = true;

    // Data type
    Qgis::DataType mDataType = Qgis::UnknownDataType;

    // Data type size in bytes, to make bits() fast
    int mTypeSize = 0;

    // Width
    int mWidth = 0;

    // Height
    int mHeight = 0;

    // Has no data value
    bool mHasNoDataValue = false;

    // No data value
    double mNoDataValue;

    static const QRgb NO_DATA_COLOR;

    // Data block for numerical data types, not used with image data types
    // QByteArray does not seem to be intended for large data blocks, does it?
    void *mData = nullptr;

    // Image for image data types, not used with numerical data types
    QImage *mImage = nullptr;

    // Bitmap of no data. One bit for each pixel. Bit is 1 if a pixels is no data.
    // Each row is represented by whole number of bytes (last bits may be unused)
    // to make processing rows easy.
    char *mNoDataBitmap = nullptr;

    // number of bytes in mNoDataBitmap row
    int mNoDataBitmapWidth = 0;

    // total size in bytes of mNoDataBitmap
    qgssize mNoDataBitmapSize = 0;

    // Error
    QgsError mError;
};

inline double QgsRasterBlock::readValue( void *data, Qgis::DataType type, qgssize index ) SIP_SKIP
{
  if ( !data )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  switch ( type )
  {
    case Qgis::Byte:
      return static_cast< double >( ( static_cast< quint8 * >( data ) )[index] );
      break;
    case Qgis::UInt16:
      return static_cast< double >( ( static_cast< quint16 * >( data ) )[index] );
      break;
    case Qgis::Int16:
      return static_cast< double >( ( static_cast< qint16 * >( data ) )[index] );
      break;
    case Qgis::UInt32:
      return static_cast< double >( ( static_cast< quint32 * >( data ) )[index] );
      break;
    case Qgis::Int32:
      return static_cast< double >( ( static_cast< qint32 * >( data ) )[index] );
      break;
    case Qgis::Float32:
      return static_cast< double >( ( static_cast< float * >( data ) )[index] );
      break;
    case Qgis::Float64:
      return static_cast< double >( ( static_cast< double * >( data ) )[index] );
      break;
    default:
      QgsDebugMsg( QString( "Data type %1 is not supported" ).arg( type ) );
      break;
  }

  return std::numeric_limits<double>::quiet_NaN();
}

inline void QgsRasterBlock::writeValue( void *data, Qgis::DataType type, qgssize index, double value ) SIP_SKIP
{
  if ( !data ) return;

  switch ( type )
  {
    case Qgis::Byte:
      ( static_cast< quint8 * >( data ) )[index] = static_cast< quint8 >( value );
      break;
    case Qgis::UInt16:
      ( static_cast< quint16 * >( data ) )[index] = static_cast< quint16 >( value );
      break;
    case Qgis::Int16:
      ( static_cast< qint16 * >( data ) )[index] = static_cast< qint16 >( value );
      break;
    case Qgis::UInt32:
      ( static_cast< quint32 * >( data ) )[index] = static_cast< quint32 >( value );
      break;
    case Qgis::Int32:
      ( static_cast< qint32 * >( data ) )[index] = static_cast< qint32 >( value );
      break;
    case Qgis::Float32:
      ( static_cast< float * >( data ) )[index] = static_cast< float >( value );
      break;
    case Qgis::Float64:
      ( static_cast< double * >( data ) )[index] = value;
      break;
    default:
      QgsDebugMsg( QString( "Data type %1 is not supported" ).arg( type ) );
      break;
  }
}

inline double QgsRasterBlock::value( qgssize index ) const SIP_SKIP
{
  if ( !mData )
  {
    QgsDebugMsg( "Data block not allocated" );
    return std::numeric_limits<double>::quiet_NaN();
  }
  return readValue( mData, mDataType, index );
}

inline bool QgsRasterBlock::isNoDataValue( double value ) const SIP_SKIP
{
  return std::isnan( value ) || qgsDoubleNear( value, mNoDataValue );
}

#endif


