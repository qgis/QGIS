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
 * \brief Raster data container.
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
     *  \returns TRUE on success
     */
    bool reset( Qgis::DataType dataType, int width, int height );

    // TODO: consider if use isValid() at all, isEmpty() should be sufficient
    // and works also if block is valid but empty - difference between valid and empty?

    /**
     * \brief Returns TRUE if the block is valid (correctly filled with data).
     *  An empty block may still be valid (if zero size block was requested).
     *  If the block is not valid, error may be retrieved by error() method.
     */
    bool isValid() const SIP_HOLDGIL { return mValid; }

    //! \brief Mark block as valid or invalid
    void setValid( bool valid ) SIP_HOLDGIL { mValid = valid; }

    /**
     * Returns TRUE if block is empty, i.e. its size is 0 (zero rows or cols).
     *  This method does not return TRUE if size is not zero and all values are
     *  'no data' (null).
     */
    bool isEmpty() const;

    /**
     * Returns the size in bytes for the specified \a dataType.
     */
    static int typeSize( Qgis::DataType dataType ) SIP_HOLDGIL
    {
      // Modified and extended copy from GDAL
      switch ( dataType )
      {
        case Qgis::DataType::Byte:
          return 1;

        case Qgis::DataType::UInt16:
        case Qgis::DataType::Int16:
          return 2;

        case Qgis::DataType::UInt32:
        case Qgis::DataType::Int32:
        case Qgis::DataType::Float32:
        case Qgis::DataType::CInt16:
          return 4;

        case Qgis::DataType::Float64:
        case Qgis::DataType::CInt32:
        case Qgis::DataType::CFloat32:
          return 8;

        case Qgis::DataType::CFloat64:
          return 16;

        case Qgis::DataType::ARGB32:
        case Qgis::DataType::ARGB32_Premultiplied:
          return 4;

        default:
          return 0;
      }
    }

    /**
     * Data type size in bytes.
     */
    int dataTypeSize() const SIP_HOLDGIL
    {
      return typeSize( mDataType );
    }

    //! Returns TRUE if data type is numeric
    static bool typeIsNumeric( Qgis::DataType type );

    //! Returns TRUE if data type is color
    static bool typeIsColor( Qgis::DataType type );

    //! Returns data type
    Qgis::DataType dataType() const  SIP_HOLDGIL { return mDataType; }

    //! For given data type returns wider type and sets no data value
    static Qgis::DataType typeWithNoDataValue( Qgis::DataType dataType, double *noDataValue );

    /**
     * TRUE if the block has no data value.
     * \returns TRUE if the block has no data value
     * \see noDataValue(), setNoDataValue(), resetNoDataValue()
     */
    bool hasNoDataValue() const SIP_HOLDGIL { return mHasNoDataValue; }

    /**
     * Returns TRUE if the block may contain no data. It does not guarantee
     * that it really contains any no data. It can be used to speed up processing.
     * Not the difference between this method and hasNoDataValue().
     * \returns TRUE if the block may contain no data
    */
    bool hasNoData() const SIP_HOLDGIL
    {
      return mHasNoDataValue || mNoDataBitmap;
    }

    /**
     * Sets cell value that will be considered as "no data".
     * \see noDataValue(), hasNoDataValue(), resetNoDataValue()
     * \since QGIS 3.0
     */
    void setNoDataValue( double noDataValue ) SIP_HOLDGIL;

    /**
     * Reset no data value: if there was a no data value previously set,
     * it will be discarded.
     * \see noDataValue(), hasNoDataValue(), setNoDataValue()
     * \since QGIS 3.0
     */
    void resetNoDataValue() SIP_HOLDGIL;

    /**
     * Returns no data value. If the block does not have a no data value the
     *  returned value is undefined.
     * \returns No data value
     * \see hasNoDataValue(), setNoDataValue(), resetNoDataValue()
     */
    double noDataValue() const SIP_HOLDGIL { return mNoDataValue; }

    /**
     * Gets byte array representing a value.
     * \param dataType data type
     * \param value value
     * \returns byte array representing the value
    */
    static QByteArray valueBytes( Qgis::DataType dataType, double value );

    /**
     * Read a single value if type of block is numeric. If type is color,
     * returned value is undefined.
     * \param row row index
     * \param column column index
     * \returns value
     * \see valueAndNoData()
    */
    double value( int row, int column ) const SIP_HOLDGIL
    {
      return value( static_cast< qgssize >( row ) * mWidth + column );
    }

    /**
     * Reads a single value from the pixel at \a row and \a column, if type of block is numeric. If type is color,
     * returned value is undefined.
     *
     * Additionally, the \a isNoData argument will be set to TRUE if the pixel represents a nodata value. This method
     * is more efficient then calling isNoData() and value() separately.
     *
     * \note Not available in Python bindings
     * \see value()
     * \see isNoData()
     * \since QGIS 3.6
     */
    double valueAndNoData( int row, int column, bool &isNoData ) const SIP_SKIP
    {
      return valueAndNoData( static_cast< qgssize >( row ) * mWidth + column, isNoData );
    }

    /**
     * Reads a single value if type of block is numeric. If type is color,
     * returned value is undefined.
     * \param index data matrix index (long type in Python)
     * \returns value
     * \see valueAndNoData()
    */
    inline double value( qgssize index ) const SIP_HOLDGIL;

    /**
     * Reads a single value from the pixel at the specified data matrix \a index, if type of block is numeric. If type is color,
     * returned value is undefined.
     *
     * Additionally, the \a isNoData argument will be set to TRUE if the pixel represents a nodata value. This method
     * is more efficient then calling isNoData() and value() separately.
     *
     * \note Not available in Python bindings
     * \see value()
     * \see isNoData()
     * \since QGIS 3.6
     */
    inline double valueAndNoData( qgssize index, bool &isNoData ) const SIP_SKIP;

    /**
     * Gives direct access to the raster block data.
     * The data type of the block must be Qgis::DataType::Byte otherwise it returns NULLPTR.
     * Useful for most efficient read access.
     * \note not available in Python bindings
     * \since QGIS 3.4
     */
    const quint8 *byteData() const SIP_SKIP
    {
      if ( mDataType != Qgis::DataType::Byte )
        return nullptr;
      return static_cast< const quint8 * >( mData );
    }

    /**
     * \brief Read a single color
     *  \param row row index
     *  \param column column index
     *  \returns color
    */
    QRgb color( int row, int column ) const SIP_HOLDGIL
    {
      if ( !mImage ) return NO_DATA_COLOR;

      return mImage->pixel( column, row );
    }

    /**
     * \brief Read a single value
     *  \param index data matrix index (long type in Python)
     *  \returns color
    */
    QRgb color( qgssize index ) const SIP_HOLDGIL
    {
      const int row = static_cast< int >( std::floor( static_cast< double >( index ) / mWidth ) );
      const int column = index % mWidth;
      return color( row, column );
    }

    /**
     * Checks if value at position is no data
     * \param row row index
     * \param column column index
     * \returns TRUE if value is no data
     * \see valueAndNoData()
    */
    bool isNoData( int row, int column ) const SIP_HOLDGIL
    {
      return isNoData( static_cast< qgssize >( row ) * mWidth + column );
    }

    /**
     * Check if value at position is no data
     * \param row row index
     * \param column column index
     * \returns TRUE if value is no data
     * \see valueAndNoData()
    */
    bool isNoData( qgssize row, qgssize column ) const SIP_HOLDGIL
    {
      return isNoData( row * static_cast< qgssize >( mWidth ) + column );
    }

    /**
     * Check if value at position is no data
     * \param index data matrix index (long type in Python)
     * \returns TRUE if value is no data
     * \see valueAndNoData()
    */
    bool isNoData( qgssize index ) const SIP_HOLDGIL
    {
      if ( !mHasNoDataValue && !mNoDataBitmap )
        return false;
      if ( index >= static_cast< qgssize >( mWidth )*mHeight )
      {
        QgsDebugMsg( QStringLiteral( "Index %1 out of range (%2 x %3)" ).arg( index ).arg( mWidth ).arg( mHeight ) );
        return true; // we consider no data if outside
      }
      if ( mHasNoDataValue )
      {
        const double value = readValue( mData, mDataType, index );
        return isNoDataValue( value );
      }
      // use no data bitmap
      if ( !mNoDataBitmap )
      {
        // no data are not defined
        return false;
      }
      // TODO: optimize
      const int row = static_cast< int >( index ) / mWidth;
      const int column = index % mWidth;
      const qgssize byte = static_cast< qgssize >( row ) * mNoDataBitmapWidth + column / 8;
      const int bit = column % 8;
      const int mask = 0x80 >> bit;
      //int x = mNoDataBitmap[byte] & mask;
      //QgsDebugMsg ( QString("byte = %1 bit = %2 mask = %3 nodata = %4 is nodata = %5").arg(byte).arg(bit).arg(mask, 0, 2 ).arg( x, 0, 2 ).arg( (bool)(x) ) );
      return mNoDataBitmap[byte] & mask;
    }

    /**
     * \brief Set value on position
     *  \param row row index
     *  \param column column index
     *  \param value the value to be set
     *  \returns TRUE on success
    */
    bool setValue( int row, int column, double value ) SIP_HOLDGIL
    {
      return setValue( static_cast< qgssize >( row ) * mWidth + column, value );
    }

    /**
     * \brief Set value on index (indexed line by line)
     *  \param index data matrix index (long type in Python)
     *  \param value the value to be set
     *  \returns TRUE on success
    */
    bool setValue( qgssize index, double value ) SIP_HOLDGIL
    {
      if ( !mData )
      {
        QgsDebugMsg( QStringLiteral( "Data block not allocated" ) );
        return false;
      }
      if ( index >= static_cast< qgssize >( mWidth ) *mHeight )
      {
        QgsDebugMsg( QStringLiteral( "Index %1 out of range (%2 x %3)" ).arg( index ).arg( mWidth ).arg( mHeight ) );
        return false;
      }
      writeValue( mData, mDataType, index, value );
      return true;
    }

    /**
     * \brief Set color on position
     *  \param row row index
     *  \param column column index
     *  \param color the color to be set, QRgb value
     *  \returns TRUE on success
    */
    bool setColor( int row, int column, QRgb color ) SIP_HOLDGIL
    {
      return setColor( static_cast< qgssize >( row ) * mWidth + column, color );
    }

    /**
     * \brief Set color on index (indexed line by line)
     *  \param index data matrix index (long type in Python)
     *  \param color the color to be set, QRgb value
     *  \returns TRUE on success
    */
    bool setColor( qgssize index, QRgb color ) SIP_HOLDGIL
    {
      if ( !mImage )
      {
        QgsDebugMsg( QStringLiteral( "Image not allocated" ) );
        return false;
      }

      if ( index >= static_cast< qgssize >( mImage->width() ) * mImage->height() )
      {
        QgsDebugMsg( QStringLiteral( "index %1 out of range" ).arg( index ) );
        return false;
      }

      // setPixel() is slow, see Qt doc -> use direct access
      QRgb *bits = reinterpret_cast< QRgb * >( mImage->bits() );
      bits[index] = color;
      return true;
    }

    /**
     * Gives direct read/write access to the raster RGB data.
     * The data type of the block must be Qgis::DataType::ARGB32 or Qgis::DataType::ARGB32_Premultiplied otherwise it returns NULLPTR.
     * Useful for most efficient read/write access to RGB blocks.
     * \note not available in Python bindings
     * \since QGIS 3.4
     */
    QRgb *colorData() SIP_SKIP
    {
      if ( !mImage )
        return nullptr;
      return reinterpret_cast< QRgb * >( mImage->bits() );
    }

    /**
     * \brief Set no data on pixel
     *  \param row row index
     *  \param column column index
     *  \returns TRUE on success
    */
    bool setIsNoData( int row, int column ) SIP_HOLDGIL
    {
      return setIsNoData( static_cast< qgssize >( row ) * mWidth + column );
    }

    /**
     * \brief Set no data on pixel
     *  \param index data matrix index (long type in Python)
     *  \returns TRUE on success
    */
    bool setIsNoData( qgssize index ) SIP_HOLDGIL
    {
      if ( mHasNoDataValue )
      {
        return setValue( index, mNoDataValue );
      }
      else
      {
        if ( !mNoDataBitmap )
        {
          if ( !createNoDataBitmap() )
          {
            return false;
          }
        }
        // TODO: optimize
        const int row = static_cast< int >( index ) / mWidth;
        const int column = index % mWidth;
        const qgssize byte = static_cast< qgssize >( row ) * mNoDataBitmapWidth + column / 8;
        const int bit = column % 8;
        const int nodata = 0x80 >> bit;
        //QgsDebugMsg ( QString("set byte = %1 bit = %2 no data by %3").arg(byte).arg(bit).arg(nodata, 0,2 ) );
        mNoDataBitmap[byte] = mNoDataBitmap[byte] | nodata;
        return true;
      }
    }

    /**
     * \brief Set the whole block to no data
     *  \returns TRUE on success
    */
    bool setIsNoData();

    /**
     * \brief Set the whole block to no data except specified rectangle
     *  \returns TRUE on success
    */
    bool setIsNoDataExcept( QRect exceptRect );

    /**
     * \brief Remove no data flag on pixel. If the raster block does not have an explicit
     * no data value set then an internal map of no data pixels is maintained for the block.
     * In this case it is possible to reset a pixel to flag it as having valid data using this
     * method. This method has no effect for raster blocks with an explicit no data value set.
     *  \param row row index
     *  \param column column index
     *  \since QGIS 2.10
    */
    void setIsData( int row, int column ) SIP_HOLDGIL
    {
      setIsData( static_cast< qgssize >( row )*mWidth + column );
    }

    /**
     * \brief Remove no data flag on pixel. If the raster block does not have an explicit
     * no data value set then an internal map of no data pixels is maintained for the block.
     * In this case it is possible to reset a pixel to flag it as having valid data using this
     * method. This method has no effect for raster blocks with an explicit no data value set.
     *  \param index data matrix index (long type in Python)
     *  \since QGIS 2.10
    */
    void setIsData( qgssize index ) SIP_HOLDGIL
    {
      if ( mHasNoDataValue )
      {
        //no data value set, so mNoDataBitmap is not being used
        return;
      }

      if ( !mNoDataBitmap )
      {
        return;
      }

      // TODO: optimize
      const int row = static_cast< int >( index ) / mWidth;
      const int column = index % mWidth;
      const qgssize byte = static_cast< qgssize >( row ) * mNoDataBitmapWidth + column / 8;
      const int bit = column % 8;
      const int nodata = 0x80 >> bit;
      mNoDataBitmap[byte] = mNoDataBitmap[byte] & ~nodata;
    }

    /**
     * Gets access to raw data.
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
     * Returns a pointer to block data.
     * \param row row index
     * \param column column index
     * \note not available in Python bindings
     */
    char *bits( int row, int column ) SIP_SKIP;

    /**
     * Returns a pointer to block data.
     * \param index data matrix index (long type in Python)
     * \note not available in Python bindings
     */
    char *bits( qgssize index ) SIP_SKIP;

    /**
     * Returns a pointer to block data.
     * \note not available in Python bindings
     */
    char *bits() SIP_SKIP;

    /**
     * \brief Print double value with all necessary significant digits.
     *         It is ensured that conversion back to double gives the same number.
     *  \param value the value to be printed
     *  \returns string representing the value
     */
    static QString printValue( double value );

    /**
     * \brief Print float value with all necessary significant digits.
     *         It is ensured that conversion back to float gives the same number.
     *  \param value the value to be printed
     *  \returns string representing the value
     * \note not available in Python bindings
     * \since QGIS 2.16
     */
    static QString printValue( float value ) SIP_SKIP;

    /**
     * \brief Convert data to different type.
     *  \param destDataType dest data type
     *  \returns TRUE on success
    */
    bool convert( Qgis::DataType destDataType );

    /**
     * Returns an image containing the block data, if the block's data type is color.
     */
    QImage image() const;

    /**
     * Sets the block data via an \a image.
     * \returns TRUE on success
    */
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

    //! Returns the last error
    QgsError error() const { return mError; }

    //! Sets the last error
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
    int width() const SIP_HOLDGIL { return mWidth; }

    /**
     * Returns the height (number of rows) of the raster block.
     * \see width
     * \since QGIS 2.10
     */
    int height() const SIP_HOLDGIL { return mHeight; }

  private:
    static QImage::Format imageFormat( Qgis::DataType dataType );
    static Qgis::DataType dataType( QImage::Format format );

    /**
     * Test if value is nodata comparing to noDataValue
     * \param value tested value
     * \param noDataValue no data value
     * \returns TRUE if value is nodata
    */
    static bool isNoDataValue( double value, double noDataValue )
    {
      // TODO: optimize no data value test by memcmp()
      // More precise would be std::isnan(value) && std::isnan(noDataValue(bandNo)), but probably
      // not important and slower
      return std::isnan( value ) ||
             qgsDoubleNear( value, noDataValue );
    }

    /**
     * Test if value is nodata for specific band
     * \param value tested value
     * \returns TRUE if value is nodata
    */
    inline bool isNoDataValue( double value ) const;

    /**
     * Allocate no data bitmap
     *  \returns TRUE on success
    */
    bool createNoDataBitmap();

    /**
     * \brief Convert block of data from one type to another. Original block memory
     *         is not release.
     *  \param srcData source data
     *  \param srcDataType source data type
     *  \param destDataType dest data type
     *  \param size block size (width * height)
     *  \returns block of data in destDataType
    */
    static void *convert( void *srcData, Qgis::DataType srcDataType, Qgis::DataType destDataType, qgssize size );

    // Valid
    bool mValid = true;

    // Data type
    Qgis::DataType mDataType = Qgis::DataType::UnknownDataType;

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
    case Qgis::DataType::Byte:
      return static_cast< double >( ( static_cast< quint8 * >( data ) )[index] );
    case Qgis::DataType::UInt16:
      return static_cast< double >( ( static_cast< quint16 * >( data ) )[index] );
    case Qgis::DataType::Int16:
      return static_cast< double >( ( static_cast< qint16 * >( data ) )[index] );
    case Qgis::DataType::UInt32:
      return static_cast< double >( ( static_cast< quint32 * >( data ) )[index] );
    case Qgis::DataType::Int32:
      return static_cast< double >( ( static_cast< qint32 * >( data ) )[index] );
    case Qgis::DataType::Float32:
      return static_cast< double >( ( static_cast< float * >( data ) )[index] );
    case Qgis::DataType::Float64:
      return static_cast< double >( ( static_cast< double * >( data ) )[index] );
    default:
      QgsDebugMsg( QStringLiteral( "Data type %1 is not supported" ).arg( qgsEnumValueToKey< Qgis::DataType >( type ) ) );
      break;
  }

  return std::numeric_limits<double>::quiet_NaN();
}

