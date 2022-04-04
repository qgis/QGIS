/***************************************************************************
                         qgspointcloudrenderer.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseptdecoder.h"
#include "qgseptpointcloudindex.h"
#include "qgspointcloudattribute.h"
#include "qgsvector3d.h"
#include "qgsconfig.h"
#include "qgslogger.h"

#include <QFile>
#include <QDir>
#include <iostream>
#include <memory>
#include <cstring>
#include <QElapsedTimer>
#include <QTemporaryFile>
#include <string>

#include <zstd.h>

#include "lazperf/las.hpp"


///@cond PRIVATE

template <typename T>
bool _storeToStream( char *s, size_t position, QgsPointCloudAttribute::DataType type, T value )
{
  switch ( type )
  {
    case QgsPointCloudAttribute::Char:
    {
      const char val = char( value );
      s[position] = val;
      break;
    }
    case QgsPointCloudAttribute::UChar:
    {
      const unsigned char val = ( unsigned char )( value );
      s[position] = val;
      break;
    }

    case QgsPointCloudAttribute::Short:
    {
      short val = short( value );
      memcpy( s + position, reinterpret_cast<char * >( &val ), sizeof( short ) );
      break;
    }
    case QgsPointCloudAttribute::UShort:
    {
      unsigned short val = static_cast< unsigned short>( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( unsigned short ) );
      break;
    }

    case QgsPointCloudAttribute::Int32:
    {
      qint32 val = qint32( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( qint32 ) );
      break;
    }
    case QgsPointCloudAttribute::UInt32:
    {
      quint32 val = quint32( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( quint32 ) );
      break;
    }

    case QgsPointCloudAttribute::Int64:
    {
      qint64 val = qint64( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( qint64 ) );
      break;
    }
    case QgsPointCloudAttribute::UInt64:
    {
      quint64 val = quint64( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( quint64 ) );
      break;
    }

    case QgsPointCloudAttribute::Float:
    {
      float val = float( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ),  sizeof( float ) );
      break;
    }
    case QgsPointCloudAttribute::Double:
    {
      double val = double( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( double ) );
      break;
    }
  }

  return true;
}

bool __serialize( char *data, size_t outputPosition, QgsPointCloudAttribute::DataType outputType,
                  const char *input, QgsPointCloudAttribute::DataType inputType, int inputSize, size_t inputPosition )
{
  if ( outputType == inputType )
  {
    memcpy( data + outputPosition, input + inputPosition, inputSize );
    return true;
  }

  switch ( inputType )
  {
    case QgsPointCloudAttribute::Char:
    {
      const char val = *( input + inputPosition );
      return _storeToStream<char>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UChar:
    {
      const unsigned char val = *( input + inputPosition );
      return _storeToStream<unsigned char>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Short:
    {
      const short val = *reinterpret_cast< const short * >( input + inputPosition );
      return _storeToStream<short>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UShort:
    {
      const unsigned short val = *reinterpret_cast< const unsigned short * >( input + inputPosition );
      return _storeToStream<unsigned short>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Int32:
    {
      const qint32 val = *reinterpret_cast<const qint32 * >( input + inputPosition );
      return _storeToStream<qint32>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UInt32:
    {
      const quint32 val = *reinterpret_cast<const quint32 * >( input + inputPosition );
      return _storeToStream<quint32>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Int64:
    {
      const qint64 val = *reinterpret_cast<const qint64 * >( input + inputPosition );
      return _storeToStream<qint64>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::UInt64:
    {
      const quint64 val = *reinterpret_cast<const quint64 * >( input + inputPosition );
      return _storeToStream<quint64>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Float:
    {
      const float val = *reinterpret_cast< const float * >( input + inputPosition );
      return _storeToStream<float>( data, outputPosition, outputType, val );
    }
    case QgsPointCloudAttribute::Double:
    {
      const double val = *reinterpret_cast< const double * >( input + inputPosition );
      return _storeToStream<double>( data, outputPosition, outputType, val );
    }
  }
  return true;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QgsPointCloudBlock *_decompressBinary( const QByteArray &dataUncompressed, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  const std::size_t pointRecordSize = attributes.pointRecordSize( );
  const std::size_t requestedPointRecordSize = requestedAttributes.pointRecordSize();
  const int count = dataUncompressed.size() / pointRecordSize;
  QByteArray data;
  data.resize( requestedPointRecordSize * count );
  char *destinationBuffer = data.data();
  const char *s = dataUncompressed.data();

  const QVector<QgsPointCloudAttribute> requestedAttributesVector = requestedAttributes.attributes();

  // calculate input attributes and offsets once in advance

  struct AttributeData
  {
    AttributeData( int inputOffset, int inputSize, QgsPointCloudAttribute::DataType inputType, int requestedSize, QgsPointCloudAttribute::DataType requestedType )
      : inputOffset( inputOffset )
      , inputSize( inputSize )
      , inputType( inputType )
      , requestedSize( requestedSize )
      , requestedType( requestedType )
    {}

    int inputOffset;
    int inputSize;
    QgsPointCloudAttribute::DataType inputType;
    int requestedSize;
    QgsPointCloudAttribute::DataType requestedType;
  };

  std::vector< AttributeData > attributeData;
  attributeData.reserve( requestedAttributesVector.size() );
  for ( const QgsPointCloudAttribute &requestedAttribute : requestedAttributesVector )
  {
    int inputAttributeOffset;
    const QgsPointCloudAttribute *inputAttribute = attributes.find( requestedAttribute.name(), inputAttributeOffset );
    if ( !inputAttribute )
    {
      return nullptr;
    }
    attributeData.emplace_back( AttributeData( inputAttributeOffset, inputAttribute->size(), inputAttribute->type(),
                                requestedAttribute.size(), requestedAttribute.type() ) );
  }

  int skippedPoints = 0;
  std::unique_ptr< QgsPointCloudBlock > block = std::make_unique< QgsPointCloudBlock >(
        count,
        requestedAttributes,
        data, scale, offset
      );

  const bool filterIsValid = filterExpression.isValid();
  if ( !filterExpression.prepare( block.get() ) && filterIsValid )
  {
    // skip processing if the expression cannot be prepared
    block->setPointCount( 0 );
    return block.release();
  }

  // now loop through points
  size_t outputOffset = 0;
  for ( int i = 0; i < count; ++i )
  {
    for ( const AttributeData &attribute : attributeData )
    {
      __serialize( destinationBuffer, outputOffset,
                   attribute.requestedType, s,
                   attribute.inputType, attribute.inputSize, i * pointRecordSize + attribute.inputOffset );

      outputOffset += attribute.requestedSize;
    }

    // check if point needs to be filtered out
    if ( filterIsValid )
    {
      // we're always evaluating the last written point in the buffer
      double eval = filterExpression.evaluate( i - skippedPoints );
      if ( !eval || std::isnan( eval ) )
      {
        // if the point is filtered out, rewind the offset so the next point is written over it
        outputOffset -= requestedPointRecordSize;
        ++skippedPoints;
      }
    }
  }
  block->setPointCount( count - skippedPoints );
  return block.release();
}

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  const bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  const QByteArray dataUncompressed = f.read( f.size() );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression );
}

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  return _decompressBinary( data, attributes, requestedAttributes, scale, offset, filterExpression );
}

/* *************************************************************************************** */

