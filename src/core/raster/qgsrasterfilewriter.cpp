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
#include "qgsreadwritelocker.h"
#include "qgsrasterpipe.h"

#include <QCoreApplication>
#include <QProgressDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QRegularExpression>

#include <cmath>

#include <gdal.h>
#include <cpl_string.h>
#include <mutex>

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


// Deprecated!
QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs, QgsRasterBlockFeedback *feedback )
{
  return writeRaster( pipe, nCols, nRows, outputExtent, crs, ( pipe && pipe->provider() ) ? pipe->provider()->transformContext() : QgsCoordinateTransformContext(), feedback );
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext,
    QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );

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

  QgsDebugMsgLevel( QStringLiteral( "reading from %1" ).arg( typeid( *iface ).name() ), 4 );

  if ( !iface->sourceInput() )
  {
    QgsDebugMsg( QStringLiteral( "iface->srcInput() == 0" ) );
    return SourceProviderError;
  }
#ifdef QGISDEBUG
  const QgsRasterInterface &srcInput = *iface->sourceInput();
  QgsDebugMsgLevel( QStringLiteral( "srcInput = %1" ).arg( typeid( srcInput ).name() ), 4 );
#endif

  mFeedback = feedback;

  QgsRasterIterator iter( pipe->last() );

  //create directory for output files
  if ( mTiledMode )
  {
    const QFileInfo fileInfo( mOutputUrl );
    if ( !fileInfo.exists() )
    {
      const QDir dir = fileInfo.dir();
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
    const WriterError e = writeImageRaster( &iter, nCols, nRows, outputExtent, crs, feedback );
    return e;
  }
  else
  {
    const WriterError e = writeDataRaster( pipe, &iter, nCols, nRows, outputExtent, crs, transformContext, feedback );
    return e;
  }
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeDataRaster( const QgsRasterPipe *pipe, QgsRasterIterator *iter, int nCols, int nRows, const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
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
    QgsDebugMsg( QStringLiteral( "Cannot get source data provider" ) );
    return SourceProviderError;
  }

  iter->setMaximumTileWidth( mMaxTileWidth );
  iter->setMaximumTileHeight( mMaxTileHeight );

  const int nBands = iface->bandCount();
  if ( nBands < 1 )
  {
    return SourceProviderError;
  }


  //check if all the bands have the same data type size, otherwise we cannot write it to the provider
  //(at least not with the current interface)
  const int dataTypeSize = QgsRasterBlock::typeSize( srcProvider->sourceDataType( 1 ) );
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

  const bool isGpkgOutput = mOutputProviderKey == "gdal" &&
                            mOutputFormat.compare( QLatin1String( "gpkg" ), Qt::CaseInsensitive ) == 0;
  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( outputExtent, nCols, nRows, geoTransform, pixelSize );
  const auto srcProviderExtent( srcProvider->extent() );

  for ( int bandNo = 1; bandNo <= nBands; bandNo++ )
  {
    QgsRasterNuller *nuller = pipe->nuller();

    const bool srcHasNoDataValue = srcProvider->sourceHasNoDataValue( bandNo );
    bool destHasNoDataValue = false;
    double destNoDataValue = std::numeric_limits<double>::quiet_NaN();
    const Qgis::DataType srcDataType = srcProvider->sourceDataType( bandNo );
    Qgis::DataType destDataType = srcDataType;
    // TODO: verify what happens/should happen if srcNoDataValue is disabled by setUseSrcNoDataValue
    QgsDebugMsgLevel( QStringLiteral( "srcHasNoDataValue = %1 srcNoDataValue = %2" ).arg( srcHasNoDataValue ).arg( srcProvider->sourceNoDataValue( bandNo ) ), 4 );
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
    // GeoPackage does not support nodata for Byte output, and does not
    // support non-Byte multiband output, so do not take the risk of an accidental
    // data type promotion.
    else if ( !( isGpkgOutput && destDataType == Qgis::DataType::Byte ) )
    {
      // Verify if we really need no data value, i.e.
      QgsRectangle outputExtentInSrcCrs = outputExtent;
      QgsRasterProjector *projector = pipe->projector();
      if ( projector && projector->destinationCrs() != projector->sourceCrs() )
      {
        QgsCoordinateTransform ct( projector->destinationCrs(), projector->sourceCrs(), transformContext );
        ct.setBallparkTransformsAreAppropriate( true );
        outputExtentInSrcCrs = ct.transformBoundingBox( outputExtent );
      }
      if ( !srcProviderExtent.contains( outputExtentInSrcCrs ) &&
           ( std::fabs( srcProviderExtent.xMinimum() - outputExtentInSrcCrs.xMinimum() ) > geoTransform[1] / 2 ||
             std::fabs( srcProviderExtent.xMaximum() - outputExtentInSrcCrs.xMaximum() ) > geoTransform[1] / 2 ||
             std::fabs( srcProviderExtent.yMinimum() - outputExtentInSrcCrs.yMinimum() ) > std::fabs( geoTransform[5] ) / 2 ||
             std::fabs( srcProviderExtent.yMaximum() - outputExtentInSrcCrs.yMaximum() ) > std::fabs( geoTransform[5] ) / 2 ) )
      {
        // Destination extent is (at least partially) outside of source extent, we need destination no data values
        // Get src sample statistics (estimation from sample)
        const QgsRasterBandStats stats = srcProvider->bandStatistics( bandNo, QgsRasterBandStats::Min | QgsRasterBandStats::Max, outputExtentInSrcCrs, 250000 );

        // Test if we have free (not used) values
        const double typeMinValue = QgsContrastEnhancement::minimumValuePossible( srcDataType );
        const double typeMaxValue = QgsContrastEnhancement::maximumValuePossible( srcDataType );
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

    QgsDebugMsgLevel( QStringLiteral( "bandNo = %1 destDataType = %2 destHasNoDataValue = %3 destNoDataValue = %4" ).arg( bandNo ).arg( qgsEnumValueToKey( destDataType ) ).arg( destHasNoDataValue ).arg( destNoDataValue ), 4 );
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

  WriterError error;
  for ( int attempt = 0; attempt < 2; attempt ++ )
  {
    //create destProvider for whole dataset here
    // initOutput() returns 0 in tile mode!
    std::unique_ptr<QgsRasterDataProvider> destProvider(
      initOutput( nCols, nRows, crs, geoTransform, nBands, destDataType, destHasNoDataValueList, destNoDataValueList ) );
    if ( !mTiledMode )
    {
      if ( !destProvider )
      {
        return CreateDatasourceError;
      }
      if ( !destProvider->isValid() )
      {
        if ( feedback && !destProvider->error().isEmpty() )
        {
          feedback->appendError( destProvider->error().summary() );
        }
        return CreateDatasourceError;
      }
      if ( nCols != destProvider->xSize() || nRows != destProvider->ySize() )
      {
        QgsDebugMsg( QStringLiteral( "Created raster does not have requested dimensions" ) );
        if ( feedback )
        {
          feedback->appendError( QObject::tr( "Created raster does not have requested dimensions" ) );
        }
        return CreateDatasourceError;
      }
      if ( nBands != destProvider->bandCount() )
      {
        QgsDebugMsg( QStringLiteral( "Created raster does not have requested band count" ) );
        if ( feedback )
        {
          feedback->appendError( QObject::tr( "Created raster does not have requested band count" ) );
        }
        return CreateDatasourceError;
      }
      if ( nBands )
      {
        // Some driver like GS7BG may accept Byte as requested data type,
        // but actually return a driver with Float64...
        destDataType = destProvider->dataType( 1 );
      }
    }

    error = writeDataRaster( pipe, iter, nCols, nRows, outputExtent, crs, destDataType, destHasNoDataValueList, destNoDataValueList, destProvider.get(), feedback );

    if ( attempt == 0 && error == NoDataConflict )
    {
      // The value used for no data was found in source data, we must use wider data type
      if ( destProvider ) // no tiles
      {
        destProvider->remove();
        destProvider.reset();
      }
      else // VRT
      {
        // TODO: remove created VRT
      }

      // But we don't know which band -> wider all
      for ( int i = 0; i < nBands; i++ )
      {
        double destNoDataValue;
        const Qgis::DataType destDataType = QgsRasterBlock::typeWithNoDataValue( destDataTypeList.value( i ), &destNoDataValue );
        destDataTypeList.replace( i, destDataType );
        destNoDataValueList.replace( i, destNoDataValue );
      }
      destDataType = destDataTypeList.value( 0 );

      // Try again
    }
    else
    {
      break;
    }
  }

  return error;
}

static int qgsDivRoundUp( int a, int b )
{
  return a / b + ( ( ( a % b ) != 0 ) ? 1 : 0 );
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
  Q_UNUSED( pipe )
  Q_UNUSED( destHasNoDataValueList )
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );

  const QgsRasterInterface *iface = iter->input();
  const QgsRasterDataProvider *srcProvider = dynamic_cast<const QgsRasterDataProvider *>( iface->sourceInput() );
  const int nBands = iface->bandCount();
  QgsDebugMsgLevel( QStringLiteral( "nBands = %1" ).arg( nBands ), 4 );

  //Get output map units per pixel
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;

  std::vector< std::unique_ptr<QgsRasterBlock> > blockList;
  std::vector< std::unique_ptr<QgsRasterBlock> > destBlockList;

  blockList.resize( nBands );
  destBlockList.resize( nBands );

  for ( int i = 1; i <= nBands; ++i )
  {
    iter->startRasterRead( i, nCols, nRows, outputExtent );
    if ( destProvider && destHasNoDataValueList.value( i - 1 ) ) // no tiles
    {
      destProvider->setNoDataValue( i, destNoDataValueList.value( i - 1 ) );
    }
  }

  int nParts = 0;
  int fileIndex = 0;
  if ( feedback )
  {
    const int nPartsX = qgsDivRoundUp( nCols, iter->maximumTileWidth() );
    const int nPartsY = qgsDivRoundUp( nRows, iter->maximumTileHeight() );
    nParts = nPartsX * nPartsY;
  }

  // hmm why is there a for(;;) here ..
  // not good coding practice IMHO, it might be better to use [ for() and break ] or  [ while (test) ]
  Q_FOREVER
  {
    for ( int i = 1; i <= nBands; ++i )
    {
      QgsRasterBlock *block = nullptr;
      if ( !iter->readNextRasterPart( i, iterCols, iterRows, &block, iterLeft, iterTop ) )
      {
        // No more parts, create VRT and return
        if ( mTiledMode )
        {
          const QString vrtFilePath( mOutputUrl + '/' + vrtFileName() );
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

        QgsDebugMsgLevel( QStringLiteral( "Done" ), 4 );
        return NoError; //reached last tile, bail out
      }
      blockList[i - 1].reset( block );
      // TODO: verify if NoDataConflict happened, to do that we need the whole pipe or nuller interface
    }

    if ( feedback && fileIndex < ( nParts - 1 ) )
    {
      feedback->setProgress( 100.0 * fileIndex / static_cast< double >( nParts ) );
      if ( feedback->isCanceled() )
      {
        break;
      }
    }

    // It may happen that internal data type (dataType) is wider than destDataType
    for ( int i = 1; i <= nBands; ++i )
    {
      if ( srcProvider && srcProvider->dataType( i ) == destDataType )
      {
        // nothing
      }
      else
      {
        // TODO: this conversion should go to QgsRasterDataProvider::write with additional input data type param
        blockList[i - 1]->convert( destDataType );
      }
      destBlockList[i - 1] = std::move( blockList[i - 1] );
    }

    if ( mTiledMode ) //write to file
    {
      std::unique_ptr< QgsRasterDataProvider > partDestProvider( createPartProvider( outputExtent,
          nCols, iterCols, iterRows,
          iterLeft, iterTop, mOutputUrl,
          fileIndex, nBands, destDataType, crs ) );

      if ( !partDestProvider || !partDestProvider->isValid() )
      {
        return DestProviderError;
      }

      //write data to output file. todo: loop over the data list
      for ( int i = 1; i <= nBands; ++i )
      {
        if ( destHasNoDataValueList.value( i - 1 ) )
        {
          partDestProvider->setNoDataValue( i, destNoDataValueList.value( i - 1 ) );
        }
        if ( destBlockList[ i - 1 ]->isEmpty() )
          continue;

        if ( !partDestProvider->write( destBlockList[i - 1]->bits( 0 ), i, iterCols, iterRows, 0, 0 ) )
        {
          return WriteError;
        }
        addToVRT( partFileName( fileIndex ), i, iterCols, iterRows, iterLeft, iterTop );
      }

    }
    else if ( destProvider )
    {
      //loop over data
      for ( int i = 1; i <= nBands; ++i )
      {
        if ( destBlockList[ i - 1 ]->isEmpty() )
          continue;

        if ( !destProvider->write( destBlockList[i - 1]->bits( 0 ), i, iterCols, iterRows, iterLeft, iterTop ) )
        {
          return WriteError;
        }
      }
    }
    ++fileIndex;
  }

  QgsDebugMsgLevel( QStringLiteral( "Done" ), 4 );
  return ( feedback && feedback->isCanceled() ) ? WriteCanceled : NoError;
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeImageRaster( QgsRasterIterator *iter, int nCols, int nRows, const QgsRectangle &outputExtent,
    const QgsCoordinateReferenceSystem &crs, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface *iface = iter->input();
  if ( !iface )
    return SourceProviderError;

  const Qgis::DataType inputDataType = iface->dataType( 1 );
  if ( inputDataType != Qgis::DataType::ARGB32 && inputDataType != Qgis::DataType::ARGB32_Premultiplied )
  {
    return SourceProviderError;
  }
  const bool isPremultiplied = ( inputDataType == Qgis::DataType::ARGB32_Premultiplied );

  iter->setMaximumTileWidth( mMaxTileWidth );
  iter->setMaximumTileHeight( mMaxTileHeight );

  const size_t nMaxPixels = static_cast<size_t>( mMaxTileWidth ) * mMaxTileHeight;
  std::vector<unsigned char> redData( nMaxPixels );
  std::vector<unsigned char> greenData( nMaxPixels );
  std::vector<unsigned char> blueData( nMaxPixels );
  std::vector<unsigned char> alphaData( nMaxPixels );
  int iterLeft = 0, iterTop = 0, iterCols = 0, iterRows = 0;
  int fileIndex = 0;

  //create destProvider for whole dataset here
  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( outputExtent, nCols, nRows, geoTransform, pixelSize );

  const int nOutputBands = 4;
  std::unique_ptr< QgsRasterDataProvider > destProvider( initOutput( nCols, nRows, crs, geoTransform, nOutputBands, Qgis::DataType::Byte ) );
  if ( !mTiledMode )
  {
    if ( !destProvider )
    {
      return CreateDatasourceError;
    }
    if ( !destProvider->isValid() )
    {
      if ( feedback && !destProvider->error().isEmpty() )
      {
        feedback->appendError( destProvider->error().summary() );
      }
      return CreateDatasourceError;
    }
    if ( nCols != destProvider->xSize() || nRows != destProvider->ySize() )
    {
      QgsDebugMsg( QStringLiteral( "Created raster does not have requested dimensions" ) );
      if ( feedback )
      {
        feedback->appendError( QObject::tr( "Created raster does not have requested dimensions" ) );
      }
      return CreateDatasourceError;
    }
    if ( nOutputBands != destProvider->bandCount() )
    {
      QgsDebugMsg( QStringLiteral( "Created raster does not have requested band count" ) );
      if ( feedback )
      {
        feedback->appendError( QObject::tr( "Created raster does not have requested band count" ) );
      }
      return CreateDatasourceError;
    }
    if ( Qgis::DataType::Byte != destProvider->dataType( 1 ) )
    {
      QgsDebugMsg( QStringLiteral( "Created raster does not have requested data type" ) );
      if ( feedback )
      {
        feedback->appendError( QObject::tr( "Created raster does not have requested data type" ) );
      }
      return CreateDatasourceError;
    }
  }

  iter->startRasterRead( 1, nCols, nRows, outputExtent, feedback );

  int nParts = 0;
  if ( feedback )
  {
    const int nPartsX = qgsDivRoundUp( nCols, iter->maximumTileWidth() );
    const int nPartsY = qgsDivRoundUp( nRows, iter->maximumTileHeight() );
    nParts = nPartsX * nPartsY;
  }

  std::unique_ptr< QgsRasterBlock > inputBlock;
  while ( iter->readNextRasterPart( 1, iterCols, iterRows, inputBlock, iterLeft, iterTop ) )
  {
    if ( !inputBlock || inputBlock->isEmpty() )
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
    const qgssize nPixels = static_cast< qgssize >( iterCols ) * iterRows;
    for ( qgssize i = 0; i < nPixels; ++i )
    {
      QRgb c = inputBlock->color( i );
      if ( isPremultiplied )
      {
        c = qUnpremultiply( c );
      }
      redData[i] = static_cast<unsigned char>( qRed( c ) );
      greenData[i] = static_cast<unsigned char>( qGreen( c ) );
      blueData[i] = static_cast<unsigned char>( qBlue( c ) );
      alphaData[i] = static_cast<unsigned char>( qAlpha( c ) );
    }

    //create output file
    if ( mTiledMode )
    {
      std::unique_ptr< QgsRasterDataProvider > partDestProvider( createPartProvider( outputExtent,
          nCols, iterCols, iterRows,
          iterLeft, iterTop, mOutputUrl, fileIndex,
          4, Qgis::DataType::Byte, crs ) );

      if ( !partDestProvider || partDestProvider->isValid() )
      {
        return DestProviderError;
      }

      //write data to output file
      if ( !partDestProvider->write( &redData[0], 1, iterCols, iterRows, 0, 0 ) ||
           !partDestProvider->write( &greenData[0], 2, iterCols, iterRows, 0, 0 ) ||
           !partDestProvider->write( &blueData[0], 3, iterCols, iterRows, 0, 0 ) ||
           !partDestProvider->write( &alphaData[0], 4, iterCols, iterRows, 0, 0 ) )
      {
        return WriteError;
      }

      addToVRT( partFileName( fileIndex ), 1, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( partFileName( fileIndex ), 2, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( partFileName( fileIndex ), 3, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( partFileName( fileIndex ), 4, iterCols, iterRows, iterLeft, iterTop );
    }
    else if ( destProvider )
    {
      if ( !destProvider->write( &redData[0], 1, iterCols, iterRows, iterLeft, iterTop ) ||
           !destProvider->write( &greenData[0], 2, iterCols, iterRows, iterLeft, iterTop ) ||
           !destProvider->write( &blueData[0], 3, iterCols, iterRows, iterLeft, iterTop ) ||
           !destProvider->write( &alphaData[0], 4, iterCols, iterRows, iterLeft, iterTop ) )
      {
        return WriteError;
      }
    }

    ++fileIndex;
  }
  destProvider.reset();

  if ( feedback )
  {
    feedback->setProgress( 100.0 );
  }

  if ( mTiledMode )
  {
    const QString vrtFilePath( mOutputUrl + '/' + vrtFileName() );
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
  const QDomText sourceFilenameText = mVRTDocument.createTextNode( filename );
  sourceFilenameElem.appendChild( sourceFilenameText );
  simpleSourceElem.appendChild( sourceFilenameElem );

  //SourceBand
  QDomElement sourceBandElem = mVRTDocument.createElement( QStringLiteral( "SourceBand" ) );
  const QDomText sourceBandText = mVRTDocument.createTextNode( QString::number( band ) );
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
  const QgsDataProvider::ProviderOptions providerOptions;
  QgsRasterDataProvider *destProvider = destProviderIn;
  if ( !destProvider )
  {
    destProvider = qobject_cast< QgsRasterDataProvider * >( QgsProviderRegistry::instance()->createProvider( mOutputProviderKey, filename, providerOptions ) );
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
    myPyramidList[myCounterInt].setBuild( true );
  }

  QgsDebugMsgLevel( QStringLiteral( "building pyramids : %1 pyramids, %2 resampling, %3 format, %4 options" ).arg( myPyramidList.count() ).arg( mPyramidsResampling ).arg( mPyramidsFormat ).arg( mPyramidsConfigOptions.count() ), 4 );
  // QApplication::setOverrideCursor( Qt::WaitCursor );
  const QString res = destProvider->buildPyramids( myPyramidList, mPyramidsResampling,
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
  Q_UNUSED( pszMessage )
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
  const QDomText crsText = mVRTDocument.createTextNode( crs.toWkt() );
  SRSElem.appendChild( crsText );
  VRTDatasetElem.appendChild( SRSElem );

  //geotransform
  if ( geoTransform )
  {
    QDomElement geoTransformElem = mVRTDocument.createElement( QStringLiteral( "GeoTransform" ) );
    const QString geoTransformString = QString::number( geoTransform[0], 'f', 6 ) + ", " + QString::number( geoTransform[1] ) + ", " + QString::number( geoTransform[2] ) +
                                       ", "  + QString::number( geoTransform[3], 'f', 6 ) + ", " + QString::number( geoTransform[4] ) + ", " + QString::number( geoTransform[5] );
    const QDomText geoTransformText = mVRTDocument.createTextNode( geoTransformString );
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
  dataTypes.insert( Qgis::DataType::Byte, QStringLiteral( "Byte" ) );
  dataTypes.insert( Qgis::DataType::UInt16, QStringLiteral( "UInt16" ) );
  dataTypes.insert( Qgis::DataType::Int16, QStringLiteral( "Int16" ) );
  dataTypes.insert( Qgis::DataType::UInt32, QStringLiteral( "Int32" ) );
  dataTypes.insert( Qgis::DataType::Float32, QStringLiteral( "Float32" ) );
  dataTypes.insert( Qgis::DataType::Float64, QStringLiteral( "Float64" ) );
  dataTypes.insert( Qgis::DataType::CInt16, QStringLiteral( "CInt16" ) );
  dataTypes.insert( Qgis::DataType::CInt32, QStringLiteral( "CInt32" ) );
  dataTypes.insert( Qgis::DataType::CFloat32, QStringLiteral( "CFloat32" ) );
  dataTypes.insert( Qgis::DataType::CFloat64, QStringLiteral( "CFloat64" ) );

  for ( int i = 1; i <= nBands; i++ )
  {
    QDomElement VRTBand = mVRTDocument.createElement( QStringLiteral( "VRTRasterBand" ) );

    VRTBand.setAttribute( QStringLiteral( "band" ), QString::number( i ) );
    const QString dataType = dataTypes.value( type );
    VRTBand.setAttribute( QStringLiteral( "dataType" ), dataType );

    if ( mMode == Image )
    {
      VRTBand.setAttribute( QStringLiteral( "dataType" ), QStringLiteral( "Byte" ) );
      QDomElement colorInterpElement = mVRTDocument.createElement( QStringLiteral( "ColorInterp" ) );
      const QDomText interpText = mVRTDocument.createTextNode( colorInterp.value( i - 1 ) );
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
  const double mup = extent.width() / nCols;
  const double mapLeft = extent.xMinimum() + iterLeft * mup;
  const double mapRight = mapLeft + mup * iterCols;
  const double mapTop = extent.yMaximum() - iterTop * mup;
  const double mapBottom = mapTop - iterRows * mup;
  const QgsRectangle mapRect( mapLeft, mapBottom, mapRight, mapTop );

  const QString outputFile = outputUrl + '/' + partFileName( fileIndex );

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
      QgsDebugMsg( QStringLiteral( "No provider created" ) );
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
  const QFileInfo outputInfo( mOutputUrl );
  return QStringLiteral( "%1.%2.tif" ).arg( outputInfo.fileName() ).arg( fileIndex );
}

QString QgsRasterFileWriter::vrtFileName()
{
  const QFileInfo outputInfo( mOutputUrl );
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
        const QStringList driverExtensions = QString( GDALGetMetadataItem( drv, GDAL_DMD_EXTENSIONS, nullptr ) ).split( ' ' );

        const auto constDriverExtensions = driverExtensions;
        for ( const QString &driver : constDriverExtensions )
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
    const QString drvName = GDALGetDriverLongName( drv );
    const QString extensionsString = QString( GDALGetMetadataItem( drv, GDAL_DMD_EXTENSIONS, nullptr ) );
    if ( extensionsString.isEmpty() )
    {
      return QString();
    }
    const QStringList extensions = extensionsString.split( ' ' );
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
  static QReadWriteLock sFilterLock;
  static QMap< RasterFormatOptions, QList< QgsRasterFileWriter::FilterFormatDetails > > sFilters;

  QgsReadWriteLocker locker( sFilterLock, QgsReadWriteLocker::Read );

  const auto it = sFilters.constFind( options );
  if ( it != sFilters.constEnd() )
    return it.value();

  GDALAllRegister();
  int const drvCount = GDALGetDriverCount();

  locker.changeMode( QgsReadWriteLocker::Write );
  QList< QgsRasterFileWriter::FilterFormatDetails > results;

  FilterFormatDetails tifFormat;

  for ( int i = 0; i < drvCount; ++i )
  {
    GDALDriverH drv = GDALGetDriver( i );
    if ( drv )
    {
      if ( QgsGdalUtils::supportsRasterCreate( drv ) )
      {
        const QString drvName = GDALGetDriverShortName( drv );
        const QString filterString = filterForDriver( drvName );
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

  sFilters.insert( options, results );

  return results;
}

QStringList QgsRasterFileWriter::supportedFormatExtensions( const RasterFormatOptions options )
{
  const auto formats = supportedFiltersAndFormats( options );
  QStringList extensions;

  const QRegularExpression rx( QStringLiteral( "\\*\\.([a-zA-Z0-9]*)" ) );

  for ( const FilterFormatDetails &format : formats )
  {
    const QString ext = format.filterString;
    const QRegularExpressionMatch match = rx.match( ext );
    if ( !match.hasMatch() )
      continue;

    const QString matched = match.captured( 1 );
    extensions << matched;
  }
  return extensions;
}
