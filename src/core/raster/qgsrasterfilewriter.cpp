/***************************************************************************
    qgsrasterfilewriter.cpp
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <typeinfo>

#include "qgsgdalutils.h"
#include "qgsrasterfilewriter.h"
#include "qgscoordinatetransform.h"
#include "qgsproviderregistry.h"
#include "qgsrasterinterface.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterprojector.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasternuller.h"

#include <QCoreApplication>
#include <QProgressDialog>
#include <QTextStream>
#include <QMessageBox>

#include <gdal.h>
#include <cpl_string.h>

QgsRasterDataProvider *QgsRasterFileWriter::createOneBandRaster( Qgis::DataType dataType, int width, int height, const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs )
{
  if ( mTiledMode )
    return nullptr;  // does not make sense with tiled mode

  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( extent, width, height, geoTransform, pixelSize );

  return initOutput( width, height, crs, geoTransform, 1, dataType, QList<bool>(), QList<double>() );
}

QgsRasterDataProvider *QgsRasterFileWriter::createMultiBandRaster( Qgis::DataType dataType, int width, int height, const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs, int nBands )
{
  if ( mTiledMode )
    return nullptr;  // does not make sense with tiled mode

  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( extent, width, height, geoTransform, pixelSize );

  return initOutput( width, height, crs, geoTransform, nBands, dataType, QList<bool>(), QList<double>() );
}

QgsRasterFileWriter::QgsRasterFileWriter( const QString &outputUrl )
  : mOutputUrl( outputUrl )
{

}

QgsRasterFileWriter::QgsRasterFileWriter()
{

}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( "Entered", 4 );

  if ( !pipe )
  {
    return SourceProviderError;
  }
  mPipe = pipe;

  //const QgsRasterInterface* iface = iter->input();
  const QgsRasterInterface *iface = pipe->last();
  if ( !iface )
  {
    return SourceProviderError;
  }
  mInput = iface;

  if ( QgsRasterBlock::typeIsColor( iface->dataType( 1 ) ) )
  {
    mMode = Image;
  }
  else
  {
    mMode = Raw;
  }

  QgsDebugMsgLevel( QString( "reading from %1" ).arg( typeid( *iface ).name() ), 4 );

  if ( !iface->sourceInput() )
  {
    QgsDebugMsg( "iface->srcInput() == 0" );
    return SourceProviderError;
  }
#ifdef QGISDEBUG
  const QgsRasterInterface &srcInput = *iface->sourceInput();
  QgsDebugMsgLevel( QString( "srcInput = %1" ).arg( typeid( srcInput ).name() ), 4 );
#endif

  mFeedback = feedback;

  QgsRasterIterator iter( pipe->last() );

  //create directory for output files
  if ( mTiledMode )
  {
    QFileInfo fileInfo( mOutputUrl );
    if ( !fileInfo.exists() )
    {
      QDir dir = fileInfo.dir();
      if ( !dir.mkdir( fileInfo.fileName() ) )
      {
        QgsDebugMsg( "Cannot create output VRT directory " + fileInfo.fileName() + " in " + dir.absolutePath() );
        return CreateDatasourceError;
      }
    }
  }

  // Remove pre-existing overview files to avoid using those with new raster
  QFile pyramidFile( mOutputUrl + ( mTiledMode ? ".vrt.ovr" : ".ovr" ) );
  if ( pyramidFile.exists() )
    pyramidFile.remove();
  pyramidFile.setFileName( mOutputUrl + ( mTiledMode ? ".vrt.rrd" : ".rrd" ) );
  if ( pyramidFile.exists() )
    pyramidFile.remove();

  if ( mMode == Image )
  {
    WriterError e = writeImageRaster( &iter, nCols, nRows, outputExtent, crs, feedback );
    return e;
  }
  else
  {
    WriterError e = writeDataRaster( pipe, &iter, nCols, nRows, outputExtent, crs, feedback );
    return e;
  }
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeDataRaster( const QgsRasterPipe *pipe, QgsRasterIterator *iter, int nCols, int nRows, const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( "Entered", 4 );
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface *iface = pipe->last();
  if ( !iface )
  {
    return SourceProviderError;
  }

  QgsRasterDataProvider *srcProvider = const_cast<QgsRasterDataProvider *>( dynamic_cast<const QgsRasterDataProvider *>( iface->sourceInput() ) );
  if ( !srcProvider )
  {
    QgsDebugMsg( "Cannot get source data provider" );
    return SourceProviderError;
  }

  iter->setMaximumTileWidth( mMaxTileWidth );
  iter->setMaximumTileHeight( mMaxTileHeight );

  int nBands = iface->bandCount();
  if ( nBands < 1 )
  {
    return SourceProviderError;
  }


  //check if all the bands have the same data type size, otherwise we cannot write it to the provider
  //(at least not with the current interface)
  int dataTypeSize = QgsRasterBlock::typeSize( srcProvider->sourceDataType( 1 ) );
  for ( int i = 2; i <= nBands; ++i )
  {
    if ( QgsRasterBlock::typeSize( srcProvider->sourceDataType( 1 ) ) != dataTypeSize )
    {
      return DestProviderError;
    }
  }

  // Output data type - source data type is preferred but it may happen that we need
  // to set 'no data' value (which was not set on source data) if output extent
  // is larger than source extent (with or without reprojection) and there is no 'free'
  // (not used) value available
  QList<bool> destHasNoDataValueList;
  QList<double> destNoDataValueList;
  QList<Qgis::DataType> destDataTypeList;
  destDataTypeList.reserve( nBands );
  destHasNoDataValueList.reserve( nBands );
  destNoDataValueList.reserve( nBands );

  for ( int bandNo = 1; bandNo <= nBands; bandNo++ )
  {
    QgsRasterNuller *nuller = pipe->nuller();

    bool srcHasNoDataValue = srcProvider->sourceHasNoDataValue( bandNo );
    bool destHasNoDataValue = false;
    double destNoDataValue = std::numeric_limits<double>::quiet_NaN();
    Qgis::DataType destDataType = srcProvider->sourceDataType( bandNo );
    // TODO: verify what happens/should happen if srcNoDataValue is disabled by setUseSrcNoDataValue
    QgsDebugMsgLevel( QString( "srcHasNoDataValue = %1 srcNoDataValue = %2" ).arg( srcHasNoDataValue ).arg( srcProvider->sourceNoDataValue( bandNo ) ), 4 );
    if ( srcHasNoDataValue )
    {

      // If source has no data value, it is used by provider
      destNoDataValue = srcProvider->sourceNoDataValue( bandNo );
      destHasNoDataValue = true;
    }
    else if ( nuller && !nuller->noData( bandNo ).isEmpty() )
    {
      // Use one user defined no data value
      destNoDataValue = nuller->noData( bandNo ).value( 0 ).min();
      destHasNoDataValue = true;
    }
    else
    {
      // Verify if we really need no data value, i.e.
      QgsRectangle srcExtent = outputExtent;
      QgsRasterProjector *projector = pipe->projector();
      if ( projector && projector->destinationCrs() != projector->sourceCrs() )
      {
        Q_NOWARN_DEPRECATED_PUSH
        QgsCoordinateTransform ct( projector->destinationCrs(), projector->sourceCrs() );
        Q_NOWARN_DEPRECATED_POP
        srcExtent = ct.transformBoundingBox( outputExtent );
      }
      if ( !srcProvider->extent().contains( srcExtent ) )
      {
        // Destination extent is larger than source extent, we need destination no data values
        // Get src sample statistics (estimation from sample)
        QgsRasterBandStats stats = srcProvider->bandStatistics( bandNo, QgsRasterBandStats::Min | QgsRasterBandStats::Max, srcExtent, 250000 );

        // Test if we have free (not used) values
        double typeMinValue = QgsContrastEnhancement::maximumValuePossible( static_cast< Qgis::DataType >( srcProvider->sourceDataType( bandNo ) ) );
        double typeMaxValue = QgsContrastEnhancement::maximumValuePossible( static_cast< Qgis::DataType >( srcProvider->sourceDataType( bandNo ) ) );
        if ( stats.minimumValue > typeMinValue )
        {
          destNoDataValue = typeMinValue;
        }
        else if ( stats.maximumValue < typeMaxValue )
        {
          destNoDataValue = typeMaxValue;
        }
        else
        {
          // We have to use wider type
          destDataType = QgsRasterBlock::typeWithNoDataValue( destDataType, &destNoDataValue );
        }
        destHasNoDataValue = true;
      }
    }

    if ( nuller && destHasNoDataValue )
    {
      nuller->setOutputNoDataValue( bandNo, destNoDataValue );
    }

    QgsDebugMsgLevel( QString( "bandNo = %1 destDataType = %2 destHasNoDataValue = %3 destNoDataValue = %4" ).arg( bandNo ).arg( destDataType ).arg( destHasNoDataValue ).arg( destNoDataValue ), 4 );
    destDataTypeList.append( destDataType );
    destHasNoDataValueList.append( destHasNoDataValue );
    destNoDataValueList.append( destNoDataValue );
  }

  Qgis::DataType destDataType = destDataTypeList.value( 0 );
  // Currently write API supports one output type for dataset only -> find the widest
  for ( int i = 1; i < nBands; i++ )
  {
    if ( destDataTypeList.value( i ) > destDataType )
    {
      destDataType = destDataTypeList.value( i );
      // no data value may be left per band (for future)
    }
  }

  //create destProvider for whole dataset here
  QgsRasterDataProvider *destProvider = nullptr;
  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( outputExtent, nCols, nRows, geoTransform, pixelSize );

  // initOutput() returns 0 in tile mode!
  destProvider = initOutput( nCols, nRows, crs, geoTransform, nBands, destDataType, destHasNoDataValueList, destNoDataValueList );

  WriterError error = writeDataRaster( pipe, iter, nCols, nRows, outputExtent, crs, destDataType, destHasNoDataValueList, destNoDataValueList, destProvider, feedback );

  if ( error == NoDataConflict )
  {
    // The value used for no data was found in source data, we must use wider data type
    if ( destProvider ) // no tiles
    {
      destProvider->remove();
      delete destProvider;
      destProvider = nullptr;
    }
    else // VRT
    {
      // TODO: remove created VRT
    }

    // But we don't know which band -> wider all
    for ( int i = 0; i < nBands; i++ )
    {
      double destNoDataValue;
      Qgis::DataType destDataType = QgsRasterBlock::typeWithNoDataValue( destDataTypeList.value( i ), &destNoDataValue );
      destDataTypeList.replace( i, destDataType );
      destNoDataValueList.replace( i, destNoDataValue );
    }
    destDataType = destDataTypeList.value( 0 );

    // Try again
    destProvider = initOutput( nCols, nRows, crs, geoTransform, nBands, destDataType, destHasNoDataValueList, destNoDataValueList );
    error = writeDataRaster( pipe, iter, nCols, nRows, outputExtent, crs, destDataType, destHasNoDataValueList, destNoDataValueList, destProvider, feedback );
  }

  delete destProvider;
  return error;
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeDataRaster( const QgsRasterPipe *pipe,
    QgsRasterIterator *iter,
    int nCols, int nRows,
    const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs,
    Qgis::DataType destDataType,
    const QList<bool> &destHasNoDataValueList,
    const QList<double> &destNoDataValueList,
    QgsRasterDataProvider *destProvider,
    QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( pipe );
  Q_UNUSED( destHasNoDataValueList );
  QgsDebugMsgLevel( "Entered", 4 );

  const QgsRasterInterface *iface = iter->input();
  const QgsRasterDataProvider *srcProvider = dynamic_cast<const QgsRasterDataProvider *>( iface->sourceInput() );
  int nBands = iface->bandCount();
  QgsDebugMsgLevel( QString( "nBands = %1" ).arg( nBands ), 4 );

  //Get output map units per pixel
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;

  QList<QgsRasterBlock *> blockList;
  blockList.reserve( nBands );
  for ( int i = 1; i <= nBands; ++i )
  {
    iter->startRasterRead( i, nCols, nRows, outputExtent );
    blockList.push_back( nullptr );
    if ( destProvider && destHasNoDataValueList.value( i - 1 ) ) // no tiles
    {
      destProvider->setNoDataValue( i, destNoDataValueList.value( i - 1 ) );
    }
  }

  int nParts = 0;
  int fileIndex = 0;
  if ( feedback )
  {
    int nPartsX = nCols / iter->maximumTileWidth() + 1;
    int nPartsY = nRows / iter->maximumTileHeight() + 1;
    nParts = nPartsX * nPartsY;
  }

  // hmm why is there a for(;;) here ..
  // not good coding practice IMHO, it might be better to use [ for() and break ] or  [ while (test) ]
  Q_FOREVER
  {
    for ( int i = 1; i <= nBands; ++i )
    {
      if ( !iter->readNextRasterPart( i, iterCols, iterRows, &( blockList[i - 1] ), iterLeft, iterTop ) )
      {
        // No more parts, create VRT and return
        if ( mTiledMode )
        {
          QString vrtFilePath( mOutputUrl + '/' + vrtFileName() );
          writeVRT( vrtFilePath );
          if ( mBuildPyramidsFlag == QgsRaster::PyramidsFlagYes )
          {
            buildPyramids( vrtFilePath );
          }
        }
        else
        {
          if ( mBuildPyramidsFlag == QgsRaster::PyramidsFlagYes )
          {
            buildPyramids( mOutputUrl, destProvider );
          }
        }

        QgsDebugMsgLevel( "Done", 4 );
        return NoError; //reached last tile, bail out
      }
      // TODO: verify if NoDataConflict happened, to do that we need the whole pipe or nuller interface
    }

    if ( feedback && fileIndex < ( nParts - 1 ) )
    {
      feedback->setProgress( 100.0 * fileIndex / static_cast< double >( nParts ) );
      if ( feedback->isCanceled() )
      {
        for ( int i = 0; i < nBands; ++i )
        {
          delete blockList[i];
        }
        break;
      }
    }

    // It may happen that internal data type (dataType) is wider than destDataType
    QList<QgsRasterBlock *> destBlockList;
    for ( int i = 1; i <= nBands; ++i )
    {
      if ( srcProvider && srcProvider->dataType( i ) == destDataType )
      {
        destBlockList.push_back( blockList[i - 1] );
      }
      else
      {
        // TODO: this conversion should go to QgsRasterDataProvider::write with additional input data type param
        blockList[i - 1]->convert( destDataType );
        destBlockList.push_back( blockList[i - 1] );
      }
      blockList[i - 1] = nullptr;
    }

    if ( mTiledMode ) //write to file
    {
      QgsRasterDataProvider *partDestProvider = createPartProvider( outputExtent,
          nCols, iterCols, iterRows,
          iterLeft, iterTop, mOutputUrl,
          fileIndex, nBands, destDataType, crs );

      if ( partDestProvider )
      {
        //write data to output file. todo: loop over the data list
        for ( int i = 1; i <= nBands; ++i )
        {
          if ( destHasNoDataValueList.value( i - 1 ) )
          {
            partDestProvider->setNoDataValue( i, destNoDataValueList.value( i - 1 ) );
          }
          partDestProvider->write( destBlockList[i - 1]->bits( 0 ), i, iterCols, iterRows, 0, 0 );
          delete destBlockList[i - 1];
          addToVRT( partFileName( fileIndex ), i, iterCols, iterRows, iterLeft, iterTop );
        }
        delete partDestProvider;
      }
    }
    else if ( destProvider )
    {
      //loop over data
      for ( int i = 1; i <= nBands; ++i )
      {
        destProvider->write( destBlockList[i - 1]->bits( 0 ), i, iterCols, iterRows, iterLeft, iterTop );
        delete destBlockList[i - 1];
      }
    }
    ++fileIndex;
  }

  QgsDebugMsgLevel( "Done", 4 );
  return ( feedback && feedback->isCanceled() ) ? WriteCanceled : NoError;
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeImageRaster( QgsRasterIterator *iter, int nCols, int nRows, const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( "Entered", 4 );
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface *iface = iter->input();
  if ( !iface )
    return SourceProviderError;

  Qgis::DataType inputDataType = iface->dataType( 1 );
  if ( inputDataType != Qgis::ARGB32 && inputDataType != Qgis::ARGB32_Premultiplied )
  {
    return SourceProviderError;
  }

  iter->setMaximumTileWidth( mMaxTileWidth );
  iter->setMaximumTileHeight( mMaxTileHeight );

  void *redData = qgsMalloc( mMaxTileWidth * mMaxTileHeight );
  void *greenData = qgsMalloc( mMaxTileWidth * mMaxTileHeight );
  void *blueData = qgsMalloc( mMaxTileWidth * mMaxTileHeight );
  void *alphaData = qgsMalloc( mMaxTileWidth * mMaxTileHeight );
  int iterLeft = 0, iterTop = 0, iterCols = 0, iterRows = 0;
  int fileIndex = 0;

  //create destProvider for whole dataset here
  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( outputExtent, nCols, nRows, geoTransform, pixelSize );

  std::unique_ptr< QgsRasterDataProvider > destProvider( initOutput( nCols, nRows, crs, geoTransform, 4, Qgis::Byte ) );

  iter->startRasterRead( 1, nCols, nRows, outputExtent, feedback );

  int nParts = 0;
  if ( feedback )
  {
    int nPartsX = nCols / iter->maximumTileWidth() + 1;
    int nPartsY = nRows / iter->maximumTileHeight() + 1;
    nParts = nPartsX * nPartsY;
  }

  std::unique_ptr< QgsRasterBlock > inputBlock;
  while ( iter->readNextRasterPart( 1, iterCols, iterRows, inputBlock, iterLeft, iterTop ) )
  {
    if ( !inputBlock )
    {
      continue;
    }

    if ( feedback && fileIndex < ( nParts - 1 ) )
    {
      feedback->setProgress( 100.0 * fileIndex / static_cast< double >( nParts ) );
      if ( feedback->isCanceled() )
      {
        break;
      }
    }

    //fill into red/green/blue/alpha channels
    qgssize nPixels = static_cast< qgssize >( iterCols ) * iterRows;
    // TODO: should be char not int? we are then copying 1 byte
    int red = 0;
    int green = 0;
    int blue = 0;
    int alpha = 255;
    for ( qgssize i = 0; i < nPixels; ++i )
    {
      QRgb c = inputBlock->color( i );
      alpha = qAlpha( c );
      red = qRed( c );
      green = qGreen( c );
      blue = qBlue( c );

      if ( inputDataType == Qgis::ARGB32_Premultiplied )
      {
        double a = alpha / 255.;
        QgsDebugMsgLevel( QString( "red = %1 green = %2 blue = %3 alpha = %4 p = %5 a = %6" ).arg( red ).arg( green ).arg( blue ).arg( alpha ).arg( static_cast< int >( c ), 0, 16 ).arg( a ), 5 );
        red /= a;
        green /= a;
        blue /= a;
      }
      memcpy( reinterpret_cast< char * >( redData ) + i, &red, 1 );
      memcpy( reinterpret_cast< char * >( greenData ) + i, &green, 1 );
      memcpy( reinterpret_cast< char * >( blueData ) + i, &blue, 1 );
      memcpy( reinterpret_cast< char * >( alphaData ) + i, &alpha, 1 );
    }

    //create output file
    if ( mTiledMode )
    {
      //delete destProvider;
      std::unique_ptr< QgsRasterDataProvider > partDestProvider( createPartProvider( outputExtent,
          nCols, iterCols, iterRows,
          iterLeft, iterTop, mOutputUrl, fileIndex,
          4, Qgis::Byte, crs ) );

      if ( partDestProvider )
      {
        //write data to output file
        partDestProvider->write( redData, 1, iterCols, iterRows, 0, 0 );
        partDestProvider->write( greenData, 2, iterCols, iterRows, 0, 0 );
        partDestProvider->write( blueData, 3, iterCols, iterRows, 0, 0 );
        partDestProvider->write( alphaData, 4, iterCols, iterRows, 0, 0 );

        addToVRT( partFileName( fileIndex ), 1, iterCols, iterRows, iterLeft, iterTop );
        addToVRT( partFileName( fileIndex ), 2, iterCols, iterRows, iterLeft, iterTop );
        addToVRT( partFileName( fileIndex ), 3, iterCols, iterRows, iterLeft, iterTop );
        addToVRT( partFileName( fileIndex ), 4, iterCols, iterRows, iterLeft, iterTop );
      }
    }
    else if ( destProvider )
    {
      destProvider->write( redData, 1, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( greenData, 2, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( blueData, 3, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( alphaData, 4, iterCols, iterRows, iterLeft, iterTop );
    }

    ++fileIndex;
  }
  destProvider.reset();

  qgsFree( redData );
  qgsFree( greenData );
  qgsFree( blueData );
  qgsFree( alphaData );

  if ( feedback )
  {
    feedback->setProgress( 100.0 );
  }

  if ( mTiledMode )
  {
    QString vrtFilePath( mOutputUrl + '/' + vrtFileName() );
    writeVRT( vrtFilePath );
    if ( mBuildPyramidsFlag == QgsRaster::PyramidsFlagYes )
    {
      buildPyramids( vrtFilePath );
    }
  }
  else
  {
    if ( mBuildPyramidsFlag == QgsRaster::PyramidsFlagYes )
    {
      buildPyramids( mOutputUrl );
    }
  }
  return ( feedback && feedback->isCanceled() ) ? WriteCanceled : NoError;
}

void QgsRasterFileWriter::addToVRT( const QString &filename, int band, int xSize, int ySize, int xOffset, int yOffset )
{
  QDomElement bandElem = mVRTBands.value( band - 1 );

  QDomElement simpleSourceElem = mVRTDocument.createElement( QStringLiteral( "SimpleSource" ) );

  //SourceFilename
  QDomElement sourceFilenameElem = mVRTDocument.createElement( QStringLiteral( "SourceFilename" ) );
  sourceFilenameElem.setAttribute( QStringLiteral( "relativeToVRT" ), QStringLiteral( "1" ) );
  QDomText sourceFilenameText = mVRTDocument.createTextNode( filename );
  sourceFilenameElem.appendChild( sourceFilenameText );
  simpleSourceElem.appendChild( sourceFilenameElem );

  //SourceBand
  QDomElement sourceBandElem = mVRTDocument.createElement( QStringLiteral( "SourceBand" ) );
  QDomText sourceBandText = mVRTDocument.createTextNode( QString::number( band ) );
  sourceBandElem.appendChild( sourceBandText );
  simpleSourceElem.appendChild( sourceBandElem );

  //SourceProperties
  QDomElement sourcePropertiesElem = mVRTDocument.createElement( QStringLiteral( "SourceProperties" ) );
  sourcePropertiesElem.setAttribute( QStringLiteral( "RasterXSize" ), xSize );
  sourcePropertiesElem.setAttribute( QStringLiteral( "RasterYSize" ), ySize );
  sourcePropertiesElem.setAttribute( QStringLiteral( "BlockXSize" ), xSize );
  sourcePropertiesElem.setAttribute( QStringLiteral( "BlockYSize" ), ySize );
  sourcePropertiesElem.setAttribute( QStringLiteral( "DataType" ), QStringLiteral( "Byte" ) );
  simpleSourceElem.appendChild( sourcePropertiesElem );

  //SrcRect
  QDomElement srcRectElem = mVRTDocument.createElement( QStringLiteral( "SrcRect" ) );
  srcRectElem.setAttribute( QStringLiteral( "xOff" ), QStringLiteral( "0" ) );
  srcRectElem.setAttribute( QStringLiteral( "yOff" ), QStringLiteral( "0" ) );
  srcRectElem.setAttribute( QStringLiteral( "xSize" ), xSize );
  srcRectElem.setAttribute( QStringLiteral( "ySize" ), ySize );
  simpleSourceElem.appendChild( srcRectElem );

  //DstRect
  QDomElement dstRectElem = mVRTDocument.createElement( QStringLiteral( "DstRect" ) );
  dstRectElem.setAttribute( QStringLiteral( "xOff" ), xOffset );
  dstRectElem.setAttribute( QStringLiteral( "yOff" ), yOffset );
  dstRectElem.setAttribute( QStringLiteral( "xSize" ), xSize );
  dstRectElem.setAttribute( QStringLiteral( "ySize" ), ySize );
  simpleSourceElem.appendChild( dstRectElem );

  bandElem.appendChild( simpleSourceElem );
}

void QgsRasterFileWriter::buildPyramids( const QString &filename, QgsRasterDataProvider *destProviderIn )
{
  QgsDebugMsgLevel( "filename = " + filename, 4 );
  // open new dataProvider so we can build pyramids with it
  QgsDataProvider::ProviderOptions providerOptions;
  QgsRasterDataProvider *destProvider = destProviderIn;
  if ( !destProvider )
  {
    destProvider = dynamic_cast< QgsRasterDataProvider * >( QgsProviderRegistry::instance()->createProvider( mOutputProviderKey, filename, providerOptions ) );
    if ( !destProvider || !destProvider->isValid() )
    {
      delete destProvider;
      return;
    }
  }

  // TODO progress report
  // TODO test mTiledMode - not tested b/c segfault at line # 289
  // connect( provider, SIGNAL( progressUpdate( int ) ), mPyramidProgress, SLOT( setValue( int ) ) );
  QList< QgsRasterPyramid> myPyramidList;
  if ( ! mPyramidsList.isEmpty() )
    myPyramidList = destProvider->buildPyramidList( mPyramidsList );
  for ( int myCounterInt = 0; myCounterInt < myPyramidList.count(); myCounterInt++ )
  {
    myPyramidList[myCounterInt].build = true;
  }

  QgsDebugMsgLevel( QString( "building pyramids : %1 pyramids, %2 resampling, %3 format, %4 options" ).arg( myPyramidList.count() ).arg( mPyramidsResampling ).arg( mPyramidsFormat ).arg( mPyramidsConfigOptions.count() ), 4 );
  // QApplication::setOverrideCursor( Qt::WaitCursor );
  QString res = destProvider->buildPyramids( myPyramidList, mPyramidsResampling,
                mPyramidsFormat, mPyramidsConfigOptions );
  // QApplication::restoreOverrideCursor();

  // TODO put this in provider or elsewhere
  if ( !res.isNull() )
  {
    QString title, message;
    if ( res == QLatin1String( "ERROR_WRITE_ACCESS" ) )
    {
      title = QObject::tr( "Building Pyramids" );
      message = QObject::tr( "Write access denied. Adjust the file permissions and try again." );
    }
    else if ( res == QLatin1String( "ERROR_WRITE_FORMAT" ) )
    {
      title = QObject::tr( "Building Pyramids" );
      message = QObject::tr( "The file was not writable. Some formats do not "
                             "support pyramid overviews. Consult the GDAL documentation if in doubt." );
    }
    else if ( res == QLatin1String( "FAILED_NOT_SUPPORTED" ) )
    {
      title = QObject::tr( "Building Pyramids" );
      message = QObject::tr( "Building pyramid overviews is not supported on this type of raster." );
    }
    else if ( res == QLatin1String( "ERROR_VIRTUAL" ) )
    {
      title = QObject::tr( "Building Pyramids" );
      message = QObject::tr( "Building pyramid overviews is not supported on this type of raster." );
    }
    QMessageBox::warning( nullptr, title, message );
    QgsDebugMsgLevel( res + " - " + message, 4 );
  }
  if ( !destProviderIn )
    delete destProvider;
}

#if 0
int QgsRasterFileWriter::pyramidsProgress( double dfComplete, const char *pszMessage, void *pData )
{
  Q_UNUSED( pszMessage );
  GDALTermProgress( dfComplete, 0, 0 );
  QProgressDialog *progressDialog = static_cast<QProgressDialog *>( pData );
  if ( pData && progressDialog->wasCanceled() )
  {
    return 0;
  }

  if ( pData )
  {
    progressDialog->setRange( 0, 100 );
    progressDialog->setValue( dfComplete * 100 );
  }
  return 1;
}
#endif

void QgsRasterFileWriter::createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem &crs, double *geoTransform, Qgis::DataType type, const QList<bool> &destHasNoDataValueList, const QList<double> &destNoDataValueList )
{
  mVRTDocument.clear();
  QDomElement VRTDatasetElem = mVRTDocument.createElement( QStringLiteral( "VRTDataset" ) );

  //xsize / ysize
  VRTDatasetElem.setAttribute( QStringLiteral( "rasterXSize" ), xSize );
  VRTDatasetElem.setAttribute( QStringLiteral( "rasterYSize" ), ySize );
  mVRTDocument.appendChild( VRTDatasetElem );

  //CRS
  QDomElement SRSElem = mVRTDocument.createElement( QStringLiteral( "SRS" ) );
  QDomText crsText = mVRTDocument.createTextNode( crs.toWkt() );
  SRSElem.appendChild( crsText );
  VRTDatasetElem.appendChild( SRSElem );

  //geotransform
  if ( geoTransform )
  {
    QDomElement geoTransformElem = mVRTDocument.createElement( QStringLiteral( "GeoTransform" ) );
    QString geoTransformString = QString::number( geoTransform[0], 'f', 6 ) + ", " + QString::number( geoTransform[1] ) + ", " + QString::number( geoTransform[2] ) +
                                 ", "  + QString::number( geoTransform[3], 'f', 6 ) + ", " + QString::number( geoTransform[4] ) + ", " + QString::number( geoTransform[5] );
    QDomText geoTransformText = mVRTDocument.createTextNode( geoTransformString );
    geoTransformElem.appendChild( geoTransformText );
    VRTDatasetElem.appendChild( geoTransformElem );
  }

  int nBands;
  if ( mMode == Raw )
  {
    nBands = mInput->bandCount();
  }
  else
  {
    nBands = 4;
  }

  QStringList colorInterp;
  colorInterp << QStringLiteral( "Red" ) << QStringLiteral( "Green" ) << QStringLiteral( "Blue" ) << QStringLiteral( "Alpha" );

  QMap<Qgis::DataType, QString> dataTypes;
  dataTypes.insert( Qgis::Byte, QStringLiteral( "Byte" ) );
  dataTypes.insert( Qgis::UInt16, QStringLiteral( "UInt16" ) );
  dataTypes.insert( Qgis::Int16, QStringLiteral( "Int16" ) );
  dataTypes.insert( Qgis::UInt32, QStringLiteral( "Int32" ) );
  dataTypes.insert( Qgis::Float32, QStringLiteral( "Float32" ) );
  dataTypes.insert( Qgis::Float64, QStringLiteral( "Float64" ) );
  dataTypes.insert( Qgis::CInt16, QStringLiteral( "CInt16" ) );
  dataTypes.insert( Qgis::CInt32, QStringLiteral( "CInt32" ) );
  dataTypes.insert( Qgis::CFloat32, QStringLiteral( "CFloat32" ) );
  dataTypes.insert( Qgis::CFloat64, QStringLiteral( "CFloat64" ) );

  for ( int i = 1; i <= nBands; i++ )
  {
    QDomElement VRTBand = mVRTDocument.createElement( QStringLiteral( "VRTRasterBand" ) );

    VRTBand.setAttribute( QStringLiteral( "band" ), QString::number( i ) );
    QString dataType = dataTypes.value( type );
    VRTBand.setAttribute( QStringLiteral( "dataType" ), dataType );

    if ( mMode == Image )
    {
      VRTBand.setAttribute( QStringLiteral( "dataType" ), QStringLiteral( "Byte" ) );
      QDomElement colorInterpElement = mVRTDocument.createElement( QStringLiteral( "ColorInterp" ) );
      QDomText interpText = mVRTDocument.createTextNode( colorInterp.value( i - 1 ) );
      colorInterpElement.appendChild( interpText );
      VRTBand.appendChild( colorInterpElement );
    }

    if ( !destHasNoDataValueList.isEmpty() && destHasNoDataValueList.value( i - 1 ) )
    {
      VRTBand.setAttribute( QStringLiteral( "NoDataValue" ), QString::number( destNoDataValueList.value( i - 1 ) ) );
    }

    mVRTBands.append( VRTBand );
    VRTDatasetElem.appendChild( VRTBand );
  }
}

bool QgsRasterFileWriter::writeVRT( const QString &file )
{
  QFile outputFile( file );
  if ( ! outputFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return false;
  }

  QTextStream outStream( &outputFile );
  mVRTDocument.save( outStream, 2 );
  return true;
}

QgsRasterDataProvider *QgsRasterFileWriter::createPartProvider( const QgsRectangle &extent, int nCols, int iterCols,
    int iterRows, int iterLeft, int iterTop, const QString &outputUrl, int fileIndex, int nBands, Qgis::DataType type,
    const QgsCoordinateReferenceSystem &crs )
{
  double mup = extent.width() / nCols;
  double mapLeft = extent.xMinimum() + iterLeft * mup;
  double mapRight = mapLeft + mup * iterCols;
  double mapTop = extent.yMaximum() - iterTop * mup;
  double mapBottom = mapTop - iterRows * mup;
  QgsRectangle mapRect( mapLeft, mapBottom, mapRight, mapTop );

  QString outputFile = outputUrl + '/' + partFileName( fileIndex );

  //geotransform
  double geoTransform[6];
  geoTransform[0] = mapRect.xMinimum();
  geoTransform[1] = mup;
  geoTransform[2] = 0.0;
  geoTransform[3] = mapRect.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -mup;

  // perhaps we need a separate createOptions for tiles ?

  QgsRasterDataProvider *destProvider = QgsRasterDataProvider::create( mOutputProviderKey, outputFile, mOutputFormat, nBands, type, iterCols, iterRows, geoTransform, crs, mCreateOptions );

  // TODO: return provider and report error
  return destProvider;
}

QgsRasterDataProvider *QgsRasterFileWriter::initOutput( int nCols, int nRows, const QgsCoordinateReferenceSystem &crs,
    double *geoTransform, int nBands, Qgis::DataType type,
    const QList<bool> &destHasNoDataValueList, const QList<double> &destNoDataValueList )
{
  if ( mTiledMode )
  {
    createVRT( nCols, nRows, crs, geoTransform, type, destHasNoDataValueList, destNoDataValueList );
    return nullptr;
  }
  else
  {
#if 0
    // TODO enable "use existing", has no effect for now, because using Create() in gdal provider
    // should this belong in provider? should also test that source provider is gdal
    if ( mBuildPyramidsFlag == -4 && mOutputProviderKey == "gdal" && mOutputFormat.compare( QLatin1String( "gtiff" ), Qt::CaseInsensitive ) == 0 )
      mCreateOptions << "COPY_SRC_OVERVIEWS=YES";
#endif

    QgsRasterDataProvider *destProvider = QgsRasterDataProvider::create( mOutputProviderKey, mOutputUrl, mOutputFormat, nBands, type, nCols, nRows, geoTransform, crs, mCreateOptions );

    if ( !destProvider )
    {
      QgsDebugMsg( "No provider created" );
    }

    return destProvider;
  }
}

void QgsRasterFileWriter::globalOutputParameters( const QgsRectangle &extent, int nCols, int &nRows,
    double *geoTransform, double &pixelSize )
{
  pixelSize = extent.width() / nCols;

  //calculate nRows automatically for providers without exact resolution
  if ( nRows < 0 )
  {
    nRows = static_cast< double >( nCols ) / extent.width() * extent.height() + 0.5; //NOLINT
  }
  geoTransform[0] = extent.xMinimum();
  geoTransform[1] = pixelSize;
  geoTransform[2] = 0.0;
  geoTransform[3] = extent.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -( extent.height() / nRows );
}

QString QgsRasterFileWriter::partFileName( int fileIndex )
{
  // .tif for now
  QFileInfo outputInfo( mOutputUrl );
  return QStringLiteral( "%1.%2.tif" ).arg( outputInfo.fileName() ).arg( fileIndex );
}

QString QgsRasterFileWriter::vrtFileName()
{
  QFileInfo outputInfo( mOutputUrl );
  return QStringLiteral( "%1.vrt" ).arg( outputInfo.fileName() );
}

QString QgsRasterFileWriter::driverForExtension( const QString &extension )
{
  QString ext = extension.trimmed();
  if ( ext.isEmpty() )
    return QString();

  if ( ext.startsWith( '.' ) )
    ext.remove( 0, 1 );

  GDALAllRegister();
  int const drvCount = GDALGetDriverCount();

  for ( int i = 0; i < drvCount; ++i )
  {
    GDALDriverH drv = GDALGetDriver( i );
    if ( drv )
    {
      char **driverMetadata = GDALGetMetadata( drv, nullptr );
      if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_RASTER, false ) )
      {
        QString drvName = GDALGetDriverShortName( drv );
        QStringList driverExtensions = QString( GDALGetMetadataItem( drv, GDAL_DMD_EXTENSIONS, nullptr ) ).split( ' ' );

        Q_FOREACH ( const QString &driver, driverExtensions )
        {
          if ( driver.compare( ext, Qt::CaseInsensitive ) == 0 )
            return drvName;
        }
      }
    }
  }
  return QString();
}

QStringList QgsRasterFileWriter::extensionsForFormat( const QString &format )
{
  GDALDriverH drv = GDALGetDriverByName( format.toLocal8Bit().data() );
  if ( drv )
  {
    char **driverMetadata = GDALGetMetadata( drv, nullptr );
    if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_RASTER, false ) )
    {
      return QString( GDALGetMetadataItem( drv, GDAL_DMD_EXTENSIONS, nullptr ) ).split( ' ' );
    }
  }
  return QStringList();
}

QString QgsRasterFileWriter::filterForDriver( const QString &driverName )
{
  GDALDriverH drv = GDALGetDriverByName( driverName.toLocal8Bit().data() );
  if ( drv )
  {
    QString drvName = GDALGetDriverLongName( drv );
    QString extensionsString = QString( GDALGetMetadataItem( drv, GDAL_DMD_EXTENSIONS, nullptr ) );
    if ( extensionsString.isEmpty() )
    {
      return QString();
    }
    QStringList extensions = extensionsString.split( ' ' );
    QString filter = drvName + " (";
    for ( const QString &ext : extensions )
    {
      filter.append( QStringLiteral( "*.%1 *.%2 " ).arg( ext.toLower(), ext.toUpper() ) );
    }
    filter = filter.trimmed().append( QStringLiteral( ")" ) );
    return filter;
  }

  return QString();
}

QList< QgsRasterFileWriter::FilterFormatDetails > QgsRasterFileWriter::supportedFiltersAndFormats( RasterFormatOptions options )
{
  QList< FilterFormatDetails > results;

  GDALAllRegister();
  int const drvCount = GDALGetDriverCount();

  FilterFormatDetails tifFormat;

  for ( int i = 0; i < drvCount; ++i )
  {
    GDALDriverH drv = GDALGetDriver( i );
    if ( drv )
    {
      if ( QgsGdalUtils::supportsRasterCreate( drv ) )
      {
        QString drvName = GDALGetDriverShortName( drv );
        QString filterString = filterForDriver( drvName );
        if ( filterString.isEmpty() )
          continue;

        FilterFormatDetails details;
        details.driverName = drvName;
        details.filterString = filterString;

        if ( options & SortRecommended )
        {
          if ( drvName == QLatin1String( "GTiff" ) )
          {
            tifFormat = details;
            continue;
          }
        }

        results << details;
      }
    }
  }

  std::sort( results.begin(), results.end(), []( const FilterFormatDetails & a, const FilterFormatDetails & b ) -> bool
  {
    return a.driverName < b.driverName;
  } );

  if ( options & SortRecommended )
  {
    if ( !tifFormat.filterString.isEmpty() )
    {
      results.insert( 0, tifFormat );
    }
  }

  return results;
}

QStringList QgsRasterFileWriter::supportedFormatExtensions( const RasterFormatOptions options )
{
  const auto formats = supportedFiltersAndFormats( options );
  QStringList extensions;

  QRegularExpression rx( QStringLiteral( "\\*\\.([a-zA-Z0-9]*)" ) );

  for ( const FilterFormatDetails &format : formats )
  {
    QString ext = format.filterString;
    QRegularExpressionMatch match = rx.match( ext );
    if ( !match.hasMatch() )
      continue;

    QString matched = match.captured( 1 );
    extensions << matched;
  }
  return extensions;
}
