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
#include "qgslogger.h"
#include "qgsrectangle.h"

#include "gdal.h" // only for data types for now

/** \ingroup core
 * Raster data container.
 */
// TODO: inherit from QObject?
class CORE_EXPORT QgsRasterBlock
{
  public:

    /** Data types.
     *  This is modified and extended copy of GDALDataType.
     */
    enum DataType
    {
      /*! Unknown or unspecified type */          UnknownDataType = 0,
      /*! Eight bit unsigned integer */           Byte = 1,
      /*! Sixteen bit unsigned integer */         UInt16 = 2,
      /*! Sixteen bit signed integer */           Int16 = 3,
      /*! Thirty two bit unsigned integer */      UInt32 = 4,
      /*! Thirty two bit signed integer */        Int32 = 5,
      /*! Thirty two bit floating point */        Float32 = 6,
      /*! Sixty four bit floating point */        Float64 = 7,
      /*! Complex Int16 */                        CInt16 = 8,
      /*! Complex Int32 */                        CInt32 = 9,
      /*! Complex Float32 */                      CFloat32 = 10,
      /*! Complex Float64 */                      CFloat64 = 11,
      /*! Color, alpha, red, green, blue, 4 bytes the same as
          QImage::Format_ARGB32 */                ARGB32 = 12,
      /*! Color, alpha, red, green, blue, 4 bytes  the same as
          QImage::Format_ARGB32_Premultiplied */  ARGB32_Premultiplied = 13
    };

    struct Range
    {
      double min;
      double max;
      inline bool operator==( const Range &o ) const
      {
        return min == o.min && max == o.max;
      }
    };

    QgsRasterBlock();

    /** \brief Constructor which allocates data block in memory
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @param theNoDataValue the value representing no data (NULL)
     */
    QgsRasterBlock( DataType theDataType, int theWidth, int theHeight, double theNoDataValue = std::numeric_limits<double>::quiet_NaN() );

    virtual ~QgsRasterBlock();

    /** \brief Reset block
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @param theNoDataValue the value representing no data (NULL)
     *  @return true on success
     */
    bool reset( DataType theDataType, int theWidth, int theHeight, double theNoDataValue = std::numeric_limits<double>::quiet_NaN() );

    // TODO: consider if use isValid() at all, isEmpty() should be sufficient
    // and works also if block is valid but empty - difference between valid and empty?
    //bool isValid() const { return mValid; }
    bool isEmpty() const;

    // Return data type size in bytes
    static int typeSize( int dataType )
    {
      // Modified and extended copy from GDAL
      switch ( dataType )
      {
        case Byte:
          return 1;

        case UInt16:
        case Int16:
          return 2;

        case UInt32:
        case Int32:
        case Float32:
        case CInt16:
          return 4;

        case Float64:
        case CInt32:
        case CFloat32:
          return 8;

        case CFloat64:
          return 16;

        case ARGB32:
        case ARGB32_Premultiplied:
          return 4;

        default:
          return 0;
      }
    }

    // Data type in bytes
    int dataTypeSize( int bandNo ) const
    {
      Q_UNUSED( bandNo );
      return typeSize( mDataType );
    }

    /** Returns true if data type is numeric */
    static bool typeIsNumeric( DataType type );

    /** Returns true if data type is color */
    static bool typeIsColor( DataType type );

    /** Returns data type */
    DataType dataType() const { return mDataType; }

    /** For given data type returns wider type and sets no data value */
    static DataType typeWithNoDataValue( DataType dataType, double *noDataValue );

    /** Return no data value.
     * @return No data value */
    double noDataValue() const { return mNoDataValue; }

    /** Set no data value.
     * @param noDataValue the value to be considered no data
     */
    void setNoDataValue( double noDataValue ) { mNoDataValue = noDataValue; }

    /** Test if value is nodata comparing to noDataValue
     * @param value tested value
     * @param noDataValue no data value
     * @return true if value is nodata */
    static bool isNoDataValue( double value, double noDataValue );

    /** Test if value is nodata for specific band
     * @param value tested value
     * @return true if value is nodata */
    bool isNoDataValue( double value ) const;

    // get byte array representing no data value
    static QByteArray valueBytes( DataType theDataType, double theValue );

    /** \brief Read a single value
     *  @param row row index
     *  @param column column index
     *  @return value */
    double value( int row, int column ) const;

    /** \brief Read a single value
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

    /** \brief Set color on index (indexed line by line)
     *  @param index data matrix index
     *  @param color the color to be set, QRgb value
     *  @return true on success */
    bool setColor( size_t index, QRgb color );