inline void QgsRasterBlock::writeValue( void *data, Qgis::DataType type, qgssize index, double value ) SIP_SKIP
{
  if ( !data ) return;

  switch ( type )
  {
    case Qgis::DataType::Byte:
      ( static_cast< quint8 * >( data ) )[index] = static_cast< quint8 >( value );
      break;
    case Qgis::DataType::UInt16:
      ( static_cast< quint16 * >( data ) )[index] = static_cast< quint16 >( value );
      break;
    case Qgis::DataType::Int16:
      ( static_cast< qint16 * >( data ) )[index] = static_cast< qint16 >( value );
      break;
    case Qgis::DataType::UInt32:
      ( static_cast< quint32 * >( data ) )[index] = static_cast< quint32 >( value );
      break;
    case Qgis::DataType::Int32:
      ( static_cast< qint32 * >( data ) )[index] = static_cast< qint32 >( value );
      break;
    case Qgis::DataType::Float32:
      ( static_cast< float * >( data ) )[index] = static_cast< float >( value );
      break;
    case Qgis::DataType::Float64:
      ( static_cast< double * >( data ) )[index] = value;
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Data type %1 is not supported" ).arg( qgsEnumValueToKey< Qgis::DataType >( type ) ) );
      break;
  }
}

inline double QgsRasterBlock::value( qgssize index ) const SIP_SKIP
{
  if ( !mData )
  {
    QgsDebugMsg( QStringLiteral( "Data block not allocated" ) );
    return std::numeric_limits<double>::quiet_NaN();
  }
  return readValue( mData, mDataType, index );
}

