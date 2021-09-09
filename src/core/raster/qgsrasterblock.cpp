/***************************************************************************
    qgsrasterblock.cpp - Class representing a block of raster data
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

#include <limits>

#include <QByteArray>
#include <QColor>

#include "qgslogger.h"
#include "qgsrasterblock.h"
#include "qgsrectangle.h"

// See #9101 before any change of NODATA_COLOR!
const QRgb QgsRasterBlock::NO_DATA_COLOR = qRgba( 0, 0, 0, 0 );

QgsRasterBlock::QgsRasterBlock()
  : mNoDataValue( std::numeric_limits<double>::quiet_NaN() )
{
}

QgsRasterBlock::QgsRasterBlock( Qgis::DataType dataType, int width, int height )
  : mDataType( dataType )
  , mWidth( width )
  , mHeight( height )
  , mNoDataValue( std::numeric_limits<double>::quiet_NaN() )
{
  ( void )reset( mDataType, mWidth, mHeight );
}

QgsRasterBlock::~QgsRasterBlock()
{
  QgsDebugMsgLevel( QStringLiteral( "mData = %1" ).arg( reinterpret_cast< quint64 >( mData ) ), 4 );
  qgsFree( mData );
  delete mImage;
  qgsFree( mNoDataBitmap );
}

bool QgsRasterBlock::reset( Qgis::DataType dataType, int width, int height )
{
  QgsDebugMsgLevel( QStringLiteral( "theWidth= %1 height = %2 dataType = %3" ).arg( width ).arg( height ).arg( qgsEnumValueToKey< Qgis::DataType >( dataType ) ), 4 );

  qgsFree( mData );
  mData = nullptr;
  delete mImage;
  mImage = nullptr;
  qgsFree( mNoDataBitmap );
  mNoDataBitmap = nullptr;
  mDataType = Qgis::DataType::UnknownDataType;
  mTypeSize = 0;
  mWidth = 0;
  mHeight = 0;
  mHasNoDataValue = false;
  mNoDataValue = std::numeric_limits<double>::quiet_NaN();
  mValid = false;

  if ( typeIsNumeric( dataType ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Numeric type" ), 4 );
    const qgssize tSize = typeSize( dataType );
    QgsDebugMsgLevel( QStringLiteral( "allocate %1 bytes" ).arg( tSize * width * height ), 4 );
    mData = qgsMalloc( tSize * width * height );
    if ( !mData )
    {
      QgsDebugMsg( QStringLiteral( "Couldn't allocate data memory of %1 bytes" ).arg( tSize * width * height ) );
      return false;
    }
  }
  else if ( typeIsColor( dataType ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Color type" ), 4 );
    const QImage::Format format = imageFormat( dataType );
    mImage = new QImage( width, height, format );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Wrong data type" ) );
    return false;
  }

  mValid = true;
  mDataType = dataType;
  mTypeSize = QgsRasterBlock::typeSize( mDataType );
  mWidth = width;
  mHeight = height;
  QgsDebugMsgLevel( QStringLiteral( "mWidth= %1 mHeight = %2 mDataType = %3 mData = %4 mImage = %5" ).arg( mWidth ).arg( mHeight ).arg( static_cast< int>( mDataType ) )
                    .arg( reinterpret_cast< quint64 >( mData ) ).arg( reinterpret_cast< quint64 >( mImage ) ), 4 );
  return true;
}

QImage::Format QgsRasterBlock::imageFormat( Qgis::DataType dataType )
{
  if ( dataType == Qgis::DataType::ARGB32 )
  {
    return QImage::Format_ARGB32;
  }
  else if ( dataType == Qgis::DataType::ARGB32_Premultiplied )
  {
    return QImage::Format_ARGB32_Premultiplied;
  }
  return QImage::Format_Invalid;
}

Qgis::DataType QgsRasterBlock::dataType( QImage::Format format )
{
  if ( format == QImage::Format_ARGB32 )
  {
    return Qgis::DataType::ARGB32;
  }
  else if ( format == QImage::Format_ARGB32_Premultiplied )
  {
    return Qgis::DataType::ARGB32_Premultiplied;
  }
  return Qgis::DataType::UnknownDataType;
}

bool QgsRasterBlock::isEmpty() const
{
  QgsDebugMsgLevel( QStringLiteral( "mWidth= %1 mHeight = %2 mDataType = %3 mData = %4 mImage = %5" ).arg( mWidth ).arg( mHeight ).arg( qgsEnumValueToKey( mDataType ) )
                    .arg( reinterpret_cast< quint64 >( mData ) ).arg( reinterpret_cast< quint64 >( mImage ) ), 4 );
  return mWidth == 0 || mHeight == 0 ||
         ( typeIsNumeric( mDataType ) && !mData ) ||
         ( typeIsColor( mDataType ) && !mImage );
}

bool QgsRasterBlock::typeIsNumeric( Qgis::DataType dataType )
{
  switch ( dataType )
  {
    case Qgis::DataType::Byte:
    case Qgis::DataType::UInt16:
    case Qgis::DataType::Int16:
    case Qgis::DataType::UInt32:
    case Qgis::DataType::Int32:
    case Qgis::DataType::Float32:
    case Qgis::DataType::CInt16:
    case Qgis::DataType::Float64:
    case Qgis::DataType::CInt32:
    case Qgis::DataType::CFloat32:
    case Qgis::DataType::CFloat64:
      return true;

    case Qgis::DataType::UnknownDataType:
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
      return false;
  }
  return false;
}

bool QgsRasterBlock::typeIsColor( Qgis::DataType dataType )
{
  switch ( dataType )
  {
    case Qgis::DataType::ARGB32:
    case Qgis::DataType::ARGB32_Premultiplied:
      return true;

    case Qgis::DataType::UnknownDataType:
    case Qgis::DataType::Byte:
    case Qgis::DataType::UInt16:
    case Qgis::DataType::Int16:
    case Qgis::DataType::UInt32:
    case Qgis::DataType::Int32:
    case Qgis::DataType::Float32:
    case Qgis::DataType::CInt16:
    case Qgis::DataType::Float64:
    case Qgis::DataType::CInt32:
    case Qgis::DataType::CFloat32:
    case Qgis::DataType::CFloat64:
      return false;
  }
  return false;
}

Qgis::DataType QgsRasterBlock::typeWithNoDataValue( Qgis::DataType dataType, double *noDataValue )
{
  Qgis::DataType newDataType;

  switch ( dataType )
  {
    case Qgis::DataType::Byte:
      *noDataValue = -32768.0;
      newDataType = Qgis::DataType::Int16;
      break;
    case Qgis::DataType::Int16:
      *noDataValue = -2147483648.0;
      newDataType = Qgis::DataType::Int32;
      break;
    case Qgis::DataType::UInt16:
      *noDataValue = -2147483648.0;
      newDataType = Qgis::DataType::Int32;
      break;
    case Qgis::DataType::UInt32:
    case Qgis::DataType::Int32:
    case Qgis::DataType::Float32:
    case Qgis::DataType::Float64:
      *noDataValue = std::numeric_limits<double>::max() * -1.0;
      newDataType = Qgis::DataType::Float64;
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Unknown data type %1" ).arg( static_cast< int >( dataType ) ) );
      return Qgis::DataType::UnknownDataType;
  }
  QgsDebugMsgLevel( QStringLiteral( "newDataType = %1 noDataValue = %2" ).arg( qgsEnumValueToKey< Qgis::DataType >( newDataType ) ).arg( *noDataValue ), 4 );
  return newDataType;
}

void QgsRasterBlock::setNoDataValue( double noDataValue )
{
  mHasNoDataValue = true;
  mNoDataValue = noDataValue;
}

void QgsRasterBlock::resetNoDataValue()
{
  mHasNoDataValue = false;
  mNoDataValue = std::numeric_limits<double>::quiet_NaN();
}

bool QgsRasterBlock::setIsNoData()
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  if ( typeIsNumeric( mDataType ) )
  {
    const size_t dataTypeSize = typeSize( mDataType );
    if ( mHasNoDataValue )
    {
      if ( !mData )
      {
        QgsDebugMsg( QStringLiteral( "Data block not allocated" ) );
        return false;
      }

      QgsDebugMsgLevel( QStringLiteral( "set mData to mNoDataValue" ), 4 );
      QByteArray noDataByteArray = valueBytes( mDataType, mNoDataValue );
      if ( mNoDataValue == 0 )
      {
        memset( mData, 0, dataTypeSize * mWidth * mHeight );
      }
      else
      {
        const char *nodata = noDataByteArray.data();
        for ( qgssize i = 0; i < static_cast< qgssize >( mWidth )*mHeight; i++ )
        {
          memcpy( reinterpret_cast< char * >( mData ) + i * dataTypeSize, nodata, dataTypeSize );
        }
      }
    }
    else
    {
      // use bitmap
      if ( !mNoDataBitmap )
      {
        if ( !createNoDataBitmap() )
        {
          return false;
        }
      }
      QgsDebugMsgLevel( QStringLiteral( "set mNoDataBitmap to 1" ), 4 );
      memset( mNoDataBitmap, 0xff, mNoDataBitmapSize );
      if ( mData )
      {
        memset( mData, 0, dataTypeSize * mWidth * mHeight );
      }
    }
    return true;
  }
  else
  {
    // image
    if ( !mImage )
    {
      QgsDebugMsg( QStringLiteral( "Image not allocated" ) );
      return false;
    }
    QgsDebugMsgLevel( QStringLiteral( "Fill image" ), 4 );
    mImage->fill( NO_DATA_COLOR );
    return true;
  }
}

bool QgsRasterBlock::setIsNoDataExcept( QRect exceptRect )
{
  int top = exceptRect.top();
  int bottom = exceptRect.bottom();
  int left = exceptRect.left();
  int right = exceptRect.right();
  top = std::min( std::max( top, 0 ), mHeight - 1 );
  left = std::min( std::max( left, 0 ), mWidth - 1 );
  bottom = std::max( 0, std::min( bottom, mHeight - 1 ) );
  right = std::max( 0, std::min( right, mWidth - 1 ) );

  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  if ( typeIsNumeric( mDataType ) )
  {
    const size_t dataTypeSize = typeSize( mDataType );
    if ( mHasNoDataValue )
    {
      if ( !mData )
      {
        QgsDebugMsg( QStringLiteral( "Data block not allocated" ) );
        return false;
      }

      QgsDebugMsgLevel( QStringLiteral( "set mData to mNoDataValue" ), 4 );
      QByteArray noDataByteArray = valueBytes( mDataType, mNoDataValue );

      char *nodata = noDataByteArray.data();
      char *nodataRow = new char[mWidth * dataTypeSize]; // full row of no data
      for ( int c = 0; c < mWidth; c++ )
      {
        memcpy( nodataRow + c * dataTypeSize, nodata, dataTypeSize );
      }

      // top and bottom
      for ( int r = 0; r < mHeight; r++ )
      {
        if ( r >= top && r <= bottom ) continue; // middle
        const qgssize i = static_cast< qgssize >( r ) * mWidth;
        memcpy( reinterpret_cast< char * >( mData ) + i * dataTypeSize, nodataRow, dataTypeSize * static_cast< qgssize >( mWidth ) );
      }
      // middle
      for ( int r = top; r <= bottom; r++ )
      {
        qgssize i = static_cast< qgssize >( r ) * mWidth;
        // middle left
        memcpy( reinterpret_cast< char * >( mData ) + i * dataTypeSize, nodataRow, dataTypeSize * static_cast< qgssize >( left ) );
        // middle right
        i += right + 1;
        const int w = mWidth - right - 1;
        memcpy( reinterpret_cast< char * >( mData ) + i * dataTypeSize, nodataRow, dataTypeSize * static_cast< qgssize >( w ) );
      }
      delete [] nodataRow;
    }
    else
    {
      // use bitmap
      if ( !mNoDataBitmap )
      {
        if ( !createNoDataBitmap() )
        {
          return false;
        }
      }
      QgsDebugMsgLevel( QStringLiteral( "set mNoDataBitmap to 1" ), 4 );

      if ( mData )
      {
        memset( mData, 0, dataTypeSize * mWidth * mHeight );
      }

      char *nodataRow = new char[mNoDataBitmapWidth]; // full row of no data
      // TODO: we can simply set all bytes to 11111111 (~0) I think
      memset( nodataRow, 0, mNoDataBitmapWidth );
      for ( int c = 0; c < mWidth; c ++ )
      {
        const int byte = c / 8;
        const int bit = c % 8;
        const char nodata = 0x80 >> bit;
        memset( nodataRow + byte, nodataRow[byte] | nodata, 1 );
      }

      // top and bottom
      for ( int r = 0; r < mHeight; r++ )
      {
        if ( r >= top && r <= bottom ) continue; // middle
        const qgssize i = static_cast< qgssize >( r ) * mNoDataBitmapWidth;
        memcpy( mNoDataBitmap + i, nodataRow, mNoDataBitmapWidth );
      }
      // middle
      memset( nodataRow, 0, mNoDataBitmapWidth );
      for ( int c = 0; c < mWidth; c ++ )
      {
        if ( c >= left && c <= right ) continue; // middle
        const int byte = c / 8;
        const int bit = c % 8;
        const char nodata = 0x80 >> bit;
        memset( nodataRow + byte, nodataRow[byte] | nodata, 1 );
      }
      for ( int r = top; r <= bottom; r++ )
      {
        const qgssize i = static_cast< qgssize >( r ) * mNoDataBitmapWidth;
        memcpy( mNoDataBitmap + i, nodataRow, mNoDataBitmapWidth );
      }
      delete [] nodataRow;
    }
    return true;
  }
  else
  {
    // image
    if ( !mImage )
    {
      QgsDebugMsg( QStringLiteral( "Image not allocated" ) );
      return false;
    }

    if ( mImage->width() != mWidth ||  mImage->height() != mHeight )
    {
      QgsDebugMsg( QStringLiteral( "Image and block size differ" ) );
      return false;
    }

    QgsDebugMsgLevel( QStringLiteral( "Fill image depth = %1" ).arg( mImage->depth() ), 4 );

    // TODO: support different depths
    if ( mImage->depth() != 32 )
    {
      QgsDebugMsg( QStringLiteral( "Unsupported image depth" ) );
      return false;
    }

    const QRgb nodataRgba = NO_DATA_COLOR;
    QRgb *nodataRow = new QRgb[mWidth]; // full row of no data
    const int rgbSize = sizeof( QRgb );
    for ( int c = 0; c < mWidth; c ++ )
    {
      nodataRow[c] = nodataRgba;
    }

    // top and bottom
    for ( int r = 0; r < mHeight; r++ )
    {
      if ( r >= top && r <= bottom ) continue; // middle
      const qgssize i = static_cast< qgssize >( r ) * mWidth;
      memcpy( reinterpret_cast< void * >( mImage->bits() + rgbSize * i ), nodataRow, rgbSize * static_cast< qgssize >( mWidth ) );
    }
    // middle
    for ( int r = top; r <= bottom; r++ )
    {
      qgssize i = static_cast< qgssize >( r ) * mWidth;
      // middle left
      if ( left > 0 )
      {
        memcpy( reinterpret_cast< void * >( mImage->bits() + rgbSize * i ), nodataRow, rgbSize * static_cast< qgssize >( left - 1 ) );
      }
      // middle right
      i += right + 1;
      const int w = mWidth - right - 1;
      memcpy( reinterpret_cast< void * >( mImage->bits() + rgbSize * i ), nodataRow, rgbSize * static_cast< qgssize >( w ) );
    }
    delete [] nodataRow;
    return true;
  }
}

QByteArray QgsRasterBlock::data() const
{
  if ( mData )
    return QByteArray::fromRawData( static_cast<const char *>( mData ), typeSize( mDataType ) * mWidth * mHeight );
  else if ( mImage && mImage->constBits() )
    return QByteArray::fromRawData( reinterpret_cast<const char *>( mImage->constBits() ), mImage->sizeInBytes() );
  else
    return QByteArray();
}

void QgsRasterBlock::setData( const QByteArray &data, int offset )
{
  if ( offset < 0 )
    return;  // negative offsets not allowed

  if ( mData )
  {
    const int len = std::min( static_cast<int>( data.size() ), typeSize( mDataType ) * mWidth * mHeight - offset );
    ::memcpy( static_cast<char *>( mData ) + offset, data.constData(), len );
  }
  else if ( mImage && mImage->constBits() )
  {
    const qsizetype len = std::min( static_cast< qsizetype >( data.size() ), mImage->sizeInBytes() - offset );
    ::memcpy( mImage->bits() + offset, data.constData(), len );
  }
}

char *QgsRasterBlock::bits( qgssize index )
{
  // Not testing type to avoid too much overhead because this method is called per pixel
  if ( index >= static_cast< qgssize >( mWidth )*mHeight )
  {
    QgsDebugMsgLevel( QStringLiteral( "Index %1 out of range (%2 x %3)" ).arg( index ).arg( mWidth ).arg( mHeight ), 4 );
    return nullptr;
  }
  if ( mData )
  {
    return reinterpret_cast< char * >( mData ) + index * mTypeSize;
  }
  if ( mImage && mImage->bits() )
  {
    return reinterpret_cast< char * >( mImage->bits() + index * 4 );
  }

  return nullptr;
}

char *QgsRasterBlock::bits( int row, int column )
{
  return bits( static_cast< qgssize >( row ) * mWidth + column );
}

char *QgsRasterBlock::bits()
{
  if ( mData )
  {
    return reinterpret_cast< char * >( mData );
  }
  if ( mImage && mImage->bits() )
  {
    return reinterpret_cast< char * >( mImage->bits() );
  }

  return nullptr;
}

bool QgsRasterBlock::convert( Qgis::DataType destDataType )
{
  if ( isEmpty() ) return false;
  if ( destDataType == mDataType ) return true;

  if ( typeIsNumeric( mDataType ) && typeIsNumeric( destDataType ) )
  {
    void *data = convert( mData, mDataType, destDataType, static_cast< qgssize >( mWidth ) * static_cast< qgssize >( mHeight ) );

    if ( !data )
    {
      QgsDebugMsg( QStringLiteral( "Cannot convert raster block" ) );
      return false;
    }
    qgsFree( mData );
    mData = data;
    mDataType = destDataType;
    mTypeSize = typeSize( mDataType );
  }
  else if ( typeIsColor( mDataType ) && typeIsColor( destDataType ) )
  {
    const QImage::Format format = imageFormat( destDataType );
    const QImage image = mImage->convertToFormat( format );
    *mImage = image;
    mDataType = destDataType;
    mTypeSize = typeSize( mDataType );
  }
  else
  {
    return false;
  }

  return true;
}

void QgsRasterBlock::applyScaleOffset( double scale, double offset )
{
  if ( isEmpty() ) return;
  if ( !typeIsNumeric( mDataType ) ) return;
  if ( scale == 1.0 && offset == 0.0 ) return;

  const qgssize size = static_cast< qgssize >( mWidth ) * mHeight;
  for ( qgssize i = 0; i < size; ++i )
  {
    if ( !isNoData( i ) ) setValue( i, value( i ) * scale + offset );
  }
}

void QgsRasterBlock::applyNoDataValues( const QgsRasterRangeList &rangeList )
{
  if ( rangeList.isEmpty() )
  {
    return;
  }

  const qgssize size = static_cast< qgssize >( mWidth ) * static_cast< qgssize >( mHeight );
  for ( qgssize i = 0; i < size; ++i )
  {
    const double val = value( i );
    if ( QgsRasterRange::contains( val, rangeList ) )
    {
      //setValue( i, mNoDataValue );
      setIsNoData( i );
    }
  }
}

QImage QgsRasterBlock::image() const
{
  if ( mImage )
  {
    return QImage( *mImage );
  }
  return QImage();
}

bool QgsRasterBlock::setImage( const QImage *image )
{
  qgsFree( mData );
  mData = nullptr;
  delete mImage;
  mImage = nullptr;
  mImage = new QImage( *image );
  mWidth = mImage->width();
  mHeight = mImage->height();
  mDataType = dataType( mImage->format() );
  mTypeSize = QgsRasterBlock::typeSize( mDataType );
  mNoDataValue = std::numeric_limits<double>::quiet_NaN();
  return true;
}

QString QgsRasterBlock::printValue( double value )
{
  /*
   *  IEEE 754 double has 15-17 significant digits. It specifies:
   *
   * "If a decimal string with at most 15 significant decimal is converted to
   *  IEEE 754 double precision and then converted back to the same number of
   *  significant decimal, then the final string should match the original;
   *  and if an IEEE 754 double precision is converted to a decimal string with at
   *  least 17 significant decimal and then converted back to double, then the final
   *  number must match the original."
   *
   * If printing only 15 digits, some precision could be lost. Printing 17 digits may
   * add some confusing digits.
   *
   * Default 'g' precision on linux is 6 digits, not all significant digits like
   * some sprintf manuals say.
   *
   * We need to ensure that the number printed and used in QLineEdit or XML will
   * give the same number when parsed.
   *
   * Is there a better solution?
   */

  QString s;

  for ( int i = 15; i <= 17; i++ )
  {
    s.setNum( value, 'g', i );
    if ( qgsDoubleNear( s.toDouble(), value ) )
    {
      return s;
    }
  }
  // Should not happen
  QgsDebugMsg( QStringLiteral( "Cannot correctly parse printed value" ) );
  return s;
}

