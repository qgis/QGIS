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
#include "qgslazdecoder.h"
#include "qgspointcloudattribute.h"
#include "qgsvector3d.h"
#include "qgspointcloudexpression.h"
#include "qgsrectangle.h"

#include <QFile>

#include <zstd.h>


///@cond PRIVATE

QgsPointCloudBlock *decompressBinary_( const QByteArray &dataUncompressed, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression, QgsRectangle &filterRect )
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

  int xAttributeOffset, yAttributeOffset;
  const QgsPointCloudAttribute *attributeX = nullptr;
  const QgsPointCloudAttribute *attributeY = nullptr;
  const bool hasFilterRect = !filterRect.isEmpty();
  if ( hasFilterRect )
  {
    attributeX = requestedAttributes.find( QLatin1String( "X" ), xAttributeOffset );
    attributeY = requestedAttributes.find( QLatin1String( "Y" ), yAttributeOffset );
    filterRect.setXMinimum( ( filterRect.xMinimum() - offset.x() ) / scale.x() );
    filterRect.setXMaximum( ( filterRect.xMaximum() - offset.x() ) / scale.x() );
    filterRect.setYMinimum( ( filterRect.yMinimum() - offset.y() ) / scale.y() );
    filterRect.setYMaximum( ( filterRect.yMaximum() - offset.y() ) / scale.y() );
  }

  // now loop through points
  size_t outputOffset = 0;
  for ( int i = 0; i < count; ++i )
  {
    for ( const AttributeData &attribute : attributeData )
    {
      lazSerialize_( destinationBuffer, outputOffset,
                     attribute.requestedType, s,
                     attribute.inputType, attribute.inputSize, i * pointRecordSize + attribute.inputOffset );

      outputOffset += attribute.requestedSize;
    }

    // check if point needs to be filtered out
    bool skipThisPoint = false;
    if ( hasFilterRect && attributeX && attributeY )
    {
      const double x = attributeX->convertValueToDouble( destinationBuffer + outputOffset - requestedPointRecordSize + xAttributeOffset );
      const double y = attributeY->convertValueToDouble( destinationBuffer + outputOffset - requestedPointRecordSize + yAttributeOffset );
      if ( !filterRect.contains( x, y ) )
        skipThisPoint = true;
    }
    if ( !skipThisPoint && filterIsValid )
    {
      // we're always evaluating the last written point in the buffer
      double eval = filterExpression.evaluate( i - skippedPoints );
      if ( !eval || std::isnan( eval ) )
        skipThisPoint = true;
    }
    if ( skipThisPoint )
    {
      // if the point is filtered out, rewind the offset so the next point is written over it
      outputOffset -= requestedPointRecordSize;
      ++skippedPoints;
    }
  }
  block->setPointCount( count - skippedPoints );
  return block.release();
}

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression, QgsRectangle &filterRect )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  const bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  const QByteArray dataUncompressed = f.read( f.size() );
  return decompressBinary_( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression, filterRect );
}

QgsPointCloudBlock *QgsEptDecoder::decompressBinary( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression, QgsRectangle &filterRect )
{
  return decompressBinary_( data, attributes, requestedAttributes, scale, offset, filterExpression, filterRect );
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

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression, QgsRectangle &filterRect )
{
  if ( ! QFile::exists( filename ) )
    return nullptr;

  QFile f( filename );
  const bool r = f.open( QIODevice::ReadOnly );
  if ( !r )
    return nullptr;

  const QByteArray dataCompressed = f.readAll();
  const QByteArray dataUncompressed = decompressZtdStream( dataCompressed );
  return decompressBinary_( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression, filterRect );
}

QgsPointCloudBlock *QgsEptDecoder::decompressZStandard( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset, QgsPointCloudExpression &filterExpression, QgsRectangle &filterRect )
{
  const QByteArray dataUncompressed = decompressZtdStream( data );
  return decompressBinary_( dataUncompressed, attributes, requestedAttributes, scale, offset, filterExpression, filterRect );
}

/* *************************************************************************************** */

///@endcond
