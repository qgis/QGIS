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

#include <limits>
#include <QImage>
#include "qgis.h"
#include "qgserror.h"
#include "qgslogger.h"
#include "qgsrasterrange.h"
#include "qgsrectangle.h"

/** \ingroup core
 * Raster data container.
 */
class CORE_EXPORT QgsRasterBlock
{
  public:
    QgsRasterBlock();

    /** \brief Constructor which allocates data block in memory
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     */
    QgsRasterBlock( QGis::DataType theDataType, int theWidth, int theHeight );

    /** \brief Constructor which allocates data block in memory
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @param theNoDataValue the value representing no data (NULL)
     */
    QgsRasterBlock( QGis::DataType theDataType, int theWidth, int theHeight, double theNoDataValue );

    virtual ~QgsRasterBlock();

    /** \brief Reset block
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @return true on success
     */
    bool reset( QGis::DataType theDataType, int theWidth, int theHeight );

    /** \brief Reset block
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @param theNoDataValue the value representing no data (NULL)
     *  @return true on success
     */
    bool reset( QGis::DataType theDataType, int theWidth, int theHeight, double theNoDataValue );

    // TODO: consider if use isValid() at all, isEmpty() should be sufficient
    // and works also if block is valid but empty - difference between valid and empty?
    /** \brief Returns true if the block is valid (correctly filled with data).
     *  An empty block may still be valid (if zero size block was requested).
     *  If the block is not valid, error may be retrieved by error() method.
     */
    bool isValid() const { return mValid; }

    /** \brief Mark block as valid or invalid */
    void setValid( bool valid ) { mValid = valid; }

    /** Returns true if block is empty, i.e. its size is 0 (zero rows or cols).
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
        case QGis::Byte:
          return 1;

        case QGis::UInt16:
        case QGis::Int16:
          return 2;

        case QGis::UInt32:
        case QGis::Int32:
        case QGis::Float32:
        case QGis::CInt16:
          return 4;

        case QGis::Float64:
        case QGis::CInt32:
        case QGis::CFloat32:
          return 8;

        case QGis::CFloat64:
          return 16;

        case QGis::ARGB32:
        case QGis::ARGB32_Premultiplied:
          return 4;

        default:
          return 0;
      }
    }

    // Data type in bytes
    int dataTypeSize( ) const
    {
      return typeSize( mDataType );
    }

    /** Returns true if data type is numeric */
    static bool typeIsNumeric( QGis::DataType type );

    /** Returns true if data type is color */
    static bool typeIsColor( QGis::DataType type );

    /** Returns data type */
    QGis::DataType dataType() const { return mDataType; }

    /** For given data type returns wider type and sets no data value */
    static QGis::DataType typeWithNoDataValue( QGis::DataType dataType, double *noDataValue );

    /** True if the block has no data value.
     * @return true if the block has no data value */
    bool hasNoDataValue() const { return mHasNoDataValue; }

    /** Returns true if thee block may contain no data. It does not guarantee
     * that it really contains any no data. It can be used to speed up processing.
     * Not the difference between this method and hasNoDataValue().
     * @return true if the block may contain no data */
    bool hasNoData() const;

    /** Return no data value. If the block does not have a no data value the
     *  returned value is undefined.
     * @return No data value */
    double noDataValue() const { return mNoDataValue; }

    /** Get byte array representing a value.
     * @param theDataType data type
     * @param theValue value
     * @return byte array representing the value */
    static QByteArray valueBytes( QGis::DataType theDataType, double theValue );

    /** \brief Read a single value if type of block is numeric. If type is color,
     *  returned value is undefined.
     *  @param row row index
     *  @param column column index
     *  @return value */
    double value( int row, int column ) const;

    /** \brief Read a single value if type of block is numeric. If type is color,
     *  returned value is undefined.
     *  @param index data matrix index
     *  @return value */
    double value( size_t index ) const;

