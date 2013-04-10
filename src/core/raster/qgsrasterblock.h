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
     *  @param theNoDataValue the value representing no data (NULL)
     */
    QgsRasterBlock( QGis::DataType theDataType, int theWidth, int theHeight, double theNoDataValue = std::numeric_limits<double>::quiet_NaN() );

    virtual ~QgsRasterBlock();

    /** \brief Reset block
     *  @param theDataType raster data type
     *  @param theWidth width of data matrix
     *  @param theHeight height of data matrix
     *  @param theNoDataValue the value representing no data (NULL)
     *  @return true on success
     */
    bool reset( QGis::DataType theDataType, int theWidth, int theHeight, double theNoDataValue = std::numeric_limits<double>::quiet_NaN() );

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
    int dataTypeSize( int bandNo ) const
    {
      Q_UNUSED( bandNo );
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
    static QByteArray valueBytes( QGis::DataType theDataType, double theValue );

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
    bool convert( QGis::DataType destDataType );

    QImage image() const;
    bool setImage( const QImage * image );

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
    static void * convert( void *srcData, QGis::DataType srcDataType, QGis::DataType destDataType, size_t size );

    inline static double readValue( void *data, QGis::DataType type, size_t index );

    inline static void writeValue( void *data, QGis::DataType type, size_t index, double value );

    void applyNodataValues( const QgsRasterRangeList & rangeList );

    /** \brief Get error */
    QgsError error() const { return mError; }

    /** \brief Set error */
    void setError( const QgsError & theError ) { mError = theError;}

  private:

    static QImage::Format imageFormat( QGis::DataType theDataType );
    static QGis::DataType dataType( QImage::Format theFormat );

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

    // No data value
    double mNoDataValue;

    // Data block for numerical data types, not used with image data types
    // QByteArray does not seem to be intended for large data blocks, does it?
    void * mData;

    // Image for image data types, not used with numerical data types
    QImage *mImage;

    // Error
    QgsError mError;
};

inline double QgsRasterBlock::readValue( void *data, QGis::DataType type, size_t index )
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
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }

  // TODO: noDataValue is per band
  //return mInput->noDataValue();
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
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }
}

inline double QgsRasterBlock::value( size_t index ) const
{
  /*if ( index >= ( size_t )mWidth*mHeight )
  {
    QgsDebugMsg( QString( "Index %1 out of range (%2 x %3)" ).arg( index ).arg( mWidth ).arg( mHeight ) );
    return mNoDataValue;
  }*/
  return readValue( mData, mDataType, index );
}

inline bool QgsRasterBlock::isNoDataValue( double value ) const
{
  // More precise would be qIsNaN(value) && qIsNaN(noDataValue(bandNo)), but probably
  // not important and slower
  if ( qIsNaN( value ) ||
       doubleNear( value, mNoDataValue ) )
  {
    return true;
  }
  return false;
}

#endif