QByteArray decompressZtdStream( const QByteArray &dataCompressed )
{
  // NOTE: this is very primitive implementation because we expect the uncompressed
  // data will be always less than 10 MB

  const int MAXSIZE = 10000000;
  QByteArray dataUncompressed;
  dataUncompressed.resize( MAXSIZE );

  ZSTD_DStream *strm = ZSTD_createDStream();
  ZSTD_initDStream( strm );

  ZSTD_inBuffer m_inBuf;
  m_inBuf.src = reinterpret_cast<const void *>( dataCompressed.constData() );
  m_inBuf.size = dataCompressed.size();
  m_inBuf.pos = 0;

  ZSTD_outBuffer outBuf { reinterpret_cast<void *>( dataUncompressed.data() ), MAXSIZE, 0 };
  const size_t ret = ZSTD_decompressStream( strm, &outBuf, &m_inBuf );
  Q_ASSERT( !ZSTD_isError( ret ) );
  Q_ASSERT( outBuf.pos );
  Q_ASSERT( outBuf.pos < outBuf.size );

  ZSTD_freeDStream( strm );
  dataUncompressed.resize( outBuf.pos );
  return dataUncompressed;
}

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  const bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  const QByteArray dataCompressed = f.readAll();
  const QByteArray dataUncompressed = decompressZtdStream( dataCompressed );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression );
}

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression )
{
  const QByteArray dataUncompressed = decompressZtdStream( data );
  return _decompressBinary( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression );
}

/* *************************************************************************************** */

///@endcond