    /** \brief Read a single color
     *  @param row row index
     *  @param column column index
     *  @return color */
    QRgb color( int row, int column ) const;

    /** \brief Read a single value
     *  @param index data matrix index
     *  @return color */
    QRgb color( size_t index ) const;

    /** \brief Check if value at position is no data
     *  @param row row index
     *  @param column column index
     *  @return true if value is no data */
    bool isNoData( int row, int column );

    /** \brief Check if value at position is no data
     *  @param index data matrix index
     *  @return true if value is no data */
    bool isNoData( size_t index );

    /** \brief Set value on position
     *  @param row row index
     *  @param column column index
     *  @param value the value to be set
     *  @return true on success */
    bool setValue( int row, int column, double value );

    /** \brief Set value on index (indexed line by line)
     *  @param index data matrix index
     *  @param value the value to be set
     *  @return true on success */
    bool setValue( size_t index, double value );

    /** \brief Set color on position
     *  @param row row index
     *  @param column column index
     *  @param color the color to be set, QRgb value
     *  @return true on success */
    bool setColor( int row, int column, QRgb color );

    /** \brief Set color on index (indexed line by line)
     *  @param index data matrix index
     *  @param color the color to be set, QRgb value
     *  @return true on success */
    bool setColor( size_t index, QRgb color );

    /** \brief Set no data on pixel
     *  @param row row index
     *  @param column column index
     *  @return true on success */
    bool setIsNoData( int row, int column );

    /** \brief Set no data on pixel
     *  @param index data matrix index
     *  @return true on success */
    bool setIsNoData( size_t index );

    /** \brief Set the whole block to no data
     *  @return true on success */
    bool setIsNoData( );

    /** \brief Set the whole block to no data except specified rectangle
     *  @return true on success */
    bool setIsNoDataExcept( const QRect & theExceptRect );

    /** \brief Get pointer to data
     *  @param row row index
     *  @param column column index
     *  @return pointer to data
     */
    char * bits( int row, int column );

    /** \brief Get pointer to data
     *  @param index data matrix index
     *  @return pointer to data */
    char * bits( size_t index );

    /** \brief Get pointer to data
     *  @return pointer to data */
    char * bits();

    /** \brief Print double value with all necessary significant digits.
     *         It is ensured that conversion back to double gives the same number.
     *  @param value the value to be printed
     *  @return string representing the value*/
    static QString printValue( double value );

    /** \brief Convert data to different type.
     *  @param destDataType dest data type
     *  @return true on success */
    bool convert( QGis::DataType destDataType );

    /** \brief Get image if type is color.
    *   @return image */
    QImage image() const;

    /** \brief set image.
     *  @param image image
     *  @return true on success */
    bool setImage( const QImage * image );

    inline static double readValue( void *data, QGis::DataType type, size_t index );

    inline static void writeValue( void *data, QGis::DataType type, size_t index, double value );

    void applyNoDataValues( const QgsRasterRangeList & rangeList );

    /** \brief Get error */
    QgsError error() const { return mError; }

    /** \brief Set error */
    void setError( const QgsError & theError ) { mError = theError;}

    /** \brief For theExtent and theWidht, theHeight find rectangle covered by subextent.
     * The output rect has x oriented from left to right and y from top to bottom
     * (upper-left to lower-right orientation).
     * @param theExtent extent, usually the larger
     * @param theWidth numbers of columns in theExtent
     * @param theHeight numbers of rows in theExtent
     * @param theSubExtent extent, usually smaller than theExtent
     * @return the rectangle covered by sub extent
     */
    static QRect subRect( const QgsRectangle & theExtent, int theWidth, int theHeight, const QgsRectangle &  theSubExtent );

  private:
    static QImage::Format imageFormat( QGis::DataType theDataType );
    static QGis::DataType dataType( QImage::Format theFormat );

