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
#include "qgsrasterfilewriter.h"
#include "qgsproviderregistry.h"
#include "qgsrasterinterface.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include <QCoreApplication>
#include <QProgressDialog>
#include <QTextStream>
#include "gdal.h"

QgsRasterFileWriter::QgsRasterFileWriter( const QString& outputUrl ): mOutputUrl( outputUrl ), mOutputProviderKey( "gdal" ), mOutputFormat( "GTiff" ), mTiledMode( false ),
    mMaxTileWidth( 500 ), mMaxTileHeight( 500 ), mProgressDialog( 0 )
{

}

QgsRasterFileWriter::QgsRasterFileWriter()
{

}

QgsRasterFileWriter::~QgsRasterFileWriter()
{

}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( QgsRasterIterator* iter, int nCols, int nRows, QgsRectangle outputExtent,
    const QgsCoordinateReferenceSystem& crs, QProgressDialog* p )
{
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface* iface = iter->input();
  if ( !iface )
  {
    return SourceProviderError;
  }

  mProgressDialog = p;

  if ( iface->dataType( 1 ) == QgsRasterInterface::ARGB32 )
  {
    WriterError e = writeImageRaster( iter, nCols, nRows, outputExtent, crs, p );
    mProgressDialog = 0;
    return e;
  }
  else
  {
    mProgressDialog = 0;
    WriterError e = writeDataRaster( iter, nCols, nRows, outputExtent, crs, p );
    return e;
  }
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeDataRaster( QgsRasterIterator* iter, int nCols, int nRows, const QgsRectangle& outputExtent,
    const QgsCoordinateReferenceSystem& crs, QProgressDialog* p )
{
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface* iface = iter->input();
  if ( !iface )
  {
    return SourceProviderError;
  }

  //create directory for output files
  QDir destDir( mOutputUrl );
  if ( mTiledMode )
  {
    destDir.mkdir( mOutputUrl );
  }

  //Get output map units per pixel
  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;

  iter->setMaximumTileWidth( mMaxTileWidth );
  iter->setMaximumTileHeight( mMaxTileHeight );

  int nBands = iface->bandCount();
  if ( nBands < 1 )
  {
    return SourceProviderError;
  }

  //create destProvider for whole dataset here
  QgsRasterDataProvider* destProvider = 0;
  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( outputExtent, nCols, nRows, geoTransform, pixelSize );

  //check if all the bands have the same data type size, otherwise we cannot write it to the provider
  //(at least not with the current interface)
  int dataTypeSize = iface->typeSize( iface->dataType( 1 ) );
  for ( int i = 2; i <= nBands; ++i )
  {
    if ( iface->typeSize( iface->dataType( 1 ) ) != dataTypeSize )
    {
      return DestProviderError;
    }
  }

  QList<void*> dataList;
  for ( int i = 1; i <= nBands; ++i )
  {
    iter->startRasterRead( i, nCols, nRows, outputExtent );
    dataList.push_back( VSIMalloc( dataTypeSize * mMaxTileWidth * mMaxTileHeight ) );
  }

  destProvider = initOutput( nCols, nRows, crs, geoTransform, nBands,  iface->dataType( 1 ) );

  int nParts = 0;
  int fileIndex = 0;
  if ( p )
  {
    int nPartsX = nCols / iter->maximumTileWidth() + 1;
    int nPartsY = nRows / iter->maximumTileHeight() + 1;
    nParts = nPartsX * nPartsY;
    p->setMaximum( nParts );
    p->show();
    p->setLabelText( QObject::tr( "Reading raster part %1 of %2" ).arg( fileIndex + 1 ).arg( nParts ) );
  }

  while ( true )
  {
    for ( int i = 1; i <= nBands; ++i )
    {
      if ( !iter->readNextRasterPart( i, iterCols, iterRows, &( dataList[i - 1] ), iterLeft, iterTop ) )
      {
        delete destProvider;
        if ( mTiledMode )
        {
          QFileInfo outputInfo( mOutputUrl );
          QString vrtFilePath( mOutputUrl + "/" + outputInfo.baseName() + ".vrt" );
          writeVRT( vrtFilePath );
          buildPyramids( vrtFilePath );
        }

        return NoError; //reached last tile, bail out
      }
    }

    if ( p && fileIndex < ( nParts - 1 ) )
    {
      p->setValue( fileIndex + 1 );
      p->setLabelText( QObject::tr( "Reading raster part %1 of %2" ).arg( fileIndex + 2 ).arg( nParts ) );
      QCoreApplication::processEvents( QEventLoop::AllEvents, 1000 );
      if ( p->wasCanceled() )
      {
        break;
      }
    }

    if ( mTiledMode ) //write to file
    {
      delete destProvider;
      destProvider = createPartProvider( outputExtent, nCols, iterCols, iterRows,
                                         iterLeft, iterTop, mOutputUrl, fileIndex, nBands, iface->dataType( 1 ), crs );

      //write data to output file. todo: loop over the data list
      for ( int i = 1; i <= nBands; ++i )
      {
        destProvider->write( dataList[i - 1], i, iterCols, iterRows, 0, 0 );
        addToVRT( QString::number( fileIndex ), i, iterCols, iterRows, iterLeft, iterTop );
      }
    }
    else
    {
      //loop over data
      for ( int i = 1; i <= nBands; ++i )
      {
        destProvider->write( dataList[i - 1], i, iterCols, iterRows, iterLeft, iterTop );
      }
    }
    ++fileIndex;
  }
  return NoError;
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeImageRaster( QgsRasterIterator* iter, int nCols, int nRows, const QgsRectangle& outputExtent,
    const QgsCoordinateReferenceSystem& crs, QProgressDialog* p )
{
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface* iface = iter->input();
  if ( !iface ||  iface->dataType( 1 ) != QgsRasterInterface::ARGB32 )
  {
    return SourceProviderError;
  }

  //create directory for output files
  QDir destDir( mOutputUrl );
  if ( mTiledMode )
  {
    destDir.mkdir( mOutputUrl );
  }

  iter->setMaximumTileWidth( mMaxTileWidth );
  iter->setMaximumTileHeight( mMaxTileHeight );

  void* data = VSIMalloc( iface->typeSize( QgsRasterInterface::ARGB32 ) * mMaxTileWidth * mMaxTileHeight );
  void* redData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
  void* greenData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
  void* blueData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
  void* alphaData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
  QgsRectangle mapRect;
  int iterLeft = 0, iterTop = 0, iterCols = 0, iterRows = 0;
  int fileIndex = 0;

  //create destProvider for whole dataset here
  QgsRasterDataProvider* destProvider = 0;
  double pixelSize;
  double geoTransform[6];
  globalOutputParameters( outputExtent, nCols, nRows, geoTransform, pixelSize );

  destProvider = initOutput( nCols, nRows, crs, geoTransform, 4, QgsRasterInterface::Byte );

  //iter->select( outputExtent, outputMapUnitsPerPixel );
  iter->startRasterRead( 1, nCols, nRows, outputExtent );

  int nParts = 0;
  if ( p )
  {
    int nPartsX = nCols / iter->maximumTileWidth() + 1;
    int nPartsY = nRows / iter->maximumTileHeight() + 1;
    nParts = nPartsX * nPartsY;
    p->setMaximum( nParts );
    p->show();
    p->setLabelText( QObject::tr( "Reading raster part %1 of %2" ).arg( fileIndex + 1 ).arg( nParts ) );
  }

  while ( iter->readNextRasterPart( 1, iterCols, iterRows, &data, iterLeft, iterTop ) )
  {
    if ( iterCols <= 5 || iterRows <= 5 ) //some wms servers don't like small values
    {
      continue;
    }

    if ( p && fileIndex < ( nParts - 1 ) )
    {
      p->setValue( fileIndex + 1 );
      p->setLabelText( QObject::tr( "Reading raster part %1 of %2" ).arg( fileIndex + 2 ).arg( nParts ) );
      QCoreApplication::processEvents( QEventLoop::AllEvents, 1000 );
      if ( p->wasCanceled() )
      {
        break;
      }
    }

    //fill into red/green/blue/alpha channels
    uint* p = ( uint* ) data;
    int nPixels = iterCols * iterRows;
    int red = 0;
    int green = 0;
    int blue = 0;
    int alpha = 255;
    for ( int i = 0; i < nPixels; ++i )
    {
      QRgb c( *p++ );
      red = qRed( c ); green = qGreen( c ); blue = qBlue( c ); alpha = qAlpha( c );
      memcpy(( char* )redData + i, &red, 1 );
      memcpy(( char* )greenData + i, &green, 1 );
      memcpy(( char* )blueData + i, &blue, 1 );
      memcpy(( char* )alphaData + i, &alpha, 1 );
    }

    //create output file
    if ( mTiledMode )
    {
      delete destProvider;
      destProvider = createPartProvider( outputExtent, nCols, iterCols, iterRows,
                                         iterLeft, iterTop, mOutputUrl, fileIndex, 4, QgsRasterInterface::Byte, crs );

      //write data to output file
      destProvider->write( redData, 1, iterCols, iterRows, 0, 0 );
      destProvider->write( greenData, 2, iterCols, iterRows, 0, 0 );
      destProvider->write( blueData, 3, iterCols, iterRows, 0, 0 );
      destProvider->write( alphaData, 4, iterCols, iterRows, 0, 0 );

      addToVRT( QString::number( fileIndex ), 1, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( QString::number( fileIndex ), 2, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( QString::number( fileIndex ), 3, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( QString::number( fileIndex ), 4, iterCols, iterRows, iterLeft, iterTop );
    }
    else
    {
      destProvider->write( redData, 1, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( greenData, 2, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( blueData, 3, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( alphaData, 4, iterCols, iterRows, iterLeft, iterTop );
    }

    ++fileIndex;
  }
  delete destProvider;
  CPLFree( data ); CPLFree( redData ); CPLFree( greenData ); CPLFree( blueData ); CPLFree( alphaData );

  if ( p )
  {
    p->setValue( p->maximum() );
  }

  if ( mTiledMode )
  {
    QFileInfo outputInfo( mOutputUrl );
    QString vrtFilePath( mOutputUrl + "/" + outputInfo.baseName() + ".vrt" );
    writeVRT( vrtFilePath );
    buildPyramids( vrtFilePath );
  }
  return NoError;
}

void QgsRasterFileWriter::addToVRT( const QString& filename, int band, int xSize, int ySize, int xOffset, int yOffset )
{
  QDomElement bandElem;

  switch ( band )
  {
    case 1:
      bandElem = mVRTRedBand;
      break;
    case 2:
      bandElem = mVRTGreenBand;
      break;
    case 3:
      bandElem = mVRTBlueBand;
      break;
    case 4:
      bandElem = mVRTAlphaBand;
      break;
    default:
      return;
  }

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

  /*if ( mProgressDialog )
  {
    mProgressDialog->setLabelText( QObject::tr( "Building Pyramids..." ) );
    mProgressDialog->setValue( 0 );
    mProgressDialog->setWindowModality( Qt::WindowModal );
    mProgressDialog->show();
  }*/
  GDALBuildOverviews( dataSet, "AVERAGE", 6, overviewList, 0, 0, /*pyramidsProgress*/ 0, /*mProgressDialog*/ 0 );
}

int QgsRasterFileWriter::pyramidsProgress( double dfComplete, const char *pszMessage, void* pData )
{
  Q_UNUSED( pszMessage );
  GDALTermProgress( dfComplete, 0, 0 );
  QProgressDialog* p = static_cast<QProgressDialog*>( pData );
  if ( pData && p->wasCanceled() )
  {
    return 0;
  }

  if ( pData )
  {
    p->setRange( 0, 100 );
    p->setValue( dfComplete * 100 );
  }
  return 1;
}

void QgsRasterFileWriter::createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem& crs, double* geoTransform )
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

  //VRT rasterbands
  mVRTRedBand = mVRTDocument.createElement( "VRTRasterBand" );
  mVRTRedBand.setAttribute( "dataType", "Byte" );
  mVRTRedBand.setAttribute( "band", "1" );
  QDomElement colorInterpRedElement = mVRTDocument.createElement( "ColorInterp" );
  QDomText redInterprText = mVRTDocument.createTextNode( "Red" );
  colorInterpRedElement.appendChild( redInterprText );
  mVRTRedBand.appendChild( colorInterpRedElement );

  mVRTGreenBand = mVRTDocument.createElement( "VRTRasterBand" );
  mVRTGreenBand.setAttribute( "dataType", "Byte" );
  mVRTGreenBand.setAttribute( "band", "2" );
  QDomElement colorInterpGreenElement = mVRTDocument.createElement( "ColorInterp" );
  QDomText greenInterprText = mVRTDocument.createTextNode( "Green" );
  colorInterpGreenElement.appendChild( greenInterprText );
  mVRTGreenBand.appendChild( colorInterpGreenElement );

  mVRTBlueBand = mVRTDocument.createElement( "VRTRasterBand" );
  mVRTBlueBand.setAttribute( "dataType", "Byte" );
  mVRTBlueBand.setAttribute( "band", "3" );
  QDomElement colorInterpBlueElement = mVRTDocument.createElement( "ColorInterp" );
  QDomText blueInterprText = mVRTDocument.createTextNode( "Blue" );
  colorInterpBlueElement.appendChild( blueInterprText );
  mVRTBlueBand.appendChild( colorInterpBlueElement );

  mVRTAlphaBand = mVRTDocument.createElement( "VRTRasterBand" );
  mVRTAlphaBand.setAttribute( "dataType", "Byte" );
  mVRTAlphaBand.setAttribute( "band", "4" );
  QDomElement colorInterpAlphaElement = mVRTDocument.createElement( "ColorInterp" );
  QDomText alphaInterprText = mVRTDocument.createTextNode( "Alpha" );
  colorInterpAlphaElement.appendChild( alphaInterprText );
  mVRTAlphaBand.appendChild( colorInterpAlphaElement );

  VRTDatasetElem.appendChild( mVRTRedBand );
  VRTDatasetElem.appendChild( mVRTGreenBand );
  VRTDatasetElem.appendChild( mVRTBlueBand );
  VRTDatasetElem.appendChild( mVRTAlphaBand );
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
    int iterRows, int iterLeft, int iterTop, const QString& outputUrl, int fileIndex, int nBands, QgsRasterInterface::DataType type,
    const QgsCoordinateReferenceSystem& crs )
{
  double mup = extent.width() / nCols;
  double mapLeft = extent.xMinimum() + iterLeft * mup;
  double mapRight = mapLeft + mup * iterCols;
  double mapTop = extent.yMaximum() - iterTop * mup;
  double mapBottom = mapTop - iterRows * mup;
  QgsRectangle mapRect( mapLeft, mapBottom, mapRight, mapTop );

  QString outputFile = outputUrl + "/" + QString::number( fileIndex );
  QgsRasterDataProvider* destProvider = QgsRasterLayer::loadProvider( mOutputProviderKey, outputFile );
  if ( !destProvider )
  {
    return 0;
  }

  //geotransform
  double geoTransform[6];
  geoTransform[0] = mapRect.xMinimum();
  geoTransform[1] = mup;
  geoTransform[2] = 0.0;
  geoTransform[3] = mapRect.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -mup;

  // perhaps we need a separate createOptions for tiles ?
  if ( !destProvider->create( mOutputFormat, nBands, type, iterCols, iterRows, geoTransform,
                              crs ) )
  {
    delete destProvider;
    return 0;
  }
  return destProvider;
}

QgsRasterDataProvider* QgsRasterFileWriter::initOutput( int nCols, int nRows, const QgsCoordinateReferenceSystem& crs,
    double* geoTransform, int nBands, QgsRasterInterface::DataType type )
{
  if ( mTiledMode )
  {
    createVRT( nCols, nRows, crs, geoTransform );
    return 0;
  }
  else
  {
    QgsRasterDataProvider* destProvider = QgsRasterLayer::loadProvider( mOutputProviderKey, mOutputUrl );
    if ( !destProvider )
    {
      return 0;
    }

    if ( !destProvider->create( mOutputFormat, nBands, type, nCols, nRows, geoTransform,
                                crs, mCreateOptions ) )
    {
      delete destProvider;
      return 0;
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
    nRows = ( double )nCols / extent.width() * extent.height() + 0.5;
  }
  geoTransform[0] = extent.xMinimum();
  geoTransform[1] = pixelSize;
  geoTransform[2] = 0.0;
  geoTransform[3] = extent.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -( extent.height() / nRows );
}