    /** \brief Get pointer to data
     *  @param row row index
     *  @param column column index
     *  @return pointer to data
     */
    char * bits( int row, int column );

    /** \brief Get pointer to data
     *  @param index data matrix index
     *  @return pointer to data
     */
    char * bits( size_t index );

    /** \brief Print double value with all necessary significant digits.
     *         It is ensured that conversion back to double gives the same number.
     *  @param value the value to be printed
     *  @return string representing the value*/
    static QString printValue( double value );

    /** \brief Convert data to different type.
     *  @param destDataType dest data type
     *  @return true on success
     */
    bool convert( QgsRasterBlock::DataType destDataType );

    QImage image() const;
    bool setImage( const QImage * image );

    /** \brief Test if value is within the list of ranges
     *  @param value value
     *  @param rangeList list of ranges
     *  @return true if value is in at least one of ranges
     *  @note not available in python bindings
     */
    inline static bool valueInRange( double value, const QList<QgsRasterBlock::Range> &rangeList );

    /** Create a new image with extraneous data, such data may be used
     *  after the image is destroyed. The memory is not initialized.
     */
    // TODO: remove, no more necessary with QgsRasterBlock
    QImage * createImage( int width, int height, QImage::Format format );

    // TODO: remove this direct access to data, it was used in transition period
    void * data() { if ( mData ) return mData; return mImage->bits(); }


    // TODO: move to private, currently used by file writer
    /** \brief Convert block of data from one type to another. Original block memory
     *         is not release.
     *  @param srcData source data
     *  @param srcDataType source data type
     *  @param destDataType dest data type
     *  @param size block size (width * height)
     *  @return block of data in destDataType */
    static void * convert( void *srcData, QgsRasterBlock::DataType srcDataType, QgsRasterBlock::DataType destDataType, size_t size );

    inline static double readValue( void *data, QgsRasterBlock::DataType type, size_t index );

    inline static void writeValue( void *data, QgsRasterBlock::DataType type, size_t index, double value );

  private:

    static QImage::Format imageFormat( QgsRasterBlock::DataType theDataType );
    static DataType dataType( QImage::Format theFormat );

    // Valid
    //bool isValid;

    // Data type
    DataType mDataType;

    // Data type size in bytes, to make bits() fast
    int mTypeSize;

    // Width
    int mWidth;

    // Height
    int mHeight;

    // No data value
    double mNoDataValue;

    // Data block for numerical data types, not used with image data types
    // QByteArray does not seem to be intended for large data blocks, does it?
    void * mData;

    // Image for image data types, not used with numerical data types
    QImage *mImage;
};

inline double QgsRasterBlock::readValue( void *data, QgsRasterBlock::DataType type, size_t index )
{
#if 0
  if ( !mInput )
  {
    return 0;
  }

  if ( !data )
  {
    return mInput->noDataValue();
  }
#endif

  // TODO: define QGIS types to avoid cpl_port.h
  switch ( type )
  {
    case QgsRasterBlock::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QgsRasterBlock::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QgsRasterBlock::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QgsRasterBlock::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QgsRasterBlock::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QgsRasterBlock::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QgsRasterBlock::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }

  // TODO: noDataValue is per band
  //return mInput->noDataValue();
  return std::numeric_limits<double>::quiet_NaN();
}

inline void QgsRasterBlock::writeValue( void *data, QgsRasterBlock::DataType type, size_t index, double value )
{
  if ( !data ) return;

  switch ( type )
  {
    case QgsRasterBlock::Byte:
      (( GByte * )data )[index] = ( GByte ) value;
      break;
    case QgsRasterBlock::UInt16:
      (( GUInt16 * )data )[index] = ( GUInt16 ) value;
      break;
    case QgsRasterBlock::Int16:
      (( GInt16 * )data )[index] = ( GInt16 ) value;
      break;
    case QgsRasterBlock::UInt32:
      (( GUInt32 * )data )[index] = ( GUInt32 ) value;
      break;
    case QgsRasterBlock::Int32:
      (( GInt32 * )data )[index] = ( GInt32 ) value;
      break;
    case QgsRasterBlock::Float32:
      (( float * )data )[index] = ( float ) value;
      break;
    case QgsRasterBlock::Float64:
      (( double * )data )[index] = value;
      break;
    default:
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }
}

inline bool QgsRasterBlock::valueInRange( double value, const QList<QgsRasterBlock::Range> &rangeList )
{
  foreach ( QgsRasterBlock::Range range, rangeList )
  {
    if (( value >= range.min && value <= range.max ) ||
        doubleNear( value, range.min ) ||
        doubleNear( value, range.max ) )
    {
      return true;
    }
  }
  return false;
}

#endif