QString QgsRasterBlock::printValue( float value )
{
  /*
   *  IEEE 754 double has 6-9 significant digits. See printValue(double)
   */

  QString s;

  for ( int i = 6; i <= 9; i++ )
  {
    s.setNum( value, 'g', i );
    if ( qgsFloatNear( s.toFloat(), value ) )
    {
      return s;
    }
  }
  // Should not happen
  QgsDebugMsg( QStringLiteral( "Cannot correctly parse printed value" ) );
  return s;
}

void *QgsRasterBlock::convert( void *srcData, Qgis::DataType srcDataType, Qgis::DataType destDataType, qgssize size )
{
  const int destDataTypeSize = typeSize( destDataType );
  void *destData = qgsMalloc( destDataTypeSize * size );
  for ( qgssize i = 0; i < size; i++ )
  {
    const double value = readValue( srcData, srcDataType, i );
    writeValue( destData, destDataType, i, value );
    //double newValue = readValue( destData, destDataType, i );
    //QgsDebugMsg( QStringLiteral("convert %1 type %2 to %3: %4 -> %5").arg(i).arg(srcDataType).arg(destDataType).arg( value ).arg( newValue ) );
  }
  return destData;
}

QByteArray QgsRasterBlock::valueBytes( Qgis::DataType dataType, double value )
{
  const qgssize size = QgsRasterBlock::typeSize( dataType );
  QByteArray ba;
  ba.resize( static_cast< int >( size ) );
  char *data = ba.data();
  quint8 uc;
  quint16 us;
  qint16 s;
  quint32 ui;
  qint32 i;
  float f;
  double d;
  switch ( dataType )
  {
    case Qgis::DataType::Byte:
      uc = static_cast< quint8 >( value );
      memcpy( data, &uc, size );
      break;
    case Qgis::DataType::UInt16:
      us = static_cast< quint16 >( value );
      memcpy( data, &us, size );
      break;
    case Qgis::DataType::Int16:
      s = static_cast< qint16 >( value );
      memcpy( data, &s, size );
      break;
    case Qgis::DataType::UInt32:
      ui = static_cast< quint32 >( value );
      memcpy( data, &ui, size );
      break;
    case Qgis::DataType::Int32:
      i = static_cast< qint32 >( value );
      memcpy( data, &i, size );
      break;
    case Qgis::DataType::Float32:
      f = static_cast< float >( value );
      memcpy( data, &f, size );
      break;
    case Qgis::DataType::Float64:
      d = static_cast< double >( value );
      memcpy( data, &d, size );
      break;
    default:
      QgsDebugMsg( QStringLiteral( "Data type is not supported" ) );
  }
  return ba;
}