inline double QgsRasterBlock::valueAndNoData( qgssize index, bool &isNoData ) const SIP_SKIP
{
  if ( !mData )
  {
    QgsDebugMsg( QStringLiteral( "Data block not allocated" ) );
    isNoData = true;
    return std::numeric_limits<double>::quiet_NaN();
  }
  if ( index >= static_cast< qgssize >( mWidth )*mHeight )
  {
    QgsDebugMsg( QStringLiteral( "Index %1 out of range (%2 x %3)" ).arg( index ).arg( mWidth ).arg( mHeight ) );
    isNoData = true; // we consider no data if outside
    return std::numeric_limits<double>::quiet_NaN();
  }

  const double val = readValue( mData, mDataType, index );

  if ( !mHasNoDataValue && !mNoDataBitmap )
  {
    isNoData = false;
    return val;
  }

  if ( mHasNoDataValue )
  {
    isNoData = isNoDataValue( val );
    return val;
  }
  // use no data bitmap
  if ( !mNoDataBitmap )
  {
    // no data are not defined
    isNoData = false;
    return val;
  }

  // no data is a bitmap
  isNoData = QgsRasterBlock::isNoData( index );
  return val;
}

inline bool QgsRasterBlock::isNoDataValue( double value ) const SIP_SKIP
{
  return std::isnan( value ) || qgsDoubleNear( value, mNoDataValue );
}

#endif