    /** Test if value is nodata comparing to noDataValue
     * @param value tested value
     * @param noDataValue no data value
     * @return true if value is nodata */
    static bool isNoDataValue( double value, double noDataValue );

    /** Test if value is nodata for specific band
     * @param value tested value
     * @return true if value is nodata */
    bool isNoDataValue( double value ) const;

    /** Allocate no data bitmap
     *  @return true on success */
    bool createNoDataBitmap();

    /** \brief Convert block of data from one type to another. Original block memory
     *         is not release.
     *  @param srcData source data
     *  @param srcDataType source data type
     *  @param destDataType dest data type
     *  @param size block size (width * height)
     *  @return block of data in destDataType */
    static void * convert( void *srcData, QGis::DataType srcDataType, QGis::DataType destDataType, size_t size );

    // Valid
    bool mValid;

    // Data type
    QGis::DataType mDataType;

    // Data type size in bytes, to make bits() fast
    int mTypeSize;

    // Width
    int mWidth;

    // Height
    int mHeight;

    // Has no data value
    bool mHasNoDataValue;

    // No data value
    double mNoDataValue;

    // Data block for numerical data types, not used with image data types
    // QByteArray does not seem to be intended for large data blocks, does it?
    void * mData;

    // Image for image data types, not used with numerical data types
    QImage *mImage;

    // Bitmap of no data. One bit for each pixel. Bit is 1 if a pixels is no data.
    // Each row is represented by whole number of bytes (last bits may be unused)
    // to make processing rows easy.
    char *mNoDataBitmap;

    // number of bytes in mNoDataBitmap row
    int mNoDataBitmapWidth;

    // total size in bytes of mNoDataBitmap
    size_t mNoDataBitmapSize;

    // Error
    QgsError mError;
};

inline double QgsRasterBlock::readValue( void *data, QGis::DataType type, size_t index )
{
  if ( !data )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  switch ( type )
  {
    case QGis::Byte:
      return ( double )(( quint8 * )data )[index];
      break;
    case QGis::UInt16:
      return ( double )(( quint16 * )data )[index];
      break;
    case QGis::Int16:
      return ( double )(( qint16 * )data )[index];
      break;
    case QGis::UInt32:
      return ( double )(( quint32 * )data )[index];
      break;
    case QGis::Int32:
      return ( double )(( qint32 * )data )[index];
      break;
    case QGis::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QGis::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      QgsDebugMsg( QString( "Data type %1 is not supported" ).arg( type ) );
      break;
  }

  return std::numeric_limits<double>::quiet_NaN();
}

inline void QgsRasterBlock::writeValue( void *data, QGis::DataType type, size_t index, double value )
{
  if ( !data ) return;

  switch ( type )
  {
    case QGis::Byte:
      (( quint8 * )data )[index] = ( quint8 ) value;
      break;
    case QGis::UInt16:
      (( quint16 * )data )[index] = ( quint16 ) value;
      break;
    case QGis::Int16:
      (( qint16 * )data )[index] = ( qint16 ) value;
      break;
    case QGis::UInt32:
      (( quint32 * )data )[index] = ( quint32 ) value;
      break;
    case QGis::Int32:
      (( qint32 * )data )[index] = ( qint32 ) value;
      break;
    case QGis::Float32:
      (( float * )data )[index] = ( float ) value;
      break;
    case QGis::Float64:
      (( double * )data )[index] = value;
      break;
    default:
      QgsDebugMsg( QString( "Data type %1 is not supported" ).arg( type ) );
      break;
  }
}

inline double QgsRasterBlock::value( size_t index ) const
{
  if ( !mData )
  {
    QgsDebugMsg( "Data block not allocated" );
    return std::numeric_limits<double>::quiet_NaN();
  }
  return readValue( mData, mDataType, index );
}

inline bool QgsRasterBlock::isNoDataValue( double value ) const
{
  return qIsNaN( value ) || qgsDoubleNear( value, mNoDataValue );
}

#endif