bool QgsRasterBlock::createNoDataBitmap()
{
  mNoDataBitmapWidth = mWidth / 8 + 1;
  mNoDataBitmapSize = static_cast< qgssize >( mNoDataBitmapWidth ) * mHeight;
  QgsDebugMsgLevel( QStringLiteral( "allocate %1 bytes" ).arg( mNoDataBitmapSize ), 4 );
  mNoDataBitmap = reinterpret_cast< char * >( qgsMalloc( mNoDataBitmapSize ) );
  if ( !mNoDataBitmap )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't allocate no data memory of %1 bytes" ).arg( mNoDataBitmapSize ) );
    return false;
  }
  memset( mNoDataBitmap, 0, mNoDataBitmapSize );
  return true;
}

QString  QgsRasterBlock::toString() const
{
  return QStringLiteral( "dataType = %1 width = %2 height = %3" )
         .arg( qgsEnumValueToKey< Qgis::DataType >( mDataType ) ).arg( mWidth ).arg( mHeight );
}

QRect QgsRasterBlock::subRect( const QgsRectangle &extent, int width, int height, const QgsRectangle   &subExtent )
{
  QgsDebugMsgLevel( "theExtent = " + extent.toString(), 4 );
  QgsDebugMsgLevel( "theSubExtent = " + subExtent.toString(), 4 );
  const double xRes = extent.width() / width;
  const double yRes = extent.height() / height;

  QgsDebugMsgLevel( QStringLiteral( "theWidth = %1 height = %2 xRes = %3 yRes = %4" ).arg( width ).arg( height ).arg( xRes ).arg( yRes ), 4 );

  int top = 0;
  int bottom = height - 1;
  int left = 0;
  int right = width - 1;

  if ( subExtent.yMaximum() < extent.yMaximum() )
  {
    top = std::round( ( extent.yMaximum() - subExtent.yMaximum() ) / yRes );
  }
  if ( subExtent.yMinimum() > extent.yMinimum() )
  {
    bottom = std::round( ( extent.yMaximum() - subExtent.yMinimum() ) / yRes ) - 1;
  }

  if ( subExtent.xMinimum() > extent.xMinimum() )
  {
    left = std::round( ( subExtent.xMinimum() - extent.xMinimum() ) / xRes );
  }
  if ( subExtent.xMaximum() < extent.xMaximum() )
  {
    right = std::round( ( subExtent.xMaximum() - extent.xMinimum() ) / xRes ) - 1;
  }
  QRect subRect = QRect( left, top, right - left + 1, bottom - top + 1 );
  QgsDebugMsgLevel( QStringLiteral( "subRect: %1 %2 %3 %4" ).arg( subRect.x() ).arg( subRect.y() ).arg( subRect.width() ).arg( subRect.height() ), 4 );
  return subRect;
}
