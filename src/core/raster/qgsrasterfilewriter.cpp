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

#include "qgsrasterfilewriter.h"
#include "qgsproviderregistry.h"
#include "qgsrasterinterface.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterprojector.h"

#include <QCoreApplication>
#include <QProgressDialog>
#include <QTextStream>
#include <QMessageBox>

QgsRasterFileWriter::QgsRasterFileWriter( const QString& outputUrl )
    : mMode( Raw )
    , mOutputUrl( outputUrl )
    , mOutputProviderKey( "gdal" )
    , mOutputFormat( "GTiff" )
    , mTiledMode( false )
    , mMaxTileWidth( 500 )
    , mMaxTileHeight( 500 )
    , mBuildPyramidsFlag( QgsRaster::PyramidsFlagNo )
    , mPyramidsFormat( QgsRaster::PyramidsGTiff )
    , mProgressDialog( nullptr )
    , mPipe( nullptr )
    , mInput( nullptr )
{

}

QgsRasterFileWriter::QgsRasterFileWriter()
    : mMode( Raw )
    , mOutputProviderKey( "gdal" )
    , mOutputFormat( "GTiff" )
    , mTiledMode( false )
    , mMaxTileWidth( 500 )
    , mMaxTileHeight( 500 )
    , mBuildPyramidsFlag( QgsRaster::PyramidsFlagNo )
    , mPyramidsFormat( QgsRaster::PyramidsGTiff )
    , mProgressDialog( nullptr )
    , mPipe( nullptr )
    , mInput( nullptr )
{

}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( const QgsRasterPipe* pipe, int nCols, int nRows, QgsRectangle outputExtent,
    const QgsCoordinateReferenceSystem& crs, QProgressDialog* progressDialog )
{
  QgsDebugMsgLevel( "Entered", 4 );

  if ( !pipe )
  {
    return SourceProviderError;
  }
  mPipe = pipe;

  //const QgsRasterInterface* iface = iter->input();
  const QgsRasterInterface* iface = pipe->last();
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

  if ( !iface->srcInput() )
  {
    QgsDebugMsg( "iface->srcInput() == 0" );
    return SourceProviderError;
  }
#ifdef QGISDEBUG
  const QgsRasterInterface &srcInput = *iface->srcInput();
  QgsDebugMsgLevel( QString( "srcInput = %1" ).arg( typeid( srcInput ).name() ), 4 );
#endif

  mProgressDialog = progressDialog;

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

  if ( mMode == Image )
  {
    WriterError e = writeImageRaster( &iter, nCols, nRows, outputExtent, crs, progressDialog );
    mProgressDialog = nullptr;
    return e;
  }
  else
  {
    mProgressDialog = nullptr;
    WriterError e = writeDataRaster( pipe, &iter, nCols, nRows, outputExtent, crs, progressDialog );
    return e;
  }
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeDataRaster( const QgsRasterPipe* pipe, QgsRasterIterator* iter, int nCols, int nRows, const QgsRectangle& outputExtent,
    const QgsCoordinateReferenceSystem& crs, QProgressDialog* progressDialog )
{
  QgsDebugMsgLevel( "Entered", 4 );
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface* iface = pipe->last();
  if ( !iface )
  {
    return SourceProviderError;
  }

  QgsRasterDataProvider* srcProvider = const_cast<QgsRasterDataProvider*>( dynamic_cast<const QgsRasterDataProvider*>( iface->srcInput() ) );
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
  int dataTypeSize = QgsRasterBlock::typeSize( srcProvider->srcDataType( 1 ) );
  for ( int i = 2; i <= nBands; ++i )
  {
    if ( QgsRasterBlock::typeSize( srcProvider->srcDataType( 1 ) ) != dataTypeSize )
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
  QList<QGis::DataType> destDataTypeList;
  for ( int bandNo = 1; bandNo <= nBands; bandNo++ )
  {
    QgsRasterNuller *nuller = pipe->nuller();

    bool srcHasNoDataValue = srcProvider->srcHasNoDataValue( bandNo );
    bool destHasNoDataValue = false;
    double destNoDataValue = std::numeric_limits<double>::quiet_NaN();
    QGis::DataType destDataType = srcProvider->srcDataType( bandNo );
    // TODO: verify what happens/should happen if srcNoDataValue is disabled by setUseSrcNoDataValue
    QgsDebugMsgLevel( QString( "srcHasNoDataValue = %1 srcNoDataValue = %2" ).arg( srcHasNoDataValue ).arg( srcProvider->srcNoDataValue( bandNo ) ), 4 );
    if ( srcHasNoDataValue )
    {

      // If source has no data value, it is used by provider
      destNoDataValue = srcProvider->srcNoDataValue( bandNo );
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
      // Verify if we realy need no data value, i.e.
      QgsRectangle srcExtent = outputExtent;
      QgsRasterProjector *projector = pipe->projector();
      if ( projector && projector->destCrs() != projector->srcCrs() )
      {
        QgsCoordinateTransform ct( projector->destCrs(), projector->srcCrs() );
        srcExtent = ct.transformBoundingBox( outputExtent );
      }
      if ( !srcProvider->extent().contains( srcExtent ) )
      {
        // Destination extent is larger than source extent, we need destination no data values
        // Get src sample statistics (estimation from sample)
        QgsRasterBandStats stats = srcProvider->bandStatistics( bandNo, QgsRasterBandStats::Min | QgsRasterBandStats::Max, srcExtent, 250000 );

        // Test if we have free (not used) values
        double typeMinValue = QgsContrastEnhancement::maximumValuePossible( static_cast< QGis::DataType >( srcProvider->srcDataType( bandNo ) ) );
        double typeMaxValue = QgsContrastEnhancement::maximumValuePossible( static_cast< QGis::DataType >( srcProvider->srcDataType( bandNo ) ) );
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

  QGis::DataType destDataType = destDataTypeList.value( 0 );
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
  QgsRasterDataProvider* destProvider = nullptr;
  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( outputExtent, nCols, nRows, geoTransform, pixelSize );

  // initOutput() returns 0 in tile mode!
  destProvider = initOutput( nCols, nRows, crs, geoTransform, nBands, destDataType, destHasNoDataValueList, destNoDataValueList );

  WriterError error = writeDataRaster( pipe, iter, nCols, nRows, outputExtent, crs, destDataType, destHasNoDataValueList, destNoDataValueList, destProvider, progressDialog );

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
      QGis::DataType destDataType = QgsRasterBlock::typeWithNoDataValue( destDataTypeList.value( i ), &destNoDataValue );
      destDataTypeList.replace( i, destDataType );
      destNoDataValueList.replace( i, destNoDataValue );
    }
    destDataType =  destDataTypeList.value( 0 );

    // Try again
    destProvider = initOutput( nCols, nRows, crs, geoTransform, nBands, destDataType, destHasNoDataValueList, destNoDataValueList );
    error = writeDataRaster( pipe, iter, nCols, nRows, outputExtent, crs, destDataType, destHasNoDataValueList, destNoDataValueList, destProvider, progressDialog );
  }

  if ( destProvider )
    delete destProvider;

  return error;
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeDataRaster(
  const QgsRasterPipe* pipe,
  QgsRasterIterator* iter,
  int nCols, int nRows,
  const QgsRectangle& outputExtent,
  const QgsCoordinateReferenceSystem& crs,
  QGis::DataType destDataType,
  const QList<bool>& destHasNoDataValueList,
  const QList<double>& destNoDataValueList,
  QgsRasterDataProvider* destProvider,
  QProgressDialog* progressDialog )
{
  Q_UNUSED( pipe );
  Q_UNUSED( destHasNoDataValueList );
  QgsDebugMsgLevel( "Entered", 4 );

  const QgsRasterInterface* iface = iter->input();
  const QgsRasterDataProvider *srcProvider = dynamic_cast<const QgsRasterDataProvider*>( iface->srcInput() );
  int nBands = iface->bandCount();
  QgsDebugMsgLevel( QString( "nBands = %1" ).arg( nBands ), 4 );

  //Get output map units per pixel
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;

  QList<QgsRasterBlock*> blockList;
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
  if ( progressDialog )
  {
    int nPartsX = nCols / iter->maximumTileWidth() + 1;
    int nPartsY = nRows / iter->maximumTileHeight() + 1;
    nParts = nPartsX * nPartsY;
    progressDialog->setMaximum( nParts );
    progressDialog->show();
    progressDialog->setLabelText( QObject::tr( "Reading raster part %1 of %2" ).arg( fileIndex + 1 ).arg( nParts ) );
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
            buildPyramids( mOutputUrl );
          }
        }

        QgsDebugMsgLevel( "Done", 4 );
        return NoError; //reached last tile, bail out
      }
      // TODO: verify if NoDataConflict happened, to do that we need the whole pipe or nuller interface
    }

    if ( progressDialog && fileIndex < ( nParts - 1 ) )
    {
      progressDialog->setValue( fileIndex + 1 );
      progressDialog->setLabelText( QObject::tr( "Reading raster part %1 of %2" ).arg( fileIndex + 2 ).arg( nParts ) );
      QCoreApplication::processEvents( QEventLoop::AllEvents, 1000 );
      if ( progressDialog->wasCanceled() )
      {
        for ( int i = 0; i < nBands; ++i )
        {
          delete blockList[i];
        }
        break;
      }
    }

    // It may happen that internal data type (dataType) is wider than destDataType
    QList<QgsRasterBlock*> destBlockList;
    for ( int i = 1; i <= nBands; ++i )
    {
      if ( srcProvider && srcProvider->dataType( i ) == destDataType )
      {
        destBlockList.push_back( blockList[i-1] );
      }
      else
      {
        // TODO: this conversion should go to QgsRasterDataProvider::write with additional input data type param
        blockList[i-1]->convert( destDataType );
        destBlockList.push_back( blockList[i-1] );
      }
      blockList[i-1] = nullptr;
    }

    if ( mTiledMode ) //write to file
    {
      QgsRasterDataProvider* partDestProvider = createPartProvider( outputExtent,
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
  return NoError;
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeImageRaster( QgsRasterIterator* iter, int nCols, int nRows, const QgsRectangle& outputExtent,
    const QgsCoordinateReferenceSystem& crs, QProgressDialog* progressDialog )
{
  QgsDebugMsgLevel( "Entered", 4 );
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface* iface = iter->input();
  if ( !iface )
    return SourceProviderError;

  QGis::DataType inputDataType = iface->dataType( 1 );
  if ( inputDataType != QGis::ARGB32 && inputDataType != QGis::ARGB32_Premultiplied )
  {
    return SourceProviderError;
  }

  iter->setMaximumTileWidth( mMaxTileWidth );
  iter->setMaximumTileHeight( mMaxTileHeight );

  void* redData = qgsMalloc( mMaxTileWidth * mMaxTileHeight );
  void* greenData = qgsMalloc( mMaxTileWidth * mMaxTileHeight );
  void* blueData = qgsMalloc( mMaxTileWidth * mMaxTileHeight );
  void* alphaData = qgsMalloc( mMaxTileWidth * mMaxTileHeight );
  QgsRectangle mapRect;
  int iterLeft = 0, iterTop = 0, iterCols = 0, iterRows = 0;
  int fileIndex = 0;

  //create destProvider for whole dataset here
  QgsRasterDataProvider* destProvider = nullptr;
  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( outputExtent, nCols, nRows, geoTransform, pixelSize );

  destProvider = initOutput( nCols, nRows, crs, geoTransform, 4, QGis::Byte );

  iter->startRasterRead( 1, nCols, nRows, outputExtent );

  int nParts = 0;
  if ( progressDialog )
  {
    int nPartsX = nCols / iter->maximumTileWidth() + 1;
    int nPartsY = nRows / iter->maximumTileHeight() + 1;
    nParts = nPartsX * nPartsY;
    progressDialog->setMaximum( nParts );
    progressDialog->show();
    progressDialog->setLabelText( QObject::tr( "Reading raster part %1 of %2" ).arg( fileIndex + 1 ).arg( nParts ) );
  }

  QgsRasterBlock *inputBlock = nullptr;
  while ( iter->readNextRasterPart( 1, iterCols, iterRows, &inputBlock, iterLeft, iterTop ) )
  {
    if ( !inputBlock )
    {
      continue;
    }

    if ( progressDialog && fileIndex < ( nParts - 1 ) )
    {
      progressDialog->setValue( fileIndex + 1 );
      progressDialog->setLabelText( QObject::tr( "Reading raster part %1 of %2" ).arg( fileIndex + 2 ).arg( nParts ) );
      QCoreApplication::processEvents( QEventLoop::AllEvents, 1000 );
      if ( progressDialog->wasCanceled() )
      {
        delete inputBlock;
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

      if ( inputDataType == QGis::ARGB32_Premultiplied )
      {
        double a = alpha / 255.;
        QgsDebugMsgLevel( QString( "red = %1 green = %2 blue = %3 alpha = %4 p = %5 a = %6" ).arg( red ).arg( green ).arg( blue ).arg( alpha ).arg( static_cast< int >( c ), 0, 16 ).arg( a ), 5 );
        red /= a;
        green /= a;
        blue /= a;
      }
      memcpy( reinterpret_cast< char* >( redData ) + i, &red, 1 );
      memcpy( reinterpret_cast< char* >( greenData ) + i, &green, 1 );
      memcpy( reinterpret_cast< char* >( blueData ) + i, &blue, 1 );
      memcpy( reinterpret_cast< char* >( alphaData ) + i, &alpha, 1 );
    }
    delete inputBlock;

    //create output file
    if ( mTiledMode )
    {
      //delete destProvider;
      QgsRasterDataProvider* partDestProvider = createPartProvider( outputExtent,
          nCols, iterCols, iterRows,
          iterLeft, iterTop, mOutputUrl, fileIndex,
          4, QGis::Byte, crs );

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
        delete partDestProvider;
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

  if ( destProvider )
    delete destProvider;

  qgsFree( redData );
  qgsFree( greenData );
  qgsFree( blueData );
  qgsFree( alphaData );

  if ( progressDialog )
  {
    progressDialog->setValue( progressDialog->maximum() );
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
  return NoError;
}

void QgsRasterFileWriter::addToVRT( const QString& filename, int band, int xSize, int ySize, int xOffset, int yOffset )
{
  QDomElement bandElem = mVRTBands.value( band - 1 );

  QDomElement simpleSourceElem = mVRTDocument.createElement( "SimpleSource" );

  //SourceFilename
  QDomElement sourceFilenameElem = mVRTDocument.createElement( "SourceFilename" );
  sourceFilenameElem.setAttribute( "relativeToVRT", "1" );
  QDomText sourceFilenameText = mVRTDocument.createTextNode( filename );
  sourceFilenameElem.appendChild( sourceFilenameText );
  simpleSourceElem.appendChild( sourceFilenameElem );

  //SourceBand
  QDomElement sourceBandElem = mVRTDocument.createElement( "SourceBand" );
  QDomText sourceBandText = mVRTDocument.createTextNode( QString::number( band ) );
  sourceBandElem.appendChild( sourceBandText );
  simpleSourceElem.appendChild( sourceBandElem );

  //SourceProperties
  QDomElement sourcePropertiesElem = mVRTDocument.createElement( "SourceProperties" );
  sourcePropertiesElem.setAttribute( "RasterXSize", xSize );
  sourcePropertiesElem.setAttribute( "RasterYSize", ySize );
  sourcePropertiesElem.setAttribute( "BlockXSize", xSize );
  sourcePropertiesElem.setAttribute( "BlockYSize", ySize );
  sourcePropertiesElem.setAttribute( "DataType", "Byte" );
  simpleSourceElem.appendChild( sourcePropertiesElem );

  //SrcRect
  QDomElement srcRectElem = mVRTDocument.createElement( "SrcRect" );
  srcRectElem.setAttribute( "xOff", "0" );
  srcRectElem.setAttribute( "yOff", "0" );
  srcRectElem.setAttribute( "xSize", xSize );
  srcRectElem.setAttribute( "ySize", ySize );
  simpleSourceElem.appendChild( srcRectElem );

  //DstRect
  QDomElement dstRectElem = mVRTDocument.createElement( "DstRect" );
  dstRectElem.setAttribute( "xOff", xOffset );
  dstRectElem.setAttribute( "yOff", yOffset );
  dstRectElem.setAttribute( "xSize", xSize );
  dstRectElem.setAttribute( "ySize", ySize );
  simpleSourceElem.appendChild( dstRectElem );

  bandElem.appendChild( simpleSourceElem );
}

#if 0
void QgsRasterFileWriter::buildPyramids( const QString& filename )
{
  GDALDatasetH dataSet;
  GDALAllRegister();
  dataSet = GDALOpen( filename.toLocal8Bit().data(), GA_Update );
  if ( !dataSet )
  {
    return;
  }

  //2,4,8,16,32,64
  int overviewList[6];
  overviewList[0] = 2;
  overviewList[1] = 4;
  overviewList[2] = 8;
  overviewList[3] = 16;
  overviewList[4] = 32;
  overviewList[5] = 64;

#if 0
  if ( mProgressDialog )
  {
    mProgressDialog->setLabelText( QObject::tr( "Building Pyramids..." ) );
    mProgressDialog->setValue( 0 );
    mProgressDialog->setWindowModality( Qt::WindowModal );
    mProgressDialog->show();
  }
#endif
  GDALBuildOverviews( dataSet, "AVERAGE", 6, overviewList, 0, 0, /*pyramidsProgress*/ 0, /*mProgressDialog*/ 0 );
}
#endif

void QgsRasterFileWriter::buildPyramids( const QString& filename )
{
  QgsDebugMsgLevel( "filename = " + filename, 4 );
  // open new dataProvider so we can build pyramids with it
  QgsRasterDataProvider* destProvider = dynamic_cast< QgsRasterDataProvider* >( QgsProviderRegistry::instance()->provider( mOutputProviderKey, filename ) );
  if ( !destProvider )
  {
    return;
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
    if ( res == "ERROR_WRITE_ACCESS" )
    {
      title = QObject::tr( "Building pyramids failed - write access denied" );
      message = QObject::tr( "Write access denied. Adjust the file permissions and try again." );
    }
    else if ( res == "ERROR_WRITE_FORMAT" )
    {
      title = QObject::tr( "Building pyramids failed." );
      message = QObject::tr( "The file was not writable. Some formats do not "
                             "support pyramid overviews. Consult the GDAL documentation if in doubt." );
    }
    else if ( res == "FAILED_NOT_SUPPORTED" )
    {
      title = QObject::tr( "Building pyramids failed." );
      message = QObject::tr( "Building pyramid overviews is not supported on this type of raster." );
    }
    else if ( res == "ERROR_JPEG_COMPRESSION" )
    {
      title = QObject::tr( "Building pyramids failed." );
      message = QObject::tr( "Building internal pyramid overviews is not supported on raster layers with JPEG compression and your current libtiff library." );
    }
    else if ( res == "ERROR_VIRTUAL" )
    {
      title = QObject::tr( "Building pyramids failed." );
      message = QObject::tr( "Building pyramid overviews is not supported on this type of raster." );
    }
    QMessageBox::warning( nullptr, title, message );
    QgsDebugMsgLevel( res + " - " + message, 4 );
  }
  delete destProvider;
}

#if 0
int QgsRasterFileWriter::pyramidsProgress( double dfComplete, const char *pszMessage, void* pData )
{
  Q_UNUSED( pszMessage );
  GDALTermProgress( dfComplete, 0, 0 );
  QProgressDialog* progressDialog = static_cast<QProgressDialog*>( pData );
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

void QgsRasterFileWriter::createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem& crs, double* geoTransform, QGis::DataType type, const QList<bool>& destHasNoDataValueList, const QList<double>& destNoDataValueList )
{
  mVRTDocument.clear();
  QDomElement VRTDatasetElem = mVRTDocument.createElement( "VRTDataset" );

  //xsize / ysize
  VRTDatasetElem.setAttribute( "rasterXSize", xSize );
  VRTDatasetElem.setAttribute( "rasterYSize", ySize );
  mVRTDocument.appendChild( VRTDatasetElem );

  //CRS
  QDomElement SRSElem = mVRTDocument.createElement( "SRS" );
  QDomText crsText = mVRTDocument.createTextNode( crs.toWkt() );
  SRSElem.appendChild( crsText );
  VRTDatasetElem.appendChild( SRSElem );

  //geotransform
  if ( geoTransform )
  {
    QDomElement geoTransformElem = mVRTDocument.createElement( "GeoTransform" );
    QString geoTransformString = QString::number( geoTransform[0] ) + ", " + QString::number( geoTransform[1] ) + ", " + QString::number( geoTransform[2] ) +
                                 ", "  + QString::number( geoTransform[3] ) + ", " + QString::number( geoTransform[4] ) + ", " + QString::number( geoTransform[5] );
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
  colorInterp << "Red" << "Green" << "Blue" << "Alpha";

  QMap<QGis::DataType, QString> dataTypes;
  dataTypes.insert( QGis::Byte, "Byte" );
  dataTypes.insert( QGis::UInt16, "UInt16" );
  dataTypes.insert( QGis::Int16, "Int16" );
  dataTypes.insert( QGis::UInt32, "Int32" );
  dataTypes.insert( QGis::Float32, "Float32" );
  dataTypes.insert( QGis::Float64, "Float64" );
  dataTypes.insert( QGis::CInt16, "CInt16" );
  dataTypes.insert( QGis::CInt32, "CInt32" );
  dataTypes.insert( QGis::CFloat32, "CFloat32" );
  dataTypes.insert( QGis::CFloat64, "CFloat64" );

  for ( int i = 1; i <= nBands; i++ )
  {
    QDomElement VRTBand = mVRTDocument.createElement( "VRTRasterBand" );

    VRTBand.setAttribute( "band", QString::number( i ) );
    QString dataType = dataTypes.value( type );
    VRTBand.setAttribute( "dataType", dataType );

    if ( mMode == Image )
    {
      VRTBand.setAttribute( "dataType", "Byte" );
      QDomElement colorInterpElement = mVRTDocument.createElement( "ColorInterp" );
      QDomText interpText = mVRTDocument.createTextNode( colorInterp.value( i - 1 ) );
      colorInterpElement.appendChild( interpText );
      VRTBand.appendChild( colorInterpElement );
    }

    if ( !destHasNoDataValueList.isEmpty() && destHasNoDataValueList.value( i - 1 ) )
    {
      VRTBand.setAttribute( "NoDataValue", QString::number( destNoDataValueList.value( i - 1 ) ) );
    }

    mVRTBands.append( VRTBand );
    VRTDatasetElem.appendChild( VRTBand );
  }
}

bool QgsRasterFileWriter::writeVRT( const QString& file )
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

QgsRasterDataProvider* QgsRasterFileWriter::createPartProvider( const QgsRectangle& extent, int nCols, int iterCols,
    int iterRows, int iterLeft, int iterTop, const QString& outputUrl, int fileIndex, int nBands, QGis::DataType type,
    const QgsCoordinateReferenceSystem& crs )
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

  QgsRasterDataProvider* destProvider = QgsRasterDataProvider::create( mOutputProviderKey, outputFile, mOutputFormat, nBands, type, iterCols, iterRows, geoTransform, crs, mCreateOptions );

  // TODO: return provider and report error
  return destProvider;
}

QgsRasterDataProvider* QgsRasterFileWriter::initOutput( int nCols, int nRows, const QgsCoordinateReferenceSystem& crs,
    double* geoTransform, int nBands, QGis::DataType type,
    const QList<bool>& destHasNoDataValueList, const QList<double>& destNoDataValueList )
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
    if ( mBuildPyramidsFlag == -4 && mOutputProviderKey == "gdal" && mOutputFormat.toLower() == "gtiff" )
      mCreateOptions << "COPY_SRC_OVERVIEWS=YES";
#endif

    QgsRasterDataProvider* destProvider = QgsRasterDataProvider::create( mOutputProviderKey, mOutputUrl, mOutputFormat, nBands, type, nCols, nRows, geoTransform, crs, mCreateOptions );

    if ( !destProvider )
    {
      QgsDebugMsg( "No provider created" );
    }

    return destProvider;
  }
}

void QgsRasterFileWriter::globalOutputParameters( const QgsRectangle& extent, int nCols, int& nRows,
    double* geoTransform, double& pixelSize )
{
  pixelSize = extent.width() / nCols;

  //calculate nRows automatically for providers without exact resolution
  if ( nRows < 0 )
  {
    nRows = static_cast< double >( nCols ) / extent.width() * extent.height() + 0.5;
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
  return QString( "%1.%2.tif" ).arg( outputInfo.fileName() ).arg( fileIndex );
}

QString QgsRasterFileWriter::vrtFileName()
{
  QFileInfo outputInfo( mOutputUrl );
  return QString( "%1.vrt" ).arg( outputInfo.fileName() );
}
